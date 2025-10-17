#include "equitherm.h"
#include "boiler.h"
#include "esphome/core/log.h"

using namespace esphome;

namespace opentherm {
namespace Equitherm {

// Linked runtime variables (declared in equitherm.h)
esphome::number::Number *eq_n = nullptr;
esphome::number::Number *eq_k = nullptr;
esphome::number::Number *eq_t = nullptr;
esphome::number::Number *eq_fb_gain = nullptr;

esphome::sensor::Sensor *id_ha_weather_temp = nullptr;
esphome::sensor::Sensor *id_ha_indoor_temp = nullptr;

esphome::climate::Climate *id_ch_climate = nullptr;

// -----------------------------------------------------------------------------
// Calculate target flow temperature using Equitherm curve
// -----------------------------------------------------------------------------
float calculate_target_temp() {
  // --- Inputs ---
  const float t_out = (id_ha_weather_temp && id_ha_weather_temp->has_state())
                          ? id_ha_weather_temp->state
                          : 10.0f;

  const float t_in = (id_ha_indoor_temp && id_ha_indoor_temp->has_state())
                         ? id_ha_indoor_temp->state
                         : 21.0f;

  float t_set = 21.0f;
  if (id_ch_climate != nullptr) {
    // Use the ESPHome climate’s target temperature
    t_set = id_ch_climate->target_temperature;
  }

  // --- Curve parameters ---
  const float n  = (eq_n && eq_n->has_state()) ? eq_n->state : 0.7f;
  const float k  = (eq_k && eq_k->has_state()) ? eq_k->state : 3.0f;
  const float t  = (eq_t && eq_t->has_state()) ? eq_t->state : 2.0f;
  const float fb = (eq_fb_gain && eq_fb_gain->has_state()) ? eq_fb_gain->state : 3.0f;

  // --- Equitherm control formula ---
  float flow_target = (n * (t_set + k - t_out)) + t;

  // Indoor feedback correction
  const float delta = t_set - t_in;
  flow_target += delta * fb;

  // --- Clamp range for safety ---
  float min_safe = 25.0f;
  float max_safe = 80.0f;

  if (opentherm::Boiler::max_heating_temp && opentherm::Boiler::max_heating_temp->has_state())
    max_safe = opentherm::Boiler::max_heating_temp->state;

  if (flow_target < min_safe)
    flow_target = min_safe;

  if (flow_target > max_safe)
    flow_target = max_safe;

  // --- Logging ---
  ESP_LOGD("equitherm", "Flow target clamped to %.1f°C (min=%.1f, max=%.1f)", flow_target, min_safe, max_safe);
  ESP_LOGI("equitherm",
           "t_out=%.1f°C t_set=%.1f°C t_in=%.1f°C Δ=%.2f → flow=%.1f°C (N=%.2f K=%.2f T=%.2f FB=%.2f)",
           t_out, t_set, t_in, delta, flow_target, n, k, t, fb);

  return flow_target;
}

}  // namespace Equitherm
}  // namespace opentherm
