// Scene MAKAN: kunigiri jatuh -> 3 gigitan (mengunyah) -> senang, hati melayang -> kembali ke home.
// Makan dihitung (pet::onFed) saat fase senang mulai. Tombol baru bisa melewati setelah itu.
// Semua angka waktu ada di anim_config.h.
#include "scenes.h"
#include "app.h"
#include "buttons.h"
#include "sound.h"
#include "gfx.h"
#include "kitten.h"
#include "pet.h"
#include "anim_config.h"

namespace {

// ---- Garis waktu (ms sejak scene mulai) ----
constexpr uint32_t T_LAND = anim::FEED_DROP_MS;
constexpr uint32_t T_DONE = T_LAND + (uint32_t)anim::FEED_BITES * anim::FEED_BITE_MS;
constexpr uint32_t T_END  = T_DONE + anim::FEED_DONE_MS;

// ---- Kunigiri: segitiga, ujung atas dimakan per gigitan ----
constexpr int16_t OG_X        = 64;    // tengah
constexpr int16_t OG_Y_START  = 2;     // atas, saat mulai jatuh
constexpr int16_t OG_Y_REST   = 38;    // ujung atas tepat di bawah mulut
constexpr uint8_t OG_H        = 15;    // tinggi (baris); dasar = lantai kaki kitten
constexpr uint8_t OG_HALF     = 8;     // setengah lebar di dasar
constexpr uint8_t OG_ROWS_PER_BITE = OG_H / anim::FEED_BITES;
constexpr uint8_t OG_NORI_HALF = 4;    // setengah lebar nori
constexpr uint8_t OG_NORI_H    = 5;    // tinggi nori (dari dasar)

// ---- Hati melayang (x posisi, di luar badan kitten) ----
const int16_t HEART_X[] = { 30, 90, 22 };
constexpr uint8_t HEART_N = sizeof(HEART_X) / sizeof(HEART_X[0]);

// ---- Kilau petunjuk makan ke-4 ----
struct Spot { uint8_t x, y; };
const Spot SPARKLES[] = { {34, 22}, {94, 18}, {98, 42} };

bool landed = false;
bool fed = false;
uint8_t bitesPlayed = 0;

// Jumlah gigitan yang sudah terjadi pada waktu t (fungsi murni, aman dipakai di draw)
uint8_t bitesTaken(uint32_t t) {
  if (t < T_LAND) return 0;
  uint32_t local = t - T_LAND;
  uint8_t k = local / anim::FEED_BITE_MS;
  if (k >= anim::FEED_BITES) return anim::FEED_BITES;
  return k + ((local % anim::FEED_BITE_MS) >= anim::FEED_BITE_CUT_MS ? 1 : 0);
}

void enter() {
  landed = false;
  fed = false;
  bitesPlayed = 0;
  sound::play(Sfx::FALL);
}

void update(uint16_t dt, uint32_t t) {
  (void)dt;
  if (!landed && t >= T_LAND) { landed = true; sound::play(Sfx::PLOP); }

  uint8_t taken = bitesTaken(t);
  if (taken > bitesPlayed) { bitesPlayed = taken; sound::play(Sfx::NOM); }

  if (!fed && t >= T_DONE) {
    fed = true;
    pet::onFed();
    sound::play(Sfx::YUM);
  }

  if (t >= T_END || (t >= T_DONE && buttons::anyPressed())) {
    app::go(pet::grown() && kitten::growth() == 0 ? SCENE_EVOLVE : SCENE_HOME);   // cukup makan -> berevolusi
  }
}

void leave() { sound::stop(); }

// y = baris paling atas kunigiri, cut = jumlah baris atas yang sudah dimakan
void drawOnigiri(int16_t y, uint8_t cut) {
  if (cut >= OG_H) return;
  U8G2& g = gfx::u8();

  // Halo hitam 1 px supaya terlihat di atas badan putih kitten
  g.setDrawColor(0);
  for (int16_t r = cut - 1; r <= OG_H; r++) {
    int16_t rr = r < cut ? cut : (r >= OG_H ? OG_H - 1 : r);
    int16_t half = (rr * OG_HALF) / (OG_H - 1) + 1;
    g.drawHLine(OG_X - half, y + r, 2 * half + 1);
  }

  // Nasi kuning: tepi putih 1 px + isi dither (kotak-kotak), baris paling atas utuh
  g.setDrawColor(1);
  for (int16_t r = cut; r < OG_H; r++) {
    int16_t half = (r * OG_HALF) / (OG_H - 1);
    g.drawHLine(OG_X - half, y + r, 2 * half + 1);
  }
  g.setDrawColor(0);
  for (int16_t r = cut + 1; r < OG_H; r++) {
    int16_t half = (r * OG_HALF) / (OG_H - 1);
    for (int16_t c = -half + 1; c < half; c++) {
      if (((c + r) & 1) == 0) g.drawPixel(OG_X + c, y + r);
    }
  }

  // Nori: pita hitam padat di dasar
  for (int16_t r = OG_H - OG_NORI_H; r < OG_H; r++) {
    if (r > cut) g.drawHLine(OG_X - OG_NORI_HALF, y + r, 2 * OG_NORI_HALF + 1);
  }
  g.setDrawColor(1);
}

int16_t onigiriY(uint32_t t) {
  if (t >= T_LAND) return OG_Y_REST;
  int32_t p = (int32_t)t * 1000 / anim::FEED_DROP_MS;          // 0..1000
  return OG_Y_START + (int32_t)(OG_Y_REST - OG_Y_START) * p / 1000 * p / 1000;   // jatuh makin cepat
}

kitten::Pose poseAt(uint32_t t) {
  kitten::Pose p = kitten::idlePose(t);
  if (t < T_LAND) {                                  // melongo melihat makanan jatuh
    p.eyes = kitten::Eyes::OPEN;
    p.mouth = kitten::Mouth::SMALL_O;
    p.look = 0;
  } else if (t < T_DONE) {                           // gigit dan kunyah
    uint32_t local = (t - T_LAND) % anim::FEED_BITE_MS;
    if (local < anim::FEED_BITE_CUT_MS) {
      p.eyes = kitten::Eyes::OPEN;
      p.mouth = kitten::Mouth::OPEN;
    } else {
      uint8_t phase = ((local - anim::FEED_BITE_CUT_MS) / anim::FEED_CHEW_MS) & 1;
      p.eyes = kitten::Eyes::HAPPY;
      p.mouth = phase ? kitten::Mouth::SMALL_O : kitten::Mouth::OPEN;
      p.legRows = anim::KITTEN_LEGS_MIN + phase;     // badan naik-turun mengikuti kunyahan
    }
    p.look = 0;
  } else {                                           // senang, lompat 2x
    p.eyes = kitten::Eyes::HAPPY;
    p.mouth = kitten::Mouth::W;
    p.look = 0;
    uint32_t td = t - T_DONE;
    if (td < 2UL * anim::FEED_HOP_MS) {
      int32_t u = (int32_t)(td % anim::FEED_HOP_MS) * 1000 / anim::FEED_HOP_MS;
      int32_t h = 4 * u * (1000 - u) / 1000;         // 0..1000..0
      p.groundY = anim::KITTEN_GROUND_Y - (int32_t)anim::FEED_HOP_PX * h / 1000;
    }
  }
  return p;
}

void drawSparkle(uint8_t x, uint8_t y, bool big) {
  U8G2& g = gfx::u8();
  g.drawPixel(x, y);
  if (big) {
    g.drawPixel(x - 1, y); g.drawPixel(x + 1, y);
    g.drawPixel(x, y - 1); g.drawPixel(x, y + 1);
  }
}

void drawDoneEffects(uint32_t t) {
  uint32_t td = t - T_DONE;
  for (uint8_t i = 0; i < HEART_N; i++) {
    int32_t ti = (int32_t)td - (int32_t)i * anim::FEED_HEART_GAP_MS;
    if (ti < 0 || ti >= anim::FEED_HEART_RISE_MS) continue;
    int16_t y = anim::FEED_HEART_Y - (int32_t)anim::FEED_HEART_RISE_PX * ti / anim::FEED_HEART_RISE_MS;
    pet::drawHeart(HEART_X[i] + (HEART_X[i] > 64 ? 1 : -1) * (int16_t)(kitten::growth() / 2), y);   // ikut lebar kitten
  }
  // Petunjuk halus: setelah makan ke-4 muncul kilau (tanpa angka)
  if (pet::feeds() == anim::FEEDS_TO_GROW - 1) {
    uint8_t beat = td / anim::FEED_SPARKLE_MS;
    for (uint8_t i = 0; i < sizeof(SPARKLES) / sizeof(SPARKLES[0]); i++) {
      drawSparkle(SPARKLES[i].x, SPARKLES[i].y, ((beat + i) & 1) == 0);
    }
  }
}

void draw(uint32_t t) {
  kitten::draw(poseAt(t));
  if (t < T_DONE) drawOnigiri(onigiriY(t), bitesTaken(t) * OG_ROWS_PER_BITE);
  else drawDoneEffects(t);
  pet::drawMood();
}

}  // namespace

const Scene sceneFeed = { enter, update, draw, leave };
