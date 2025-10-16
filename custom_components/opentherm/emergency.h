#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

namespace opentherm {
namespace Emergency {

// -----------------------------------------------------------------------------
// 🔹 Emergency Mode logic
// -----------------------------------------------------------------------------

// Enables or disables emergency mode override
void enable(bool state);

// Sets the manual target temperature (used when emergency mode is active)
void set_target(float temp);

// Returns the currently set manual target temperature (°C)
float get_target();

// Returns true if emergency mode is currently active
bool is_active();

// -----------------------------------------------------------------------------
// 🔹 Exposed switch entities (linked in on_boot)
// -----------------------------------------------------------------------------

// These are linked to Home Assistant switches in YAML, then controlled via C++
extern esphome::switch_::Switch *emergency_switch;   // Toggles emergency mode
extern esphome::switch_::Switch *force_heat_switch;  // Forces CH on/off
extern esphome::switch_::Switch *force_dhw_switch;   // Forces DHW on/off

// -----------------------------------------------------------------------------
// 🔹 Optional internal state helpers (defined in emergency.cpp)
// -----------------------------------------------------------------------------
extern bool active;          // Emergency mode active state
extern float manual_target;  // Manual flow setpoint in °C

}  // namespace Emergency
}  // namespace opentherm
