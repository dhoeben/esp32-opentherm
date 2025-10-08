#pragma once

// -----------------------------------------------------------------------------
// Default hardware and timing settings
// (Overridden by YAML if defined there)
// -----------------------------------------------------------------------------

#define OT_IN_PIN        GPIO_NUM_5
#define OT_OUT_PIN       GPIO_NUM_6
#define OT_POLL_INTERVAL 10000  // ms
#define OT_RX_TIMEOUT    40     // ms
#define OT_DEBUG         1
