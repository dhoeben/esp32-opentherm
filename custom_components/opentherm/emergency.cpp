#include "emergency.h"
#include "esphome/core/log.h"
#include "boiler.h"
#include "dhw.h"

namespace opentherm {
namespace Emergency {

static const char *const TAG = "ot_emergency";

bool  active = false;
float manual_target = DEFAULT_MAX_HEATING_TEMP;

// Exposed switches (optionally linked via __init__.py)
esphome::switch_::Switch *emergency_switch = nullptr;
esphome::switch_::Switch *force_heat_switch = nullptr;
esphome::switch_::Switch *force_dhw_switch = nullptr;

void enable(bool state) {
#if !ENABLE_EMERGENCY_MODULE
  (void)state;
  return;
#endif
  active = state;
  ESP_LOGW(TAG, "Emergency mode %s", state ? "ENABLED" : "DISABLED");
  if (!state) {
    opentherm::DHW::set_forced(false);
    // No persistent CH forced mode in new design; handled via Diagnostics/loop
  }
}

bool is_active() { return active; }

void set_target(float temp) {
  manual_target = temp;
  ESP_LOGI(TAG, "Emergency manual target set to %.1f°C", temp);
}

float get_target() { return manual_target; }

} // namespace Emergency
} // namespace opentherm
