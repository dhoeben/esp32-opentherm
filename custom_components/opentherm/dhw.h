#pragma once
#include "esphome/components/number/number.h"
#include "esphome/components/climate/climate.h"

namespace opentherm {
class OpenThermComponent;

namespace DHW {

enum class Mode { OFF, ECO, HEAT };

// Linked from component setup
extern esphome::number::Number *max_water_temp;
extern esphome::climate::Climate *dhw_climate;

// Flags / state
extern bool comfort_mode_enabled;

void update(OpenThermComponent *ot);
void set_enabled(OpenThermComponent *ot, bool enabled);
void set_target_temp(OpenThermComponent *ot, float temp);
void set_mode(OpenThermComponent *ot, Mode mode);
void set_comfort_mode(OpenThermComponent *ot, bool enabled);
void update_comfort_mode(OpenThermComponent *ot);
void set_forced(bool on);

} // namespace DHW
} // namespace opentherm
