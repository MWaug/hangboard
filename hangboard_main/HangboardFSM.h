#ifndef HangboardFSM_h
#define HangboardFSM_h

#include "Arduino.h"
#include <Ticker.h>

class HangboardFSM {
private:
  enum state { IN_HANG, ON_GROUND };

  unsigned long _StartHang;
  unsigned long _EndHang;
  float _hang_threshold_lbs;
  float _integratedWeight;
  unsigned long _lastMeasuredTime;
  enum state _cur_state;

public:
  float _last_hang_secs;
  float _cur_hang_secs;
  float _cur_weight;
  float _maxWeight;
  float _aveWeight;
  HangboardFSM(float hang_threshold_lbs);
  ~HangboardFSM();
  void tick(float weight, unsigned long time_ms);
};

#endif