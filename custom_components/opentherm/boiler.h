#pragma once
#include "esphome.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
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

// User-configurable temperature limits
extern esphome::number::Number *max_heating_temp;

// Emergency mode
extern bool forced;
void bind_sensors(esphome::sensor::Sensor *water, esphome::sensor::Sensor *ret, esphome::sensor::Sensor *mod, esphome::sensor::Sensor *setp);

void set_forced(bool on);

}  // namespace Boiler
}  // namespace opentherm
