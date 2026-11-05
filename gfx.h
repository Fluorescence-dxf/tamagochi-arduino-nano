#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "scene.h"

// Modul layar: membungkus U8g2 (page buffer mode, hemat RAM).
namespace gfx {
  void begin();
  U8G2& u8();                                   // akses U8g2 langsung dari scene
  void render(const Scene& s, uint32_t t);      // dipanggil oleh app

  // Fungsi yang dipanggil di antara halaman layar saat menggambar.
  // Satu frame butuh puluhan ms; hook ini menjaga tombol dan suara tetap dibaca.
  typedef void (*PageHook)();
  void setPageHook(PageHook hook);

  // Garis Bresenham sendiri (lebih kecil dari drawLine U8g2, dan sama dengan preview).
  void line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

  // Teks dari flash. Contoh: gfx::textF(0, 10, F("HALO"));
  void textF(int16_t x, int16_t y, const __FlashStringHelper* s);
  void textCenterF(int16_t cx, int16_t y, const __FlashStringHelper* s);
}
