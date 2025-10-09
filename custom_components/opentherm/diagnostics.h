#pragma once
#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome.h"

namespace opentherm {
class OpenThermComponent;  // forward declaration

namespace Diagnostics {

// --- Binary sensors for boiler state ---
extern esphome::binary_sensor::BinarySensor *ch_active_sensor;
extern esphome::binary_sensor::BinarySensor *dhw_active_sensor;
extern esphome::binary_sensor::BinarySensor *flame_sensor;
extern esphome::binary_sensor::BinarySensor *fault_sensor;
extern esphome::binary_sensor::BinarySensor *diagnostic_sensor;
extern esphome::binary_sensor::BinarySensor *comms_ok_sensor;

// --- Text sensors for detailed faults ---
extern esphome::text_sensor::TextSensor *fault_text_sensor;

// --- State tracking ---
extern bool last_comms_ok;
extern uint32_t last_rx_time;

// --- Core update function ---
void update(OpenThermComponent *ot);

// --- Fault helpers ---
bool has_fault();
void clear_faults();

}  // namespace Diagnostics
}  // namespace opentherm
