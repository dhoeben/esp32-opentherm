#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"  // For number components

// Forward declaration to avoid circular includes
namespace opentherm {
class OpenThermComponent;
}

namespace opentherm {
namespace DHW {

// --- DHW operating modes ---
enum class Mode {
  OFF,
  ECO,
  HEAT
};

// --- Global Number handles (for Home Assistant adjustable setpoints) ---
extern esphome::number::Number *eco_temp_number;
extern esphome::number::Number *normal_temp_number;
extern esphome::number::Number *max_water_temp;

// --- Comfort mode (CW5/CW6 preheat function) ---
extern bool comfort_mode_enabled;

// --- Function prototypes ---
void update(OpenThermComponent* ot);
void set_enabled(OpenThermComponent* ot, bool enabled);
void set_target_temp(OpenThermComponent* ot, float temp);
void set_mode(OpenThermComponent* ot, Mode mode);

// Comfort-mode interface
void set_comfort_mode(OpenThermComponent* ot, bool enabled);
void update_comfort_mode(OpenThermComponent* ot);

}  // namespace DHW
}  // namespace opentherm
