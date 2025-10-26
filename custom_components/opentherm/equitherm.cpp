#include "equitherm.h"
#include "boiler.h"
#include "esphome/core/log.h"

using namespace esphome;

namespace opentherm {
namespace Equitherm {

static const char *const TAG = "equitherm";

// Linked runtime variables (declared in equitherm.h)
esphome::number::Number *eq_n = nullptr;
esphome::number::Number *eq_k = nullptr;
esphome::number::Number *eq_t = nullptr;
esphome::number::Number *eq_fb_gain = nullptr;

esphome::sensor::Sensor *id_ha_weather_temp = nullptr;
esphome::sensor::Sensor *id_adaptive_indoor_temp  = nullptr;
esphome::climate::Climate *id_ch_climate     = nullptr;

// -----------------------------------------------------------------------------
// Calculate target flow temperature using adaptive Equitherm curve
// -----------------------------------------------------------------------------
float calculate_target_temp() {
  // --- 1. Inputs ---
  const float t_out = (id_ha_weather_temp && id_ha_weather_temp->has_state())
                          ? id_ha_weather_temp->state
                          : 10.0f;

  const float t_in = (id_adaptive_indoor_temp && id_adaptive_indoor_temp->has_state())
                         ? id_adaptive_indoor_temp->state
                         : 21.0f;

  float t_set = 21.0f;
  if (id_ch_climate != nullptr)
    t_set = id_ch_climate->target_temperature;

  // --- 2. Parameters (from HA or defaults) ---
  const float n  = (eq_n && eq_n->has_state()) ? eq_n->state : 1.14f;   // slope
  const float k  = (eq_k && eq_k->has_state()) ? eq_k->state : 4.0f;    // horizontal offset
  const float t  = (eq_t && eq_t->has_state()) ? eq_t->state : 20.2f;   // vertical offset
  const float fb = (eq_fb_gain && eq_fb_gain->has_state()) ? eq_fb_gain->state : 2.0f; // indoor gain

  // --- 3. Safety limits (dynamic from boiler number) ---
  float min_safe = 30.0f;   // Minimum practical flow temp
  float max_safe = 60.0f;   // Default maximum

  if (opentherm::Boiler::max_heating_temp && opentherm::Boiler::max_heating_temp->has_state())
    max_safe = opentherm::Boiler::max_heating_temp->state;

  // --- 4. Adaptive scaling setup ---
  constexpr float OUT_MIN = -10.0f;  // Coldest outdoor temp
  constexpr float OUT_MAX =  15.0f;  // Warm cut-off outdoor temp

  // Base (raw) linear equation
  float base = (n * (t_set + k - t_out)) + t;

  // Compute base at warm and cold boundaries
  float base_min = (n * (t_set + k - OUT_MAX)) + t;  // flow at +15 °C outdoor
  float base_max = (n * (t_set + k - OUT_MIN)) + t;  // flow at −10 °C outdoor

  // Protect against divide-by-zero if base_max==base_min
  if (fabs(base_max - base_min) < 0.001f)
    base_max = base_min + 0.001f;

  // --- 5. Normalize to [min_safe, max_safe] range ---
  float flow_target = min_safe + (base - base_min) * (max_safe - min_safe) / (base_max - base_min);

  // --- 6. Indoor feedback correction ---
  const float delta = t_set - t_in;  // positive if room is colder than target
  flow_target += delta * fb;

  // --- 7. Clamp for safety ---
  if (flow_target < min_safe)
    flow_target = min_safe;
  if (flow_target > max_safe)
    flow_target = max_safe;

  // --- 8. Optional smoothing (to prevent jitter) ---
  static float smoothed = flow_target;
  smoothed = smoothed * 0.9f + flow_target * 0.1f;

  // --- 9. Logging ---
  ESP_LOGI(TAG,
           "t_out=%.1f°C t_in=%.1f°C t_set=%.1f°C Δ=%.2f → flow=%.1f°C (smoothed=%.1f°C, "
           "n=%.2f k=%.2f t=%.2f fb=%.2f max=%.1f)",
           t_out, t_in, t_set, delta, flow_target, smoothed, n, k, t, fb, max_safe);

  return smoothed;
}

}  // namespace Equitherm
}  // namespace opentherm
