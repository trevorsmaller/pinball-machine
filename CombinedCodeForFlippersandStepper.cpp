/*
  Pinball machine - Arduino Uno #1
  Two button-operated flipper driver outputs + worm elevator stepper motor.

  Combines/corrects the flipper logic from the Trevor_Flipper branch.
  Motor driver ASSUMPTION: STEP/DIR/ENABLE interface (A4988/DRV8825/etc).
  Flipper driver ASSUMPTION: active-HIGH, low-voltage logic MOSFET drivers.

  IMPORTANT: Arduino pins must NEVER power the solenoids or stepper directly.
  Confirm the coil/driver electrical ratings, current limiting, and protection
  before connecting the real hardware. FLIPPER_PULSE_MS is a placeholder,
  NOT a verified safe pulse duration for your particular coils.
*/

#include <Arduino.h>

// ------------------------- PIN ASSIGNMENTS -------------------------
constexpr uint8_t LEFT_BUTTON_PIN   = 2;   // button between pin and GND
constexpr uint8_t RIGHT_BUTTON_PIN  = 3;   // button between pin and GND
constexpr uint8_t LEFT_FLIPPER_PIN  = 5;   // MOSFET driver logic input
constexpr uint8_t RIGHT_FLIPPER_PIN = 6;   // MOSFET driver logic input

constexpr uint8_t GAME_ENABLE_PIN = 7;     // optional: LOW = game active
constexpr uint8_t STEPPER_STEP_PIN = 8;
constexpr uint8_t STEPPER_DIR_PIN = 9;
constexpr uint8_t STEPPER_ENABLE_PIN = 10; // STEP/DIR driver active-LOW enable

// -------------------------- CONFIGURATION --------------------------
constexpr bool USE_GAME_ENABLE_INPUT = false; // set true once wired to Uno #2
constexpr bool ELEVATOR_AUTO_START = true;     // continuous elevator by default
constexpr bool ELEVATOR_DIRECTION = HIGH;      // flip to LOW if backwards

// Adjust ONLY after confirming the actual solenoid type and its ratings.
// Software timing alone is not sufficient protection for a solenoid coil.
constexpr unsigned long FLIPPER_PULSE_MS = 50;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 12;

// Pulse timing for a STEP/DIR driver: steps/sec = 1,000,000 / interval.
// 5000 us => 200 microsteps/full steps per second, depending on driver mode.
constexpr unsigned long STEP_INTERVAL_US = 5000;
constexpr unsigned long STEP_HIGH_US = 5;

// --------------------------- FLIPPERS ------------------------------
struct Flipper {
  uint8_t buttonPin;
  uint8_t driverPin;
  bool rawPressed;
  bool stablePressed;
  bool coilOn;
  bool firedThisPress;
  unsigned long rawChangeMs;
  unsigned long pulseStartedMs;
};

Flipper leftFlipper  = {LEFT_BUTTON_PIN, LEFT_FLIPPER_PIN, false, false, false, false, 0, 0};
Flipper rightFlipper = {RIGHT_BUTTON_PIN, RIGHT_FLIPPER_PIN, false, false, false, false, 0, 0};

void switchFlipperOff(Flipper &flipper) {
  digitalWrite(flipper.driverPin, LOW);
  flipper.coilOn = false;
}

void initFlipper(Flipper &flipper) {
  pinMode(flipper.driverPin, OUTPUT);
  switchFlipperOff(flipper);
  pinMode(flipper.buttonPin, INPUT_PULLUP);
  const bool initiallyPressed = digitalRead(flipper.buttonPin) == LOW;
  flipper.rawPressed = initiallyPressed;
  flipper.stablePressed = initiallyPressed;
  // Do not fire at boot if the button was already held down.
  flipper.firedThisPress = initiallyPressed;
  flipper.rawChangeMs = millis();
}

