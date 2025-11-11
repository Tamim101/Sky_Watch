#pragma once
#include "Arduino.h"
#include <math.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
bool ledcAttach(uint8_t pin, uint32_t freq, uint8_t resolution) { return true; }
bool ledcWrite(uint8_t pin, uint32_t duty) { return true; }