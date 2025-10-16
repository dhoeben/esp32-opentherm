#include "dhw.h"
#include "opentherm.h"
#include "esphome/core/log.h"

namespace opentherm {
namespace DHW {

static const char *const TAG = "dhw";

// Global number pointers
esphome::number::Number *eco_temp_number = nullptr;
esphome::number::Number *normal_temp_number = nullptr;

// Current mode (default = HEAT)
static Mode current_mode = Mode::HEAT;

// Comfort mode flag
bool comfort_mode_enabled = true;

// ============================================================
// Update function – called from opentherm.cpp
// ============================================================
void update(OpenThermComponent* ot) {
  if (ot == nullptr) return;

  // Sync comfort mode state from boiler
  update_comfort_mode(ot);

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

  ESP_LOGI(TAG, "Mode=%s, Comfort=%s, Setpoint=%.1f°C",
           (current_mode == Mode::OFF ? "OFF" :
            current_mode == Mode::ECO ? "ECO" : "HEAT"),
           (comfort_mode_enabled ? "Comfort" : "Eco"),
           setpoint);
}

// ============================================================
// Mode and enable helpers
// ============================================================
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
  if (ot == nullptr) return;
  uint16_t raw = static_cast<uint16_t>(temp * 256.0f);
  uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, raw);
  ot->send_frame(frame);
  ESP_LOGI(TAG, "Manual DHW target set to %.1f°C", temp);
}

// ============================================================
// Comfort (preheat) mode control — OpenTherm ID 0x33
// ============================================================
void set_comfort_mode(OpenThermComponent* ot, bool enabled) {
  if (ot == nullptr) return;

  comfort_mode_enabled = enabled;

  // DataID 0x33 = DHW Mode
  // 0 = Off, 1 = On-demand, 2 = Continuous (Comfort)
  uint8_t mode = comfort_mode_enabled ? 2 : 1;
  uint16_t data = static_cast<uint16_t>(mode) << 8;

  uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x33, data);
  ot->send_frame(frame);

  ESP_LOGI(TAG, "Comfort mode set to %s",
           comfort_mode_enabled ? "Comfort (preheat)" : "Eco (on-demand)");
}

// Called periodically to keep local state synced with boiler
void update_comfort_mode(OpenThermComponent* ot) {
  if (ot == nullptr) return;

  const uint32_t raw33 = ot->read_did(0x33);
  if (raw33 != 0) {
    const uint16_t data = (raw33 >> 8) & 0xFFFF;
    const uint8_t mode = data >> 8;
    comfort_mode_enabled = (mode == 2);
  }
}

}  // namespace DHW
}  // namespace opentherm
