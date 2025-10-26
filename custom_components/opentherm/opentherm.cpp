#include "opentherm.h"
#include "config.h"
#include "sensors.h"
#include "boiler.h"
#include "dhw.h"
#include "equitherm.h"
#include "diagnostics.h"
#include "emergency.h"
#include "esphome/components/number/number.h"

using namespace esphome;

namespace opentherm {

static OpenThermComponent *g_singleton = nullptr;

// Mode default
CompensationMode g_compensation_mode = CompensationMode::EQUITHERM;

void set_compensation_mode(CompensationMode m) { g_compensation_mode = m; }
void set_compensation_mode_from_string(const std::string &s) {
  if (s == "Boiler" || s == "boiler") {
    g_compensation_mode = CompensationMode::BOILER;
    ESP_LOGI(OT_LOG_TAG, "Compensation mode: BOILER (internal curve).");
  } else {
    g_compensation_mode = CompensationMode::EQUITHERM;
    ESP_LOGI(OT_LOG_TAG, "Compensation mode: EQUITHERM (external setpoint).");
  }
}

OpenThermComponent *OpenThermComponent::get_singleton() { return g_singleton; }
OpenThermComponent::OpenThermComponent() { g_singleton = this; }

void OpenThermComponent::setup() {
  if (in_pin_)  in_pin_->setup();
  if (out_pin_) out_pin_->setup();
  line_tx_level(true); // idle high

  ESP_LOGI(OT_LOG_TAG, "Pins IN=%d OUT=%d, poll=%u ms, timeout=%u ms, debug=%d",
           in_pin_ ? in_pin_->get_pin() : OT_IN_PIN,
           out_pin_ ? out_pin_->get_pin() : OT_OUT_PIN,
           poll_interval_ms_, rx_timeout_ms_, debug_);

  // -----------------------------------------------------------------------
  // Central sensor registry binding
  // -----------------------------------------------------------------------
  OT_SENSOR(boiler_temp) = boiler_temp_;
  OT_SENSOR(return_temp) = return_temp_;
  OT_SENSOR(modulation)  = modulation_;
  OT_SENSOR(setpoint)    = setpoint_;
  OT_SENSOR(dhw_temp)    = dhw_temp_;

  OT_SENSOR(ha_weather_temp)      = ha_weather_;
  OT_SENSOR(ha_indoor_temp)       = ha_indoor_;
  OT_SENSOR(adaptive_indoor_temp) = adaptive_indoor_;
  OT_SENSOR(dhw_flow_rate)        = dhw_flow_rate_;

  OT_SENSOR(max_boiler_temp) = boiler_limit_;
  OT_SENSOR(max_dhw_temp)    = dhw_limit_;

  OT_SENSOR(eq_n)       = eq_n_;
  OT_SENSOR(eq_k)       = eq_k_;
  OT_SENSOR(eq_t)       = eq_t_;
  OT_SENSOR(eq_fb_gain) = eq_fb_;

  OT_SENSOR(ch_climate) = ch_climate_;

  // NOTE: External/HA sensors & optional diagnostics are linked in a future pass
  // (they are safe to remain nullptr; code uses defaults if missing)

  // Boiler numbers defaults (visible traits)
  using namespace esphome::number;
  if (boiler_limit_) {
    Boiler::max_heating_temp = boiler_limit_;
    boiler_limit_->set_name("Max Boiler Temp Heating");
    boiler_limit_->traits.set_min_value(30);
    boiler_limit_->traits.set_max_value(90);
    boiler_limit_->traits.set_step(1);
    if (!boiler_limit_->has_state())
      boiler_limit_->publish_state(DEFAULT_MAX_HEATING_TEMP);
  } else {
    ESP_LOGW(OT_LOG_TAG, "No boiler limit number configured.");
  }

  if (dhw_limit_) {
    DHW::max_water_temp = dhw_limit_;
    dhw_limit_->set_name("Max DHW Temp");
    dhw_limit_->traits.set_min_value(30);
    dhw_limit_->traits.set_max_value(90);
    dhw_limit_->traits.set_step(1);
    if (!dhw_limit_->has_state())
      dhw_limit_->publish_state(DEFAULT_MAX_DHW_TEMP);
  } else {
    ESP_LOGW(OT_LOG_TAG, "No DHW limit number configured.");
  }
}

void OpenThermComponent::loop() {
  const uint32_t now = millis();
  if (now - last_poll_ms_ < poll_interval_ms_) return;
  last_poll_ms_ = now;

  float flow_target = 0.0f;

#if ENABLE_EMERGENCY_MODULE
  if (Emergency::is_active()) {
    flow_target = Emergency::get_target();
    ESP_LOGW(OT_LOG_TAG, "Emergency override → %.1f°C", flow_target);
  } else
#endif
  if (g_compensation_mode == CompensationMode::EQUITHERM) {
#if ENABLE_EQUITHERM_MODULE
    flow_target = Equitherm::calculate_target_temp();
    ESP_LOGD(OT_LOG_TAG, "Equitherm flow target (pre-clamp): %.1f°C", flow_target);
#else
    flow_target = DEFAULT_MIN_FLOW_TEMP;
#endif
  } else {
    // Boiler internal curve path (send DID 0x1C if outdoor temp available)
    auto *t_out = OT_SENSOR(ha_weather_temp);
    if (t_out && t_out->has_state()) {
      float v = t_out->state;
      uint16_t raw = static_cast<uint16_t>(v * 256.0f);
      uint32_t frame = build_request(WRITE_DATA, 0x1C, raw);
      send_frame(frame);
      ESP_LOGI(OT_LOG_TAG, "Boiler mode: sent T_out=%.1f°C (DID 0x1C)", v);
    } else {
      ESP_LOGW(OT_LOG_TAG, "Boiler mode active, but no outdoor temperature bound.");
    }
  }

  // Limit enforcement (DHW vs CH)
  float heating_limit = OT_SENSOR(max_boiler_temp) && OT_SENSOR(max_boiler_temp)->has_state()
                        ? OT_SENSOR(max_boiler_temp)->state
                        : DEFAULT_MAX_HEATING_TEMP;
  float dhw_limit = OT_SENSOR(max_dhw_temp) && OT_SENSOR(max_dhw_temp)->has_state()
                        ? OT_SENSOR(max_dhw_temp)->state
                        : DEFAULT_MAX_DHW_TEMP;

  const bool dhw_active_now = this->tap_flow();
  const float active_limit = dhw_active_now ? dhw_limit : heating_limit;
  if (flow_target > active_limit) flow_target = active_limit;

  if (g_compensation_mode == CompensationMode::EQUITHERM
#if ENABLE_EMERGENCY_MODULE
      || Emergency::is_active()
#endif
  ) {
    const uint16_t raw = static_cast<uint16_t>(flow_target * 256.0f);
    const uint32_t frame = build_request(WRITE_DATA, 0x11, raw);
    send_frame(frame);
    if (debug_) ESP_LOGI(OT_LOG_TAG, "Flow setpoint sent = %.1f°C", flow_target);
  }

#if ENABLE_DIAGNOSTICS_MODULE
  Diagnostics::update(this);
#endif
#if ENABLE_DHW_MODULE
  DHW::update(this);
#endif
  Boiler::update(this);
}

// --- Framing helpers (unchanged) ---
uint8_t OpenThermComponent::parity32(uint32_t v) {
  v >>= 1; v ^= v >> 16; v ^= v >> 8; v ^= v >> 4; v &= 0xF;
  static const uint8_t lut[16] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
  return lut[v] & 1u;
}

uint32_t OpenThermComponent::build_request(OTMsgType mt, uint8_t did, uint16_t data) {
  uint32_t f = 0;
  f |= (1u << 31);
  f |= (static_cast<uint32_t>(mt) & 0x7u) << 28;
  f |= (static_cast<uint32_t>(did) & 0xFFu) << 20;
  f |= (static_cast<uint32_t>(data) & 0xFFFFu) << 4;
  f |= parity32(f);
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
  if (logical_one) { line_tx_level(true);  wait_us(HALF_BIT_US); line_tx_level(false); wait_us(HALF_BIT_US); }
  else             { line_tx_level(false); wait_us(HALF_BIT_US); line_tx_level(true);  wait_us(HALF_BIT_US); }
}

bool OpenThermComponent::send_frame(uint32_t frame) {
  line_tx_level(true); wait_us(3 * BIT_US);
  for (int i = 31; i >= 0; --i) tx_manchester_bit((frame >> i) & 1u);
  line_tx_level(true); wait_us(3 * BIT_US);
  if (debug_) ESP_LOGD(OT_LOG_TAG, "TX frame: 0x%08X", frame);
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
      if (first && !second) v = (v << 1) | 1u;
      else if (!first && second) v = (v << 1) | 0u;
      else return false;
    }
    resp = v;
    if (((resp & 0x1u) != parity32(resp))) return false;
    if (debug_) ESP_LOGD(OT_LOG_TAG, "RX frame: 0x%08X", resp);
    return true;
  }
  return false;
}

uint32_t OpenThermComponent::read_did(uint8_t did) {
  const uint32_t req = build_request(READ_DATA, did, 0);
  if (!send_frame(req)) return 0;
  uint32_t resp = 0;
  if (!recv_frame(resp)) {
    if (debug_) ESP_LOGW(OT_LOG_TAG, "Timeout/bad frame for DID 0x%02X", did);
    return 0;
  }
  if (((resp >> 31) & 1u) != 1u) return 0;
  return resp;
}

float OpenThermComponent::parse_f88(uint16_t raw) {
  return static_cast<int16_t>(raw) / 256.0f;
}

} // namespace opentherm
