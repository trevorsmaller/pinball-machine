#include "LightEffects.h"
#include "Config.h"

void LightEffects::begin() {
  pinMode(Pins::LED_SCORE, OUTPUT);
  pinMode(Pins::LED_LOSS, OUTPUT);
  pinMode(Pins::LED_STATUS, OUTPUT);
  digitalWrite(Pins::LED_SCORE, LOW);
  digitalWrite(Pins::LED_LOSS, LOW);
  digitalWrite(Pins::LED_STATUS, LOW);
}
void LightEffects::trigger(LightEvent event, unsigned long nowMs) {
  event_ = event;
  startedMs_ = nowMs;
  switch (event) {
    case LightEvent::Start: durationMs_ = 1200; break;
    case LightEvent::Score: durationMs_ = 450; break;
    case LightEvent::Bumper: durationMs_ = 500; break;
    case LightEvent::Lost: durationMs_ = 900; break;
    case LightEvent::Release: durationMs_ = 350; break;
    case LightEvent::GameOver: durationMs_ = 2400; break;
    case LightEvent::NewRecord: durationMs_ = 2000; break;
  }
}
void LightEffects::update(bool running, unsigned long nowMs) {
  bool score = false, loss = false, status = running;
  if (nowMs - startedMs_ < durationMs_) {
    unsigned long elapsed = nowMs - startedMs_;
    bool blink = (elapsed / 110UL) % 2UL == 0;
    switch (event_) {
      case LightEvent::Start: status = blink; break;
      case LightEvent::Score: score = blink; break;
      case LightEvent::Bumper: score = blink; status = blink; break;
      case LightEvent::Lost: loss = blink; break;
      case LightEvent::Release: status = blink; break;
      case LightEvent::GameOver: score = blink; loss = blink; status = blink; break;
      case LightEvent::NewRecord: score = blink; loss = !blink; status = blink; break;
    }
  }
  digitalWrite(Pins::LED_SCORE, score ? HIGH : LOW);
  digitalWrite(Pins::LED_LOSS, loss ? HIGH : LOW);
  digitalWrite(Pins::LED_STATUS, status ? HIGH : LOW);
}
