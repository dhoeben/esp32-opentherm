#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/climate/climate.h"

namespace opentherm {
class OpenThermComponent;
}

namespace opentherm {
namespace DHW {

// ============================================================
// DHW Operating Modes
// ============================================================
//
// These represent the internal state of the DHW controller.
//
//  - OFF   → DHW completely disabled (OpenTherm 0x38 = 0 °C)
//  - ECO   → DHW active, preheat disabled (on-demand mode)
//  - HEAT  → DHW active, preheat enabled (comfort mode)
//
enum class Mode {
  OFF,
  ECO,
  HEAT
};

// ============================================================
// Global Handles
// ============================================================

// Maximum allowed water temperature limit (configurable via HA number)
extern esphome::number::Number *max_water_temp;

// Linked climate entity for bidirectional HA control
extern esphome::climate::Climate *dhw_climate;

// Comfort preheat mode flag (OpenTherm DID 0x33 → Continuous)
extern bool comfort_mode_enabled;

// Manual / emergency override flag
extern bool forced;

// ============================================================
// Public API
// ============================================================

// Main periodic update called by opentherm.cpp
void update(OpenThermComponent *ot);

// Enable or disable DHW (maps to HVAC mode ON/OFF)
void set_enabled(OpenThermComponent *ot, bool enabled);

// Set new DHW temperature target (sent via OpenTherm DID 0x38)
void set_target_temp(OpenThermComponent *ot, float temp);

// Change DHW operating mode (OFF / ECO / HEAT)
void set_mode(OpenThermComponent *ot, Mode mode);

// Comfort preheat mode control (OpenTherm DID 0x33)
void set_comfort_mode(OpenThermComponent *ot, bool enabled);
void update_comfort_mode(OpenThermComponent *ot);

// Manual “forced” heating override
void set_forced(bool on);

}  // namespace DHW
}  // namespace opentherm