void updateFlipper(Flipper &flipper, unsigned long nowMs, bool gameEnabled) {
  // Physical wiring: pin -> button -> GND. Pressed = LOW.
  const bool readingPressed = digitalRead(flipper.buttonPin) == LOW;

  if (readingPressed != flipper.rawPressed) {
    flipper.rawPressed = readingPressed;
    flipper.rawChangeMs = nowMs;
  }

  if ((nowMs - flipper.rawChangeMs >= BUTTON_DEBOUNCE_MS) &&
      flipper.stablePressed != flipper.rawPressed) {
    flipper.stablePressed = flipper.rawPressed;

    if (!flipper.stablePressed) {
      // Release re-arms the flipper for the next press.
      switchFlipperOff(flipper);
      flipper.firedThisPress = false;
    } else if (gameEnabled && !flipper.firedThisPress) {
      // New press: fire the coil through the external driver.
      digitalWrite(flipper.driverPin, HIGH);
      flipper.coilOn = true;
      flipper.firedThisPress = true;
      flipper.pulseStartedMs = nowMs;
    }
  }

  if (!gameEnabled) {
    // A button held during game-over must be released before reuse.
    switchFlipperOff(flipper);
    if (flipper.stablePressed || flipper.rawPressed) {
      flipper.firedThisPress = true;
    }
  }

  if (flipper.coilOn &&
      nowMs - flipper.pulseStartedMs >= FLIPPER_PULSE_MS) {
    // One-shot timeout: NO retrigger until the button is released.
    switchFlipperOff(flipper);
  }
}

// ---------------------------- ELEVATOR -----------------------------
bool elevatorRequested = ELEVATOR_AUTO_START;
bool elevatorDriverEnabled = false;
bool stepPinHigh = false;
unsigned long lastStepRiseUs = 0;
unsigned long stepRiseUs = 0;

void initElevator() {
  // Set safe inactive levels before enabling outputs.
  digitalWrite(STEPPER_STEP_PIN, LOW);
  digitalWrite(STEPPER_ENABLE_PIN, HIGH); // disable active-LOW driver
  pinMode(STEPPER_STEP_PIN, OUTPUT);
  pinMode(STEPPER_DIR_PIN, OUTPUT);
  pinMode(STEPPER_ENABLE_PIN, OUTPUT);
  digitalWrite(STEPPER_DIR_PIN, ELEVATOR_DIRECTION);
  lastStepRiseUs = micros();
}

void updateElevator(unsigned long nowUs, bool shouldRun) {
  if (!shouldRun) {
    digitalWrite(STEPPER_STEP_PIN, LOW);
    digitalWrite(STEPPER_ENABLE_PIN, HIGH);
    stepPinHigh = false;
    elevatorDriverEnabled = false;
    lastStepRiseUs = nowUs;
    return;
  }

  if (!elevatorDriverEnabled) {
    digitalWrite(STEPPER_DIR_PIN, ELEVATOR_DIRECTION);
    digitalWrite(STEPPER_STEP_PIN, LOW);
    digitalWrite(STEPPER_ENABLE_PIN, LOW); // enable active-LOW driver
    elevatorDriverEnabled = true;
    stepPinHigh = false;
    lastStepRiseUs = nowUs;
    return;
  }

  // End each STEP pulse after at least STEP_HIGH_US microseconds.
  if (stepPinHigh) {
    if (nowUs - stepRiseUs >= STEP_HIGH_US) {
      digitalWrite(STEPPER_STEP_PIN, LOW);
      stepPinHigh = false;
    }
  } else if (nowUs - lastStepRiseUs >= STEP_INTERVAL_US) {
    digitalWrite(STEPPER_STEP_PIN, HIGH);
    stepRiseUs = nowUs;
    lastStepRiseUs = nowUs;
    stepPinHigh = true;
  }
}

// ------------------------- GAME / TEST INPUT -----------------------
bool gameIsEnabled() {
  return !USE_GAME_ENABLE_INPUT || digitalRead(GAME_ENABLE_PIN) == LOW;
}

void readSerialCommands() {
  while (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'e' || cmd == 'E') {
      elevatorRequested = true;
      Serial.println(F("Elevator requested ON"));
    } else if (cmd == 's' || cmd == 'S') {
      elevatorRequested = false;
      Serial.println(F("Elevator requested OFF"));
    }
  }
}

void setup() {
  Serial.begin(9600);
  if (USE_GAME_ENABLE_INPUT) {
    pinMode(GAME_ENABLE_PIN, INPUT_PULLUP);
  }
  initFlipper(leftFlipper);
  initFlipper(rightFlipper);
  initElevator();

  Serial.println(F("Uno #1 ready: flippers + elevator"));
  Serial.println(F("Serial: E = elevator ON, S = elevator OFF"));
}

void loop() {
  const unsigned long nowMs = millis();
  const unsigned long nowUs = micros();
  const bool gameEnabled = gameIsEnabled();

  // Prioritize flipper checks; never wait or delay for the motor.
  updateFlipper(leftFlipper, nowMs, gameEnabled);
  updateFlipper(rightFlipper, nowMs, gameEnabled);

  updateElevator(nowUs, gameEnabled && elevatorRequested);
  readSerialCommands();
}
