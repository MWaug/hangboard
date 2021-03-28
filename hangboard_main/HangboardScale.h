#ifndef HangboardScale_h
#define HangboardScale_h

#include "HangboardDefines.h"
#include "Q2HX711.h"

class HangboardScale {
private:
  Q2HX711 _hx711; //
  long _hx711_offset;
  long _hx711_scale_unit_per_lb;
  float _weight;

public:
  HangboardScale(uint8_t hx711_dout_pin, uint8_t hx711_clk_pin);
  void updateScaleCalibration(long c);
  long zeroOutScale();
  float readWeight();
  ~HangboardScale();
};

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

#endif