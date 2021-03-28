#include "HangboardScale.h"

HangboardScale::HangboardScale(uint8_t hx711_dout_pin, uint8_t hx711_clk_pin)
    : _hx711(hx711_dout_pin, hx711_clk_pin),
      _hx711_offset(HX711_DEFAULT_OFFSET),
      _hx711_scale_unit_per_lb(HX711_DEFAULT_SCALE_UNIT), _weight(0) {}

HangboardScale::~HangboardScale() {}

void HangboardScale::updateScaleCalibration(long c) {
  _hx711_scale_unit_per_lb = c;
}

long HangboardScale::zeroOutScale() {
  long zero_value = 0;
  if (TEST_MODE == false) {
    zero_value = (_hx711_offset - _hx711.read());
  }
  _hx711_offset = _hx711_offset - zero_value;
  return _hx711_offset;
}

float HangboardScale::readWeight() {
  if (TEST_MODE == true) {
    return (millis() % 10000) / 100; // Simulated weight measurement
  }
  _weight = (_hx711_offset - _hx711.read()) / _hx711_scale_unit_per_lb;
  return _weight;
}