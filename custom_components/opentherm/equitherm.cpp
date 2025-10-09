#include "equitherm.h"
#include "opentherm.h"
#include "esphome/core/log.h"

namespace opentherm {
namespace Equitherm {

// -----------------------------------------------------------------------------
// Runtime-tunable Equitherm parameters
// These will be linked to ESPHome Number entities (see on_boot lambda in YAML)
// -----------------------------------------------------------------------------
esphome::number::Number *eq_n = nullptr;
esphome::number::Number *eq_k = nullptr;
esphome::number::Number *eq_t = nullptr;
esphome::number::Number *eq_fb_gain = nullptr;

// -----------------------------------------------------------------------------
// Main Equitherm calculation
// -----------------------------------------------------------------------------
float calculate_target_temp() {
  // --- Read inputs from HA sensors (with fallbacks) ---
  float t_out = 10.0f;
  if (id_ha_weather_temp && id_ha_weather_temp->has_state())
    t_out = id_ha_weather_temp->state;

  float t_set = 21.0f;
  if (id_ha_target_temp && id_ha_target_temp->has_state())
    t_set = id_ha_target_temp->state;

  float t_in = 21.0f;
  if (id_ha_indoor_temp && id_ha_indoor_temp->has_state())
    t_in = id_ha_indoor_temp->state;

  // --- Get coefficients (from HA Numbers or fallback to constants) ---
  float n  = (eq_n && eq_n->has_state()) ? eq_n->state : EQ_N;
  float k  = (eq_k && eq_k->has_state()) ? eq_k->state : EQ_K;
  float t  = (eq_t && eq_t->has_state()) ? eq_t->state : EQ_T;
  float fb = (eq_fb_gain && eq_fb_gain->has_state()) ? eq_fb_gain->state : EQ_FB_GAIN;

  // --- Equitherm base formula ---
  float flow_target = (n * (t_set + k - t_out)) + t;

  // --- Indoor feedback correction ---
  float delta = t_set - t_in;
  float correction = delta * fb;
  flow_target += correction;

  // --- Clamp range ---
  if (flow_target < 25.0f) flow_target = 25.0f;
  if (flow_target > 80.0f) flow_target = 80.0f;

  // --- Logging ---
  ESP_LOGI("equitherm",
           "t_out=%.1f°C t_set=%.1f°C t_in=%.1f°C Δ=%.2f -> flow=%.1f°C "
           "(N=%.2f K=%.2f T=%.2f FB=%.2f)",
           t_out, t_set, t_in, delta, flow_target, n, k, t, fb);

  return flow_target;
}

}  // namespace Equitherm
}  // namespace opentherm
