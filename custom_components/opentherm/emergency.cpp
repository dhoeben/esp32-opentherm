#include "emergency.h"
#include "esphome/core/log.h"
#include "boiler.h"
#include "dhw.h"
#include "opentherm.h"

namespace opentherm {
namespace Emergency {

static const char *const TAG = "ot_emergency";

// -----------------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------------
bool active = false;
float manual_target = 55.0f;  // Default fallback temperature (°C)

// -----------------------------------------------------------------------------
// Core emergency logic
// -----------------------------------------------------------------------------
void enable(bool state) {
  active = state;
  ESP_LOGW(TAG, "Emergency mode %s", state ? "ENABLED" : "DISABLED");

  if (!state) {
    // Reset forced modes when emergency disabled
    opentherm::Boiler::set_forced(false);
    opentherm::DHW::set_forced(false);
  }
}

bool is_active() {
  return active;
}

void set_target(float temp) {
  manual_target = temp;
  ESP_LOGI(TAG, "Emergency manual target temperature set to %.1f°C", temp);
}

float get_target() {
  return manual_target;
}

// -----------------------------------------------------------------------------
// Switch entities
// -----------------------------------------------------------------------------
class EmergencyModeSwitch : public esphome::switch_::Switch {
 protected:
  void write_state(bool state) override {
    enable(state);
    publish_state(state);
  }
};

class ForceHeatSwitch : public esphome::switch_::Switch {
 protected:
  void write_state(bool state) override {
    if (state) {
      ESP_LOGW(TAG, "🔥 Forcing central heating ON (manual override)");
      opentherm::Boiler::set_forced(true);
    } else {
      ESP_LOGI(TAG, "Central heating override OFF");
      opentherm::Boiler::set_forced(false);
    }
    publish_state(state);
  }
};

class ForceDHWSwitch : public esphome::switch_::Switch {
 protected:
  void write_state(bool state) override {
    if (state) {
      ESP_LOGW(TAG, "🚿 Forcing DHW ON (manual override)");
      opentherm::DHW::set_forced(true);
    } else {
      ESP_LOGI(TAG, "DHW override OFF");
      opentherm::DHW::set_forced(false);
    }
    publish_state(state);
  }
};

// -----------------------------------------------------------------------------
// Exposed globals (linked in on_boot)
// -----------------------------------------------------------------------------
esphome::switch_::Switch *emergency_switch = nullptr;
esphome::switch_::Switch *force_heat_switch = nullptr;
esphome::switch_::Switch *force_dhw_switch = nullptr;

}  // namespace Emergency
}  // namespace opentherm
