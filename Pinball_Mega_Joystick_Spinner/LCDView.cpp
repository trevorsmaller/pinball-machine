
#include "LCDView.h"
#include "Config.h"
#include <stdio.h>
#include <string.h>

// LCD backlight control pin
#define LCD_BACKLIGHT_PIN 6

LCDView::LCDView()
    : lcd_(Pins::LCD_RS, Pins::LCD_EN, Pins::LCD_D4,
           Pins::LCD_D5, Pins::LCD_D6, Pins::LCD_D7) {}

// Initialize LCD and set brightness
void LCDView::begin() {
  lcd_.begin(16, 2);

  // Set backlight to maximum brightness
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  analogWrite(LCD_BACKLIGHT_PIN, 255);

  first_ = true;
}

// Print a message on a specific LCD row
void LCDView::printLine(uint8_t row, const char *message) {
  lcd_.setCursor(0, row);

  bool padding = false;

  for (uint8_t i = 0; i < 16; ++i) {
    char c = padding ? '\0' : message[i];

    if (c == '\0') {
      padding = true;
      lcd_.write(' ');
    } else {
      lcd_.write(c);
    }
  }
}

// Update LCD with game information
void LCDView::update(DisplayState state, uint32_t score,
                     uint32_t high, uint8_t lost,
                     unsigned long nowMs) {

  if (!first_ &&
      nowMs - lastDrawMs_ < Settings::LCD_REFRESH_MS)
    return;

  first_ = false;
  lastDrawMs_ = nowMs;

  char firstLine[32];
  char secondLine[32];

  switch (state) {

    case DisplayState::Waiting:
      snprintf(firstLine, sizeof(firstLine),
               "PRESS START");
      snprintf(secondLine, sizeof(secondLine),
               "High:%lu", (unsigned long)high);
      break;

    case DisplayState::Running:
      snprintf(firstLine, sizeof(firstLine),
               "Score:%lu", (unsigned long)score);

      snprintf(secondLine, sizeof(secondLine),
               "High:%lu L:%u",
               (unsigned long)high, lost);

      if (strlen(secondLine) > 16) {
        snprintf(secondLine, sizeof(secondLine),
                 "High:%lu", (unsigned long)high);
      }
      break;

    case DisplayState::GameOver:
      snprintf(firstLine, sizeof(firstLine),
               "END Score:%lu", (unsigned long)score);

      snprintf(secondLine, sizeof(secondLine),
               "High:%lu", (unsigned long)high);
      break;
  }

  printLine(0, firstLine);
  printLine(1, secondLine);
}
