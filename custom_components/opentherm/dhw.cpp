#include "dhw.h"
#include "opentherm.h"
#include "esphome/core/log.h"
#include "esphome/components/climate/climate.h"
#include <cmath>      // for std::isnan
#include <limits>     // for std::numeric_limits<float>::quiet_NaN()

namespace opentherm {
namespace DHW {

static const char *const TAG = "dhw";

// ------------------------------------------------------------
// Global pointers
// ------------------------------------------------------------
esphome::number::Number *max_water_temp = nullptr;
esphome::climate::Climate *dhw_climate = nullptr;  // For bidirectional sync

// ------------------------------------------------------------
// State tracking
// ------------------------------------------------------------
static Mode current_mode = Mode::HEAT;  // Default: comfort (HEAT)
bool comfort_mode_enabled = true;
bool forced = false;

// ============================================================
// Forced (emergency) override
// ============================================================
void set_forced(bool on) {
  forced = on;
  ESP_LOGI(TAG, "Forced DHW mode %s", on ? "ENABLED" : "DISABLED");
}

// ============================================================
// Update function – called from opentherm.cpp
// ============================================================
void update(OpenThermComponent *ot) {
  if (ot == nullptr) return;

  // --- Forced override mode ---
  if (forced) {
    const float temp = 60.0f;
    const uint16_t raw = static_cast<uint16_t>(temp * 256.0f);
    const uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, raw);
    ot->send_frame(frame);
    ESP_LOGW(TAG, "Forced DHW heating active at %.1f°C", temp);
    return;
  }

  // --- Sync comfort mode from boiler (OpenTherm DID 0x33) ---
  update_comfort_mode(ot);

  // --- Determine target setpoint ---
  float setpoint = 60.0f;  // Default fallback

  if (current_mode == Mode::OFF) {
    setpoint = 0.0f;  // Disables DHW
  } else if (dhw_climate && !std::isnan(dhw_climate->target_temperature)) {
    setpoint = dhw_climate->target_temperature;
  } else if (max_water_temp && max_water_temp->has_state()) {
    setpoint = max_water_temp->state;
  }

  // --- Safety clamp ---
  float limit = (max_water_temp && max_water_temp->has_state()) ? max_water_temp->state : 65.0f;
  if (setpoint < 30.0f && current_mode != Mode::OFF) setpoint = 30.0f;
  if (setpoint > limit) setpoint = limit;

  // --- Send DHW setpoint (OpenTherm DID 0x38) ---
  const uint16_t raw = static_cast<uint16_t>(setpoint * 256.0f);
  const uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, raw);
  ot->send_frame(frame);

  // --- Comfort preheat behavior (OpenTherm DID 0x33) ---
  if (current_mode == Mode::HEAT)
    set_comfort_mode(ot, comfort_mode_enabled);
  else if (current_mode == Mode::OFF)
    set_comfort_mode(ot, false);  // Always off in HVAC OFF mode

  ESP_LOGI(TAG, "HVAC=%s | Comfort=%s | Setpoint=%.1f°C",
           (current_mode == Mode::OFF ? "OFF" : "HEAT"),
           (comfort_mode_enabled ? "Comfort" : "Eco"),
           setpoint);

  // --- Sync with Home Assistant DHW Climate Entity ---
  if (dhw_climate != nullptr) {
    // Reflect HVAC mode and comfort preset back to HA
    dhw_climate->current_temperature =
        (ot->dhw_temp && ot->dhw_temp->has_state())
          ? ot->dhw_temp->state
          : std::numeric_limits<float>::quiet_NaN();

    dhw_climate->target_temperature = setpoint;
    dhw_climate->preset = comfort_mode_enabled
        ? esphome::climate::CLIMATE_PRESET_COMFORT
        : esphome::climate::CLIMATE_PRESET_ECO;
    dhw_climate->publish_state();
  }
}

// ============================================================
// Mode and enable helpers
// ============================================================
void set_mode(OpenThermComponent *ot, Mode mode) {
  current_mode = mode;
  update(ot);
}

// Called by climate when toggling HVAC mode between Off and Heat
void set_enabled(OpenThermComponent *ot, bool enabled) {
  if (ot == nullptr) return;

  if (!enabled) {
    current_mode = Mode::OFF;
    ESP_LOGI(TAG, "HVAC mode set to OFF (DHW disabled)");
  } else {
    if (current_mode == Mode::OFF)
      current_mode = Mode::HEAT;
    ESP_LOGI(TAG, "HVAC mode set to HEAT (DHW enabled)");
  }

  update(ot);
}

void set_target_temp(OpenThermComponent *ot, float temp) {
  if (ot == nullptr) return;
  const uint16_t raw = static_cast<uint16_t>(temp * 256.0f);
  const uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, raw);
  ot->send_frame(frame);
  ESP_LOGI(TAG, "Manual DHW target set to %.1f°C", temp);
}

// ============================================================
// Comfort (preheat) mode control — OpenTherm DID 0x33
// ============================================================
void set_comfort_mode(OpenThermComponent *ot, bool enabled) {
  if (ot == nullptr) return;

  comfort_mode_enabled = enabled;

  // DataID 0x33 = DHW Mode
  // 0 = Off, 1 = On-demand, 2 = Continuous (Comfort)
  const uint8_t mode = comfort_mode_enabled ? 2 : 1;
  const uint16_t data = static_cast<uint16_t>(mode) << 8;

  const uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x33, data);
  ot->send_frame(frame);

  ESP_LOGI(TAG, "Comfort mode set to %s",
           comfort_mode_enabled ? "Comfort (preheat)" : "Eco (on-demand)");
}

void update_comfort_mode(OpenThermComponent *ot) {
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
