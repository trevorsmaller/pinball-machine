#pragma once
#include <Arduino.h>

// Two-axis analog joystick used as a one-event-per-deflection bumper sensor.
// Detects either X or Y deviation, then waits for the stick to return to
// its measured center. No blocking delays or repeated events while held.
class JoystickHitSensor {
public:
  void begin();
  bool poll(unsigned long nowMs);
private:
  int centerX_ = 512, centerY_ = 512;
  bool armed_ = true;
  bool movePending_ = false, recenterPending_ = false;
  unsigned long moveSinceMs_ = 0, recenterSinceMs_ = 0;
};
