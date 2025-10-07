#include "opentherm.h"
using namespace esphome;

namespace opentherm {

OpenThermComponent::OpenThermComponent() {}

void IRAM_ATTR OpenThermComponent::handle_interrupt(void* arg) {
  // Placeholder ISR
}

void OpenThermComponent::setup() {
  // Configure pins
  in_pin_->setup();   // handled by ESPHome
  out_pin_->setup();

  ESP_LOGI("opentherm", "Configured pins IN=%d, OUT=%d, interval=%u ms",
           in_pin_->get_pin(), out_pin_->get_pin(), poll_interval_);
}

float OpenThermComponent::read_boiler_temp() {
  static float t = 50.0;
  t += 0.2;
  if (t > 70) t = 50;
  return t;
}

void OpenThermComponent::loop() {
  static uint32_t last = 0;
  uint32_t now = millis();

  if (now - last >= poll_interval_) {
    last = now;

    float temp = read_boiler_temp();
    boiler_temp_sensor->publish_state(temp);
    ESP_LOGD("opentherm", "Boiler temp %.1f°C", temp);
  }
}

}  // namespace opentherm
