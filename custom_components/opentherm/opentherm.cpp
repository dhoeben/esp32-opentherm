#include "opentherm.h"
#include "config.h"
#include "boiler.h"
#include "dhw.h"
#include "equitherm.h"
#include "diagnostics.h"
#include "esphome/components/number/number.h"


using namespace esphome;

namespace opentherm {

static OpenThermComponent* g_singleton = nullptr;

// ---------------------------------------------
// Global links to Home Assistant sensors
// ---------------------------------------------
esphome::sensor::Sensor *id_ha_weather_temp = nullptr;
esphome::sensor::Sensor *id_ha_target_temp  = nullptr;
esphome::sensor::Sensor *id_ha_indoor_temp  = nullptr;

OpenThermComponent* OpenThermComponent::get_singleton() { return g_singleton; }

OpenThermComponent::OpenThermComponent() {
  g_singleton = this;
}

// ---------------------------------------------
// Setup
// ---------------------------------------------
void OpenThermComponent::setup() {
  in_pin_->setup();
  out_pin_->setup();
  line_tx_level(true);  // line idle high

  ESP_LOGI("opentherm",
           "Pins configured IN=%d OUT=%d, poll=%u ms, timeout=%u ms, debug=%d",
           in_pin_ ? in_pin_->get_pin() : OT_IN_PIN,
           out_pin_ ? out_pin_->get_pin() : OT_OUT_PIN,
           poll_interval_ms_ ? poll_interval_ms_ : OT_POLL_INTERVAL,
           rx_timeout_ms_,
           debug_ ? debug_ : OT_DEBUG);

  // ---------------------------------------------------------------------
  // Register boiler temperature limit numbers for Home Assistant
  // ---------------------------------------------------------------------
  using namespace esphome::number;

  // Boiler
  Boiler::max_heating_temp->set_name("Max Boiler Temp Heating");
  Boiler::max_heating_temp->traits.set_min_value(30);
  Boiler::max_heating_temp->traits.set_max_value(90);
  Boiler::max_heating_temp->traits.set_step(1);
  Boiler::max_heating_temp->publish_state(70);

  // DHW
  DHW::max_water_temp->set_name("Max DHW Temp");
  DHW::max_water_temp->traits.set_min_value(30);
  DHW::max_water_temp->traits.set_max_value(90);
  DHW::max_water_temp->traits.set_step(1);
  DHW::max_water_temp->publish_state(50);
}


void OpenThermComponent::loop() {
  const uint32_t now = millis();
  if (now - last_poll_ms_ < poll_interval_ms_) return;
  last_poll_ms_ = now;

  // Calculate weather-compensated flow target (Equitherm)
  float flow_target = Equitherm::calculate_target_temp();

  // Enforce boiler / DHW max limits
  float heating_limit = opentherm::Boiler::max_heating_temp
                        ? opentherm::Boiler::max_heating_temp->state
                        : 70.0f;  // fallback

  float dhw_limit = opentherm::DHW::max_water_temp
                    ? opentherm::DHW::max_water_temp->state
                    : 60.0f;  // fallback

  // Decide which limit applies
  bool dhw_active = false;  // replace this with your actual mode flag
  float active_limit = dhw_active ? dhw_limit : heating_limit;

  // Clamp
  if (flow_target > active_limit)
    flow_target = active_limit;

  // Write Control Setpoint (DID 0x11, F8.8)
  const uint16_t raw = static_cast<uint16_t>(flow_target * 256.0f);
  const uint32_t frame = build_request(WRITE_DATA, 0x11, raw);
  send_frame(frame);

  if (debug_) ESP_LOGI("opentherm", "Flow target (setpoint) sent = %.1f°C", flow_target);

  // Update all subsystem modules
  Boiler::update(this);       
  DHW::update(this);          
  Diagnostics::update(this); 
}

// ---------------------------------------------
// Frame helpers
// ---------------------------------------------
uint8_t OpenThermComponent::parity32(uint32_t v) {
  v >>= 1;
  v ^= v >> 16;
  v ^= v >> 8;
  v ^= v >> 4;
  v &= 0xF;
  static const uint8_t lut[16] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
  return lut[v] & 1u;
}

uint32_t OpenThermComponent::build_request(OTMsgType mt, uint8_t did, uint16_t data) {
  uint32_t f = 0;
  f |= (1u << 31);
  f |= (static_cast<uint32_t>(mt) & 0x7u) << 28;
  f |= (static_cast<uint32_t>(did) & 0xFFu) << 20;
  f |= (static_cast<uint32_t>(data) & 0xFFFFu) << 4;
  const uint8_t p = parity32(f);
  f |= static_cast<uint32_t>(p);
  return f;
}

bool OpenThermComponent::wait_us(uint32_t us) {
  const int64_t start = esp_timer_get_time();
  while ((esp_timer_get_time() - start) < static_cast<int64_t>(us)) {}
  return true;
}

void OpenThermComponent::line_tx_level(bool high) { out_pin_->digital_write(high); }
bool OpenThermComponent::line_rx_level() const { return in_pin_->digital_read(); }

void OpenThermComponent::tx_manchester_bit(bool logical_one) {
  if (logical_one) {
    line_tx_level(true);  wait_us(HALF_BIT_US);
    line_tx_level(false); wait_us(HALF_BIT_US);
  } else {
    line_tx_level(false); wait_us(HALF_BIT_US);
    line_tx_level(true);  wait_us(HALF_BIT_US);
  }
}

bool OpenThermComponent::send_frame(uint32_t frame) {
  line_tx_level(true);
  wait_us(3 * BIT_US);

  for (int i = 31; i >= 0; --i)
    tx_manchester_bit((frame >> i) & 1u);

  line_tx_level(true);
  wait_us(3 * BIT_US);

  if (debug_) ESP_LOGD("opentherm", "TX frame: 0x%08X", frame);
  return true;
}

bool OpenThermComponent::recv_frame(uint32_t &resp) {
  const int64_t start = esp_timer_get_time();
  resp = 0;

  while ((esp_timer_get_time() - start) < static_cast<int64_t>(rx_timeout_ms_) * 1000) {
    uint32_t v = 0;
    for (int i = 31; i >= 0; --i) {
      wait_us(HALF_BIT_US);
      const bool first = line_rx_level();
      wait_us(HALF_BIT_US);
      const bool second = line_rx_level();
      if (first && !second)       v = (v << 1) | 1u;
      else if (!first && second)  v = (v << 1) | 0u;
      else return false;
    }

    resp = v;
    const uint8_t p = parity32(resp);
    if (((resp & 0x1u) != p)) return false;

    if (debug_) ESP_LOGD("opentherm", "RX frame: 0x%08X", resp);
    return true;
  }

  return false;
}

uint32_t OpenThermComponent::read_did(uint8_t did) {
  const uint32_t req = build_request(READ_DATA, did, 0);
  if (!send_frame(req)) return 0;

  uint32_t resp = 0;
  if (!recv_frame(resp)) {
    if (debug_) ESP_LOGW("opentherm", "Timeout / bad frame for DID 0x%02X", did);
    return 0;
  }
  if (((resp >> 31) & 1u) != 1u) {
    if (debug_) ESP_LOGW("opentherm", "Bad start bit DID 0x%02X", did);
    return 0;
  }

  return resp;
}

float OpenThermComponent::parse_f88(uint16_t raw) {
  return static_cast<int16_t>(raw) / 256.0f;
}

}  // namespace opentherm
