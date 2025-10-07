#include "opentherm.h"
using namespace esphome;

namespace opentherm {
  // Link definitions for HA sensors
esphome::sensor::Sensor *id_ha_weather_temp = nullptr;
esphome::sensor::Sensor *id_ha_target_temp = nullptr;
esphome::sensor::Sensor *id_ha_indoor_temp = nullptr;


static OpenThermComponent* g_singleton = nullptr;

OpenThermComponent::OpenThermComponent() {
  // Configure all HA-visible sensors
  boiler_temp->set_name("Boiler Water Temperature");
  boiler_temp->set_unit_of_measurement("°C");
  boiler_temp->set_icon("mdi:thermometer");
  boiler_temp->set_accuracy_decimals(1);

  return_temp->set_name("Return Temperature");
  return_temp->set_unit_of_measurement("°C");
  return_temp->set_icon("mdi:thermometer");
  return_temp->set_accuracy_decimals(1);

  modulation->set_name("Boiler Modulation");
  modulation->set_unit_of_measurement("%");
  modulation->set_icon("mdi:percent");
  modulation->set_accuracy_decimals(0);

  setpoint->set_name("Control Setpoint");
  setpoint->set_unit_of_measurement("°C");
  setpoint->set_icon("mdi:thermostat");
  setpoint->set_accuracy_decimals(1);

  g_singleton = this;
}

OpenThermComponent* OpenThermComponent::get_singleton() { return g_singleton; }

void OpenThermComponent::setup() {
  in_pin_->setup();
  out_pin_->setup();
  line_tx_level(true);  // idle high
  ESP_LOGI("opentherm", "Pins configured IN=%d OUT=%d, poll=%u ms, rx_timeout=%u ms, debug=%d",
           in_pin_->get_pin(), out_pin_->get_pin(),
           poll_interval_ms_, rx_timeout_ms_, debug_);
}

void OpenThermComponent::loop() {
  uint32_t now = millis();
  if (now - last_poll_ms_ < poll_interval_ms_) return;
  last_poll_ms_ = now;

  // -----------------------------------------------------------
  // Weather-compensated control (Equitherm)
  // -----------------------------------------------------------
  float t_out = 10.0f;
  if (id_ha_weather_temp != nullptr && id_ha_weather_temp->has_state())
    t_out = id_ha_weather_temp->state;

  float t_set = 21.0f;
  if (id_ha_target_temp != nullptr && id_ha_target_temp->has_state())
    t_set = id_ha_target_temp->state;

  float t_in = 21.0f;
  if (id_ha_indoor_temp != nullptr && id_ha_indoor_temp->has_state())
    t_in = id_ha_indoor_temp->state;

  // --- Base Equitherm curve ---
  float flow_target = (EQ_N * (t_set + EQ_K - t_out)) + EQ_T;

  float delta = t_set - t_in;  // positive = too cold, negative = too warm
  float correction = delta * EQ_FB_GAIN;  // gain factor, tune between 1.0 and 5.0

  flow_target += correction;

  // Clamp for safety
  if (flow_target < 25.0f) flow_target = 25.0f;
  if (flow_target > 80.0f) flow_target = 80.0f;

  uint16_t raw = (uint16_t)(flow_target * 256.0f);
  uint32_t frame = build_request(WRITE_DATA, 0x11, raw);
  send_frame(frame);

  if (debug_) {
    ESP_LOGI("opentherm",
            "Equitherm+FB: out=%.1f°C set=%.1f°C in=%.1f°C Δ=%.2f flow=%.1f°C (N=%.2f,K=%.2f,T=%.2f)",
            t_out, t_set, t_in, delta, flow_target, EQ_N, EQ_K, EQ_T);
  }



  // -----------------------------------------------------------
  // Poll boiler for diagnostic data
  // -----------------------------------------------------------
  if (uint32_t raw18 = read_did(0x18)) {
    uint16_t data = (raw18 >> 8) & 0xFFFF;
    boiler_temp->publish_state(parse_f88(data));
  }

  if (uint32_t raw19 = read_did(0x19)) {
    uint16_t data = (raw19 >> 8) & 0xFFFF;
    return_temp->publish_state(parse_f88(data));
  }

  if (uint32_t raw1D = read_did(0x1D)) {
    uint16_t data = (raw1D >> 8) & 0xFFFF;
    modulation->publish_state(parse_f88(data));
  }

  if (uint32_t raw11 = read_did(0x11)) {
    uint16_t data = (raw11 >> 8) & 0xFFFF;
    setpoint->publish_state(parse_f88(data));
  }
}

// ============================
//  Frame helpers
// ============================
uint8_t OpenThermComponent::parity32(uint32_t v) {
  v >>= 1;
  v ^= v >> 16; v ^= v >> 8; v ^= v >> 4; v &= 0xF;
  static const uint8_t lut[16] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
  return lut[v] & 1u;
}

uint32_t OpenThermComponent::build_request(OTMsgType mt, uint8_t did, uint16_t data) {
  uint32_t f = 0;
  f |= (1u << 31);
  f |= (static_cast<uint32_t>(mt) & 0x7u) << 28;
  f |= (static_cast<uint32_t>(did) & 0xFFu) << 20;
  f |= (static_cast<uint32_t>(data) & 0xFFFFu) << 4;
  uint8_t p = parity32(f);
  f |= static_cast<uint32_t>(p);
  return f;
}

bool OpenThermComponent::wait_us(uint32_t us) {
  int64_t start = esp_timer_get_time();
  while ((esp_timer_get_time() - start) < static_cast<int64_t>(us)) { }
  return true;
}

void OpenThermComponent::line_tx_level(bool high) {
  out_pin_->digital_write(high);
}

bool OpenThermComponent::line_rx_level() const {
  return in_pin_->digital_read();
}

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

  if (debug_)
    ESP_LOGD("opentherm", "TX frame: 0x%08X", frame);
  return true;
}

bool OpenThermComponent::recv_frame(uint32_t &resp) {
  int64_t start = esp_timer_get_time();
  resp = 0;
  while ((esp_timer_get_time() - start) < (int64_t)rx_timeout_ms_ * 1000) {
    uint32_t v = 0;
    for (int i = 31; i >= 0; --i) {
      wait_us(HALF_BIT_US);
      bool first = line_rx_level();
      wait_us(HALF_BIT_US);
      bool second = line_rx_level();
      if (first && !second)       v = (v << 1) | 1u;
      else if (!first && second)  v = (v << 1) | 0u;
      else return false;
    }
    resp = v;
    uint8_t p = parity32(resp);
    if (((resp & 0x1u) != p)) return false;
    if (debug_) ESP_LOGD("opentherm", "RX frame: 0x%08X", resp);
    return true;
  }
  return false;
}

uint32_t OpenThermComponent::read_did(uint8_t did) {
  uint32_t req = build_request(READ_DATA, did, 0);
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
