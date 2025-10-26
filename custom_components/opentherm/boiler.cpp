#include "boiler.h"
#include "sensors.h"
#include "opentherm.h"
#include "esphome/core/log.h"

using namespace esphome;

namespace opentherm {
namespace Boiler {

static const char *const TAG = "ot_boiler";

// keep convenience handle (assigned in OpenThermComponent::setup)
esphome::number::Number *max_heating_temp = nullptr;

void update(OpenThermComponent *ot) {
  if (!ot) return;

  // Flow temp (0x18)
  if (uint32_t raw18 = ot->read_did(0x18)) {
    uint16_t data = (raw18 >> 8) & 0xFFFF;
    float value = ot->parse_f88(data);
    PUBLISH_IF(OT_SENSOR(boiler_temp), value);
    ESP_LOGD(TAG, "Water Temp: %.1f°C", value);
  }

  // Return temp (0x19)
  if (uint32_t raw19 = ot->read_did(0x19)) {
    uint16_t data = (raw19 >> 8) & 0xFFFF;
    float value = ot->parse_f88(data);
    PUBLISH_IF(OT_SENSOR(return_temp), value);
    ESP_LOGD(TAG, "Return Temp: %.1f°C", value);
  }

  // Modulation (0x1D)
  if (uint32_t raw1D = ot->read_did(0x1D)) {
    uint16_t data = (raw1D >> 8) & 0xFFFF;
    float value = ot->parse_f88(data);
    PUBLISH_IF(OT_SENSOR(modulation), value);
    ESP_LOGD(TAG, "Modulation: %.0f%%", value);
  }

  // Setpoint echo (0x11)
  if (uint32_t raw11 = ot->read_did(0x11)) {
    uint16_t data = (raw11 >> 8) & 0xFFFF;
    float value = ot->parse_f88(data);
    PUBLISH_IF(OT_SENSOR(setpoint), value);
    ESP_LOGD(TAG, "Setpoint Echo: %.1f°C", value);
  }
}

} // namespace Boiler
} // namespace opentherm
