#include "dhw.h"
#include "opentherm.h"

namespace opentherm {
namespace DHW {

// Linked number entities (populated via YAML)
esphome::number::Number *eco_temp_number = nullptr;
esphome::number::Number *normal_temp_number = nullptr;

void update(OpenThermComponent* ot) {
  if (!ot) return;

  // Read DHW temperature (0x1A)
  if (uint32_t raw = ot->read_did(0x1A)) {
    uint16_t data = (raw >> 8) & 0xFFFF;
    float temp = OpenThermComponent::parse_f88(data);
    ot->dhw_temp->publish_state(temp);
  }

  // Read DHW setpoint (0x38)
  if (uint32_t raw = ot->read_did(0x38)) {
    uint16_t data = (raw >> 8) & 0xFFFF;
    float temp = OpenThermComponent::parse_f88(data);
    ot->dhw_setpoint->publish_state(temp);
  }
}

void set_enabled(OpenThermComponent* ot, bool enabled) {
  if (!ot) return;
  uint16_t data = enabled ? 1 : 0;
  uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x33, data);
  ot->send_frame(frame);
  ESP_LOGI("dhw", "DHW %s", enabled ? "enabled" : "disabled");
}

void set_target_temp(OpenThermComponent* ot, float temp) {
  if (!ot) return;
  if (temp < 35.0f) temp = 35.0f;
  if (temp > 65.0f) temp = 65.0f;
  uint16_t data = static_cast<uint16_t>(temp * 256.0f);
  uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, data);
  ot->send_frame(frame);
  ESP_LOGI("dhw", "Set DHW target: %.1f°C", temp);
}

void set_mode(OpenThermComponent* ot, Mode mode) {
  if (!ot) return;
  float eco_temp = (eco_temp_number && eco_temp_number->has_state()) ? eco_temp_number->state : 45.0f;
  float normal_temp = (normal_temp_number && normal_temp_number->has_state()) ? normal_temp_number->state : 60.0f;

  switch (mode) {
    case Mode::OFF:
      set_enabled(ot, false);
      ESP_LOGI("dhw", "DHW mode -> OFF");
      break;
    case Mode::ECO:
      set_enabled(ot, true);
      set_target_temp(ot, eco_temp);
      ESP_LOGI("dhw", "DHW mode -> ECO (%.1f°C)", eco_temp);
      break;
    case Mode::HEAT:
      set_enabled(ot, true);
      set_target_temp(ot, normal_temp);
      ESP_LOGI("dhw", "DHW mode -> HEAT (%.1f°C)", normal_temp);
      break;
  }
}

}  // namespace DHW
}  // namespace opentherm
