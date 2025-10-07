#pragma once
#include "esphome.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esphome/components/sensor/sensor.h"

namespace opentherm {

// ============================
// OpenTherm message constants
// ============================
enum OTMsgType : uint8_t {
  READ_DATA  = 0b000,  // Master → Read
  WRITE_DATA = 0b001,  // Master → Write
};

// ============================
// Default Equitherm Coefficients
// (can be overridden via YAML -> secrets)
// ============================
#ifndef EQ_N
#define EQ_N 0.7f
#endif
#ifndef EQ_K
#define EQ_K 3.0f
#endif
#ifndef EQ_T
#define EQ_T 2.0f
#endif
#define EQ_FB_GAIN atof(esphome::App.get_secret("eq_fb_gain").c_str())


// ============================================================
//  Main Component Class
// ============================================================
class OpenThermComponent : public esphome::Component {
 public:
  OpenThermComponent();

  // Exposed sensors (published to HA)
  esphome::sensor::Sensor *boiler_temp  = new esphome::sensor::Sensor();
  esphome::sensor::Sensor *return_temp  = new esphome::sensor::Sensor();
  esphome::sensor::Sensor *modulation   = new esphome::sensor::Sensor();
  esphome::sensor::Sensor *setpoint     = new esphome::sensor::Sensor();

  // Component lifecycle
  void setup() override;
  void loop() override;

  // Helpers to read boiler data
  uint32_t read_did(uint8_t did);
  static float parse_f88(uint16_t raw);

  // Config setters (called from Python binding)
  void set_pins(esphome::InternalGPIOPin *in_pin, esphome::InternalGPIOPin *out_pin) {
    in_pin_ = in_pin;
    out_pin_ = out_pin;
  }
  void set_poll_interval(uint32_t ms) { poll_interval_ms_ = ms; }
  void set_rx_timeout(uint32_t ms) { rx_timeout_ms_ = ms; }
  void set_debug(bool dbg) { debug_ = dbg; }

  // Singleton accessor
  static OpenThermComponent* get_singleton();

 private:
  // ---------------- Config ----------------
  esphome::InternalGPIOPin *in_pin_{nullptr};
  esphome::InternalGPIOPin *out_pin_{nullptr};
  uint32_t poll_interval_ms_{10000};
  uint32_t rx_timeout_ms_{40};
  bool debug_{false};
  uint32_t last_poll_ms_{0};

  // ---------------- Timing ----------------
  static constexpr uint32_t HALF_BIT_US = 500; // 0.5 ms (Manchester half-bit)
  static constexpr uint32_t BIT_US      = 1000;

  // ---------------- Low-level OT ----------------
  void line_tx_level(bool high);
  bool line_rx_level() const;
  void tx_manchester_bit(bool logical_one);
  bool send_frame(uint32_t frame);
  bool recv_frame(uint32_t &resp);
  static uint32_t build_request(OTMsgType mt, uint8_t did, uint16_t data);
  static uint8_t parity32(uint32_t v);
  bool wait_us(uint32_t us);
};

}  // namespace opentherm
