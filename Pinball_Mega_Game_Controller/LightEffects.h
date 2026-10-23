#pragma once
#include <Arduino.h>
enum class LightEvent : uint8_t { Start, Score, Lost, Release, Bumper, GameOver, NewRecord };

class LightEffects {
public:
  void begin();
  void trigger(LightEvent event, unsigned long nowMs);
  void update(bool running, unsigned long nowMs);
private:
  LightEvent event_ = LightEvent::Start;
  unsigned long startedMs_ = 0, durationMs_ = 0;
};
