#include "LightEffects.h"
#include "Config.h"

#if PINBALL_LED_STRIP_TYPE == 1
#include <Adafruit_NeoPixel.h>
// Change NEO_GRB to NEO_RGB if colors look swapped. Common WS2812B is GRB.
static Adafruit_NeoPixel strip(Settings::LED_TOTAL_PIXELS, Pins::LED_DATA,
                              NEO_GRB + NEO_KHZ800);
#endif

struct RGB { uint8_t r, g, b; };

static RGB wheel(uint8_t pos) {
  // Bright rainbow used when the player sets a new high score.
  pos = 255 - pos;
  if (pos < 85) return {uint8_t(255 - pos * 3), 0, uint8_t(pos * 3)};
  if (pos < 170) {
    pos -= 85;
    return {0, uint8_t(pos * 3), uint8_t(255 - pos * 3)};
  }
  pos -= 170;
  return {uint8_t(pos * 3), uint8_t(255 - pos * 3), 0};
}

void LightEffects::begin() {
#if PINBALL_LED_STRIP_TYPE == 1
  strip.begin();
  strip.setBrightness(Settings::LED_BRIGHTNESS);
  strip.clear();
  strip.show();
#else
  // Common-positive 4-wire strips: Mega PWM drives driver INPUTS, not strip loads.
  pinMode(Pins::LED_RED_PWM, OUTPUT);
  pinMode(Pins::LED_GREEN_PWM, OUTPUT);
  pinMode(Pins::LED_BLUE_PWM, OUTPUT);
  analogWrite(Pins::LED_RED_PWM, 0);
  analogWrite(Pins::LED_GREEN_PWM, 0);
  analogWrite(Pins::LED_BLUE_PWM, 0);
#endif
}

void LightEffects::trigger(LightEvent event, unsigned long nowMs) {
  event_ = event;
  startedMs_ = nowMs;
  // Keep important old event durations. Events are non-blocking.
  switch (event) {
    case LightEvent::Start:     durationMs_ = 1200UL; break;
    case LightEvent::Score:     durationMs_ = 450UL; break;
    case LightEvent::Bumper:    durationMs_ = 500UL; break;
    case LightEvent::Lost:      durationMs_ = 900UL; break;
    case LightEvent::Release:   durationMs_ = 350UL; break;
    case LightEvent::GameOver:  durationMs_ = 2400UL; break;
    case LightEvent::NewRecord: durationMs_ = 3000UL; break;
  }
}

void LightEffects::update(bool running, unsigned long nowMs) {
  if (nowMs - lastFrameMs_ < Settings::LED_FRAME_INTERVAL_MS) return;
  lastFrameMs_ = nowMs;

  const unsigned long elapsed = nowMs - startedMs_;
  const bool active = elapsed < durationMs_;
  const bool pulse = ((elapsed / 120UL) % 2UL) == 0;
  const bool rapidPulse = ((elapsed / 80UL) % 2UL) == 0;
  RGB color = running ? RGB{18, 0, 65} : RGB{0, 0, 0}; // dim purple while playing

  if (active) {
    switch (event_) {
      case LightEvent::Start:     color = pulse ? RGB{0, 255, 40}   : RGB{0, 25, 4}; break;
      case LightEvent::Score:     color = pulse ? RGB{0, 70, 255}   : RGB{0, 0, 15}; break;
      case LightEvent::Lost:      color = pulse ? RGB{255, 0, 0}    : RGB{20, 0, 0}; break;
      case LightEvent::Release:   color = pulse ? RGB{0, 255, 200}  : RGB{0, 20, 16}; break;
      case LightEvent::Bumper:    color = rapidPulse ? RGB{255, 160, 0} : RGB{30, 8, 0}; break;
      case LightEvent::GameOver:  color = pulse ? RGB{255, 0, 0} : RGB{0, 0, 0}; break;
      case LightEvent::NewRecord: color = wheel(uint8_t(nowMs / 12UL)); break;
    }
  }

#if PINBALL_LED_STRIP_TYPE == 1
  // Both strips chained end to end are treated as one continuous run.
  // A moving white highlight accompanies a score; new records get a rainbow chase.
  const uint16_t count = strip.numPixels();
  const uint16_t head = count ? (uint16_t)((nowMs / 35UL) % count) : 0;
  for (uint16_t i = 0; i < count; ++i) {
    RGB pixel = color;
    if (active && event_ == LightEvent::NewRecord) {
      pixel = wheel(uint8_t((uint32_t)i * 256UL / count + nowMs / 12UL));
    } else if (active && event_ == LightEvent::Score && pulse && count) {
      // Three-pixel moving white sparkle across BOTH strips.
      const uint16_t distance = (i + count - head) % count;
      if (distance < 3) pixel = {255, 255, 255};
    }
    strip.setPixelColor(i, strip.Color(pixel.r, pixel.g, pixel.b));
  }
  strip.show();
#else
  // The three MOSFET driver input channels set the whole strip pair to one color.
  // The LED_BRIGHTNESS setting applies here too.
  analogWrite(Pins::LED_RED_PWM,
              uint8_t((uint16_t)color.r * Settings::LED_BRIGHTNESS / 255U));
  analogWrite(Pins::LED_GREEN_PWM,
              uint8_t((uint16_t)color.g * Settings::LED_BRIGHTNESS / 255U));
  analogWrite(Pins::LED_BLUE_PWM,
              uint8_t((uint16_t)color.b * Settings::LED_BRIGHTNESS / 255U));
#endif
}
