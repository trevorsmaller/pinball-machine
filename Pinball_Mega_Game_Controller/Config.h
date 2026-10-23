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
  constexpr uint8_t BUMPER_SENSOR = 34;        // microswitch to GND
  constexpr uint8_t START_BUTTON = 35;         // button to GND
  constexpr uint8_t UNO_GAME_ENABLE = 36;      // LOW = Uno game ON; floating = OFF
  constexpr uint8_t BUMPER_DRIVER = 37;        // driver logic input, NOT the coil
  constexpr uint8_t LED_SCORE = 40;
  constexpr uint8_t LED_LOSS = 41;
  constexpr uint8_t LED_STATUS = 42;
  constexpr uint8_t LCD_RS = 22;
  constexpr uint8_t LCD_EN = 23;
  constexpr uint8_t LCD_D4 = 24;
  constexpr uint8_t LCD_D5 = 25;
  constexpr uint8_t LCD_D6 = 26;
  constexpr uint8_t LCD_D7 = 27;
}
namespace Settings {
  constexpr uint8_t MAX_LOSSES = 5;
  constexpr uint16_t SCORE1_POINTS = 100;
  constexpr uint16_t SCORE2_POINTS = 200;
  constexpr uint16_t SCORE3_POINTS = 300;
  constexpr uint16_t BUMPER_POINTS = 500;

  // Option B: time from game start, then every 15s; first ball at 15s.
  constexpr unsigned long RELEASE_INTERVAL_MS = 15000UL;
  constexpr unsigned long RELEASE_OPEN_MS = 400UL; // MECHANICAL TEST VALUE
  constexpr uint8_t RELEASE_OPEN_ANGLE = 90;
  constexpr uint8_t RELEASE_CLOSED_ANGLE = 0;
  constexpr uint8_t TUNNEL_OPEN_ANGLE = 0;
  constexpr uint8_t TUNNEL_CLOSED_ANGLE = 90;

  constexpr unsigned long SENSOR_STABLE_MS = 25UL;
  constexpr unsigned long SENSOR_MIN_GAP_MS = 250UL;
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
