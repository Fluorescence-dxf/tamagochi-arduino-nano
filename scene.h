#pragma once
#include <stdint.h>

// Satu "scene" = satu layar/mode (boot, menu utama, game, dst).
// Isi fungsi yang kamu butuhkan, sisanya boleh nullptr.
//
//   enter()        dipanggil sekali saat scene mulai
//   update(dt, t)  dipanggil tiap frame. Taruh logika di sini (ubah state, baca tombol).
//                  dt = ms sejak frame lalu, t = ms sejak scene mulai
//   draw(t)        menggambar. JANGAN ubah state di sini: fungsi ini dipanggil
//                  beberapa kali per frame (U8g2 page buffer).
//   leave()        dipanggil sekali saat scene berakhir
struct Scene {
  void (*enter)();
  void (*update)(uint16_t dt, uint32_t t);
  void (*draw)(uint32_t t);
  void (*leave)();
};
