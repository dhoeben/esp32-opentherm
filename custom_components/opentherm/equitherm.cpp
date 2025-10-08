#include "equitherm.h"
#include "opentherm.h"

namespace opentherm {
namespace Equitherm {

float calculate_target_temp() {
  float t_out = 10.0f;
  if (id_ha_weather_temp && id_ha_weather_temp->has_state())
    t_out = id_ha_weather_temp->state;

  float t_set = 21.0f;
  if (id_ha_target_temp && id_ha_target_temp->has_state())
    t_set = id_ha_target_temp->state;

  float t_in = 21.0f;
  if (id_ha_indoor_temp && id_ha_indoor_temp->has_state())
    t_in = id_ha_indoor_temp->state;

  // Equitherm base formula
  float flow_target = (EQ_N * (t_set + EQ_K - t_out)) + EQ_T;

  // Indoor feedback correction
  float delta = t_set - t_in;
  float correction = delta * EQ_FB_GAIN;
  flow_target += correction;

  // Clamp range
  if (flow_target < 25.0f) flow_target = 25.0f;
  if (flow_target > 80.0f) flow_target = 80.0f;

  ESP_LOGI("equitherm",
           "t_out=%.1f°C t_set=%.1f°C t_in=%.1f°C Δ=%.2f flow=%.1f°C (N=%.2f K=%.2f T=%.2f)",
           t_out, t_set, t_in, delta, flow_target, EQ_N, EQ_K, EQ_T);

  return flow_target;
}

}  // namespace Equitherm
}  // namespace opentherm
