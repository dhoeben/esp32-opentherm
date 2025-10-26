#include "dhw.h"
#include "sensors.h"
#include "opentherm.h"
#include "esphome/core/log.h"
#include <cmath>
#include <limits>

namespace opentherm {
namespace DHW {

static const char *const TAG = "dhw";

esphome::number::Number *max_water_temp = nullptr;
esphome::climate::Climate *dhw_climate = nullptr;

static Mode current_mode = Mode::HEAT;
bool comfort_mode_enabled = true;
static bool forced = false;

void set_forced(bool on) {
  forced = on;
  ESP_LOGI(TAG, "Forced DHW mode %s", on ? "ENABLED" : "DISABLED");
}

void update(OpenThermComponent *ot) {
#if !ENABLE_DHW_MODULE
  (void)ot;
  return;
#endif
  if (!ot) return;

  // Forced
  if (forced) {
    const float temp = 60.0f;
    const uint16_t raw = static_cast<uint16_t>(temp * 256.0f);
    const uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, raw);
    ot->send_frame(frame);
    ESP_LOGW(TAG, "Forced DHW heating active at %.1f°C", temp);
    return;
  }

  // Sync comfort mode from boiler (0x33)
  update_comfort_mode(ot);

  // Compute setpoint
  float setpoint = 60.0f; // default
  if (current_mode == Mode::OFF) setpoint = 0.0f;
  else if (dhw_climate && !std::isnan(dhw_climate->target_temperature)) setpoint = dhw_climate->target_temperature;
  else if (max_water_temp && max_water_temp->has_state()) setpoint = max_water_temp->state;

  // Clamp
  float limit = (max_water_temp && max_water_temp->has_state()) ? max_water_temp->state : DEFAULT_MAX_DHW_TEMP;
  if (setpoint < MIN_DHW_TEMP && current_mode != Mode::OFF) setpoint = MIN_DHW_TEMP;
  if (setpoint > limit) setpoint = limit;

  // Send (0x38)
  const uint16_t raw = static_cast<uint16_t>(setpoint * 256.0f);
  const uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, raw);
  ot->send_frame(frame);

  // Comfort preheat (0x33)
  if (current_mode == Mode::HEAT) set_comfort_mode(ot, comfort_mode_enabled);
  else if (current_mode == Mode::OFF) set_comfort_mode(ot, false);

  ESP_LOGI(TAG, "HVAC=%s | Comfort=%s | Setpoint=%.1f°C",
           (current_mode == Mode::OFF ? "OFF" : "HEAT"),
           (comfort_mode_enabled ? "Comfort" : "Eco"), setpoint);

  // Sync to HA climate
  if (dhw_climate) {
    dhw_climate->current_temperature =
      (OT_SENSOR(dhw_temp) && OT_SENSOR(dhw_temp)->has_state())
        ? OT_SENSOR(dhw_temp)->state
        : std::numeric_limits<float>::quiet_NaN();
    dhw_climate->target_temperature = setpoint;
    dhw_climate->preset = comfort_mode_enabled
      ? esphome::climate::CLIMATE_PRESET_COMFORT
      : esphome::climate::CLIMATE_PRESET_ECO;
    dhw_climate->publish_state();
  }
}

void set_mode(OpenThermComponent *ot, Mode mode) {
  current_mode = mode;
  update(ot);
}

void set_enabled(OpenThermComponent *ot, bool enabled) {
  if (!ot) return;
  if (!enabled) {
    current_mode = Mode::OFF;
    ESP_LOGI(TAG, "HVAC OFF → DHW disabled");
  } else {
    if (current_mode == Mode::OFF) current_mode = Mode::HEAT;
    ESP_LOGI(TAG, "HVAC HEAT → DHW enabled");
  }
  update(ot);
}

void set_target_temp(OpenThermComponent *ot, float temp) {
  if (!ot) return;
  const uint16_t raw = static_cast<uint16_t>(temp * 256.0f);
  const uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x38, raw);
  ot->send_frame(frame);
  ESP_LOGI(TAG, "Manual DHW target set to %.1f°C", temp);
}

void set_comfort_mode(OpenThermComponent *ot, bool enabled) {
  if (!ot) return;
  comfort_mode_enabled = enabled;
  const uint8_t mode = comfort_mode_enabled ? 2 : 1; // 2=Continuous, 1=On-demand
  const uint16_t data = static_cast<uint16_t>(mode) << 8;
  const uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x33, data);
  ot->send_frame(frame);
  ESP_LOGI(TAG, "Comfort mode = %s", comfort_mode_enabled ? "Comfort" : "Eco");
}

void update_comfort_mode(OpenThermComponent *ot) {
  if (!ot) return;
  const uint32_t raw33 = ot->read_did(0x33);
  if (raw33 != 0) {
    const uint16_t data = (raw33 >> 8) & 0xFFFF;
    const uint8_t mode = data >> 8;
    comfort_mode_enabled = (mode == 2);
  }
}

} // namespace DHW
} // namespace opentherm
