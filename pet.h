#pragma once
#include <Arduino.h>

// Status kitten yang dipakai banyak scene: mood dan jumlah makan.
// Semua di RAM, reset tiap boot (sesuai rancangan: tanpa penyimpanan).
namespace pet {
  constexpr uint8_t HEART_W = 7;   // ukuran ikon hati (px)
  constexpr uint8_t HEART_H = 6;

  uint8_t mood();                 // 0..anim::MOOD_MAX
  void    addMood(int8_t delta);  // otomatis dibatasi 0..MOOD_MAX
  void    setMood(uint8_t m);

  uint8_t feeds();                // berapa kali makan sejak boot
  void    onFed();                // dipanggil scene makan: feeds+1, mood+1
  bool    grown();                // true kalau sudah makan FEEDS_TO_GROW kali

  bool    hungry();               // true = minta makan (diset saat game selesai, dihapus onFed)
  void    setHungry(bool h);

  void drawMood();                // 3 hati di kiri atas
  void drawHeart(int16_t x, int16_t y);   // satu hati penuh (7x6), untuk efek
}
