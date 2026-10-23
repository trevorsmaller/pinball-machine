#pragma once
#include <Arduino.h>
class PopBumper {
public:
  void begin();
  bool trigger(unsigned long nowMs); // true for accepted hit (even if driver unarmed)
  void update(unsigned long nowMs);
  void stop();
private:
  bool energized_ = false, hasHit_ = false;
  unsigned long firedMs_ = 0, lastHitMs_ = 0;
};
