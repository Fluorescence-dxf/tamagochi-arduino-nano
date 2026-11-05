// Scene GAME 1: tangkap hati. Karakter = kitten yang membawa keranjang di atas kepalanya.
// Tombol 1 = kiri, Tombol 3 = kanan. Hati = +1, bom = -GAME_BOMB_PENALTY (biarkan jatuh).
//   intro -> main 30 dtk -> hasil (kitten kelelahan) -> home (kitten lapar)
// Skor >= GAME_SCORE_GOOD memberi mood +1. Intro, HUD, hasil, dan sprite ada di game_common.cpp.
// Semua angka ada di anim_config.h.
//
// Aturan scene: state hanya diubah di update(); draw() hanya membaca state dan waktu t.
#include "scenes.h"
#include "app.h"
#include "buttons.h"
#include "sound.h"
#include "gfx.h"
#include "pet.h"
#include "game_common.h"
#include "config.h"
#include "anim_config.h"

namespace {

// ---- Garis waktu (ms sejak scene mulai) ----
constexpr uint32_t T_PLAY   = anim::CATCH_INTRO_MS;
constexpr uint32_t T_RESULT = T_PLAY + anim::CATCH_MS;

// ---- Layar ----
constexpr int16_t SCR_W = cfg::SCREEN_W;
constexpr int16_t SCR_H = cfg::SCREEN_H;
constexpr int16_t FP = 16;                         // posisi disimpan x16 (1/16 px) supaya gerak halus di frame 66 ms

// ---- Karakter: kepala kitten di dasar layar, keranjang seperti topi di atasnya ----
constexpr int16_t CAT_Y        = SCR_H - game::CAT_H;      // atas kepala
constexpr int16_t BASKET_ROWS  = 8;                // tinggi keranjang
constexpr int16_t BASKET_Y     = CAT_Y - BASKET_ROWS;      // bibir keranjang (tempat benda ditangkap)
constexpr int16_t BASKET_HALF  = 9;                // setengah lebar bibir (lebar = 2*HALF+1)
constexpr int16_t BASKET_TAPER = 3;                // badan menyempit sebanyak ini di dasar (ke lebar kepala)
constexpr int16_t CATCH_REACH  = 1;                // toleransi horizontal tangkapan (px)
constexpr int16_t CATCH_DEPTH  = 4;                // benda tertangkap kalau dasarnya masuk sedalam ini dari bibir

constexpr int16_t ITEM_START_Y = 11;               // benda muncul di bawah HUD

struct Item {
  int16_t x;        // px, kiri atas
  int16_t y16;      // x16
  bool    bomb;
  bool    on;
};

Item items[anim::CATCH_MAX_HEARTS];
int16_t basketX16 = (SCR_W / 2) * FP;
uint8_t score = 0;
uint16_t spawnTimer = 0;
uint32_t hitAt = 0;
bool everHit = false;
bool resultStarted = false;

int16_t basketX() { return basketX16 / FP; }

void enter() {
  for (uint8_t i = 0; i < anim::CATCH_MAX_HEARTS; i++) items[i].on = false;
  basketX16 = (SCR_W / 2) * FP;
  score = 0;
  spawnTimer = anim::CATCH_SPAWN_START_MS / 2;     // benda pertama cepat muncul
  everHit = false;
  resultStarted = false;
  randomSeed(micros());
  sound::play(Sfx::CLICK);
}

// Nilai yang naik dari a ke b selama permainan (gt = ms sejak mulai main)
int32_t ramp(int32_t a, int32_t b, uint32_t gt) {
  return a + (b - a) * (int32_t)gt / (int32_t)anim::CATCH_MS;
}

void moveBasket(uint16_t dt) {
  int8_t dir = (int8_t)buttons::held(Btn::BTN3) - (int8_t)buttons::held(Btn::BTN1);
  basketX16 += (int32_t)dir * anim::CATCH_BASKET_SPEED * FP * dt / 1000;
  int16_t lo = BASKET_HALF * FP, hi = (SCR_W - 1 - BASKET_HALF) * FP;
  if (basketX16 < lo) basketX16 = lo;
  if (basketX16 > hi) basketX16 = hi;
}

void spawn() {
  for (uint8_t i = 0; i < anim::CATCH_MAX_HEARTS; i++) {
    if (items[i].on) continue;
    items[i].on = true;
    items[i].x = random(0, SCR_W - game::ITEM_W + 1);
    items[i].y16 = ITEM_START_Y * FP;
    items[i].bomb = random(0, 100) < anim::CATCH_BOMB_PERCENT;
    return;
  }
}

void stepItems(uint16_t dt, uint32_t t, uint32_t gt) {
  spawnTimer += dt;
  uint16_t interval = ramp(anim::CATCH_SPAWN_START_MS, anim::CATCH_SPAWN_END_MS, gt);
  if (spawnTimer >= interval) { spawnTimer -= interval; spawn(); }

  int32_t fall = ramp(anim::CATCH_FALL_MIN, anim::CATCH_FALL_MAX, gt);       // px/dtk
  int16_t bx = basketX();
  for (uint8_t i = 0; i < anim::CATCH_MAX_HEARTS; i++) {
    Item& it = items[i];
    if (!it.on) continue;
    it.y16 += fall * FP * dt / 1000;

    int16_t y = it.y16 / FP;
    if (y >= SCR_H) { it.on = false; continue; }                             // lewat bawah layar
    int16_t bottom = y + (it.bomb ? game::BOMB_H : game::HEART_H);
    if (bottom < BASKET_Y || bottom > BASKET_Y + CATCH_DEPTH) continue;

    int16_t cx = it.x + game::ITEM_W / 2;
    int16_t d = cx > bx ? cx - bx : bx - cx;
    if (d > BASKET_HALF + CATCH_REACH) continue;                             // meleset

    it.on = false;
    if (!it.bomb) {
      if (score < 255) score++;
      sound::play(Sfx::CATCH);
    } else {
      everHit = true;
      hitAt = t;
      score = score > anim::GAME_BOMB_PENALTY ? score - anim::GAME_BOMB_PENALTY : 0;
      sound::play(Sfx::BONK);
    }
  }
}

void update(uint16_t dt, uint32_t t) {
  if (t < T_RESULT) {
    moveBasket(dt);
    if (t >= T_PLAY) stepItems(dt, t, t - T_PLAY);
    return;
  }
  if (!resultStarted) {
    resultStarted = true;
    for (uint8_t i = 0; i < anim::CATCH_MAX_HEARTS; i++) items[i].on = false;
    game::startResult(score);
  }
  if (game::resultDone(t - T_RESULT)) app::go(SCENE_HOME);
}

void leave() { sound::stop(); }

// ================= Gambar =================

void drawBasket(int16_t bx) {
  U8G2& g = gfx::u8();
  g.setDrawColor(1);
  g.drawHLine(bx - BASKET_HALF, BASKET_Y, 2 * BASKET_HALF + 1);              // bibir
  for (int16_t r = 1; r < BASKET_ROWS; r++) {
    int16_t half = BASKET_HALF - 1 - (r - 1) * BASKET_TAPER / (BASKET_ROWS - 2);
    g.drawHLine(bx - half, BASKET_Y + r, 2 * half + 1);
  }
  g.setDrawColor(0);                                                         // anyaman
  for (int16_t r = 2; r < BASKET_ROWS - 1; r++) {
    int16_t half = BASKET_HALF - 1 - (r - 1) * BASKET_TAPER / (BASKET_ROWS - 2);
    for (int16_t c = -half + 1; c < half; c++) {
      if (((c + r) & 1) == 0) g.drawPixel(bx + c, BASKET_Y + r);
    }
  }
  g.setDrawColor(1);
}

void draw(uint32_t t) {
  if (t >= T_RESULT) {
    game::drawResult(t - T_RESULT, score);
    return;
  }
  game::drawHud(score, t, T_PLAY, T_RESULT);
  for (uint8_t i = 0; i < anim::CATCH_MAX_HEARTS; i++) {
    const Item& it = items[i];
    if (!it.on) continue;
    if (it.bomb) game::drawBomb(it.x, it.y16 / FP, t);
    else game::drawItemHeart(it.x, it.y16 / FP);
  }
  bool hidden = everHit && t - hitAt < anim::GAME_HIT_BLINK_MS &&
                ((t - hitAt) / anim::GAME_BLINK_STEP_MS) & 1;                // berkedip setelah kena bom
  if (!hidden) {
    game::drawCatHead(basketX(), CAT_Y);                                     // kepala dulu, keranjang menimpa di atasnya
    drawBasket(basketX());
  }
  if (t < T_PLAY) game::drawIntro(F("TANGKAP HATI!"));
}

}  // namespace

const Scene sceneCatch = { enter, update, draw, leave };
