#pragma once
#include <Arduino.h>
#include <Servo.h>

class ServoGates {
public:
  void begin();
  void startGame(unsigned long nowMs);
  void stopGame();
  bool update(unsigned long nowMs); // true when a periodic release begins
  bool isReleasing() const { return releasing_; }
private:
  Servo releaseServo_, tunnelServo_;
  bool running_ = false;
  bool releasing_ = false;
  unsigned long previousReleaseMs_ = 0;
  unsigned long openedMs_ = 0;
};
