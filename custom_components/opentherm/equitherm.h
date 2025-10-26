#pragma once

#include "esphome.h"
#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/climate/climate.h"

namespace opentherm {
namespace Equitherm {

// Parameters editable via HA (linked at runtime)
extern esphome::number::Number *eq_n;
extern esphome::number::Number *eq_k;
extern esphome::number::Number *eq_t;
extern esphome::number::Number *eq_fb_gain;

// External sensors (linked via ESPHome Home Assistant sensors)
extern esphome::sensor::Sensor *id_ha_weather_temp;
extern esphome::sensor::Sensor *id_adaptive_indoor_temp;

// Local climate entity 
extern esphome::climate::Climate *id_ch_climate;

// Function to compute target boiler flow temperature
float calculate_target_temp();

}  // namespace Equitherm
}  // namespace opentherm
