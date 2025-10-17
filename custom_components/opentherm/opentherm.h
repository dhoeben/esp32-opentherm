#pragma once
#include "config.h"

#include "esphome.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "esphome/components/sensor/sensor.h"


namespace opentherm {

enum OTMsgType : uint8_t {
  READ_DATA  = 0b000,
  WRITE_DATA = 0b001,
};

class OpenThermComponent : public esphome::Component {
 public:
  OpenThermComponent();

  // Exposed sensors
  esphome::sensor::Sensor *boiler_temp  = new esphome::sensor::Sensor();
  esphome::sensor::Sensor *return_temp  = new esphome::sensor::Sensor();
  esphome::sensor::Sensor *modulation   = new esphome::sensor::Sensor();
  esphome::sensor::Sensor *setpoint     = new esphome::sensor::Sensor();

  // DHW sensors
  esphome::sensor::Sensor *dhw_temp     = new esphome::sensor::Sensor();
  esphome::sensor::Sensor *dhw_setpoint = new esphome::sensor::Sensor();


  void set_boiler_temp_sensor(esphome::sensor::Sensor *s) { boiler_temp = s; }
  void set_return_temp_sensor(esphome::sensor::Sensor *s) { return_temp = s; }
  void set_modulation_sensor(esphome::sensor::Sensor *s) { modulation = s; }
  void set_setpoint_sensor(esphome::sensor::Sensor *s) { setpoint = s; }

  void set_dhw_temp_sensor(esphome::sensor::Sensor *s) { dhw_temp = s; }
  void set_dhw_setpoint_sensor(esphome::sensor::Sensor *s) { dhw_setpoint = s; }

  bool dhw_active() const { return dhw_active_; }
  bool tap_flow()  const { return tap_flow_;  }
  void set_tap_flow(bool active) { tap_flow_ = active; }


  void setup() override;
  void loop() override;

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

  // ✅ Moved here so YAML lambdas can use them
  bool send_frame(uint32_t frame);
  static uint32_t build_request(OTMsgType mt, uint8_t did, uint16_t data);

 private:
  esphome::InternalGPIOPin *in_pin_{nullptr};
  esphome::InternalGPIOPin *out_pin_{nullptr};
  uint32_t poll_interval_ms_{10000};
  uint32_t rx_timeout_ms_{40};
  bool debug_{false};
  uint32_t last_poll_ms_{0};

  static constexpr uint32_t HALF_BIT_US = 500;
  static constexpr uint32_t BIT_US      = 1000;

  void line_tx_level(bool high);
  bool line_rx_level() const;
  void tx_manchester_bit(bool logical_one);
  bool recv_frame(uint32_t &resp);
  static uint8_t parity32(uint32_t v);
  bool wait_us(uint32_t us);

  bool dhw_active_{false};
  bool tap_flow_{false};
};

extern esphome::sensor::Sensor *id_ha_weather_temp;
extern esphome::sensor::Sensor *id_ha_indoor_temp;

}  // namespace opentherm
