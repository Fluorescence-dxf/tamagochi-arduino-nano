// Scene GAME 2: lari 3 jalur dari atas (top-down). Hati = +1 poin, bom = -GAME_BOMB_PENALTY.
// Tombol 1 = pindah jalur ke kiri, Tombol 3 = ke kanan. Waktu 30 dtk, tanpa game over.
// Intro, HUD, dan layar hasil (kitten kelelahan -> lapar) ada di game_common.cpp.
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
constexpr uint32_t T_PLAY   = anim::RUN_INTRO_MS;
constexpr uint32_t T_RESULT = T_PLAY + anim::RUN_MS;

constexpr int16_t FP = 16;                          // posisi disimpan x16 (1/16 px)
constexpr uint8_t LANES = 3;
constexpr int16_t LANE_X[LANES] = { 32, 64, 96 };   // tengah tiap jalur

// ---- Kitten (kepala dari atas, sprite di game_common) ----
constexpr int16_t PLAYER_Y = 50;
constexpr int16_t PLAYER_H = game::CAT_H;

// ---- Jalan: garis putus-putus di antara jalur ----
constexpr int16_t DIVIDER_X0 = 48, DIVIDER_X1 = 80;
constexpr int16_t DASH_LEN = 6, DASH_PERIOD = 12;
constexpr int16_t ROAD_TOP = 3;                    // di bawah bar waktu

struct Item {
  int16_t y16;
  uint8_t lane;
  bool bomb;
  bool on;
};

Item items[anim::RUN_MAX_ITEMS];
uint8_t lane = 1;
int16_t playerX16 = LANE_X[1] * FP;
uint8_t score = 0;
uint16_t spawnTimer = 0;
uint16_t scroll16 = 0;                             // 0 .. DASH_PERIOD*FP
uint32_t hitAt = 0;
bool everHit = false;
bool resultStarted = false;

void enter() {
  for (uint8_t i = 0; i < anim::RUN_MAX_ITEMS; i++) items[i].on = false;
  lane = 1;
  playerX16 = LANE_X[1] * FP;
  score = 0;
  spawnTimer = anim::RUN_SPAWN_START_MS / 2;
  scroll16 = 0;
  everHit = false;
  resultStarted = false;
  randomSeed(micros());
  sound::play(Sfx::CLICK);
}

// Nilai yang naik dari a ke b selama permainan (gt = ms sejak mulai main)
int32_t ramp(int32_t a, int32_t b, uint32_t gt) {
  return a + (b - a) * (int32_t)gt / (int32_t)anim::RUN_MS;
}

void moveLane(uint16_t dt) {
  if (buttons::pressed(Btn::BTN1) && lane > 0) lane--;
  if (buttons::pressed(Btn::BTN3) && lane < LANES - 1) lane++;

  int16_t target = LANE_X[lane] * FP;                                         // geser halus ke jalur tujuan
  int16_t step = (int32_t)anim::RUN_SLIDE_SPEED * FP * dt / 1000;
  if (playerX16 < target) playerX16 = (target - playerX16 < step) ? target : playerX16 + step;
  else                    playerX16 = (playerX16 - target < step) ? target : playerX16 - step;
}

void spawn() {
  for (uint8_t i = 0; i < anim::RUN_MAX_ITEMS; i++) {
    if (items[i].on) continue;
    items[i].on = true;
    items[i].lane = random(0, LANES);
    items[i].bomb = random(0, 100) < anim::RUN_BOMB_PERCENT;
    items[i].y16 = ROAD_TOP * FP;
    return;
  }
}

void stepItems(uint16_t dt, uint32_t t, uint32_t gt) {
  spawnTimer += dt;
  uint16_t interval = ramp(anim::RUN_SPAWN_START_MS, anim::RUN_SPAWN_END_MS, gt);
  if (spawnTimer >= interval) { spawnTimer -= interval; spawn(); }

  int32_t speed = ramp(anim::RUN_FALL_MIN, anim::RUN_FALL_MAX, gt);           // px/dtk
  int16_t move = speed * FP * dt / 1000;
  scroll16 = (scroll16 + move) % (DASH_PERIOD * FP);

  for (uint8_t i = 0; i < anim::RUN_MAX_ITEMS; i++) {
    Item& it = items[i];
    if (!it.on) continue;
    it.y16 += move;

    int16_t y = it.y16 / FP;
    int16_t h = it.bomb ? game::BOMB_H : game::HEART_H;
    if (y >= cfg::SCREEN_H) { it.on = false; continue; }                      // lewat bawah layar
    if (it.lane != lane || y + h <= PLAYER_Y + 1 || y >= PLAYER_Y + PLAYER_H - 1) continue;

    it.on = false;                                                            // mengenai kitten
    if (!it.bomb) {
      if (score < 255) score++;
      sound::play(Sfx::CATCH);
    } else if (!everHit || t - hitAt >= anim::GAME_HIT_BLINK_MS) {            // kebal sesaat setelah kena
      everHit = true;
      hitAt = t;
      score = score > anim::GAME_BOMB_PENALTY ? score - anim::GAME_BOMB_PENALTY : 0;
      sound::play(Sfx::BONK);
    }
  }
}

void update(uint16_t dt, uint32_t t) {
  if (t < T_RESULT) {
    moveLane(dt);
    if (t >= T_PLAY) stepItems(dt, t, t - T_PLAY);
    return;
  }
  if (!resultStarted) {
    resultStarted = true;
    for (uint8_t i = 0; i < anim::RUN_MAX_ITEMS; i++) items[i].on = false;
    game::startResult(score);
  }
  if (game::resultDone(t - T_RESULT)) app::go(SCENE_HOME);
}

void leave() { sound::stop(); }

// ================= Gambar =================

void drawRoad() {
  U8G2& g = gfx::u8();
  int16_t off = scroll16 / FP;
  for (int16_t y = ROAD_TOP + off; y < cfg::SCREEN_H; y += DASH_PERIOD) {
    int16_t len = cfg::SCREEN_H - y < DASH_LEN ? cfg::SCREEN_H - y : DASH_LEN;
    g.drawVLine(DIVIDER_X0, y, len);
    g.drawVLine(DIVIDER_X1, y, len);
  }
}

void drawPlayer(uint32_t t) {
  if (everHit && t - hitAt < anim::GAME_HIT_BLINK_MS && ((t - hitAt) / anim::GAME_BLINK_STEP_MS) & 1) return;
  int16_t y = PLAYER_Y + (((t / anim::RUN_BOB_MS) & 1) ? 1 : 0);
  game::drawCatHead(playerX16 / FP, y);
}

void draw(uint32_t t) {
  if (t >= T_RESULT) {
    game::drawResult(t - T_RESULT, score);
    return;
  }
  game::drawHud(score, t, T_PLAY, T_RESULT);
  drawRoad();
  for (uint8_t i = 0; i < anim::RUN_MAX_ITEMS; i++) {
    const Item& it = items[i];
    if (!it.on) continue;
    int16_t cx = LANE_X[it.lane], y = it.y16 / FP;
    if (it.bomb) game::drawBomb(cx - game::ITEM_W / 2, y, t);
    else game::drawItemHeart(cx - game::ITEM_W / 2, y);
  }
  drawPlayer(t);
  if (t < T_PLAY) game::drawIntro(F("LARI! AMBIL HATI"));
}

}  // namespace

const Scene sceneRun = { enter, update, draw, leave };
