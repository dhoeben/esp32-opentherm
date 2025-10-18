#pragma once
#include "config.h"

#include "esphome.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include <string>

namespace opentherm {

// -----------------------------------------------------------------------------
// OpenTherm message types
// -----------------------------------------------------------------------------
enum OTMsgType : uint8_t {
  READ_DATA  = 0b000,
  WRITE_DATA = 0b001,
};

// -----------------------------------------------------------------------------
// Compensation mode selection (Equitherm vs Boiler internal curve)
// -----------------------------------------------------------------------------
enum class CompensationMode {
  EQUITHERM,
  BOILER
};

// Global variable and helper functions (defined in opentherm.cpp)
extern CompensationMode g_compensation_mode;
void set_compensation_mode(CompensationMode m);
void set_compensation_mode_from_string(const std::string &s);

// -----------------------------------------------------------------------------
// Main OpenTherm component
// -----------------------------------------------------------------------------
class OpenThermComponent : public esphome::Component {
 public:
  OpenThermComponent();

  // Exposed sensors
  esphome::sensor::Sensor *boiler_temp = nullptr;
  esphome::sensor::Sensor *return_temp = nullptr;
  esphome::sensor::Sensor *modulation = nullptr;
  esphome::sensor::Sensor *setpoint = nullptr;

  // DHW sensors
  esphome::sensor::Sensor *dhw_temp = nullptr;
  esphome::sensor::Sensor *dhw_setpoint = nullptr;

  // ---------------------------------------------------------------------------
  // Boiler and DHW sensor setters
  // ---------------------------------------------------------------------------
  void set_boiler_temp_sensor(esphome::sensor::Sensor *s) { boiler_temp = s; }
  void set_return_temp_sensor(esphome::sensor::Sensor *s) { return_temp = s; }
  void set_modulation_sensor(esphome::sensor::Sensor *s) { modulation = s; }
  void set_setpoint_sensor(esphome::sensor::Sensor *s) { setpoint = s; }

  void set_dhw_temp_sensor(esphome::sensor::Sensor *s) { dhw_temp = s; }
  void set_dhw_setpoint_sensor(esphome::sensor::Sensor *s) { dhw_setpoint = s; }

  // ---------------------------------------------------------------------------
  // Boiler & DHW limit Number entities (bound from __init__.py)
  // ---------------------------------------------------------------------------
  void set_boiler_limit_number(esphome::number::Number *n) { boiler_limit_ = n; }
  void set_dhw_limit_number(esphome::number::Number *n) { dhw_limit_ = n; }

  // ---------------------------------------------------------------------------
  // DHW & flow control flags
  // ---------------------------------------------------------------------------
  bool dhw_active() const { return dhw_active_; }
  bool tap_flow() const { return tap_flow_; }
  void set_tap_flow(bool active) { tap_flow_ = active; }

  // ---------------------------------------------------------------------------
  // Standard ESPHome lifecycle
  // ---------------------------------------------------------------------------
  void setup() override;
  void loop() override;

  // ---------------------------------------------------------------------------
  // OpenTherm helpers
  // ---------------------------------------------------------------------------
  uint32_t read_did(uint8_t did);
  static float parse_f88(uint16_t raw);

  void set_pins(esphome::InternalGPIOPin *in_pin, esphome::InternalGPIOPin *out_pin) {
    in_pin_ = in_pin;
    out_pin_ = out_pin;
  }

  void set_poll_interval(uint32_t ms) { poll_interval_ms_ = ms; }
  void set_rx_timeout(uint32_t ms) { rx_timeout_ms_ = ms; }
  void set_debug(bool dbg) { debug_ = dbg; }

  static OpenThermComponent* get_singleton();

  // Expose frame helpers for YAML lambdas
  bool send_frame(uint32_t frame);
  static uint32_t build_request(OTMsgType mt, uint8_t did, uint16_t data);

 private:
  // Hardware pins
  esphome::InternalGPIOPin *in_pin_{nullptr};
  esphome::InternalGPIOPin *out_pin_{nullptr};

  // Config values
  uint32_t poll_interval_ms_{10000};
  uint32_t rx_timeout_ms_{40};
  bool debug_{false};
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

  // Linked limit entities (optional)
  esphome::number::Number *boiler_limit_{nullptr};
  esphome::number::Number *dhw_limit_{nullptr};
};

// -----------------------------------------------------------------------------
// Global references to HA-linked sensors
// -----------------------------------------------------------------------------
extern esphome::sensor::Sensor *id_ha_weather_temp;
extern esphome::sensor::Sensor *id_ha_indoor_temp;

}  // namespace opentherm
