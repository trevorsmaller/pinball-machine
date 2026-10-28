#pragma once
#include <Arduino.h>

// Arduino Mega 2560: unique pin assignments for ALL Mega components.
namespace Pins {
  constexpr uint8_t RELEASE_SERVO = 9;
  constexpr uint8_t TUNNEL_SERVO = 10;
  constexpr uint8_t BUZZER = 8;                 // passive piezo buzzer
  constexpr uint8_t SCORE_1 = 30;              // active-LOW beam break
  constexpr uint8_t SCORE_2 = 31;
  constexpr uint8_t SCORE_3 = 32;
  constexpr uint8_t LOSS = 33;
  constexpr uint8_t JOYSTICK_X = A0;         // analog VRx
  constexpr uint8_t JOYSTICK_Y = A1;         // analog VRy
  constexpr uint8_t SPINNER_SWITCH = 2;       // COM+NO switch to GND
  // Digital pin 34, previously BUMPER_SENSOR, is now unused.
  constexpr uint8_t START_BUTTON = 35;         // button to GND
  constexpr uint8_t UNO_GAME_ENABLE = 36;      // LOW = Uno game ON; floating = OFF
  constexpr uint8_t BUMPER_DRIVER = 37;        // driver logic input, NOT the coil
  // Two connected RGB strips: choose strip type below.
  // 3-wire addressable: one combined data line on D40.
  constexpr uint8_t LED_DATA = 40;
  // 4-wire non-addressable RGB: three PWM signals to external MOSFET drivers.
  constexpr uint8_t LED_RED_PWM = 44;
  constexpr uint8_t LED_GREEN_PWM = 45;
  constexpr uint8_t LED_BLUE_PWM = 46;
  constexpr uint8_t LCD_RS = 22;
  constexpr uint8_t LCD_EN = 23;
  constexpr uint8_t LCD_D4 = 24;
  constexpr uint8_t LCD_D5 = 25;
  constexpr uint8_t LCD_D6 = 26;
  constexpr uint8_t LCD_D7 = 27;
}
// 1 = WS2812B / NeoPixel / compatible 3-wire addressable LEDs (default)
// 2 = common-positive 4-wire analog RGB strip, with EXTERNAL MOSFET drivers
// If you have 4 wires, change 1 to 2. Other strip types need different code.
#ifndef PINBALL_LED_STRIP_TYPE
#define PINBALL_LED_STRIP_TYPE 1
#endif
#if PINBALL_LED_STRIP_TYPE != 1 && PINBALL_LED_STRIP_TYPE != 2
#error "PINBALL_LED_STRIP_TYPE must be 1 or 2"
#endif

namespace Settings {
  // Count LEDs on BOTH addressable strips (ignored for analog RGB strips).
  // Example: 30 LEDs on strip 1 + 30 LEDs on strip 2 = 60.
  constexpr uint16_t LED_STRIP_1_PIXELS = 30;
  constexpr uint16_t LED_STRIP_2_PIXELS = 30;
  constexpr uint16_t LED_TOTAL_PIXELS = LED_STRIP_1_PIXELS + LED_STRIP_2_PIXELS;
  constexpr uint8_t LED_BRIGHTNESS = 65; // 0-255, keep low during testing
  constexpr unsigned long LED_FRAME_INTERVAL_MS = 50UL; // about 20 FPS

  constexpr uint8_t MAX_LOSSES = 5;
  constexpr uint16_t SCORE1_POINTS = 100;
  constexpr uint16_t SCORE2_POINTS = 200;
  constexpr uint16_t SCORE3_POINTS = 300;
  constexpr uint16_t BUMPER_POINTS = 500;
  constexpr uint16_t SPINNER_POINTS = 100;

  // Option B: time from game start, then every 15s; first ball at 15s.
  constexpr unsigned long RELEASE_INTERVAL_MS = 15000UL;
  constexpr unsigned long RELEASE_OPEN_MS = 400UL; // MECHANICAL TEST VALUE
  constexpr uint8_t RELEASE_OPEN_ANGLE = 90;
  constexpr uint8_t RELEASE_CLOSED_ANGLE = 0;
  constexpr uint8_t TUNNEL_OPEN_ANGLE = 0;
  constexpr uint8_t TUNNEL_CLOSED_ANGLE = 90;


constexpr unsigned long SENSOR_STABLE_MS = 1UL;
constexpr unsigned long SENSOR_MIN_GAP_MS = 100UL;

  // Joystick: center is measured during startup with stick released.
  constexpr int JOYSTICK_TRIGGER_DELTA = 170; // counts from measured center
  constexpr int JOYSTICK_RELEASE_DELTA = 90;  // hysteresis: must recenter
  constexpr unsigned long JOYSTICK_HIT_STABLE_MS = 5UL;
  constexpr unsigned long JOYSTICK_RECENTER_MS = 30UL;
  // Mechanical spinner contacts bounce; tune while observing Serial Monitor.
  constexpr unsigned long SPINNER_DEBOUNCE_US = 3000UL;
  constexpr unsigned long BUMPER_COOLDOWN_MS = 500UL;
  constexpr unsigned long BUMPER_PULSE_MS = 20UL; // PLACEHOLDER, NOT COIL-RATED
  // Leave false until external coil driver, protective components, supply,
  // and safe pulse limits have been verified. Scores/sounds still simulate.
  constexpr bool ARM_BUMPER_OUTPUT = false;

  constexpr unsigned long LONG_PRESS_TO_END_MS = 3000UL;
  constexpr unsigned long LCD_REFRESH_MS = 200UL;
  // Serial Monitor test commands (not needed in production)
  constexpr bool ALLOW_SERIAL_TEST_COMMANDS = true;
}
