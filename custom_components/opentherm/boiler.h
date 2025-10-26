#pragma once
#include "esphome.h"
namespace opentherm {
class OpenThermComponent;
namespace Boiler {
void update(OpenThermComponent *ot);

// Provided by OpenThermComponent during setup via sensors registry
extern esphome::number::Number *max_heating_temp; // kept for convenience
} // namespace Boiler
} // namespace opentherm
