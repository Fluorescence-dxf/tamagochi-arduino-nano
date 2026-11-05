#include "game_common.h"
#include "buttons.h"
#include "sound.h"
#include "gfx.h"
#include "kitten.h"
#include "pet.h"
#include "config.h"
#include "anim_config.h"
#include <avr/pgmspace.h>

namespace {

// ---- Sprite (format XBM, bit 0 = kolom paling kiri) ----
const uint8_t SPR_HEART[] PROGMEM = { 0xEE, 0x00, 0xFF, 0x01, 0xFF, 0x01, 0xFF, 0x01, 0xFE, 0x00, 0x7C, 0x00, 0x38, 0x00, 0x10, 0x00 };
const uint8_t SPR_BOMB[] PROGMEM = {
  0x00, 0x00, 0x40, 0x00, 0x30, 0x00, 0x7C, 0x00, 0x82, 0x00, 0x45, 0x01,
  0x29, 0x01, 0x11, 0x01, 0x29, 0x01, 0x45, 0x01, 0x82, 0x00, 0x7C, 0x00,
};
const uint8_t SPR_CAT[] PROGMEM = {
  0x03, 0x18, 0x07, 0x1C, 0xFF, 0x1F, 0xFF, 0x1F, 0xF7, 0x1D,
  0xF7, 0x1D, 0xFF, 0x1F, 0xFE, 0x0F, 0xFC, 0x07, 0xF8, 0x03,
};
// Percikan sumbu bom (titik di sekitar ujung sumbu), berkedip
constexpr int16_t SPARK_X = 7, SPARK_Y = 1;

constexpr int16_t SCR_W = cfg::SCREEN_W;

// ---- HUD ----
constexpr int16_t TIMEBAR_H = 2;
constexpr int16_t SCORE_X   = 1;
constexpr int16_t SCORE_Y   = 10;     // baseline teks skor

// ---- Intro ----
constexpr int16_t INTRO_TITLE_Y = 26;
constexpr int16_t INTRO_HINT_Y  = 40;
constexpr int16_t INTRO_WARN_Y  = 52;

// ---- Layar hasil ----
constexpr uint8_t TIRED_X      = 40;  // kitten di kiri, teks di kanan
constexpr int16_t RES_CX       = 104; // tengah kolom teks kanan
constexpr int16_t RES_LABEL_Y  = 22;
constexpr int16_t RES_SCORE_Y  = 34;
constexpr int16_t RES_BONUS_Y  = 46;  // "+1" di bawah angka
constexpr int16_t RES_MSG_Y    = 58;
constexpr int16_t SWEAT_X      = 62;
constexpr int16_t SWEAT_Y      = 18;
constexpr uint8_t SWEAT_STEPS  = 6;

}  // namespace

void game::drawItemHeart(int16_t x, int16_t y) {
  U8G2& g = gfx::u8();
  g.setBitmapMode(1);
  g.drawXBMP(x, y, ITEM_W, HEART_H, SPR_HEART);
}

void game::drawBomb(int16_t x, int16_t y, uint32_t t) {
  U8G2& g = gfx::u8();
  g.setBitmapMode(1);
  bool on = (t / anim::GAME_SPARK_MS) & 1;
  if (on) x++;                                            // bom bergetar 1 px (hati meluncur mulus)
  g.drawXBMP(x, y, ITEM_W, BOMB_H, SPR_BOMB);
  if (on) {                                               // sumbu menyala
    int16_t sx = x + SPARK_X, sy = y + SPARK_Y;
    g.drawPixel(sx, sy - 1); g.drawPixel(sx - 1, sy); g.drawPixel(sx, sy);
    g.drawPixel(sx + 1, sy); g.drawPixel(sx, sy + 1);
  }
}

void game::drawCatHead(int16_t cx, int16_t y) {
  U8G2& g = gfx::u8();
  g.setBitmapMode(1);
  g.drawXBMP(cx - CAT_W / 2, y, CAT_W, CAT_H, SPR_CAT);
}

void game::drawHud(uint8_t score, uint32_t t, uint32_t tPlay, uint32_t tEnd) {
  U8G2& g = gfx::u8();
  if (t >= tPlay) g.drawBox(0, 0, (int32_t)SCR_W * (tEnd - t) / (tEnd - tPlay), TIMEBAR_H);
  char b[4];
  itoa(score, b, 10);
  g.drawStr(SCORE_X, SCORE_Y, b);
}

void game::drawIntro(const __FlashStringHelper* title) {
  gfx::textCenterF(SCR_W / 2, INTRO_TITLE_Y, title);
  gfx::textCenterF(SCR_W / 2, INTRO_HINT_Y, F("1: KIRI   3: KANAN"));
  gfx::textCenterF(SCR_W / 2, INTRO_WARN_Y, F("HINDARI BOM!"));
}

void game::startResult(uint8_t score) {
  pet::setHungry(true);                                   // lelah -> minta makan
  if (score >= anim::GAME_SCORE_GOOD) {
    pet::addMood(1);
    sound::play(Sfx::YUM);
  } else {
    sound::play(Sfx::TIRED);
  }
}

void game::drawResult(uint32_t rt, uint8_t score) {
  U8G2& g = gfx::u8();
  bool good = score >= anim::GAME_SCORE_GOOD;

  kitten::Pose p = kitten::idlePose(rt);                  // kelelahan: mata terpejam, terengah
  uint8_t pant = (rt / anim::GAME_PANT_MS) & 1;
  p.centerX = TIRED_X;
  p.eyes = kitten::Eyes::CLOSED;
  p.mouth = pant ? kitten::Mouth::OPEN : kitten::Mouth::SMALL_O;
  p.legRows = anim::KITTEN_LEGS_MIN + pant;
  p.look = 0;
  kitten::draw(p);

  uint8_t step = (rt / anim::GAME_SWEAT_MS) % SWEAT_STEPS;   // tetes keringat
  int16_t sx = SWEAT_X + kitten::growth() / 2;                 // mengikuti tepi kepala
  g.drawVLine(sx, SWEAT_Y + step, 2);
  g.drawPixel(sx, SWEAT_Y + step + 3);

  pet::drawMood();
  gfx::textCenterF(RES_CX, RES_LABEL_Y, F("SKOR"));
  char b[4];
  itoa(score, b, 10);
  int16_t x = RES_CX - g.getStrWidth(b) / 2;
  g.drawStr(x, RES_SCORE_Y, b);
  g.drawStr(x + 1, RES_SCORE_Y, b);                       // tebal
  if (good) {
    pet::drawHeart(RES_CX - 10, RES_BONUS_Y - pet::HEART_H + 1);
    gfx::textCenterF(RES_CX + 6, RES_BONUS_Y, F("+1"));
  }
  gfx::textCenterF(RES_CX, RES_MSG_Y, good ? F("HEBAT!") : F("SEMANGAT!"));
}

bool game::resultDone(uint32_t rt) {
  return rt >= anim::GAME_RESULT_MS || (rt >= anim::GAME_RESULT_LOCK_MS && buttons::anyPressed());
}
