#pragma once
#include "config.h"
#include "esphome.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/switch/switch.h"

namespace opentherm {

// Message types
enum OTMsgType : uint8_t {
  READ_DATA  = 0b000,
  WRITE_DATA = 0b001,
};

// Compensation mode
enum class CompensationMode {
  EQUITHERM,
  BOILER
};

class OpenThermComponent : public esphome::Component {
 public:
  OpenThermComponent();

  // ---- Sensor setters (from codegen) ----
  void set_boiler_temp_sensor(esphome::sensor::Sensor *s) { boiler_temp_ = s; }
  void set_return_temp_sensor(esphome::sensor::Sensor *s) { return_temp_ = s; }
  void set_modulation_sensor(esphome::sensor::Sensor *s)  { modulation_  = s; }
  void set_setpoint_sensor(esphome::sensor::Sensor *s)    { setpoint_    = s; }

  // External HA / diagnostic sensors
  void set_ha_weather_sensor(esphome::sensor::Sensor *s) { ha_weather_ = s; }
  void set_ha_indoor_sensor(esphome::sensor::Sensor *s)  { ha_indoor_ = s; }
  void set_adaptive_indoor_sensor(esphome::sensor::Sensor *s) { adaptive_indoor_ = s; }
  void set_dhw_flow_rate_sensor(esphome::sensor::Sensor *s)   { dhw_flow_rate_ = s; }

  // optional DHW sensors (not required by YAML today)
  void set_dhw_temp_sensor(esphome::sensor::Sensor *s)    { dhw_temp_    = s; }
  void set_dhw_setpoint_sensor(esphome::sensor::Sensor *s){ dhw_setpoint_= s; }

  // Limit numbers
  void set_boiler_limit_number(esphome::number::Number *n) { boiler_limit_ = n; }
  void set_dhw_limit_number(esphome::number::Number *n)    { dhw_limit_    = n; }

  // Equitherm numbers (called by __init__.py)
  void set_eq_n_number(esphome::number::Number *n)      { eq_n_  = n; }
  void set_eq_k_number(esphome::number::Number *n)      { eq_k_  = n; }
  void set_eq_t_number(esphome::number::Number *n)      { eq_t_  = n; }
  void set_eq_fb_gain_number(esphome::number::Number *n){ eq_fb_ = n; }

  // Optional linked entities
  void set_climate_entity(esphome::climate::Climate *c)    { ch_climate_ = c; }
  void set_emergency_switch(esphome::switch_::Switch *s)   { emergency_sw_ = s; }
  void set_force_heat_switch(esphome::switch_::Switch *s)  { force_heat_sw_ = s; }
  void set_force_dhw_switch(esphome::switch_::Switch *s)   { force_dhw_sw_ = s; }

  // Runtime flags (DHW)
  bool dhw_active() const { return dhw_active_; }
  bool tap_flow()  const { return tap_flow_; }
  void set_tap_flow(bool active) { tap_flow_ = active; }

  // ESPHome lifecycle
  void setup() override;
  void loop() override;

  // OpenTherm helpers
  uint32_t read_did(uint8_t did);
  static float parse_f88(uint16_t raw);
  bool send_frame(uint32_t frame);
  static uint32_t build_request(OTMsgType mt, uint8_t did, uint16_t data);

  // Hardware / timings / debug
  void set_pins(esphome::InternalGPIOPin *in_pin, esphome::InternalGPIOPin *out_pin) { in_pin_ = in_pin; out_pin_ = out_pin; }
  void set_poll_interval(uint32_t ms) { poll_interval_ms_ = ms; }
  void set_rx_timeout(uint32_t ms)    { rx_timeout_ms_ = ms; }
  void set_debug(bool dbg)            { debug_ = dbg; }

  static OpenThermComponent* get_singleton();

 private:
  // IO
  esphome::InternalGPIOPin *in_pin_{nullptr};
  esphome::InternalGPIOPin *out_pin_{nullptr};

  // Config/state
  uint32_t poll_interval_ms_{OT_POLL_INTERVAL};
  uint32_t rx_timeout_ms_{OT_RX_TIMEOUT};
  bool     debug_{OT_DEBUG};
  uint32_t last_poll_ms_{0};

  // Bit timing
  static constexpr uint32_t HALF_BIT_US = 500;
  static constexpr uint32_t BIT_US      = 1000;

  // Low-level helpers
  void line_tx_level(bool high);
  bool line_rx_level() const;
  void tx_manchester_bit(bool logical_one);
  bool recv_frame(uint32_t &resp);
  static uint8_t parity32(uint32_t v);
  bool wait_us(uint32_t us);

  // DHW flags
  bool dhw_active_{false};
  bool tap_flow_{false};

  // Linked entities
  esphome::sensor::Sensor *boiler_temp_{nullptr};
  esphome::sensor::Sensor *return_temp_{nullptr};
  esphome::sensor::Sensor *modulation_{nullptr};
  esphome::sensor::Sensor *setpoint_{nullptr};

  esphome::sensor::Sensor *dhw_temp_{nullptr};
  esphome::sensor::Sensor *dhw_setpoint_{nullptr};

  esphome::sensor::Sensor *ha_weather_{nullptr};
  esphome::sensor::Sensor *ha_indoor_{nullptr};
  esphome::sensor::Sensor *adaptive_indoor_{nullptr};
  esphome::sensor::Sensor *dhw_flow_rate_{nullptr};

  esphome::number::Number *boiler_limit_{nullptr};
  esphome::number::Number *dhw_limit_{nullptr};

  esphome::number::Number *eq_n_{nullptr};
  esphome::number::Number *eq_k_{nullptr};
  esphome::number::Number *eq_t_{nullptr};
  esphome::number::Number *eq_fb_{nullptr};

  esphome::climate::Climate   *ch_climate_{nullptr};
  esphome::switch_::Switch    *emergency_sw_{nullptr};
  esphome::switch_::Switch    *force_heat_sw_{nullptr};
  esphome::switch_::Switch    *force_dhw_sw_{nullptr};
};

// Global mode API (unchanged)
extern CompensationMode g_compensation_mode;
void set_compensation_mode(CompensationMode m);
void set_compensation_mode_from_string(const std::string &s);

} // namespace opentherm
