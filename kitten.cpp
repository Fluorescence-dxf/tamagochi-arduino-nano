#include "kitten.h"
#include "anim_config.h"
#include "gfx.h"
#include <avr/pgmspace.h>

// CATATAN: di Nano, koordinat U8g2 bertipe uint8_t. Jangan menggambar di koordinat
// negatif (misalnya centerX < 25): hasilnya terpotong, bukan tergeser.

namespace kitten {
namespace {

// ============================================================
//  BENTUK KITTEN (ubah angka di bawah untuk mengubah siluet)
//  Semua dalam piksel. Kolom 0 = tepi kiri sprite. Sisi kanan = cermin sisi kiri.
// ============================================================

constexpr uint8_t HALF_W    = 19;               // lebar setengah sprite
constexpr uint8_t FULL_W    = 2 * HALF_W;       // 38 piksel (+ extraCols di tengah)
constexpr uint8_t EDGE_NONE = 255;              // baris kosong

// Siluet kepala + bahu: satu angka per baris = kolom paling kiri yang terisi putih.
const uint8_t TOP_EDGE[] PROGMEM = {
  EDGE_NONE, EDGE_NONE, EDGE_NONE, EDGE_NONE, EDGE_NONE,   // 0-4   area telinga (digambar terpisah)
  4, 3,                                                    // 5-6   puncak kepala
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,    // 7-24  kepala selebar penuh
  3, 4, 5, 7, 10, 13,                                      // 25-30 pipi menyempit ke dagu
  EDGE_NONE,                                               // 31    celah kepala dan badan
  10, 8, 8                                                 // 32-34 bahu
};
constexpr uint8_t TOP_ROWS  = sizeof(TOP_EDGE);
// Dua kaki depan dipisah celah gelap di tengah. Tiap kaki = kolom LEG_OUTER..LEG_INNER
// (diulang legRows kali); telapak = 1 baris terakhir, FOOT_OUTER..FOOT_INNER.
constexpr uint8_t LEG_OUTER  = 8;
constexpr uint8_t LEG_INNER  = 17;
constexpr uint8_t FOOT_OUTER = 9;
constexpr uint8_t FOOT_INNER = 16;

// Telinga (5 baris): kolom paling kiri dan paling kanan per baris.
// Kiri = abu-abu (kotak-kotak), kanan = putih dengan bagian dalam gelap.
constexpr uint8_t EAR_ROWS = 5;
const uint8_t EAR_LO[] PROGMEM = { 6, 5, 4, 4, 3 };
const uint8_t EAR_HI[] PROGMEM = { 7, 8, 9, 10, 11 };

// Ekor (17 baris, dari ujung ke pangkal): kolom kiri dan kanan per baris.
// 5 baris teratas (ujung) abu-abu. Pangkal ada di kanan bawah badan.
constexpr uint8_t TAIL_ROWS      = 17;
constexpr uint8_t TAIL_TIP_ROWS  = 5;
constexpr uint8_t TAIL_BASE_X    = 28;          // posisi pangkal ekor dari tepi kiri sprite
const uint8_t TAIL_LO[] PROGMEM = { 11, 10, 10, 10, 10, 10, 10, 10, 10, 9, 7, 5, 3, 1, 0, 0, 0 };
const uint8_t TAIL_HI[] PROGMEM = { 12, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 11, 10, 8, 6, 4, 3 };

// Baris kepala di TOP_EDGE (baris 0-30). Baris 31 ke bawah = celah dan badan duduk.
constexpr uint8_t HEAD_ROWS = 31;

// ---- Pose meringkuk (tidur) ----
// Kepala = sprite yang sama (tanpa badan duduk). Badan = oval, ekor = pita di depan badan.
constexpr int8_t  CURL_HEAD_DX   = -18;    // pusat kepala dari centerX
constexpr int8_t  CURL_BODY_DX   = 12;     // pusat badan dari centerX
constexpr uint8_t CURL_BODY_RX   = 26;     // setengah lebar badan
constexpr uint8_t CURL_BODY_RY   = 10;     // setengah tinggi badan (+1 saat napas naik)
constexpr uint8_t CURL_TAIL_ROWS = 4;      // tebal ekor
constexpr uint8_t CURL_TAIL_TIP  = 8;      // kolom paling kiri ekor yang abu-abu
// Tepi kiri dan kanan ekor per baris (dari pusat badan), baris teratas dulu.
const int8_t CURL_TAIL_L[] PROGMEM = { -18, -22, -22, -20 };
const int8_t CURL_TAIL_R[] PROGMEM = {  20,  22,  23,  22 };

// ============================================================
//  PENGGAMBAR (tidak perlu diubah untuk mengganti bentuk)
// ============================================================

U8G2*   g;        // layar yang sedang dipakai
int16_t ox, oy;   // pojok kiri atas sprite di layar
int16_t mx;       // kolom terakhir sprite (lebar - 1), untuk mencerminkan
int16_t csh;      // geser fitur tengah wajah (= extraCols / 2)
uint8_t growthCols = 0;   // wujud saat ini: 0 = kecil, >0 = lebar tambahan (besar)

inline uint8_t rom(const uint8_t* table, uint8_t i) { return pgm_read_byte(&table[i]); }

inline void dot(int16_t c, int16_t r)       { g->drawPixel(ox + c, oy + r); }
inline void dotMirror(int16_t c, int16_t r) { dot(c, r); dot(mx - c, r); }

// Fitur tengah wajah (dahi, hidung, mulut) tetap selebar sprite kecil dan berada di tengah,
// supaya tidak melar saat badan melebar. Mata, pipi, dan kumis tetap menempel ke sisi.
constexpr int16_t SMALL_MX = FULL_W - 1;

void drawTail(int8_t swing, uint8_t totalRows) {
  int16_t tx = ox + TAIL_BASE_X + (mx - (FULL_W - 1));   // ikut bergeser kalau badan melebar
  int16_t ty = oy + totalRows - TAIL_ROWS;
  for (uint8_t j = 0; j < TAIL_ROWS; j++) {
    // ujung ekor bergeser paling jauh; pembulatan 0,5 ke atas (integer, tanpa float)
    int16_t dx = ((int16_t)swing * (TAIL_ROWS - 1 - j) + 8 + 160) / 16 - 10;
    int16_t lo = tx + rom(TAIL_LO, j) + dx;
    int16_t hi = tx + rom(TAIL_HI, j) + dx;
    int16_t y  = ty + j;
    if (j >= TAIL_TIP_ROWS) {
      g->drawHLine(lo, y, hi - lo + 1);                         // badan ekor: satu garis per baris
    } else {
      for (int16_t x = lo; x <= hi; x++)                        // ujung ekor: kotak-kotak
        if (((x + y) & 1) == 0) g->drawPixel(x, y);
    }
  }
}

void drawEars(const Pose& p) {
  for (uint8_t r = 0; r < EAR_ROWS; r++) {
    int16_t yl = oy + r + p.earLeftDrop;
    int16_t yr = oy + r + p.earRightDrop;
    uint8_t lo = rom(EAR_LO, r), hi = rom(EAR_HI, r);
    for (uint8_t c = lo; c <= hi; c++)                          // kiri: kotak-kotak
      if (((ox + c + yl) & 1) == 0) g->drawPixel(ox + c, yl);
    g->drawHLine(ox + mx - hi, yr, hi - lo + 1);                // kanan: putih penuh
  }
  g->setDrawColor(0);
  g->drawBox(ox + mx - 7, oy + 2 + p.earRightDrop, 2, 2);       // bagian dalam telinga kanan
  g->setDrawColor(1);
}

// Kepala saja (baris 0-30), tanpa badan duduk.
void drawHeadRows() {
  int16_t w = mx + 1;
  for (uint8_t r = 0; r < HEAD_ROWS; r++) {
    uint8_t c = rom(TOP_EDGE, r);
    if (c != EDGE_NONE) g->drawHLine(ox + c, oy + r, w - 2 * c);
  }
}

// Satu baris kaki: kaki kiri (outer..inner) dan kaki kanan (cerminannya).
void twoLegs(int16_t y, uint8_t outer, uint8_t inner) {
  uint8_t len = inner - outer + 1;
  g->drawHLine(ox + outer, y, len);
  g->drawHLine(ox + mx - inner, y, len);
}

void drawSilhouette(uint8_t legRows) {
  int16_t w = mx + 1;
  int16_t y = oy;
  for (uint8_t r = 0; r < TOP_ROWS; r++, y++) {
    uint8_t c = rom(TOP_EDGE, r);
    if (c != EDGE_NONE) g->drawHLine(ox + c, y, w - 2 * c);
  }
  for (uint8_t i = 0; i < legRows; i++, y++) twoLegs(y, LEG_OUTER, LEG_INNER);
  twoLegs(y, FOOT_OUTER, FOOT_INNER);
}

// ---- Wajah sebagai data ----
// Tiap operasi 3 byte: [putih<<7 | jenis<<5 | baris], a, w. Data jauh lebih kecil daripada
// ratusan pemanggilan gambar. Urutan operasi = urutan gambar (hitam menimpa putih).
//   PIX_M  : piksel di kolom a dan pantulannya (menempel ke sisi sprite, ikut melebar)
//   PIX_C  : piksel di kolom a dan pantulannya (fitur tengah, tetap selebar sprite kecil)
//   SPAN_C : garis dari kolom a sampai pantulannya (fitur tengah)
//   HL_M   : garis selebar w di kolom a, dan pantulannya
enum : uint8_t { PIX_M, PIX_C, SPAN_C, HL_M };
struct Op { uint8_t k, a, w; };
constexpr Op mk(uint8_t kind, uint8_t r, uint8_t a, uint8_t w, uint8_t white) {
  return Op{ (uint8_t)((white << 7) | (kind << 5) | r), a, w };
}
constexpr Op pixM(uint8_t r, uint8_t a, uint8_t white = 0)            { return mk(PIX_M, r, a, 0, white); }
constexpr Op pixC(uint8_t r, uint8_t a, uint8_t white = 0)            { return mk(PIX_C, r, a, 0, white); }
constexpr Op spanC(uint8_t r, uint8_t a, uint8_t white = 0)           { return mk(SPAN_C, r, a, 0, white); }
constexpr Op hlM(uint8_t r, uint8_t a, uint8_t w, uint8_t white = 0)  { return mk(HL_M, r, a, w, white); }

// Bercak hati di dahi + pipi merona (kotak-kotak)
const Op FACE_TOP[] PROGMEM = {
  pixC(7, 16), pixC(7, 17), spanC(8, 16), spanC(9, 17), spanC(10, 18),
  pixM(19, 5), pixM(19, 7), pixM(20, 4), pixM(20, 6), pixM(20, 8), pixM(21, 5), pixM(21, 7),
};
// Mata terbuka (dasar gelap, kilau putih) : dipakai OPEN dan SAD
const Op EYES_OPEN[] PROGMEM = {
  hlM(12, 10, 3), hlM(13, 9, 5), hlM(14, 9, 5), hlM(15, 9, 5), hlM(16, 9, 5), hlM(17, 9, 5), hlM(18, 10, 3),
  hlM(13, 10, 2, 1), hlM(14, 10, 2, 1), pixM(17, 12, 1),
};
const Op EYES_HAPPY[]  PROGMEM = { hlM(15, 10, 3), pixM(16, 9), pixM(16, 13) };           // ^ ^
const Op EYES_CLOSED[] PROGMEM = { hlM(16, 9, 5) };
const Op EYES_SLEEP[]  PROGMEM = { pixM(15, 9), pixM(15, 13), hlM(16, 10, 3) };           // U santai
// Hidung + garis atas mulut
const Op NOSE[] PROGMEM = { spanC(19, 17), spanC(20, 18), spanC(21, 18) };
const Op MOUTH_W[]       PROGMEM = { pixC(22, 15), pixC(23, 16), pixC(23, 17), pixC(22, 18) };
const Op MOUTH_SMALL_O[] PROGMEM = { spanC(22, 17), spanC(23, 17), spanC(24, 17) };
const Op MOUTH_OPEN[]    PROGMEM = { spanC(22, 16), spanC(23, 16), spanC(24, 16), spanC(24, 18, 1) };   // baris bawah putih (lidah)
const Op MOUTH_FROWN[]   PROGMEM = { spanC(23, 17), pixC(24, 16) };

#define OPS(arr) arr, (uint8_t)(sizeof(arr) / sizeof(Op))

void runOps(const Op* ops, uint8_t n, int16_t dx = 0, int16_t dy = 0) {
  for (; n; n--, ops++) {
    uint8_t k = pgm_read_byte(&ops->k), a = pgm_read_byte(&ops->a), w = pgm_read_byte(&ops->w);
    int16_t y = oy + (k & 31) + dy;
    g->setDrawColor(k >> 7);
    switch ((k >> 5) & 3) {
      case PIX_M:
        g->drawPixel(ox + a + dx, y);
        g->drawPixel(ox + mx - a + dx, y);
        break;
      case PIX_C:
        g->drawPixel(ox + csh + a, y);
        g->drawPixel(ox + csh + SMALL_MX - a, y);
        break;
      case SPAN_C:
        g->drawHLine(ox + csh + a, y, SMALL_MX - 2 * a + 1);
        break;
      default:   // HL_M
        g->drawHLine(ox + a + dx, y, w);
        g->drawHLine(ox + mx - a - w + 1 + dx, y, w);
        break;
    }
  }
}

void drawEyes(const Pose& p) {
  switch (p.eyes) {
    case Eyes::OPEN:
    case Eyes::SAD: {
      uint8_t sh = (p.eyes == Eyes::SAD) ? 1 : 0;   // mata sedih turun 1 px
      runOps(OPS(EYES_OPEN), p.look, sh);
      g->setDrawColor(0);
      if (p.eyes == Eyes::SAD) {                    // alis miring
        gfx::line(ox + 8, oy + 12, ox + 13, oy + 9);
        gfx::line(ox + mx - 8, oy + 12, ox + mx - 13, oy + 9);
      }
      break;
    }
    case Eyes::HAPPY:  runOps(OPS(EYES_HAPPY));  break;
    case Eyes::CLOSED: runOps(OPS(EYES_CLOSED)); break;
    case Eyes::SLEEP:  runOps(OPS(EYES_SLEEP));  break;
  }
}

void drawMouth(Mouth m) {
  runOps(OPS(NOSE));
  switch (m) {
    case Mouth::W:       runOps(OPS(MOUTH_W));       break;
    case Mouth::SMALL_O: runOps(OPS(MOUTH_SMALL_O)); break;
    case Mouth::OPEN:    runOps(OPS(MOUTH_OPEN));    break;
    case Mouth::FROWN:   runOps(OPS(MOUTH_FROWN));   break;
  }
}

void drawFace(const Pose& p, bool toes) {
  runOps(OPS(FACE_TOP));
  drawEyes(p);
  drawMouth(p.mouth);

  g->setDrawColor(1);                               // kumis: dua baris, kiri dan kanan
  for (uint8_t i = 0; i < 2; i++) {
    int16_t y0 = oy + 19 + 3 * i, y1 = oy + 18 + 5 * i;
    gfx::line(ox + 1, y0, ox - 5, y1);
    gfx::line(ox + mx - 1, y0, ox + mx + 5, y1);
  }

  if (toes) {                                       // garis jari kaki (hanya pose duduk)
    g->setDrawColor(0);
    for (uint8_t i = 0; i < 2; i++) {
      int16_t r = TOP_ROWS + p.legRows - 1 - i;
      dotMirror(11, r);
      dotMirror(14, r);
    }
  }
  g->setDrawColor(1);
}

// Akar kuadrat bilangan bulat (untuk bentuk oval tanpa float).
uint8_t isqrt(uint16_t v) {
  uint8_t r = 0;
  while ((uint16_t)((r + 1) * (r + 1)) <= v) r++;
  return r;
}

// Badan oval + ekor melingkar di depannya.
void drawCurledBody(int16_t cx, int16_t groundY, uint8_t breath) {
  uint8_t ry = CURL_BODY_RY + breath;
  int16_t bx = cx + CURL_BODY_DX;
  int16_t cy = groundY - ry;                        // dasar badan tepat di groundY

  for (int8_t dy = -(int8_t)ry; dy <= (int8_t)ry; dy++) {
    uint16_t d2 = (uint16_t)(ry * ry + 1 - dy * dy);
    uint8_t hw = (uint8_t)(((uint16_t)CURL_BODY_RX * isqrt(d2)) / ry);
    g->drawHLine(bx - hw, cy + dy, 2 * hw + 1);
  }

  // garis gelap tipis di atas ekor supaya ekor terpisah dari badan
  int16_t l0 = bx + (int8_t)rom((const uint8_t*)CURL_TAIL_L, 0);
  int16_t r0 = bx + (int8_t)rom((const uint8_t*)CURL_TAIL_R, 0);
  int16_t y0 = groundY - CURL_TAIL_ROWS;
  g->setDrawColor(0);
  g->drawHLine(l0 + 3, y0, r0 - l0 - 5);
  g->setDrawColor(1);

  for (uint8_t i = 0; i < CURL_TAIL_ROWS; i++) {
    int16_t y = y0 + 1 + i;
    int16_t l = bx + (int8_t)rom((const uint8_t*)CURL_TAIL_L, i);
    int16_t r = bx + (int8_t)rom((const uint8_t*)CURL_TAIL_R, i);
    for (int16_t x = l; x < l + CURL_TAIL_TIP; x++)  // ujung ekor: kotak-kotak
      if (((x + y) & 1) == 0) g->drawPixel(x, y);
    g->drawHLine(l + CURL_TAIL_TIP, y, r - l - CURL_TAIL_TIP + 1);
  }
}

}  // namespace

// ============================================================
//  API PUBLIK
// ============================================================

uint8_t growth() { return growthCols; }
void setGrowth(uint8_t extraCols) { growthCols = extraCols; }

Pose basePose() {
  Pose p;
  p.centerX      = anim::KITTEN_CENTER_X;
  p.groundY      = anim::KITTEN_GROUND_Y;
  p.legRows      = anim::KITTEN_LEGS_MIN;
  p.extraCols    = growthCols;
  p.tailSwing    = 0;
  p.look         = 0;
  p.earLeftDrop  = 0;
  p.earRightDrop = 0;
  p.eyes         = Eyes::OPEN;
  p.mouth        = Mouth::W;
  return p;
}

Pose idlePose(uint32_t t) {
  static const int8_t SWING[4] = { -2, 0, 2, 0 };
  static const int8_t LOOK[4]  = { 0, -1, 0, 1 };

  Pose p = basePose();
  p.legRows   = anim::KITTEN_LEGS_MIN + (uint8_t)((t / anim::IDLE_BREATH_MS) & 1);
  p.tailSwing = SWING[(t / anim::IDLE_TAIL_STEP_MS) & 3];
  p.look      = LOOK[(t / anim::IDLE_LOOK_STEP_MS) & 3];
  if ((t % anim::IDLE_BLINK_PERIOD_MS) < anim::IDLE_BLINK_MS) p.eyes = Eyes::CLOSED;

  uint16_t ear = t % anim::IDLE_EAR_PERIOD_MS;
  if (ear >= anim::IDLE_EAR_AT_MS && ear < anim::IDLE_EAR_AT_MS + anim::IDLE_EAR_MS) p.earLeftDrop = 1;
  return p;
}

void draw(const Pose& p) {
  g = &gfx::u8();
  uint8_t totalRows = TOP_ROWS + p.legRows + 1;
  uint8_t width     = FULL_W + p.extraCols;
  mx = width - 1;
  csh = p.extraCols / 2;
  ox = p.centerX - width / 2;
  oy = p.groundY - (totalRows - 1);

  g->setDrawColor(1);
  drawTail(p.tailSwing, totalRows);
  drawEars(p);
  drawSilhouette(p.legRows);
  drawFace(p, true);
  g->setDrawColor(1);
}

void drawCurled(uint8_t centerX, uint8_t groundY, uint8_t breath, Eyes eyes, Mouth mouth) {
  g = &gfx::u8();
  g->setDrawColor(1);
  drawCurledBody(centerX, groundY, breath);

  mx = FULL_W - 1;
  csh = 0;
  ox = centerX + CURL_HEAD_DX - FULL_W / 2;
  oy = groundY - (HEAD_ROWS - 1);                   // dagu tepat di groundY

  // halo gelap 1 px di sekeliling kepala supaya terpisah dari badan
  static const int8_t HALO[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
  int16_t sx = ox, sy = oy;
  g->setDrawColor(0);
  for (uint8_t k = 0; k < 4; k++) {
    ox = sx + HALO[k][0];
    oy = sy + HALO[k][1];
    drawHeadRows();
  }
  ox = sx;
  oy = sy;
  g->setDrawColor(1);

  Pose p = basePose();
  p.eyes  = eyes;
  p.mouth = mouth;
  drawEars(p);
  drawHeadRows();
  drawFace(p, false);
  g->setDrawColor(1);
}

}  // namespace kitten
