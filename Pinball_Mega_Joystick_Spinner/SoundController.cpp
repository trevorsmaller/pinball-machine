#include "SoundController.h"
#include "Config.h"

// Background melody migrated from your team's Trevor_sound_system/
// buzzerbackground.cpp (the Star Wars melody you supplied).
// Notes are stored directly as Hz to avoid depending on a missing pitches.h.
// Durations retain the original note divisions and musicSpeed = 1.2f,
// including the original 1.3x spacing between background notes.
// Gameplay sound-effect notes below are unchanged.
static const ToneStep startTune[] = {{523,180},{659,180},{784,180},{1047,450}};
static const ToneStep pointsTune[] = {{1200,100},{1600,100},{2000,170}};
static const ToneStep gainBallTune[] = {{659,150},{784,150},{988,150},{1319,400}};
static const ToneStep loseBallTune[] = {{800,230},{600,230},{400,400}};
static const ToneStep gameOverTune[] = {{784,300},{659,300},{523,300},{392,350},{262,650}};
static const ToneStep bumperTune[] = {{1175,65},{1568,95}};

// Background melody in original sequence order. A frequency of zero means rest.
static const ToneStep background[] = {
  {466, 195}, {466, 195}, {466, 195}, {698, 780}, {1047, 780}, {932, 195},
  {880, 195}, {784, 195}, {1397, 780}, {1047, 390}, {932, 195}, {880, 195},
  {784, 195}, {1397, 780}, {1047, 390}, {932, 195}, {880, 195}, {932, 195},
  {784, 780}, {523, 195}, {523, 195}, {523, 195}, {698, 780}, {1047, 780},
  {932, 195}, {880, 195}, {784, 195}, {1397, 780}, {1047, 390}, {932, 195},
  {880, 195}, {784, 195}, {1397, 780}, {1047, 390}, {932, 195}, {880, 195},
  {932, 195}, {784, 780}, {523, 195}, {523, 98}, {587, 390}, {587, 195},
  {932, 195}, {880, 195}, {784, 195}, {698, 195}, {698, 195}, {784, 195},
  {880, 195}, {784, 390}, {587, 195}, {659, 390}, {523, 195}, {523, 98},
  {587, 390}, {587, 195}, {932, 195}, {880, 195}, {784, 195}, {698, 195},
  {1047, 195}, {784, 98}, {784, 780}, {0, 195}, {523, 195}, {587, 390},
  {587, 195}, {932, 195}, {880, 195}, {784, 195}, {698, 195}, {698, 195},
  {784, 195}, {880, 195}, {784, 390}, {587, 195}, {659, 390}, {1047, 195},
  {1047, 98}, {1397, 390}, {1245, 195}, {1109, 390}, {1047, 195}, {932, 390},
  {831, 195}, {784, 390}, {698, 195}, {1047, 1560}
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
  else {
    // Event effects use 90% of each note; background uses the same
    // note/rest spacing as the supplied buzzerbackground.cpp (1 / 1.3).
    const unsigned long soundingMs = effect_
      ? (unsigned long)step.durationMs * 9UL / 10UL
      : (unsigned long)step.durationMs * 10UL / 13UL;
    tone(Pins::BUZZER, step.freq, soundingMs);
  }
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
