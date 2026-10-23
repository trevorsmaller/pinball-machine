#include "LCDView.h"
#include "Config.h"
#include <stdio.h>

LCDView::LCDView()
    : lcd_(Pins::LCD_RS, Pins::LCD_EN, Pins::LCD_D4,
           Pins::LCD_D5, Pins::LCD_D6, Pins::LCD_D7) {}
void LCDView::begin() { lcd_.begin(16, 2); first_ = true; }

void LCDView::printLine(uint8_t row, const char *message) {
  lcd_.setCursor(0, row);
  for (uint8_t i = 0; i < 16; ++i)
    lcd_.write(message[i] ? message[i] : ' ');
}

void LCDView::update(DisplayState state, uint32_t score, uint32_t high,
                     uint8_t lost, unsigned long nowMs) {
  if (!first_ && nowMs - lastDrawMs_ < Settings::LCD_REFRESH_MS) return;
  first_ = false;
  lastDrawMs_ = nowMs;
  char firstLine[32], secondLine[32];
  switch (state) {
    case DisplayState::Waiting:
      snprintf(firstLine, sizeof(firstLine), "PRESS START");
      snprintf(secondLine, sizeof(secondLine), "High:%lu", (unsigned long)high);
      break;
    case DisplayState::Running:
      snprintf(firstLine, sizeof(firstLine), "Score:%lu", (unsigned long)score);
      snprintf(secondLine, sizeof(secondLine), "High:%lu L:%u", (unsigned long)high, lost);
      // If high score is long, prioritize high score over the ball count.
      if (strlen(secondLine) > 16)
        snprintf(secondLine, sizeof(secondLine), "High:%lu", (unsigned long)high);
      break;
    case DisplayState::GameOver:
      snprintf(firstLine, sizeof(firstLine), "END Score:%lu", (unsigned long)score);
      snprintf(secondLine, sizeof(secondLine), "High:%lu", (unsigned long)high);
      break;
  }
  printLine(0, firstLine);
  printLine(1, secondLine);
}
