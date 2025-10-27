#pragma once

// ============================================================================
//  OpenTherm Gateway – Hardware & Firmware Defaults
//  (All values here can be overridden by secrets.yaml or YAML includes)
// ============================================================================

// ---------------------------------------------------------------------------
// 💡 Hardware Pin Defaults
// ---------------------------------------------------------------------------
#ifndef OT_IN_PIN
  #define OT_IN_PIN   GPIO_NUM_18    // OpenTherm input pin (RX)
#endif

#ifndef OT_OUT_PIN
  #define OT_OUT_PIN  GPIO_NUM_17    // OpenTherm output pin (TX)
#endif

#ifndef TEMP_PIN
  #define TEMP_PIN    GPIO_NUM_4     // 1-Wire / Dallas temperature sensor bus
#endif

#ifndef LIGHT_PIN
  #define LIGHT_PIN   GPIO_NUM_48   // Onboard status LED
#endif

// ---------------------------------------------------------------------------
// ⚙️ Timing and Communication
// ---------------------------------------------------------------------------
#ifndef OT_POLL_INTERVAL
  #define OT_POLL_INTERVAL 10000    // ms, main polling interval
#endif

#ifndef OT_RX_TIMEOUT
  #define OT_RX_TIMEOUT 40          // ms, OpenTherm RX frame timeout
#endif

// ---------------------------------------------------------------------------
// 🧩 Feature Toggles (compile-time flags)
// ---------------------------------------------------------------------------
// Set to 1 to include, 0 to disable optional subsystems.
// These make it easy to build "lite" or "diagnostic" variants.
#define ENABLE_DHW_MODULE          1   // Domestic Hot Water control
#define ENABLE_EQUITHERM_MODULE    1   // Equitherm (weather-compensated) heating
#define ENABLE_DIAGNOSTICS_MODULE  1   // Fault, comms, and status monitoring
#define ENABLE_EMERGENCY_MODULE    1   // Emergency override system
#define ENABLE_LED_STATUS          1   // Status LED logic

// ---------------------------------------------------------------------------
// 🔍 Debugging and Logging
// ---------------------------------------------------------------------------
#ifndef OT_DEBUG
  #define OT_DEBUG 1                // 1 = verbose OpenTherm logs
#endif

// You can optionally redirect component logs via ESPHome categories
#define OT_LOG_TAG "opentherm"

// ---------------------------------------------------------------------------
// 🌡️ Default Temperature Limits and Safety Values
// ---------------------------------------------------------------------------
#define DEFAULT_MAX_HEATING_TEMP   70.0f   // °C – fallback CH limit
#define DEFAULT_MAX_DHW_TEMP       60.0f   // °C – fallback DHW limit
#define DEFAULT_MIN_FLOW_TEMP      30.0f   // °C – minimum safe flow temp
#define DEFAULT_BOOST_TEMP         20.0f   // °C – used in boost mode
#define DEFAULT_ROOM_TEMP_FALLBACK 21.0f   // °C – used if indoor sensor missing

// ---------------------------------------------------------------------------
// 📡 External HA Sensor Entity Defaults (for documentation/reference)
// These are not used directly in C++ but help organize expected secrets.yaml keys.
// ---------------------------------------------------------------------------
//   weather_entity:   "weather.home"
//   indoor_entity:    "sensor.living_room_temperature"
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 🧠 Safety & Timeout Thresholds
// ---------------------------------------------------------------------------
#define COMM_TIMEOUT_MS            30000   // ms – comms lost threshold
#define MIN_DHW_TEMP               30.0f   // °C – minimum DHW safety temperature

// ---------------------------------------------------------------------------
// 🧩 Versioning
// ---------------------------------------------------------------------------
#define OT_FW_VERSION "0.4.0"
