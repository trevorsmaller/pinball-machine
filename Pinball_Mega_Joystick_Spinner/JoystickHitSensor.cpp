#include "JoystickHitSensor.h"
#include "Config.h"

static int distanceFromCenter(int sample, int center) {
  int delta = sample - center;
  return delta < 0 ? -delta : delta;
}

void JoystickHitSensor::begin() {
  pinMode(Pins::JOYSTICK_X, INPUT);
  pinMode(Pins::JOYSTICK_Y, INPUT);
  // Calibrate only with joystick physically centered and not being struck.
  long sumX = 0, sumY = 0;
  for (uint8_t i = 0; i < 16; ++i) {
    sumX += analogRead(Pins::JOYSTICK_X);
    sumY += analogRead(Pins::JOYSTICK_Y);
  }
  centerX_ = (int)(sumX / 16);
  centerY_ = (int)(sumY / 16);
  armed_ = true;
  movePending_ = false;
  recenterPending_ = false;
  Serial.print(F("Joystick center X=")); Serial.print(centerX_);
  Serial.print(F(" Y=")); Serial.println(centerY_);
}

bool JoystickHitSensor::poll(unsigned long nowMs) {
  const int xDelta = distanceFromCenter(analogRead(Pins::JOYSTICK_X), centerX_);
  const int yDelta = distanceFromCenter(analogRead(Pins::JOYSTICK_Y), centerY_);
  const bool moved = xDelta >= Settings::JOYSTICK_TRIGGER_DELTA ||
                     yDelta >= Settings::JOYSTICK_TRIGGER_DELTA;
  const bool centered = xDelta <= Settings::JOYSTICK_RELEASE_DELTA &&
                        yDelta <= Settings::JOYSTICK_RELEASE_DELTA;

  if (armed_) {
    if (!moved) {
      movePending_ = false;
      return false;
    }
    if (!movePending_) {
      movePending_ = true;
      moveSinceMs_ = nowMs;
      return false;
    }
    if (nowMs - moveSinceMs_ >= Settings::JOYSTICK_HIT_STABLE_MS) {
      armed_ = false;
      movePending_ = false;
      recenterPending_ = false;
      return true;
    }
    return false;
  }

  // Already hit: require near-center position for a short stable interval.
  if (!centered) {
    recenterPending_ = false;
    return false;
  }
  if (!recenterPending_) {
    recenterPending_ = true;
    recenterSinceMs_ = nowMs;
  } else if (nowMs - recenterSinceMs_ >= Settings::JOYSTICK_RECENTER_MS) {
    armed_ = true;
    recenterPending_ = false;
  }
  return false;
}
