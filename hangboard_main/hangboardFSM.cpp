#include "HangboardFSM.h"

HangboardFSM::HangboardFSM(float hang_threshold_lbs)
    : _StartHang(0), _EndHang(0), _hang_threshold_lbs(hang_threshold_lbs),
      _integratedWeight(0), _lastMeasuredTime(0), _cur_state(ON_GROUND),
      _last_hang_secs(0), _cur_hang_secs(0), _cur_weight(0), _maxWeight(0),
      _aveWeight(0) {}

HangboardFSM::~HangboardFSM() {}

/*
 *  FSM update logic
 */
void HangboardFSM::tick(float weight, unsigned long time_ms) {
  _cur_weight = weight;
  if ((_cur_state == ON_GROUND) && (weight > _hang_threshold_lbs)) {
    _StartHang = time_ms;
    Serial.println("Hanging!");
    _cur_state = IN_HANG;
    _maxWeight = 0;
    _integratedWeight = 0;
    _aveWeight = 0;
  }
  // Calculate hang statistics
  if (_cur_state == IN_HANG) {
    if (weight > _maxWeight) {
      _maxWeight = weight;
    }
    _integratedWeight =
        _integratedWeight + (weight * (time_ms - _lastMeasuredTime));
    _aveWeight = _integratedWeight / (time_ms - _StartHang);
    _cur_hang_secs = (time_ms - _StartHang) / 1000.0;
  }
  if ((_cur_state == IN_HANG) && weight < _hang_threshold_lbs - 5) {
    _EndHang = time_ms;
    unsigned long hang_dur = _EndHang - _StartHang;
    float hang_dur_secs = hang_dur / 1000.0;
    Serial.println("On the ground");
    Serial.println(hang_dur_secs);
    _last_hang_secs = hang_dur_secs;
    _cur_hang_secs = 0;
    _cur_state = ON_GROUND;
  }
  _lastMeasuredTime = time_ms;
}
