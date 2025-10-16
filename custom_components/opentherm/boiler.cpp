#include "boiler.h"
#include "opentherm.h"
#include "esphome/core/log.h"

using namespace esphome;

namespace opentherm {
namespace Boiler {

static const char *const TAG = "ot_boiler";

// Boiler sensors
esphome::sensor::Sensor *water_temp = new esphome::sensor::Sensor();
esphome::sensor::Sensor *return_temp = new esphome::sensor::Sensor();
esphome::sensor::Sensor *modulation = new esphome::sensor::Sensor();
esphome::sensor::Sensor *setpoint = new esphome::sensor::Sensor();

// Boiler limits (numbers)
esphome::number::Number *max_heating_temp = nullptr;

bool forced = false;

void set_forced(bool on) {
  forced = on;
  ESP_LOGI("boiler", "Forced CH mode %s", on ? "ENABLED" : "DISABLED");
}

void update(OpenThermComponent *ot) {
  if (!ot) return;
  if (forced) {
    // Send constant heat request (DID 0x00)
    uint32_t frame = OpenThermComponent::build_request(WRITE_DATA, 0x00, 0x0400); // CH mode bit set
    ot->send_frame(frame);
    return;
  }
  
  float limit = Boiler::max_heating_temp->state;

  // Boiler water temperature (DID 0x18)
  if (uint32_t raw18 = ot->read_did(0x18)) {
    uint16_t data = (raw18 >> 8) & 0xFFFF;
    float value = ot->parse_f88(data);
    water_temp->publish_state(value);
    ESP_LOGD(TAG, "Water Temp: %.1f°C", value);
  }

  // Return temperature (DID 0x19)
  if (uint32_t raw19 = ot->read_did(0x19)) {
    uint16_t data = (raw19 >> 8) & 0xFFFF;
    float value = ot->parse_f88(data);
    return_temp->publish_state(value);
    ESP_LOGD(TAG, "Return Temp: %.1f°C", value);
  }

  // Modulation level (DID 0x1D)
  if (uint32_t raw1D = ot->read_did(0x1D)) {
    uint16_t data = (raw1D >> 8) & 0xFFFF;
    float value = ot->parse_f88(data);
    modulation->publish_state(value);
    ESP_LOGD(TAG, "Modulation: %.0f%%", value);
  }

  // Control setpoint echo (DID 0x11)
  if (uint32_t raw11 = ot->read_did(0x11)) {
    uint16_t data = (raw11 >> 8) & 0xFFFF;
    float value = ot->parse_f88(data);
    setpoint->publish_state(value);
    ESP_LOGD(TAG, "Setpoint Echo: %.1f°C", value);
  }
}

}  // namespace Boiler
}  // namespace opentherm
