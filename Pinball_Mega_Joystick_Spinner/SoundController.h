#pragma once
#include <Arduino.h>

enum class SoundEvent : uint8_t { Start, Points, GainBall, LoseBall, Bumper, GameOver };
struct ToneStep { uint16_t freq; uint16_t durationMs; };

class SoundController {
public:
  void begin();
  void play(SoundEvent event, unsigned long nowMs);
  void update(bool gameRunning, unsigned long nowMs);
  void stop();
private:
  void startNote(unsigned long nowMs);
  void startBackground(unsigned long nowMs);
  const ToneStep *sequence_ = nullptr;
  uint8_t count_ = 0, index_ = 0, priority_ = 0;
  bool effect_ = false, playing_ = false;
  unsigned long noteStartedMs_ = 0, eventFinishedMs_ = 0;
  uint8_t backgroundIndex_ = 0;
};
