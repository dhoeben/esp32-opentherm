#pragma once
#include "esphome.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/climate/climate.h"

namespace opentherm {

// ============================================================================
//  Central Sensor / Entity Registry
// ============================================================================
struct Sensors {
  // --- Boiler (flow loop) ---
  esphome::sensor::Sensor *boiler_temp   = nullptr;
  esphome::sensor::Sensor *return_temp   = nullptr;
  esphome::sensor::Sensor *modulation    = nullptr;
  esphome::sensor::Sensor *setpoint      = nullptr;

  // --- DHW ---
  esphome::sensor::Sensor *dhw_temp      = nullptr;
  esphome::sensor::Sensor *dhw_flow_rate = nullptr;   // optional

  // --- External / HA / helper temps (optional) ---
  esphome::sensor::Sensor *ha_weather_temp      = nullptr;
  esphome::sensor::Sensor *ha_indoor_temp       = nullptr;
  esphome::sensor::Sensor *adaptive_indoor_temp = nullptr;

  // --- Diagnostics ---
  esphome::binary_sensor::BinarySensor *ch_active   = nullptr;
  esphome::binary_sensor::BinarySensor *dhw_active  = nullptr;
  esphome::binary_sensor::BinarySensor *flame       = nullptr;
  esphome::binary_sensor::BinarySensor *fault       = nullptr;
  esphome::binary_sensor::BinarySensor *diagnostic  = nullptr;
  esphome::binary_sensor::BinarySensor *comms_ok    = nullptr;
  esphome::binary_sensor::BinarySensor *dhw_flowing = nullptr;

  esphome::text_sensor::TextSensor *fault_text = nullptr;

  // --- Limits & parameters ---
  esphome::number::Number *max_boiler_temp = nullptr;
  esphome::number::Number *max_dhw_temp    = nullptr;

  esphome::number::Number *eq_n = nullptr;
  esphome::number::Number *eq_k = nullptr;
  esphome::number::Number *eq_t = nullptr;
  esphome::number::Number *eq_fb_gain = nullptr;

  // --- Climate entities ---
  esphome::climate::Climate *ch_climate  = nullptr;
  esphome::climate::Climate *dhw_climate = nullptr;

  // --- Switches (emergency / dev tools) ---
  esphome::switch_::Switch *emergency_mode = nullptr;
  esphome::switch_::Switch *force_heat     = nullptr;
  esphome::switch_::Switch *force_dhw      = nullptr;
  esphome::switch_::Switch *developer_mode = nullptr;
  esphome::switch_::Switch *led_rainbow    = nullptr;
};

// Global instance
extern Sensors sensors;

// Helpers
#define OT_SENSOR(name) ::opentherm::sensors.name
#define PUBLISH_IF(ptr, value) do { if (ptr) (ptr)->publish_state(value); } while(0)

} // namespace opentherm
