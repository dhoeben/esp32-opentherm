#pragma once
#include "esphome.h"
#include "esphome/components/sensor/sensor.h"
#include "opentherm.h"

namespace opentherm {

class OpenThermComponent;

namespace Boiler {

void update(OpenThermComponent *ot);

// Sensors for boiler-side values
extern esphome::sensor::Sensor *water_temp;
extern esphome::sensor::Sensor *return_temp;
extern esphome::sensor::Sensor *modulation;
extern esphome::sensor::Sensor *setpoint;

}  // namespace Boiler
}  // namespace opentherm
