#include "PopBumper.h"
#include "Config.h"
void PopBumper::begin() {
  digitalWrite(Pins::BUMPER_DRIVER, LOW);
  pinMode(Pins::BUMPER_DRIVER, OUTPUT);
  stop();
}
bool PopBumper::trigger(unsigned long nowMs) {
  if (hasHit_ && nowMs - lastHitMs_ < Settings::BUMPER_COOLDOWN_MS) return false;
  hasHit_ = true;
  lastHitMs_ = nowMs;
  if (Settings::ARM_BUMPER_OUTPUT) {
    digitalWrite(Pins::BUMPER_DRIVER, HIGH);
    energized_ = true;
    firedMs_ = nowMs;
  }
  return true;
}
void PopBumper::update(unsigned long nowMs) {
  if (energized_ && nowMs - firedMs_ >= Settings::BUMPER_PULSE_MS) stop();
}
void PopBumper::stop() {
  digitalWrite(Pins::BUMPER_DRIVER, LOW);
  energized_ = false;
}
