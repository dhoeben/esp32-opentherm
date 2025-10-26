#include "diagnostics.h"
#include "sensors.h"
#include "opentherm.h"
#include "esphome/core/log.h"
#include <vector>
#include <string>

using namespace esphome;

namespace opentherm {
namespace Diagnostics {

static const char *const TAG = "ot_diag";

// comm state (private to this TU)
static bool     last_comms_ok = false;
static uint32_t last_rx_time  = 0;

static std::string decode_fault_flags(uint16_t data) {
  std::vector<std::string> faults;
  if (data & (1 << 0))  faults.push_back("Service Required");
  if (data & (1 << 1))  faults.push_back("Lockout Active");
  if (data & (1 << 2))  faults.push_back("Low Water Pressure");
  if (data & (1 << 3))  faults.push_back("Flame Loss");
  if (data & (1 << 4))  faults.push_back("Sensor Failure");
  if (data & (1 << 5))  faults.push_back("Overheat Protection");
  if (data & (1 << 6))  faults.push_back("Gas Fault");
  if (data & (1 << 7))  faults.push_back("Air Pressure Fault");
  if (data & (1 << 8))  faults.push_back("Fan Fault");
  if (data & (1 << 9))  faults.push_back("Communication Error");
  if (data & (1 << 10)) faults.push_back("Return Temp Sensor Fault");
  if (data & (1 << 11)) faults.push_back("Flow Temp Sensor Fault");
  if (data & (1 << 12)) faults.push_back("Ignition Failure");
  if (data & (1 << 13)) faults.push_back("Flue Blocked");
  if (data & (1 << 14)) faults.push_back("Circulation Fault");
  if (data & (1 << 15)) faults.push_back("Unknown Fault");
  if (faults.empty()) return "No faults";
  std::string out;
  for (size_t i = 0; i < faults.size(); ++i) {
    out += faults[i];
    if (i + 1 < faults.size()) out += ", ";
  }
  return out;
}

void update(OpenThermComponent *ot) {
#if !ENABLE_DIAGNOSTICS_MODULE
  (void)ot;
  return;
#endif
  if (!ot) return;

  const uint32_t now = esphome::millis();
  bool comms_ok = false;

  // 0x00 Status
  const uint32_t raw00 = ot->read_did(0x00);
  if (raw00 != 0) {
    comms_ok = true;
    last_rx_time = now;

    const uint16_t data = (raw00 >> 8) & 0xFFFF;
    const bool fault  = data & (1 << 0);
    const bool ch     = data & (1 << 1);
    const bool dhw    = data & (1 << 2);
    const bool flame  = data & (1 << 3);
    const bool diag   = data & (1 << 4);

    PUBLISH_IF(OT_SENSOR(ch_active), ch);
    PUBLISH_IF(OT_SENSOR(dhw_active), dhw);
    PUBLISH_IF(OT_SENSOR(dhw_flowing), dhw);
    PUBLISH_IF(OT_SENSOR(flame), flame);
    PUBLISH_IF(OT_SENSOR(fault), fault);
    PUBLISH_IF(OT_SENSOR(diagnostic), diag);

    ot->set_tap_flow(dhw);

    ESP_LOGD(TAG, "Status: Fault=%d, CH=%d, DHW=%d, Flame=%d, Diag=%d",
             fault, ch, dhw, flame, diag);
  }

  // 0x01 Fault word
  const uint32_t raw01 = ot->read_did(0x01);
  if (raw01 != 0) {
    const uint16_t data = (raw01 >> 8) & 0xFFFF;
    const std::string msg = decode_fault_flags(data);
    if (OT_SENSOR(fault_text)) OT_SENSOR(fault_text)->publish_state(msg);
    ESP_LOGI(TAG, "Fault 0x%04X → %s", data, msg.c_str());
  }

  // 0x3E DHW Flow Rate (optional)
  const uint32_t raw3E = ot->read_did(0x3E);
  if (raw3E != 0 && OT_SENSOR(dhw_flow_rate)) {
    const uint16_t data = (raw3E >> 8) & 0xFFFF;
    float flow_lpm = ot->parse_f88(data);
    OT_SENSOR(dhw_flow_rate)->publish_state(flow_lpm);
    ESP_LOGD(TAG, "DHW flow rate: %.2f L/min", flow_lpm);
  }

  // Comm health
  if (OT_SENSOR(comms_ok)) {
    const bool timed_out = (now - last_rx_time) > COMM_TIMEOUT_MS;
    const bool comms_ok_now = comms_ok || !timed_out;
    if (comms_ok_now != last_comms_ok) {
      OT_SENSOR(comms_ok)->publish_state(comms_ok_now);
      if (comms_ok_now) ESP_LOGI(TAG, "OpenTherm communication restored");
      else              ESP_LOGW(TAG, "OpenTherm communication lost!");
      last_comms_ok = comms_ok_now;
    }
  }
}

bool has_fault() {
  return OT_SENSOR(fault) && OT_SENSOR(fault)->state;
}

void clear_faults() {
  if (OT_SENSOR(fault)) OT_SENSOR(fault)->publish_state(false);
  if (OT_SENSOR(fault_text)) OT_SENSOR(fault_text)->publish_state("No faults");
}

} // namespace Diagnostics
} // namespace opentherm
