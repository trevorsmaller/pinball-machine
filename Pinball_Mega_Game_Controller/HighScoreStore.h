#pragma once
#include <Arduino.h>
class HighScoreStore {
public:
  void begin();
  uint32_t value() const { return high_; }
  void saveIfHigher(uint32_t score);
private:
  uint32_t high_ = 0;
};
