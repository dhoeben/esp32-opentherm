#include "diagnostics.h"
#include "opentherm.h"
#include "esphome/core/log.h"

using namespace esphome;


namespace opentherm {
namespace Diagnostics {

static const char *const TAG = "ot_diagnostics";

// Binary sensors
esphome::binary_sensor::BinarySensor *ch_active_sensor = nullptr;
esphome::binary_sensor::BinarySensor *dhw_active_sensor = nullptr;
esphome::binary_sensor::BinarySensor *flame_sensor = nullptr;
esphome::binary_sensor::BinarySensor *fault_sensor = nullptr;
esphome::binary_sensor::BinarySensor *diagnostic_sensor = nullptr;
esphome::binary_sensor::BinarySensor *comms_ok_sensor = nullptr;

// Text sensors
esphome::text_sensor::TextSensor *fault_text_sensor = nullptr;

// State tracking
bool last_comms_ok = false;
uint32_t last_rx_time = 0;

// ------------------------------------------------------------
// Decode DID 0x01 Fault Flags → String summary
// ------------------------------------------------------------
std::string decode_fault_flags(uint16_t data) {
  std::vector<std::string> faults;

  if (data & (1 << 0)) faults.push_back("Service Required");
  if (data & (1 << 1)) faults.push_back("Lockout Active");
  if (data & (1 << 2)) faults.push_back("Low Water Pressure");
  if (data & (1 << 3)) faults.push_back("Flame Loss");
  if (data & (1 << 4)) faults.push_back("Sensor Failure");
  if (data & (1 << 5)) faults.push_back("Overheat Protection");
  if (data & (1 << 6)) faults.push_back("Gas Fault");
  if (data & (1 << 7)) faults.push_back("Air Pressure Fault");
  if (data & (1 << 8)) faults.push_back("Fan Fault");
  if (data & (1 << 9)) faults.push_back("Communication Error");
  if (data & (1 << 10)) faults.push_back("Return Temp Sensor Fault");
  if (data & (1 << 11)) faults.push_back("Flow Temp Sensor Fault");
  if (data & (1 << 12)) faults.push_back("Ignition Failure");
  if (data & (1 << 13)) faults.push_back("Flue Blocked");
  if (data & (1 << 14)) faults.push_back("Circulation Fault");
  if (data & (1 << 15)) faults.push_back("Unknown Fault");

  if (faults.empty()) return "No faults";
  std::string result;
  for (size_t i = 0; i < faults.size(); ++i) {
    result += faults[i];
    if (i != faults.size() - 1) result += ", ";
  }
  return result;
}

// ------------------------------------------------------------
// Update all diagnostic sensors
// ------------------------------------------------------------
void update(OpenThermComponent *ot) {
  if (ot == nullptr) return;

  const uint32_t now = millis();
  bool comms_ok = false;

  // -------------------------------------------------------
  // Status Word (DID 0x00)
  // -------------------------------------------------------
  const uint32_t raw00 = ot->read_did(0x00);
  if (raw00 != 0) {
    comms_ok = true;
    last_rx_time = now;

    const uint16_t data = (raw00 >> 8) & 0xFFFF;
    const bool ch_active   = data & (1 << 6);
    const bool dhw_active  = data & (1 << 7);
    const bool flame_on    = data & (1 << 8);
    const bool fault       = data & (1 << 5);
    const bool diagnostic  = data & (1 << 11);

    if (ch_active_sensor)   ch_active_sensor->publish_state(ch_active);
    if (dhw_active_sensor)  dhw_active_sensor->publish_state(dhw_active);
    if (flame_sensor)       flame_sensor->publish_state(flame_on);
    if (fault_sensor)       fault_sensor->publish_state(fault);
    if (diagnostic_sensor)  diagnostic_sensor->publish_state(diagnostic);

    ESP_LOGD(TAG, "Status bits: CH=%d DHW=%d Flame=%d Fault=%d Diagnostic=%d",
             ch_active, dhw_active, flame_on, fault, diagnostic);
  }

  // -------------------------------------------------------
  // Fault Word (DID 0x01)
  // -------------------------------------------------------
  const uint32_t raw01 = ot->read_did(0x01);
  if (raw01 != 0 && fault_text_sensor != nullptr) {
    const uint16_t data = (raw01 >> 8) & 0xFFFF;
    const std::string msg = decode_fault_flags(data);
    fault_text_sensor->publish_state(msg);
    ESP_LOGI(TAG, "Fault flags 0x%04X → %s", data, msg.c_str());
  }

  // -------------------------------------------------------
  // Communication OK status
  // -------------------------------------------------------
  if (comms_ok_sensor) {
    if (comms_ok) {
      if (!last_comms_ok)
        ESP_LOGI(TAG, "OpenTherm communication restored");
      comms_ok_sensor->publish_state(true);
      last_comms_ok = true;
    } else {
      // Timeout if no comms within 30 s
      if (last_comms_ok && (now - last_rx_time > 30000)) {
        ESP_LOGW(TAG, "OpenTherm communication lost!");
        comms_ok_sensor->publish_state(false);
        last_comms_ok = false;
      }
    }
  }
}

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
bool has_fault() {
  return fault_sensor && fault_sensor->state;
}

void clear_faults() {
  if (fault_sensor) fault_sensor->publish_state(false);
  if (fault_text_sensor) fault_text_sensor->publish_state("No faults");
}

}  // namespace Diagnostics
}  // namespace opentherm
