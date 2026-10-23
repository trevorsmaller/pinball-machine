#pragma once
#include <Arduino.h>

// Active-low edge detector with stable-state debounce, plus lockout.
// A held-low sensor cannot register again until it has returned HIGH.
class EventSensor {
public:
  EventSensor(uint8_t pin, unsigned long stableMs, unsigned long minGapMs);
  void begin();
  bool poll(unsigned long nowMs);  // true ONCE for each confirmed event
  bool active() const { return stableLow_; }
private:
  uint8_t pin_;
  unsigned long stableMs_, minGapMs_;
  unsigned long rawChangeMs_ = 0, lastEventMs_ = 0;
  bool rawLow_ = false, stableLow_ = false, hasEvent_ = false;
};
