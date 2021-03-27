#include "Q2HX711.h"
// hx711(OUT,CLK);
// D2 = GPIO4, D1 = GPIO5
Q2HX711 hx711(D2,D1);

long hx711_offset = 7785000; // Scale at zero lbs
long hx711_scale_unit_per_lb = calib;

void update_scale_calibration(long c) {
  hx711_scale_unit_per_lb = c;
}

void zero_out_scale(){
  long zero_value = 0;
  if(test_mode == false) {
    zero_value = (hx711_offset - hx711.read());
  }
  hx711_offset = hx711_offset - zero_value;
}

float read_hx711_weight() {
  if(test_mode == true) {
    return (millis() % 10000) / 100; // Simulated weight measurement
  }
  return (hx711_offset - hx711.read()) / hx711_scale_unit_per_lb;
}