#include "SensorInputs.h"

EventSensor::EventSensor(uint8_t pin, unsigned long stableMs, unsigned long minGapMs)
    : pin_(pin), stableMs_(stableMs), minGapMs_(minGapMs) {}

void EventSensor::begin() {
  pinMode(pin_, INPUT_PULLUP);
  rawLow_ = digitalRead(pin_) == LOW;
  stableLow_ = rawLow_;
  rawChangeMs_ = millis();
  lastEventMs_ = 0;
  hasEvent_ = false;
}

bool EventSensor::poll(unsigned long nowMs) {
  bool low = digitalRead(pin_) == LOW;
  if (low != rawLow_) {
    rawLow_ = low;
    rawChangeMs_ = nowMs;
  }
  if (stableLow_ != rawLow_ && nowMs - rawChangeMs_ >= stableMs_) {
    stableLow_ = rawLow_;
    if (stableLow_ && (!hasEvent_ || nowMs - lastEventMs_ >= minGapMs_)) {
      hasEvent_ = true;
      lastEventMs_ = nowMs;
      return true;
    }
  }
  return false;
}
