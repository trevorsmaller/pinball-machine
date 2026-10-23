#include "SoundController.h"
#include "Config.h"

// Sound-event notes and note lengths adapted from the team's buzzerbackground.cpp.
// The original branch's background song needed pitches.h, absent from the ZIP.
// This replacement background riff is original and has no extra file dependency.
static const ToneStep startTune[] = {{523,180},{659,180},{784,180},{1047,450}};
static const ToneStep pointsTune[] = {{1200,100},{1600,100},{2000,170}};
static const ToneStep gainBallTune[] = {{659,150},{784,150},{988,150},{1319,400}};
static const ToneStep loseBallTune[] = {{800,230},{600,230},{400,400}};
static const ToneStep gameOverTune[] = {{784,300},{659,300},{523,300},{392,350},{262,650}};
static const ToneStep bumperTune[] = {{1175,65},{1568,95}};
static const ToneStep background[] = {
  {392,180},{523,180},{659,180},{784,280},{659,160},{523,160},
  {440,200},{587,200},{698,200},{880,300},{698,170},{587,170},
  {523,180},{659,180},{784,220},{988,310},{784,210},{0,180}
};
static const uint8_t backgroundCount = sizeof(background) / sizeof(background[0]);

void SoundController::begin() {
  pinMode(Pins::BUZZER, OUTPUT);
  noTone(Pins::BUZZER);
}

void SoundController::startNote(unsigned long nowMs) {
  noteStartedMs_ = nowMs;
  const ToneStep &step = sequence_[index_];
  if (step.freq == 0) noTone(Pins::BUZZER);
  else tone(Pins::BUZZER, step.freq, (unsigned long)step.durationMs * 9UL / 10UL);
}

void SoundController::play(SoundEvent event, unsigned long nowMs) {
  const ToneStep *next = nullptr;
  uint8_t n = 0, newPriority = 0;
  switch (event) {
    case SoundEvent::Start: next = startTune; n = 4; newPriority = 5; break;
    case SoundEvent::Points: next = pointsTune; n = 3; newPriority = 2; break;
    case SoundEvent::GainBall: next = gainBallTune; n = 4; newPriority = 3; break;
    case SoundEvent::LoseBall: next = loseBallTune; n = 3; newPriority = 4; break;
    case SoundEvent::Bumper: next = bumperTune; n = 2; newPriority = 2; break;
    case SoundEvent::GameOver: next = gameOverTune; n = 5; newPriority = 6; break;
  }
  // Keep important event sounds recognizable instead of cutting them off.
  if (effect_ && playing_ && newPriority <= priority_) return;
  if (!effect_ && playing_) backgroundIndex_ = index_;
  noTone(Pins::BUZZER);
  sequence_ = next;
  count_ = n;
  index_ = 0;
  priority_ = newPriority;
  playing_ = true;
  effect_ = true;
  startNote(nowMs);
}

void SoundController::startBackground(unsigned long nowMs) {
  sequence_ = background;
  count_ = backgroundCount;
  index_ = backgroundIndex_;
  priority_ = 0;
  playing_ = true;
  effect_ = false;
  startNote(nowMs);
}

void SoundController::update(bool gameRunning, unsigned long nowMs) {
  if (playing_) {
    if (nowMs - noteStartedMs_ < sequence_[index_].durationMs) return;
    ++index_;
    if (index_ >= count_) {
      noTone(Pins::BUZZER);
      playing_ = false;
      if (effect_) eventFinishedMs_ = nowMs;
      else backgroundIndex_ = 0;
    } else {
      startNote(nowMs);
    }
  }
  if (!playing_ && gameRunning && nowMs - eventFinishedMs_ >= 1000UL)
    startBackground(nowMs);
}

void SoundController::stop() {
  noTone(Pins::BUZZER);
  sequence_ = nullptr;
  playing_ = false;
  effect_ = false;
  priority_ = 0;
  backgroundIndex_ = 0;
  eventFinishedMs_ = millis();
}
