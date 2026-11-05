#include "pet.h"
#include "gfx.h"
#include "anim_config.h"
#include <avr/pgmspace.h>

namespace {

// ---- Ikon hati (7x6 piksel, format XBM) ----
const uint8_t ICON_HEART_FULL[]  PROGMEM = { 0x36, 0x7F, 0x7F, 0x3E, 0x1C, 0x08 };
const uint8_t ICON_HEART_EMPTY[] PROGMEM = { 0x36, 0x49, 0x41, 0x22, 0x14, 0x08 };

uint8_t moodNow = anim::MOOD_START;
uint8_t feedCount = 0;
bool hungryNow = anim::HUNGRY_AT_BOOT;

}  // namespace

uint8_t pet::mood() { return moodNow; }

void pet::setMood(uint8_t m) { moodNow = m > anim::MOOD_MAX ? anim::MOOD_MAX : m; }

void pet::addMood(int8_t delta) {
  int8_t m = (int8_t)moodNow + delta;
  if (m < 0) m = 0;
  setMood((uint8_t)m);
}

uint8_t pet::feeds() { return feedCount; }

void pet::onFed() {
  if (feedCount < 255) feedCount++;
  hungryNow = false;
  addMood(1);
}

bool pet::hungry() { return hungryNow; }

void pet::setHungry(bool h) { hungryNow = h; }

bool pet::grown() { return feedCount >= anim::FEEDS_TO_GROW; }

void pet::drawHeart(int16_t x, int16_t y) {
  U8G2& g = gfx::u8();
  g.setBitmapMode(1);   // transparan
  g.drawXBMP(x, y, HEART_W, HEART_H, ICON_HEART_FULL);
}

void pet::drawMood() {
  U8G2& g = gfx::u8();
  g.setBitmapMode(1);
  for (uint8_t i = 0; i < anim::MOOD_MAX; i++) {
    g.drawXBMP(anim::MOOD_X + i * anim::MOOD_STEP, anim::MOOD_Y, HEART_W, HEART_H,
               i < moodNow ? ICON_HEART_FULL : ICON_HEART_EMPTY);
  }
}
