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

#endif