#pragma once
#include <Arduino.h>

// Bagian yang sama untuk semua game: HUD (skor + bar waktu), petunjuk awal, dan layar hasil
// (kitten kelelahan -> lapar). Game baru cukup memanggil fungsi di sini.
namespace game {
  // Ukuran sprite (px). Hati dan bom sengaja sama lebar tapi beda bentuk:
  // hati = padat putih, bom = cincin kosong + tanda X + sumbu berkedip.
  constexpr uint8_t ITEM_W = 9;
  constexpr uint8_t HEART_H = 8;
  constexpr uint8_t BOMB_H = 12;
  constexpr uint8_t CAT_W = 13;
  constexpr uint8_t CAT_H = 10;

  void drawItemHeart(int16_t x, int16_t y);            // x, y = kiri atas
  void drawBomb(int16_t x, int16_t y, uint32_t t);     // t untuk kedip sumbu
  void drawCatHead(int16_t cx, int16_t y);             // cx = tengah, y = atas

  // Bar waktu menyusut (mulai di tPlay, habis di tEnd) dan skor di kiri atas.
  void drawHud(uint8_t score, uint32_t t, uint32_t tPlay, uint32_t tEnd);

  // Judul + petunjuk tombol di tengah layar (dipakai selama intro).
  void drawIntro(const __FlashStringHelper* title);

  // Panggil sekali saat permainan habis: kitten jadi lapar, mood +1 kalau skor bagus, bunyi.
  void startResult(uint8_t score);

  // rt = ms sejak hasil mulai.
  void drawResult(uint32_t rt, uint8_t score);
  bool resultDone(uint32_t rt);     // true setelah waktunya habis, atau tombol ditekan (setelah jeda singkat)
}
