// Scene ULTAH (30 dtk): kembang api di belakang kitten, kitten bernyanyi dan melompat kecil.
// Di putaran lagu ke-2 pesawat masuk menarik banner ucapan yang menutupi kitten,
// banner berbalik menampilkan nama penerima saat lirik lagu, lalu pesawat pergi dan hati melayang.
// Semua angka waktu ada di anim_config.h. Lagunya di sound.cpp (Sfx::BIRTHDAY).
//
// Catatan AVR: koordinat U8g2 bertipe uint8_t, jadi nilai negatif akan "membungkus".
// Semua gambar di file ini lewat fungsi c*() yang memotong di tepi layar.
#include "scenes.h"
#include "app.h"
#include "buttons.h"
#include "sound.h"
#include "gfx.h"
#include "kitten.h"
#include "pet.h"
#include "config.h"
#include "anim_config.h"
#include <avr/pgmspace.h>

namespace {

constexpr int16_t SCR_W = cfg::SCREEN_W;
constexpr int16_t SCR_H = cfg::SCREEN_H;

// ---- Banner + pesawat (koordinat saat s = 0; s = geser horizontal seluruh rombongan) ----
constexpr int16_t BANNER_X  = 12;
constexpr int16_t BANNER_W  = 80;
constexpr int16_t BANNER_H  = 21;
constexpr int16_t BANNER_CY = 32;                 // tengah vertikal (menutupi wajah kitten)
constexpr int16_t ROPE_X0   = BANNER_X + BANNER_W;
constexpr int16_t PLANE_X   = 102;
constexpr int16_t PLANE_W   = 22;
constexpr int16_t PLANE_ROW = 4;                  // baris badan pesawat tempat tali menempel
constexpr int16_t SHIFT_LEFT  = -(PLANE_X + PLANE_W + 2);      // seluruhnya di luar kiri
constexpr int16_t SHIFT_RIGHT = SCR_W - BANNER_X + 2;          // seluruhnya di luar kanan
constexpr int16_t FRAME_INSET = 2;                // bingkai hitam dalam banner
constexpr int16_t TEXT_MIN_H  = 12;               // teks disembunyikan kalau banner lebih pendek (saat berbalik)

// ---- Teks banner (5x7: lebar 5 px, tinggi 7, konfigurasi di config.h) ----
const char TXT_GREET[] PROGMEM = PET_GIFT_GREET;
const char TXT_NAME[]  PROGMEM = PET_GIFT_NAME;
constexpr int16_t GLYPH_W      = 5;
constexpr int16_t GLYPH_ASCENT = 3;               // baseline = tengah banner + ini
constexpr int16_t NAME_STEP    = 7;               // Nama ditebalkan (2x geser 1 px), jarak huruf lebih lebar

// ---- Kembang api: 12 arah, nilai x64 ----
constexpr uint8_t FW_DIRS = 12;
const int8_t FW_COS[FW_DIRS] PROGMEM = { 64, 55, 32, 0, -32, -55, -64, -55, -32, 0, 32, 55 };
const int8_t FW_SIN[FW_DIRS] PROGMEM = { 0, 32, 55, 64, 55, 32, 0, -32, -55, -64, -55, -32 };
const uint8_t FW_X[] PROGMEM = { 22, 100, 64, 38, 112, 14, 88, 52 };
const uint8_t FW_Y[] PROGMEM = { 16, 12, 9, 24, 22, 28, 18, 26 };
constexpr uint8_t FW_POS_N   = sizeof(FW_X);
constexpr int16_t FW_FLASH_P = 120;               // permil umur: kilatan di pusat
constexpr int16_t FW_BIG_P   = 600;               // sampai sini percikan 2x2, setelah itu 1 px
constexpr int16_t FW_INNER_PCT = 55;              // radius cincin dalam (% dari luar)
constexpr int16_t FW_FADE_P  = 800;               // setelah ini hanya setengah percikan yang tersisa

// ---- Hati finale (x di luar badan kitten) ----
const int16_t HEART_X[] = { 30, 90, 22 };
constexpr uint8_t HEART_N = sizeof(HEART_X) / sizeof(HEART_X[0]);

bool moodSet = false;

// ================= Gambar dengan pemotongan di tepi layar =================

void cPix(int16_t x, int16_t y) {
  if (x < 0 || y < 0 || x >= SCR_W || y >= SCR_H) return;
  gfx::u8().drawPixel(x, y);
}

void cHLine(int16_t x, int16_t y, int16_t w) {
  if (y < 0 || y >= SCR_H || w <= 0) return;
  int16_t x1 = x + w;
  if (x < 0) x = 0;
  if (x1 > SCR_W) x1 = SCR_W;
  if (x1 <= x) return;
  gfx::u8().drawHLine(x, y, x1 - x);
}

void cVLine(int16_t x, int16_t y, int16_t h) {
  for (int16_t i = 0; i < h; i++) cPix(x, y + i);
}

void cBox(int16_t x, int16_t y, int16_t w, int16_t h) {
  for (int16_t i = 0; i < h; i++) cHLine(x, y + i, w);
}

void cGlyph(int16_t x, int16_t y, char c) {
  if (x < 0 || x + GLYPH_W > SCR_W) return;   // huruf yang terpotong tidak digambar
  gfx::u8().drawGlyph(x, y, c);
}

// ================= Kembang api =================

void drawBurst(uint8_t i, uint32_t t) {
  uint32_t start = anim::BDAY_FW_FIRST_MS + (uint32_t)i * anim::BDAY_FW_GAP_MS;
  if (t < start || t >= start + anim::BDAY_FW_LIFE_MS) return;

  int32_t p = (int32_t)(t - start) * 1000 / anim::BDAY_FW_LIFE_MS;                    // 0..1000
  int16_t cx = pgm_read_byte(&FW_X[i % FW_POS_N]);
  int16_t cy = pgm_read_byte(&FW_Y[(i * 3 + 1) % FW_POS_N]);
  int16_t rmax = anim::BDAY_FW_RADIUS_MIN + (i % 3) * anim::BDAY_FW_RADIUS_STEP;
  int16_t r = (int32_t)rmax * (1000 - (1000 - p) * (1000 - p) / 1000) / 1000;        // melambat di ujung
  int16_t fall = (int32_t)anim::BDAY_FW_GRAVITY_PX * p * p / 1000000;

  if (p < FW_FLASH_P) {                                                               // kilatan "plus"
    cPix(cx, cy); cPix(cx - 1, cy); cPix(cx + 1, cy); cPix(cx, cy - 1); cPix(cx, cy + 1);
  }
  for (uint8_t ring = 0; ring < 2; ring++) {                                          // cincin luar + dalam
    if (ring && p >= FW_FADE_P) break;                                                // cincin dalam padam duluan
    int16_t rr = ring ? r * FW_INNER_PCT / 100 : r;
    for (uint8_t k = 0; k < FW_DIRS; k++) {
      if (p >= FW_FADE_P && (k & 1)) continue;
      int16_t x = cx + (int16_t)(int8_t)pgm_read_byte(&FW_COS[k]) * rr / 64;
      int16_t y = cy + (int16_t)(int8_t)pgm_read_byte(&FW_SIN[k]) * rr / 64 + fall;
      cPix(x, y);
      if (p < FW_BIG_P && !ring) { cPix(x + 1, y); cPix(x, y + 1); cPix(x + 1, y + 1); }
    }
  }
}

void drawFireworks(uint32_t t) {
  for (uint8_t i = 0; i < anim::BDAY_FW_COUNT; i++) drawBurst(i, t);
}

// ================= Kitten =================

kitten::Pose petPose(uint32_t t) {
  kitten::Pose p = kitten::idlePose(t);
  p.eyes = kitten::Eyes::HAPPY;
  p.look = 0;
  p.mouth = (t >= anim::BDAY_FINALE_AT) ? kitten::Mouth::W
          : (((t / anim::BDAY_SING_MS) & 1) ? kitten::Mouth::SMALL_O : kitten::Mouth::OPEN);

  uint32_t u = t % anim::BDAY_HOP_MS;
  if (u < anim::BDAY_HOP_UP_MS) {
    int32_t q = (int32_t)u * 1000 / anim::BDAY_HOP_UP_MS;
    int32_t h = 4 * q * (1000 - q) / 1000;                       // 0..1000..0
    p.groundY = anim::KITTEN_GROUND_Y - (int32_t)anim::BDAY_HOP_PX * h / 1000;
  }
  return p;
}

// ================= Pesawat + banner =================

// Geser horizontal rombongan pada waktu t. Mengembalikan false kalau sedang tidak terlihat.
bool groupShift(uint32_t t, int16_t& s) {
  if (t < anim::BDAY_PLANE_IN_AT) return false;
  if (t < anim::BDAY_PLANE_IN_AT + anim::BDAY_PLANE_IN_MS) {      // masuk: melambat di akhir
    int32_t p = (int32_t)(t - anim::BDAY_PLANE_IN_AT) * 1000 / anim::BDAY_PLANE_IN_MS;
    int32_t e = 1000 - (1000 - p) * (1000 - p) / 1000;
    s = SHIFT_LEFT + (int32_t)(0 - SHIFT_LEFT) * e / 1000;
    return true;
  }
  if (t < anim::BDAY_PLANE_OUT_AT) { s = 0; return true; }
  if (t < anim::BDAY_PLANE_OUT_AT + anim::BDAY_PLANE_OUT_MS) {    // pergi: makin cepat
    int32_t p = (int32_t)(t - anim::BDAY_PLANE_OUT_AT) * 1000 / anim::BDAY_PLANE_OUT_MS;
    s = (int32_t)SHIFT_RIGHT * (p * p / 1000) / 1000;
    return true;
  }
  return false;
}

// Badan pesawat sebagai tabel garis {baris, kolom, lebar}; baris terakhir (kaca kokpit) digambar hitam.
struct PlaneRow { uint8_t y, x, w; };
const PlaneRow PLANE[] PROGMEM = {
  {0, 1, 3}, {1, 1, 4}, {2, 2, 4},                 // sirip ekor
  {3, 3, 15}, {4, 1, 19}, {5, 1, 19}, {6, 3, 15},  // badan
  {7, 8, 7}, {8, 9, 5},                            // sayap
  {4, 12, 3},                                      // kaca kokpit
};
constexpr uint8_t PLANE_ROWS = sizeof(PLANE) / sizeof(PLANE[0]);

void drawPlane(int16_t px, int16_t py, uint32_t t) {
  for (uint8_t i = 0; i < PLANE_ROWS; i++) {
    gfx::u8().setDrawColor(i < PLANE_ROWS - 1);
    cHLine(px + pgm_read_byte(&PLANE[i].x), py + pgm_read_byte(&PLANE[i].y), pgm_read_byte(&PLANE[i].w));
  }
  gfx::u8().setDrawColor(1);
  if ((t / 60) & 1) cVLine(px + PLANE_W - 1, py + 1, 8);   // baling-baling berkedip
  else              cVLine(px + PLANE_W - 1, py + 3, 4);
}

void drawText(int16_t x, int16_t baseline, const char* pgmText, bool bold) {
  U8G2& g = gfx::u8();
  g.setFontMode(1);                                // transparan, supaya huruf tebal tidak saling menghapus
  g.setDrawColor(0);
  for (uint8_t i = 0; ; i++) {
    char c = pgm_read_byte(&pgmText[i]);
    if (!c) break;
    int16_t cx = x + i * (bold ? NAME_STEP : GLYPH_W);
    cGlyph(cx, baseline, c);
    if (bold) cGlyph(cx + 1, baseline, c);
  }
  g.setDrawColor(1);
  g.setFontMode(0);
}

void drawBanner(int16_t s, uint32_t t, int16_t bob) {
  // Berbalik: tinggi mengecil sampai garis lalu membesar lagi, teks berganti di titik tengah
  int16_t h = BANNER_H;
  uint32_t flipEnd = (uint32_t)anim::BDAY_FLIP_AT + anim::BDAY_FLIP_MS;
  uint32_t half = anim::BDAY_FLIP_MS / 2;
  if (t >= anim::BDAY_FLIP_AT && t < flipEnd) {
    uint32_t p = t - anim::BDAY_FLIP_AT;
    uint32_t d = p < half ? half - p : p - half;                  // half..0..half
    h = 1 + (int32_t)(BANNER_H - 1) * (int32_t)d / (int32_t)half;
  }
  bool showName = t >= anim::BDAY_FLIP_AT + half;

  int16_t x = BANNER_X + s;
  int16_t cy = BANNER_CY + bob;
  int16_t y0 = cy - h / 2;

  gfx::u8().setDrawColor(1);
  cBox(x, y0, BANNER_W, h);
  if (h < TEXT_MIN_H) return;

  gfx::u8().setDrawColor(0);                                       // bingkai dalam
  cHLine(x + FRAME_INSET, y0 + FRAME_INSET, BANNER_W - 2 * FRAME_INSET);
  cHLine(x + FRAME_INSET, y0 + h - 1 - FRAME_INSET, BANNER_W - 2 * FRAME_INSET);
  cVLine(x + FRAME_INSET, y0 + FRAME_INSET, h - 2 * FRAME_INSET);
  cVLine(x + BANNER_W - 1 - FRAME_INSET, y0 + FRAME_INSET, h - 2 * FRAME_INSET);
  gfx::u8().setDrawColor(1);

  const char* txt = showName ? TXT_NAME : TXT_GREET;
  int16_t n = strlen_P(txt);
  int16_t textW = showName ? (n * NAME_STEP - (NAME_STEP - GLYPH_W - 1)) : n * GLYPH_W;
  drawText(x + (BANNER_W - textW) / 2, cy + GLYPH_ASCENT, txt, showName);
}

void drawGroup(uint32_t t) {
  int16_t s;
  if (!groupShift(t, s)) return;
  int16_t bob = ((t / anim::BDAY_BOB_MS) & 1) ? 1 : 0;

  gfx::u8().setDrawColor(1);
  cHLine(ROPE_X0 + s, BANNER_CY + bob, PLANE_X - ROPE_X0 + 2);       // tali
  drawBanner(s, t, bob);
  drawPlane(PLANE_X + s, BANNER_CY + bob - PLANE_ROW, t);
}

// ================= Finale =================

void drawHearts(uint32_t t) {
  for (uint8_t i = 0; i < HEART_N; i++) {
    int32_t since = (int32_t)t - anim::BDAY_FINALE_AT - (int32_t)i * anim::BDAY_HEART_GAP_MS;
    if (since < 0) continue;
    int32_t ti = since % anim::BDAY_HEART_PERIOD_MS;
    if (ti >= anim::BDAY_HEART_RISE_MS) continue;
    pet::drawHeart(HEART_X[i] + (HEART_X[i] > 64 ? 1 : -1) * (int16_t)(kitten::growth() / 2), anim::BDAY_HEART_Y - (int32_t)anim::BDAY_HEART_RISE_PX * ti / anim::BDAY_HEART_RISE_MS);
  }
}

// ================= Scene =================

void enter() {
  moodSet = false;
  sound::play(Sfx::BIRTHDAY);
}

void update(uint16_t dt, uint32_t t) {
  (void)dt;
  if (!moodSet && t >= anim::BDAY_FINALE_AT) {
    moodSet = true;
    pet::setMood(anim::MOOD_MAX);
  }
  if (t >= anim::BDAY_MS || (t >= anim::BDAY_SKIP_LOCK_MS && buttons::anyPressed())) app::go(SCENE_HOME);
}

void leave() {
  pet::setMood(anim::MOOD_MAX);   // ulang tahun selalu membuat mood penuh, juga kalau dilewati
  sound::stop();
}

void draw(uint32_t t) {
  drawFireworks(t);                 // paling belakang
  kitten::draw(petPose(t));
  drawGroup(t);                     // pesawat + banner menutupi kitten
  if (t >= anim::BDAY_FINALE_AT) drawHearts(t);
  pet::drawMood();
}

}  // namespace

const Scene sceneBirthday = { enter, update, draw, leave };
