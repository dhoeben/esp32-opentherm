#pragma once
#include "esphome.h"
#include "driver/gpio.h"
#include "esp_timer.h"

namespace opentherm {

class OpenThermComponent : public Component {
 public:
  OpenThermComponent();

  // ESPHome sensors
  Sensor *boiler_temp_sensor = new Sensor();

  // Setup and loop
  void setup() override;
  void loop() override;

  // Setters called from Python (YAML config)
  void set_pins(InternalGPIOPin *in_pin, InternalGPIOPin *out_pin) {
    in_pin_ = in_pin;
    out_pin_ = out_pin;
  }
  void set_poll_interval(uint32_t interval) { poll_interval_ = interval; }

 private:
  InternalGPIOPin *in_pin_;
  InternalGPIOPin *out_pin_;
  uint32_t poll_interval_ = 10000;  // default 10s

  static void IRAM_ATTR handle_interrupt(void *arg);
  float read_boiler_temp();
};

}  // namespace opentherm
