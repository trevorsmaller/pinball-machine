#pragma once
#include <Arduino.h>
#include <LiquidCrystal.h>

enum class DisplayState : uint8_t { Waiting, Running, GameOver };
class LCDView {
public:
  LCDView();
  void begin();
  void update(DisplayState state, uint32_t score, uint32_t high, uint8_t lost,
              unsigned long nowMs);
private:
  LiquidCrystal lcd_;
  unsigned long lastDrawMs_ = 0;
  bool first_ = true;
  void printLine(uint8_t row, const char *message);
};
