#include "ServoGates.h"
#include "Config.h"

void ServoGates::begin() {
  releaseServo_.attach(Pins::RELEASE_SERVO);
  tunnelServo_.attach(Pins::TUNNEL_SERVO);
  stopGame();
}

void ServoGates::startGame(unsigned long nowMs) {
  running_ = true;
  releasing_ = false;
  previousReleaseMs_ = nowMs;
  releaseServo_.write(Settings::RELEASE_CLOSED_ANGLE);
  tunnelServo_.write(Settings::TUNNEL_OPEN_ANGLE);
}

void ServoGates::stopGame() {
  running_ = false;
  releasing_ = false;
  releaseServo_.write(Settings::RELEASE_CLOSED_ANGLE);
  tunnelServo_.write(Settings::TUNNEL_CLOSED_ANGLE);
}

bool ServoGates::update(unsigned long nowMs) {
  if (!running_) return false;
  if (releasing_ && nowMs - openedMs_ >= Settings::RELEASE_OPEN_MS) {
    releaseServo_.write(Settings::RELEASE_CLOSED_ANGLE);
    releasing_ = false;
  }
  if (!releasing_ && nowMs - previousReleaseMs_ >= Settings::RELEASE_INTERVAL_MS) {
    releaseServo_.write(Settings::RELEASE_OPEN_ANGLE);
    previousReleaseMs_ = nowMs;
    openedMs_ = nowMs;
    releasing_ = true;
    return true;
  }
  return false;
}
