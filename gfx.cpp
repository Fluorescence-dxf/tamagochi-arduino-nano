#include "gfx.h"
#include "config.h"
#include <avr/pgmspace.h>

namespace {

// "_1_" = page buffer (128 byte RAM). Kalau layarmu SH1106, ganti SSD1306_128X64_NONAME
// menjadi SH1106_128X64_NONAME.
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

constexpr uint8_t TEXT_BUF = 24;

gfx::PageHook pageHook = nullptr;

void copyFlash(char* dst, const __FlashStringHelper* src) {
  strncpy_P(dst, (PGM_P)src, TEXT_BUF - 1);
  dst[TEXT_BUF - 1] = 0;
}

}  // namespace

void gfx::begin() {
  u8g2.begin();
  u8g2.setBusClock(cfg::I2C_HZ);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.setFontPosBaseline();
}

U8G2& gfx::u8() { return u8g2; }

void gfx::setPageHook(PageHook hook) { pageHook = hook; }

void gfx::render(const Scene& s, uint32_t t) {
  if (!s.draw) return;
  u8g2.firstPage();
  do {
    s.draw(t);
    if (pageHook) pageHook();     // dipanggil 8x per frame (satu per halaman)
  } while (u8g2.nextPage());
}

void gfx::textF(int16_t x, int16_t y, const __FlashStringHelper* s) {
  char b[TEXT_BUF];
  copyFlash(b, s);
  u8g2.drawStr(x, y, b);
}

void gfx::textCenterF(int16_t cx, int16_t y, const __FlashStringHelper* s) {
  char b[TEXT_BUF];
  copyFlash(b, s);
  int16_t w = u8g2.getStrWidth(b);
  u8g2.drawStr(cx - w / 2, y, b);
}

void gfx::line(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t dx = x1 > x0 ? x1 - x0 : x0 - x1;
  int16_t dy = y1 > y0 ? y0 - y1 : y1 - y0;          // negatif
  int16_t sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
  int16_t e = dx + dy;
  for (;;) {
    u8g2.drawPixel(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    int16_t e2 = 2 * e;
    if (e2 >= dy) { e += dy; x0 += sx; }
    if (e2 <= dx) { e += dx; y0 += sy; }
  }
}
