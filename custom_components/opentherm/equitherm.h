#pragma once
#include "esphome.h"
#include "config.h"

namespace opentherm {
namespace Equitherm {

extern esphome::number::Number *eq_n;
extern esphome::number::Number *eq_k;
extern esphome::number::Number *eq_t;
extern esphome::number::Number *eq_fb_gain;

float calculate_target_temp();

}  // namespace Equitherm
}  // namespace opentherm
