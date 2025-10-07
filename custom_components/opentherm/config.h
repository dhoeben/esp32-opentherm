#pragma once

// -----------------------------------------------------------------------------
// OpenTherm Hardware and Timing Configuration
// -----------------------------------------------------------------------------

// GPIO pin numbers for your ESP32-C6
#define OT_IN_PIN   GPIO_NUM_5
#define OT_OUT_PIN  GPIO_NUM_6

// Polling interval (ms)
#define OT_POLL_INTERVAL 10000

// Debug logging (1 = enabled, 0 = disabled)
#define OT_DEBUG 1
