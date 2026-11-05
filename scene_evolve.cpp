// Scene EVOLUSI (setelah makan ke-FEEDS_TO_GROW): kitten bergetar sementara kilau berkumpul,
// layar berkedip putih, lalu kitten melebar jadi wujud besar dan kilau menyebar.
// Wujud besar disimpan lewat kitten::setGrowth(), jadi semua scene berikutnya ikut besar.
// Semua angka ada di anim_config.h.
#include "scenes.h"
#include "app.h"
#include "buttons.h"
#include "sound.h"
#include "gfx.h"
#include "kitten.h"
#include "config.h"
#include "anim_config.h"
#include <avr/pgmspace.h>

namespace {

// ---- Garis waktu (ms sejak scene mulai) ----
constexpr uint32_t T_FLASH = anim::EVOLVE_SHAKE_MS;
constexpr uint32_t T_GROW  = T_FLASH + anim::EVOLVE_FLASH_MS;
constexpr uint32_t T_SHOW  = T_GROW + anim::EVOLVE_GROW_MS;
constexpr uint32_t T_END   = T_SHOW + anim::EVOLVE_SHOW_MS;

// ---- Kilat: putih penuh dua kali (ms sejak kilat mulai) ----
constexpr uint16_t FLASH1_END = 200;
constexpr uint16_t FLASH2_START = 280;
constexpr uint16_t FLASH2_END = 380;

// ---- Kilau: 8 arah (nilai x64), tanda plus 5 px ----
constexpr uint8_t DIRS = 8;
const int8_t DIR_COS[DIRS] PROGMEM = { 64, 45, 0, -45, -64, -45, 0, 45 };
const int8_t DIR_SIN[DIRS] PROGMEM = { 0, 45, 64, 45, 0, -45, -64, -45 };
constexpr uint8_t SPARKLE_BLINK_MS = 80;
constexpr int16_t SPARKLE_MARGIN = 2;                 // kilau di luar batas ini tidak digambar (cegah koordinat negatif)

void enter() { sound::play(Sfx::EVOLVE); }

void update(uint16_t dt, uint32_t t) {
  (void)dt;
  if (t >= T_END || (t >= T_SHOW && buttons::anyPressed())) app::go(SCENE_HOME);
}

void leave() {
  kitten::setGrowth(anim::KITTEN_BIG_EXTRA);
  sound::stop();
}

void drawSparkles(uint32_t t, int16_t r) {
  U8G2& g = gfx::u8();
  for (uint8_t i = 0; i < DIRS; i++) {
    int16_t x = anim::KITTEN_CENTER_X + (int16_t)(int8_t)pgm_read_byte(&DIR_COS[i]) * r / 64;
    int16_t y = anim::EVOLVE_CENTER_Y + (int16_t)(int8_t)pgm_read_byte(&DIR_SIN[i]) * r / 64;
    if (x < SPARKLE_MARGIN || y < SPARKLE_MARGIN || x >= cfg::SCREEN_W - SPARKLE_MARGIN || y >= cfg::SCREEN_H - SPARKLE_MARGIN) continue;
    g.drawPixel(x, y);
    if (((t / SPARKLE_BLINK_MS) + i) & 1) {
      g.drawPixel(x - 1, y); g.drawPixel(x + 1, y); g.drawPixel(x, y - 1); g.drawPixel(x, y + 1);
    }
  }
}

void draw(uint32_t t) {
  kitten::Pose p = kitten::idlePose(t);

  if (t < T_FLASH) {                                            // getar + kilau berkumpul
    int32_t q = (int32_t)t * 1000 / anim::EVOLVE_SHAKE_MS;
    p.centerX += ((t / anim::EVOLVE_SHAKE_STEP_MS) & 1) ? 1 : -1;
    p.eyes = kitten::Eyes::CLOSED;
    p.mouth = kitten::Mouth::SMALL_O;
    p.tailSwing = ((t / anim::EVOLVE_SHAKE_STEP_MS) & 1) ? 3 : -3;
    kitten::draw(p);
    int16_t r = anim::EVOLVE_SPARKLE_R0 - (int32_t)(anim::EVOLVE_SPARKLE_R0 - anim::EVOLVE_SPARKLE_R1) * (q * q / 1000) / 1000;
    drawSparkles(t, r);
    return;
  }

  if (t < T_GROW) {                                             // kilat putih
    uint32_t ft = t - T_FLASH;
    if (ft < FLASH1_END || (ft >= FLASH2_START && ft < FLASH2_END)) gfx::u8().drawBox(0, 0, cfg::SCREEN_W, cfg::SCREEN_H);
    return;
  }

  p.eyes = kitten::Eyes::HAPPY;                                 // wujud besar muncul
  p.mouth = (t < T_SHOW) ? kitten::Mouth::OPEN : kitten::Mouth::W;
  p.look = 0;
  if (t < T_SHOW) {
    int32_t q = (int32_t)(t - T_GROW) * 1000 / anim::EVOLVE_GROW_MS;
    int32_t e = 1000 - (1000 - q) * (1000 - q) / 1000;          // melambat di akhir
    p.extraCols = anim::KITTEN_BIG_EXTRA * e / 1000;
    kitten::draw(p);
    drawSparkles(t, anim::EVOLVE_SPARKLE_R1 + (int32_t)(anim::EVOLVE_SPARKLE_R0 - anim::EVOLVE_SPARKLE_R1) * q / 1000);
  } else {
    p.extraCols = anim::KITTEN_BIG_EXTRA;
    kitten::draw(p);
  }
}

}  // namespace

const Scene sceneEvolve = { enter, update, draw, leave };
