#pragma once
namespace esphome { namespace switch_ { class Switch; } }

namespace opentherm {
namespace Emergency {

void enable(bool state);
void set_target(float temp);
float get_target();
bool is_active();

// Linked to HA switches (optionally)
extern esphome::switch_::Switch *emergency_switch;
extern esphome::switch_::Switch *force_heat_switch;
extern esphome::switch_::Switch *force_dhw_switch;

// Internal (optional)
extern bool  active;
extern float manual_target;

} // namespace Emergency
} // namespace opentherm
