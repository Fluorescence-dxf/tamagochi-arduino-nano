// Scene BOOT: kitten tidur meringkuk -> mata terbuka -> bangun duduk -> mengeong -> menu utama.
// Tombol apa saja = lewati. Semua angka waktu ada di anim_config.h.
#include "scenes.h"
#include "app.h"
#include "buttons.h"
#include "sound.h"
#include "gfx.h"
#include "kitten.h"
#include "config.h"
#include "anim_config.h"

namespace {

// ---- Garis waktu (ms sejak scene mulai) ----
constexpr uint32_t T_WAKE = anim::BOOT_SLEEP_MS;              // mata terbuka
constexpr uint32_t T_SIT  = T_WAKE + anim::BOOT_WAKE_MS;      // bangun duduk dan mengeong

// ---- Z melayang (posisi awal, geser kanan, naik) ----
constexpr uint8_t Z_SMALL_SIZE = 5;
constexpr uint8_t Z_BIG_SIZE   = 7;
constexpr int16_t Z_SMALL_X = 66, Z_SMALL_Y = 26;
constexpr int16_t Z_BIG_X   = 72, Z_BIG_Y   = 22;
constexpr int16_t Z_DRIFT_X = 8, Z_RISE_Y = 14;

// ---- Balon MEONG! ----
constexpr int16_t BUBBLE_X = 86, BUBBLE_Y = 5, BUBBLE_W = 38, BUBBLE_H = 13;
constexpr int16_t BUBBLE_TEXT_Y = BUBBLE_Y + 10;
constexpr int16_t BUBBLE_TIP_X = 88, BUBBLE_TIP_Y = 25;       // ujung ekor balon (menunjuk ke kepala)

bool meowPlayed = false;

void enter() {
  meowPlayed = false;
  sound::play(Sfx::SNORE);
}

void update(uint16_t dt, uint32_t t) {
  (void)dt;
  if (!meowPlayed && t >= T_SIT) {
    meowPlayed = true;
    sound::play(Sfx::MEONG);
  }
  if (t >= cfg::BOOT_MS || buttons::anyPressed()) app::go(SCENE_HOME);
}

void leave() {
  sound::stop();   // kalau dilewati di tengah suara
}

void drawZ(int16_t x, int16_t y, uint8_t s) {
  U8G2& g = gfx::u8();
  g.drawHLine(x, y, s);
  gfx::line(x + s - 1, y, x, y + s - 1);
  g.drawHLine(x, y + s - 1, s);
}

// Dua Z naik dan bergeser ke kanan, bergantian setengah siklus.
void drawSnoreZ(uint32_t t) {
  int32_t p = (int32_t)(t % anim::BOOT_ZZZ_MS) * 1000 / anim::BOOT_ZZZ_MS;
  int32_t q = (int32_t)((t + anim::BOOT_ZZZ_MS / 2) % anim::BOOT_ZZZ_MS) * 1000 / anim::BOOT_ZZZ_MS;
  drawZ(Z_SMALL_X + p * Z_DRIFT_X / 1000, Z_SMALL_Y - p * Z_RISE_Y / 1000, Z_SMALL_SIZE);
  drawZ(Z_BIG_X + q * Z_DRIFT_X / 1000, Z_BIG_Y - q * Z_RISE_Y / 1000, Z_BIG_SIZE);
}

void drawBubble() {
  U8G2& g = gfx::u8();
  g.drawFrame(BUBBLE_X, BUBBLE_Y, BUBBLE_W, BUBBLE_H);
  gfx::line(BUBBLE_X + 8, BUBBLE_Y + BUBBLE_H, BUBBLE_TIP_X, BUBBLE_TIP_Y);
  gfx::line(BUBBLE_X + 14, BUBBLE_Y + BUBBLE_H, BUBBLE_TIP_X, BUBBLE_TIP_Y);
  gfx::textCenterF(BUBBLE_X + BUBBLE_W / 2, BUBBLE_TEXT_Y, F("MEONG!"));
}

// t2 = ms sejak bangun duduk
void drawAwake(uint32_t t2) {
  kitten::Pose p = kitten::idlePose(t2);
  if (t2 < anim::BOOT_SQUASH_MS) {            // gepeng sesaat saat bangun
    p.legRows = 2;
    p.extraCols = 2;
  }
  if (t2 < anim::BOOT_MEOW_MS) {              // mengeong
    p.eyes = kitten::Eyes::HAPPY;
    p.mouth = ((t2 / anim::BOOT_MOUTH_MS) & 1) ? kitten::Mouth::SMALL_O : kitten::Mouth::OPEN;
    p.tailSwing = ((t2 / anim::BOOT_TAIL_MS) & 1) ? 3 : -3;
    p.look = 0;
    drawBubble();
  }
  kitten::draw(p);
}

void draw(uint32_t t) {
  if (t < T_SIT) {
    bool asleep = t < T_WAKE;
    uint8_t breath = (t / anim::BOOT_BREATH_MS) & 1;
    kitten::drawCurled(anim::KITTEN_CENTER_X, anim::KITTEN_GROUND_Y, breath,
                       asleep ? kitten::Eyes::SLEEP : kitten::Eyes::OPEN, kitten::Mouth::W);
    if (asleep) drawSnoreZ(t);
  } else {
    drawAwake(t - T_SIT);
  }
}

}  // namespace

const Scene sceneBoot = { enter, update, draw, leave };
