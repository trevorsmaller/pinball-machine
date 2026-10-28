/*
 * Arduino Mega 2560 - Pinball gameplay controller
 *
 * Based on the team's predev losing/scoring/play logic, servos branch and
 * Trevor_sound_system event tones. Original integration and improvements:
 * sensor rearm, event-safe millis timing, EEPROM records, coordinated Uno signal,
 * 3 scoring holes, analog-joystick bumper, switch spinner, LCD, RGB and sound.
 *
 * GAME_ENABLE to Uno is LOW when game running (open-drain-style output),
 * else Hi-Z so Uno INPUT_PULLUP reads HIGH. Share logic GND between boards.
 */
#include <Arduino.h>
#include "Config.h"
#include "SensorInputs.h"
#include "ServoGates.h"
#include "SoundController.h"
#include "LightEffects.h"
#include "LCDView.h"
#include "HighScoreStore.h"
#include "PopBumper.h"
#include "JoystickHitSensor.h"

EventSensor loseSensor(Pins::LOSS, Settings::SENSOR_STABLE_MS, Settings::SENSOR_MIN_GAP_MS);
EventSensor scoreSensor1(Pins::SCORE_1, Settings::SENSOR_STABLE_MS, Settings::SENSOR_MIN_GAP_MS);
EventSensor scoreSensor2(Pins::SCORE_2, Settings::SENSOR_STABLE_MS, Settings::SENSOR_MIN_GAP_MS);
EventSensor scoreSensor3(Pins::SCORE_3, Settings::SENSOR_STABLE_MS, Settings::SENSOR_MIN_GAP_MS);
JoystickHitSensor joystickBumper;
EventSensor startButton(Pins::START_BUTTON, Settings::SENSOR_STABLE_MS, 300UL);
ServoGates gates;
SoundController sound;
LightEffects lights;
LCDView display;
HighScoreStore storedHighScore;
PopBumper bumper;

// D2 external interrupt records short spinner switch closures.
// A mechanical switch pulse is accepted once after a short bounce interval.
volatile uint32_t spinnerPulseCount = 0;
volatile unsigned long spinnerLastPulseUs = 0;
uint32_t spinnerLastProcessed = 0;

void spinnerPulseISR() {
  unsigned long nowUs = micros();
  if (nowUs - spinnerLastPulseUs >= Settings::SPINNER_DEBOUNCE_US) {
    spinnerLastPulseUs = nowUs;
    ++spinnerPulseCount;
  }
}

uint32_t snapshotSpinnerCount() {
  noInterrupts();
  const uint32_t n = spinnerPulseCount;
  interrupts();
  return n;
}


DisplayState state = DisplayState::Waiting;
uint32_t currentScore = 0;
uint8_t lostBalls = 0;
unsigned long startHoldBeganMs = 0;
bool resetHoldUsed = false;

void setUnoGameEnabled(bool enabled) {
  // Important: NOT OUTPUT HIGH. Setting INPUT releases Uno's pull-up.
  if (enabled) {
    digitalWrite(Pins::UNO_GAME_ENABLE, LOW);
    pinMode(Pins::UNO_GAME_ENABLE, OUTPUT);
  } else {
    pinMode(Pins::UNO_GAME_ENABLE, INPUT);
    digitalWrite(Pins::UNO_GAME_ENABLE, LOW); // leave pull-up disabled on Mega
  }
}

uint32_t shownHighScore() {
  uint32_t saved = storedHighScore.value();
  return currentScore > saved ? currentScore : saved;
}

void startNewGame(unsigned long nowMs) {
  setUnoGameEnabled(false);
  bumper.stop();
  spinnerLastProcessed = snapshotSpinnerCount(); // no old spins in a new game
  currentScore = 0;
  lostBalls = 0;
  state = DisplayState::Running;
  gates.startGame(nowMs);
  setUnoGameEnabled(true);
  sound.stop();
  sound.play(SoundEvent::Start, nowMs);
  lights.trigger(LightEvent::Start, nowMs);
  resetHoldUsed = false;
  startHoldBeganMs = nowMs;
  Serial.println(F("GAME START - first ball release after 15 seconds"));
}

void finishGame(unsigned long nowMs) {
  if (state != DisplayState::Running) return;
  state = DisplayState::GameOver;
  setUnoGameEnabled(false);
  gates.stopGame();
  bumper.stop();
  bool newRecord = currentScore > storedHighScore.value();
  storedHighScore.saveIfHigher(currentScore);
  sound.stop();
  sound.play(SoundEvent::GameOver, nowMs);
  lights.trigger(newRecord ? LightEvent::NewRecord : LightEvent::GameOver, nowMs);
  Serial.print(F("GAME OVER; score=")); Serial.print(currentScore);
  Serial.print(F(" high=")); Serial.println(storedHighScore.value());
}

void addPoints(uint16_t points, bool isBumper, unsigned long nowMs) {
  if (state != DisplayState::Running) return;
  // Saturate at uint32_t max rather than wrap to zero.
  if (UINT32_MAX - currentScore < points) currentScore = UINT32_MAX;
  else currentScore += points;
  sound.play(isBumper ? SoundEvent::Bumper : SoundEvent::Points, nowMs);
  lights.trigger(isBumper ? LightEvent::Bumper : LightEvent::Score, nowMs);
  Serial.print(F("+")); Serial.print(points);
  Serial.print(F(" Score=")); Serial.println(currentScore);
}

