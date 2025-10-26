#include "equitherm.h"
#include "sensors.h"
#include "boiler.h"
#include "config.h"
#include "esphome/core/log.h"
#include <cmath>

using namespace esphome;

namespace opentherm {
namespace Equitherm {

static const char *const TAG = "equitherm";

float calculate_target_temp() {
  // Inputs (fall back if missing)
  const float t_out = (OT_SENSOR(ha_weather_temp) && OT_SENSOR(ha_weather_temp)->has_state())
                        ? OT_SENSOR(ha_weather_temp)->state : 10.0f;
  const float t_in  = (OT_SENSOR(adaptive_indoor_temp) && OT_SENSOR(adaptive_indoor_temp)->has_state())
                        ? OT_SENSOR(adaptive_indoor_temp)->state : DEFAULT_ROOM_TEMP_FALLBACK;

  float t_set = 21.0f;
  if (OT_SENSOR(ch_climate))
    t_set = OT_SENSOR(ch_climate)->target_temperature;

  // Parameters
  const float n  = (OT_SENSOR(eq_n)  && OT_SENSOR(eq_n)->has_state())  ? OT_SENSOR(eq_n)->state  : 1.14f;
  const float k  = (OT_SENSOR(eq_k)  && OT_SENSOR(eq_k)->has_state())  ? OT_SENSOR(eq_k)->state  : 4.0f;
  const float t  = (OT_SENSOR(eq_t)  && OT_SENSOR(eq_t)->has_state())  ? OT_SENSOR(eq_t)->state  : 20.2f;
  const float fb = (OT_SENSOR(eq_fb_gain) && OT_SENSOR(eq_fb_gain)->has_state()) ? OT_SENSOR(eq_fb_gain)->state : 2.0f;

  float max_safe = DEFAULT_MAX_HEATING_TEMP;
  if (Boiler::max_heating_temp && Boiler::max_heating_temp->has_state())
    max_safe = Boiler::max_heating_temp->state;

  const float min_safe = DEFAULT_MIN_FLOW_TEMP;

  constexpr float OUT_MIN = -10.0f;
  constexpr float OUT_MAX =  15.0f;

  // Raw linear
  float base = (n * (t_set + k - t_out)) + t;

  float base_min = (n * (t_set + k - OUT_MAX)) + t;
  float base_max = (n * (t_set + k - OUT_MIN)) + t;
  if (std::fabs(base_max - base_min) < 0.001f)
    base_max = base_min + 0.001f;

  float flow_target = min_safe + (base - base_min) * (max_safe - min_safe) / (base_max - base_min);

  // Indoor feedback
  const float delta = t_set - t_in;
  flow_target += delta * fb;

  // Clamp
  if (flow_target < min_safe) flow_target = min_safe;
  if (flow_target > max_safe) flow_target = max_safe;

  // Smoothing
  static float smoothed = flow_target;
  smoothed = smoothed * 0.9f + flow_target * 0.1f;

  ESP_LOGI(TAG,
           "t_out=%.1f t_in=%.1f t_set=%.1f Δ=%.2f → flow=%.1f (smoothed=%.1f, n=%.2f k=%.2f t=%.2f fb=%.2f max=%.1f)",
           t_out, t_in, t_set, delta, flow_target, smoothed, n, k, t, fb, max_safe);

  return smoothed;
}

} // namespace Equitherm
} // namespace opentherm
