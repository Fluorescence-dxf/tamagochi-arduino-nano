// Scene HOME: kitten idle + hati mood + bar tombol (EAT | GAME | SPECIAL).
// Tiap tekan berbunyi klik. Tahan Tombol 1 + Tombol 3 bersamaan (anim::MUTE_CHORD_MS) = mute/unmute.
// Tombol 1 dan 3 bereaksi saat DILEPAS (supaya kombinasi mute tidak memicu aksi tunggal).
// Kitten melakukan aksi idle acak (menguap, melirik, kibas ekor, melompat) saat sedang santai.
// Salin file ini sebagai contoh scene baru.
#include "scenes.h"
#include "app.h"
#include "buttons.h"
#include "sound.h"
#include "gfx.h"
#include "kitten.h"
#include "pet.h"
#include "config.h"
#include "anim_config.h"

namespace {

// ---- Bar tombol bawah ----
constexpr int16_t BAR_LINE_Y   = 54;
constexpr int16_t BAR_TEXT_Y   = 62;
constexpr int16_t BAR_BOX_Y    = 55;
constexpr int16_t BAR_BOX_H    = 9;
constexpr int16_t CELL_W       = 42;
constexpr int16_t CELL_GAP     = 1;
constexpr int16_t CELL_STEP    = CELL_W + CELL_GAP;
constexpr int16_t SEPARATOR_Y  = 56;
constexpr int16_t SEPARATOR_H  = 8;

// ---- Teks singkat (mute/unmute, kenyang) di atas ----
constexpr int16_t TOAST_Y      = 7;

// ---- Balon minta makan (kanan atas kepala kitten) ----
constexpr int16_t BUBBLE_X = 90, BUBBLE_Y = 8, BUBBLE_W = 19, BUBBLE_H = 17;
constexpr int16_t BUBBLE_TAIL_X = 86, BUBBLE_TAIL_Y = 29;          // ujung ekor balon (ke arah kepala)
constexpr int16_t ICON_CX = BUBBLE_X + BUBBLE_W / 2;               // kunigiri mini di dalam balon
constexpr int16_t ICON_Y = BUBBLE_Y + 3;
constexpr int16_t ICON_H = 10;
constexpr int16_t ICON_HALF = 5;
constexpr int16_t ICON_NORI_H = 3;
constexpr int16_t ICON_NORI_HALF = 2;

// ---- Ikon speaker mati (pojok kanan atas, tampil saat mute) ----
constexpr int16_t MUTE_X = 118, MUTE_Y = 0;

// ---- Aksi idle ----
enum Act : uint8_t { ACT_NONE, ACT_YAWN, ACT_GLANCE, ACT_WAG, ACT_HOP, ACT_COUNT };
constexpr uint8_t GLANCE_STEPS = 6;
const int8_t GLANCE_SEQ[GLANCE_STEPS] = { -1, -1, 0, 1, 1, 0 };
constexpr uint16_t YAWN_OPEN_FROM = 250;   // permil: mulut mulai lebar
constexpr uint16_t YAWN_OPEN_TO   = 750;
constexpr int8_t   WAG_SWING      = 3;

uint8_t heldMask = 0;
uint32_t toastUntil = 0;
const __FlashStringHelper* toastText = nullptr;
uint32_t nextMeowAt = 0;
bool pickGame = false;           // submenu pilih game (bar tombol berganti)
uint32_t pickUntil = 0;

uint8_t armed = 0;               // bit tombol: ditekan di scene ini, belum dilepas, bukan bagian kombinasi
bool chordOn = false;            // Tombol 1 + 3 sedang ditahan bersamaan
bool chordFired = false;
uint32_t chordSince = 0;

Act act = ACT_NONE;
Act lastAct = ACT_NONE;
uint32_t actStart = 0;
uint32_t nextActAt = 0;

uint16_t actMs(Act a) {
  switch (a) {
    case ACT_YAWN:   return anim::IDLE_YAWN_MS;
    case ACT_GLANCE: return GLANCE_STEPS * anim::IDLE_GLANCE_STEP_MS;
    case ACT_WAG:    return anim::IDLE_WAG_MS;
    default:         return anim::IDLE_HOP_COUNT * anim::IDLE_HOP_MS;
  }
}

uint32_t randomGap() { return random(anim::IDLE_ACT_MIN_MS, anim::IDLE_ACT_MAX_MS + 1); }

void enter() {
  static bool seeded = false;
  if (!seeded) {
    seeded = true;
    randomSeed(analogRead(anim::IDLE_RNG_PIN) ^ micros());
  }
  heldMask = 0;
  toastUntil = 0;
  nextMeowAt = anim::HUNGRY_FIRST_MEOW_MS;
  pickGame = false;
  armed = 0;
  chordOn = chordFired = false;
  act = ACT_NONE;
  nextActAt = anim::IDLE_ACT_FIRST_MS;
}

void showToast(const __FlashStringHelper* text, uint32_t t) {
  toastText = text;
  toastUntil = t + cfg::TOAST_MS;
}

void onTap1(uint32_t t) {                                     // EAT: hanya kalau lapar
  if (pet::hungry()) app::go(SCENE_FEED);
  else showToast(F("KENYANG"), t);
}

void updateButtons(uint32_t t) {
  constexpr uint8_t BIT1 = 1 << (uint8_t)Btn::BTN1;
  constexpr uint8_t BIT3 = 1 << (uint8_t)Btn::BTN3;

  if (buttons::pressed(Btn::BTN1) && !chordOn) armed |= BIT1;
  if (buttons::pressed(Btn::BTN3) && !chordOn) armed |= BIT3;

  // Kombinasi mute: kedua tombol ditahan >= MUTE_CHORD_MS. Sejak keduanya turun, tap dibatalkan.
  if (buttons::held(Btn::BTN1) && buttons::held(Btn::BTN3)) {
    if (!chordOn) {
      chordOn = true;
      chordFired = false;
      chordSince = t;
      armed &= ~(BIT1 | BIT3);
    } else if (!chordFired && t - chordSince >= anim::MUTE_CHORD_MS) {
      chordFired = true;
      sound::toggleMute();
      showToast(sound::muted() ? F("SUARA MATI") : F("SUARA NYALA"), t);
    }
  } else if (!buttons::held(Btn::BTN1) && !buttons::held(Btn::BTN3)) {
    chordOn = false;
  }

  bool tap1 = buttons::released(Btn::BTN1) && (armed & BIT1);
  bool tap3 = buttons::released(Btn::BTN3) && (armed & BIT3);
  if (buttons::released(Btn::BTN1)) armed &= ~BIT1;
  if (buttons::released(Btn::BTN3)) armed &= ~BIT3;

  if (pickGame) {                                             // submenu: CATCH | RUN | CANCEL
    if (tap1) app::go(SCENE_CATCH);
    else if (buttons::pressed(Btn::BTN2)) app::go(SCENE_RUN);
    else if (tap3 || t >= pickUntil) pickGame = false;
    return;
  }
  if (tap1) onTap1(t);
  if (buttons::pressed(Btn::BTN2)) {                          // GAME: hanya kalau kenyang
    if (pet::hungry()) {
      showToast(F("MAKAN DULU"), t);
    } else {
      pickGame = true;
      pickUntil = t + anim::HOME_PICK_MS;
    }
  }
  if (tap3) app::go(SCENE_BIRTHDAY);                          // SPECIAL
}

void updateIdle(uint32_t t) {
  if (buttons::anyPressed()) {                                // interaksi membatalkan aksi
    act = ACT_NONE;
    nextActAt = t + anim::IDLE_ACT_MIN_MS;
    return;
  }
  if (act != ACT_NONE) {
    if (t - actStart >= actMs(act)) {
      act = ACT_NONE;
      nextActAt = t + randomGap();
    }
    return;
  }
  if (t < nextActAt) return;
  if (pet::hungry() || pickGame || heldMask) {
    nextActAt = t + anim::IDLE_ACT_RETRY_MS;
    return;
  }
  Act a = (Act)random(1, ACT_COUNT);
  if (a == lastAct) a = (Act)(a % (ACT_COUNT - 1) + 1);       // jangan sama dua kali berturut-turut
  act = lastAct = a;
  actStart = t;
  if (a == ACT_HOP) sound::play(Sfx::MEONG);
}

void update(uint16_t dt, uint32_t t) {
  (void)dt;

  heldMask = 0;
  for (uint8_t i = 0; i < (uint8_t)Btn::COUNT; i++) {
    if (buttons::held((Btn)i)) heldMask |= (1 << i);
  }

  if (buttons::anyPressed()) sound::play(Sfx::CLICK);

  updateButtons(t);
  updateIdle(t);

  // Lapar: meong berkala
  if (pet::hungry() && t >= nextMeowAt) {
    sound::play(Sfx::MEONG);
    nextMeowAt = t + anim::HUNGRY_MEOW_PERIOD_MS;
  }
}

// Pose aksi idle, fungsi murni dari waktu sejak aksi mulai (u).
void applyAct(kitten::Pose& p, Act a, uint32_t u) {
  switch (a) {
    case ACT_YAWN: {
      uint32_t q = u * 1000 / anim::IDLE_YAWN_MS;
      p.eyes = kitten::Eyes::CLOSED;
      p.mouth = (q >= YAWN_OPEN_FROM && q < YAWN_OPEN_TO) ? kitten::Mouth::OPEN : kitten::Mouth::SMALL_O;
      p.look = 0;
      break;
    }
    case ACT_GLANCE: {
      uint32_t i = u / anim::IDLE_GLANCE_STEP_MS;
      p.look = GLANCE_SEQ[i < GLANCE_STEPS ? i : GLANCE_STEPS - 1];
      break;
    }
    case ACT_WAG:
      p.tailSwing = ((u / anim::IDLE_WAG_STEP_MS) & 1) ? WAG_SWING : -WAG_SWING;
      p.eyes = kitten::Eyes::HAPPY;
      p.look = 0;
      break;
    default: {                                                // ACT_HOP: parabola per lompatan
      int32_t q = (int32_t)(u % anim::IDLE_HOP_MS) * 1000 / anim::IDLE_HOP_MS;
      p.groundY -= anim::IDLE_HOP_PEAK * 4 * q * (1000 - q) / 1000000;
      p.eyes = kitten::Eyes::HAPPY;
      p.mouth = kitten::Mouth::OPEN;
      p.look = 0;
      break;
    }
  }
}

// Speaker dengan tanda silang
void drawMuteIcon() {
  U8G2& g = gfx::u8();
  g.drawBox(MUTE_X, MUTE_Y + 2, 2, 3);
  g.drawVLine(MUTE_X + 2, MUTE_Y + 1, 5);
  g.drawVLine(MUTE_X + 3, MUTE_Y, 7);
  gfx::line(MUTE_X + 5, MUTE_Y + 1, MUTE_X + 9, MUTE_Y + 5);
  gfx::line(MUTE_X + 9, MUTE_Y + 1, MUTE_X + 5, MUTE_Y + 5);
}

void drawCell(uint8_t index, const __FlashStringHelper* label) {
  U8G2& g = gfx::u8();
  int16_t x = index * CELL_STEP;
  int16_t cx = x + CELL_W / 2;
  bool held = heldMask & (1 << index);

  if (held) {
    g.setDrawColor(1);
    g.drawBox(x, BAR_BOX_Y, CELL_W, BAR_BOX_H);
    g.setDrawColor(0);
    gfx::textCenterF(cx, BAR_TEXT_Y, label);
    g.setDrawColor(1);
  } else {
    gfx::textCenterF(cx, BAR_TEXT_Y, label);
  }
}

// Balon dengan kunigiri mini: kitten minta makan
void drawHungryBubble(uint32_t t) {
  U8G2& g = gfx::u8();
  int16_t dx = kitten::growth() / 2;                           // geser ke kanan kalau kitten besar
  g.drawFrame(BUBBLE_X + dx, BUBBLE_Y, BUBBLE_W, BUBBLE_H);
  gfx::line(BUBBLE_X + dx + 3, BUBBLE_Y + BUBBLE_H, BUBBLE_TAIL_X + dx, BUBBLE_TAIL_Y);
  gfx::line(BUBBLE_X + dx + 8, BUBBLE_Y + BUBBLE_H, BUBBLE_TAIL_X + dx, BUBBLE_TAIL_Y);

  int16_t y = ICON_Y + (((t / anim::HUNGRY_BOB_MS) & 1) ? 1 : 0);
  for (int16_t r = 0; r < ICON_H; r++) {
    int16_t half = r * ICON_HALF / (ICON_H - 1);
    g.drawHLine(ICON_CX + dx - half, y + r, 2 * half + 1);
  }
  g.setDrawColor(0);                                           // nori
  for (int16_t r = ICON_H - ICON_NORI_H; r < ICON_H; r++) {
    g.drawHLine(ICON_CX + dx - ICON_NORI_HALF, y + r, 2 * ICON_NORI_HALF + 1);
  }
  g.setDrawColor(1);
}

void draw(uint32_t t) {
  U8G2& g = gfx::u8();

  kitten::Pose p = kitten::idlePose(t);
  if (act != ACT_NONE && t >= actStart) applyAct(p, act, t - actStart);
  if (pet::hungry()) p.mouth = kitten::Mouth::FROWN;
  kitten::draw(p);
  pet::drawMood();
  if (sound::muted()) drawMuteIcon();
  if (pet::hungry()) drawHungryBubble(t);

  if (t < toastUntil && toastText) gfx::textCenterF(cfg::SCREEN_W / 2, TOAST_Y, toastText);

  g.drawHLine(0, BAR_LINE_Y, cfg::SCREEN_W);
  g.drawVLine(CELL_STEP - 1, SEPARATOR_Y, SEPARATOR_H);
  g.drawVLine(2 * CELL_STEP - 1, SEPARATOR_Y, SEPARATOR_H);

  if (pickGame) {
    drawCell(0, F("CATCH"));
    drawCell(1, F("RUN"));
    drawCell(2, F("CANCEL"));
  } else {
    drawCell(0, F("EAT"));
    drawCell(1, F("GAME"));
    drawCell(2, F("SPECIAL"));
  }
}

}  // namespace

const Scene sceneHome = { enter, update, draw, nullptr };
