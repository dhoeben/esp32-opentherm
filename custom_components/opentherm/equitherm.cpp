#include "equitherm.h"
#include "opentherm.h"
#include "esphome/core/log.h"

using namespace esphome;

namespace opentherm {
namespace Equitherm {

// Extern pointers to the runtime "number" entities (defined elsewhere)
esphome::number::Number *eq_n = nullptr;
esphome::number::Number *eq_k = nullptr;
esphome::number::Number *eq_t = nullptr;
esphome::number::Number *eq_fb_gain = nullptr;

float calculate_target_temp() {
  // Read linked Home Assistant sensors
  float t_out = (id_ha_weather_temp && id_ha_weather_temp->has_state()) ? id_ha_weather_temp->state : 10.0f;
  float t_set = (id_ha_target_temp && id_ha_target_temp->has_state()) ? id_ha_target_temp->state : 21.0f;
  float t_in  = (id_ha_indoor_temp && id_ha_indoor_temp->has_state()) ? id_ha_indoor_temp->state : 21.0f;

  // Runtime-configurable parameters (fallback to defaults if no state yet)
  float n  = (eq_n && eq_n->has_state()) ? eq_n->state : 0.7f;
  float k  = (eq_k && eq_k->has_state()) ? eq_k->state : 3.0f;
  float t  = (eq_t && eq_t->has_state()) ? eq_t->state : 2.0f;
  float fb = (eq_fb_gain && eq_fb_gain->has_state()) ? eq_fb_gain->state : 3.0f;

  // Equitherm control curve
  float flow_target = (n * (t_set + k - t_out)) + t;

  // Indoor feedback correction
  float delta = t_set - t_in;
  flow_target += delta * fb;

  // ------------------------------------------------------------------
  // Clamp range for safety and respect user-defined boiler limits
  // ------------------------------------------------------------------
  float min_safe = 25.0f;

  // Default max if pointers aren't linked yet
  float max_safe = 80.0f;

  // Read current user limits from HA
  if (opentherm::Boiler::max_heating_temp && opentherm::Boiler::max_heating_temp->has_state())
    max_safe = opentherm::Boiler::max_heating_temp->state;

  if (flow_target < min_safe)
    flow_target = min_safe;

  if (flow_target > max_safe)
    flow_target = max_safe;

  ESP_LOGD("equitherm", "Flow target clamped to %.1f°C (min=%.1f, max=%.1f)", flow_target, min_safe, max_safe);

  ESP_LOGI("equitherm",
           "t_out=%.1f°C t_set=%.1f°C t_in=%.1f°C Δ=%.2f → flow=%.1f°C (N=%.2f K=%.2f T=%.2f FB=%.2f)",
           t_out, t_set, t_in, delta, flow_target, n, k, t, fb);

  return flow_target;
}

}  // namespace Equitherm
}  // namespace opentherm
