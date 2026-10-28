#include "HighScoreStore.h"
#include <EEPROM.h>
struct Record { uint32_t magic, high, inverse; };
static const uint32_t SCORE_MAGIC = 0x50424C32UL;
void HighScoreStore::begin() {
  Record rec;
  EEPROM.get(0, rec);
  if (rec.magic == SCORE_MAGIC && rec.inverse == ~rec.high) high_ = rec.high;
  else high_ = 0;
}
void HighScoreStore::saveIfHigher(uint32_t score) {
  if (score <= high_) return;
  high_ = score;
  const Record rec = {SCORE_MAGIC, high_, ~high_};
  EEPROM.put(0, rec);   // only when new high score at game end
}
