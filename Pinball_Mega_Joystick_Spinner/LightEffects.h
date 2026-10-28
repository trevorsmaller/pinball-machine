#pragma once
#include <Arduino.h>

enum class LightEvent : uint8_t {
  Start, Score, Lost, Release, Bumper, GameOver, NewRecord
};

// Shared effect controller for TWO connected RGB LED strips.
// No change to the calls in the main game sketch.
class LightEffects {
public:
  void begin();
  void trigger(LightEvent event, unsigned long nowMs);
  void update(bool running, unsigned long nowMs);
private:
  LightEvent event_ = LightEvent::Start;
  unsigned long startedMs_ = 0;
  unsigned long durationMs_ = 0;
  unsigned long lastFrameMs_ = 0;
};
