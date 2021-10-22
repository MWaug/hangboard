#include "HangboardFSM.h"

HangboardFSM::HangboardFSM(float hang_threshold_lbs)
    : _hang_threshold_lbs(hang_threshold_lbs), _integratedWeight(0),
      _lastMeasuredTime(0), _cur_state(ON_GROUND), _finished_hang(false),
      _started_hang(false), _StartHang(0), _EndHang(0), _last_hang_secs(0),
      _cur_hang_secs(0), _cur_weight(0), _maxWeight(0), _aveWeight(0) {}

HangboardFSM::~HangboardFSM() {}

/*
 *  FSM update logic
 */
void HangboardFSM::tick(float weight, unsigned long time_ms) {
  _finished_hang = false;
  _started_hang = false;
  _cur_weight = weight;
  // TODO: Update the hysteresis amount via MQTT
  float weight_hysteresis = 5;

  // Detect the start of a hang event
  if ((_cur_state == ON_GROUND) && (weight > _hang_threshold_lbs)) {
    _StartHang = time_ms;
    Serial.println("Hanging!");
    _cur_state = IN_HANG;
    _maxWeight = 0;
    _integratedWeight = 0;
    _aveWeight = 0;
    _started_hang = true;
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

  // Detect the end of a hang event
  if ((_cur_state == IN_HANG) && weight < _hang_threshold_lbs - weight_hysteresis) {
    _EndHang = time_ms;
    unsigned long hang_dur = _EndHang - _StartHang;
    float hang_dur_secs = hang_dur / 1000.0;
    Serial.println("On the ground");
    Serial.println(hang_dur_secs);
    _last_hang_secs = hang_dur_secs;
    _cur_hang_secs = 0;
    _cur_state = ON_GROUND;
    _finished_hang = true;
  }
  _lastMeasuredTime = time_ms;
}

bool HangboardFSM::finishedHang() { return _finished_hang; }
bool HangboardFSM::startedHang() { return _started_hang; }
