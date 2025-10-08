#include "dhw.h"
#include "opentherm.h"
#include "esphome/core/log.h"

namespace opentherm {
namespace DHW {

// Global number pointers
esphome::number::Number *eco_temp_number = nullptr;
esphome::number::Number *normal_temp_number = nullptr;

// Current mode (default = HEAT)
static Mode current_mode = Mode::HEAT;

// Update function – called from opentherm.cpp
void update(OpenThermComponent* ot) {
  if (ot == nullptr) return;

  float setpoint = 50.0f;  // default fallback temperature

  if (current_mode == Mode::OFF) {
    setpoint = 0.0f;  // disables DHW heating
  } else if (current_mode == Mode::ECO && eco_temp_number && eco_temp_number->has_state()) {
    setpoint = eco_temp_number->state;
  } else if (current_mode == Mode::HEAT && normal_temp_number && normal_temp_number->has_state()) {
    setpoint = normal_temp_number->state;
  }

  // Clamp for safety
  if (setpoint < 30.0f) setpoint = 30.0f;
  if (setpoint > 65.0f) setpoint = 65.0f;

  // Send new DHW setpoint (OpenTherm DID 0x38)
  uint16_t raw = static_cast<uint16_t>(setpoint * 256.0f);
  uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, raw);
  ot->send_frame(frame);

  ESP_LOGI("dhw", "Mode=%s, Setpoint=%.1f°C", 
           (current_mode == Mode::OFF ? "OFF" :
            current_mode == Mode::ECO ? "ECO" : "HEAT"),
           setpoint);
}

void set_mode(OpenThermComponent* ot, Mode mode) {
  current_mode = mode;
  update(ot);
}

void set_enabled(OpenThermComponent* ot, bool enabled) {
  if (!enabled) {
    current_mode = Mode::OFF;
  } else if (current_mode == Mode::OFF) {
    current_mode = Mode::HEAT;
  }
  update(ot);
}

void set_target_temp(OpenThermComponent* ot, float temp) {
  uint16_t raw = static_cast<uint16_t>(temp * 256.0f);
  uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, raw);
  ot->send_frame(frame);
}

}  // namespace DHW
}  // namespace opentherm