void processSpinner(unsigned long nowMs) {
  const uint32_t currentCount = snapshotSpinnerCount();
  uint32_t pending = currentCount - spinnerLastProcessed;
  spinnerLastProcessed = currentCount;
  if (state != DisplayState::Running) return; // discard spins between games

  // Combine pulses into safe chunks for the existing uint16_t addPoints API.
  // Every accepted pulse still gives SPINNER_POINTS.
  const uint16_t maxChunk = 65535U / Settings::SPINNER_POINTS;
  while (pending > 0) {
    const uint16_t count = pending > maxChunk ? maxChunk : (uint16_t)pending;
    addPoints(count * Settings::SPINNER_POINTS, false, nowMs);
    pending -= count;
  }
}

void lostBall(unsigned long nowMs) {
  if (state != DisplayState::Running) return;
  if (lostBalls < Settings::MAX_LOSSES) ++lostBalls;
  Serial.print(F("Ball lost: ")); Serial.println(lostBalls);
  if (lostBalls >= Settings::MAX_LOSSES) {
    finishGame(nowMs);  // close BOTH servos before any other work
  } else {
    sound.play(SoundEvent::LoseBall, nowMs);
    lights.trigger(LightEvent::Lost, nowMs);
  }
}

void hitBumper(unsigned long nowMs) {
  if (state == DisplayState::Running && bumper.trigger(nowMs))
    addPoints(Settings::BUMPER_POINTS, true, nowMs);
}

void processStartButton(unsigned long nowMs, bool justPressed) {
  if (justPressed && state != DisplayState::Running) {
    startNewGame(nowMs);
    return;
  }
  // Retain the predev team's 3-second hold-to-end functionality.
  if (state == DisplayState::Running) {
    if (!startButton.active()) {
      startHoldBeganMs = nowMs;
      resetHoldUsed = false;
    } else if (!resetHoldUsed && nowMs - startHoldBeganMs >= Settings::LONG_PRESS_TO_END_MS) {
      resetHoldUsed = true;
      finishGame(nowMs);
    }
  }
}

void serialTest(unsigned long nowMs) {
  // T cycles through strip event animations independently of sensors/game.
  static uint8_t nextLightTest = 0;
  if (!Settings::ALLOW_SERIAL_TEST_COMMANDS) return;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'N' || c == 'n') { if (state != DisplayState::Running) startNewGame(nowMs); }
    else if (c == 'X' || c == 'x') finishGame(nowMs);
    else if (c == 'L' || c == 'l') lostBall(nowMs);
    else if (c == '1') addPoints(Settings::SCORE1_POINTS, false, nowMs);
    else if (c == '2') addPoints(Settings::SCORE2_POINTS, false, nowMs);
    else if (c == '3') addPoints(Settings::SCORE3_POINTS, false, nowMs);
    else if (c == 'B' || c == 'b') hitBumper(nowMs);
    else if (c == 'S' || c == 's') addPoints(Settings::SPINNER_POINTS, false, nowMs);
    else if (c == 'T' || c == 't') {
      lights.trigger(static_cast<LightEvent>(nextLightTest), nowMs);
      Serial.print(F("LED effect test #")); Serial.println(nextLightTest + 1);
      nextLightTest = (nextLightTest + 1) % 7;
    }
  }
}

void setup() {
  // Safe default: Uno disabled and bumper driver off before other setup.
  setUnoGameEnabled(false);
  bumper.begin();
  Serial.begin(9600);
  loseSensor.begin();
  scoreSensor1.begin();
  scoreSensor2.begin();
  scoreSensor3.begin();
  joystickBumper.begin();
  pinMode(Pins::SPINNER_SWITCH, INPUT_PULLUP);
  spinnerLastProcessed = snapshotSpinnerCount();
  attachInterrupt(digitalPinToInterrupt(Pins::SPINNER_SWITCH), spinnerPulseISR, FALLING);
  startButton.begin();
  gates.begin();
  sound.begin();
  lights.begin();
  display.begin();
  storedHighScore.begin();
  // AUTO-START: begin a new game immediately after all hardware is initialized.
  // First timed ball release occurs 15 seconds after this call.
  // With no start button installed, press the Mega RESET button to play again
  // after game over (saved high score is retained in EEPROM).
  startNewGame(millis());
  Serial.println(F("Mega ready - game started automatically."));
  Serial.println(F("Test: 1/2/3=holes, B=bumper, S=spinner, L=loss, X=end, N=restart, T=LED test."));
}

void loop() {
  const unsigned long nowMs = millis();

  // Read EVERY sensor continuously, including in idle, to track release/rearm.
  bool startPressed = startButton.poll(nowMs);
  bool loss = loseSensor.poll(nowMs);
  bool score1 = scoreSensor1.poll(nowMs);
  bool score2 = scoreSensor2.poll(nowMs);
  bool score3 = scoreSensor3.poll(nowMs);
  bool pop = joystickBumper.poll(nowMs);

  processStartButton(nowMs, startPressed);

  if (state == DisplayState::Running) {
    // Process loss before scoring/release: game-over takes precedence.
    if (loss) lostBall(nowMs);
    if (state == DisplayState::Running) {
      if (score1) addPoints(Settings::SCORE1_POINTS, false, nowMs);
      if (score2) addPoints(Settings::SCORE2_POINTS, false, nowMs);
      if (score3) addPoints(Settings::SCORE3_POINTS, false, nowMs);
      if (pop) hitBumper(nowMs);
      if (gates.update(nowMs)) {
        sound.play(SoundEvent::GainBall, nowMs);
        lights.trigger(LightEvent::Release, nowMs);
        Serial.println(F("Ball release gate opened"));
      }
    }
  }

  processSpinner(nowMs);
  bumper.update(nowMs);
  serialTest(nowMs);
  bool running = state == DisplayState::Running;
  sound.update(running, nowMs);
  lights.update(running, nowMs);
  display.update(state, currentScore, shownHighScore(), lostBalls, nowMs);
}
