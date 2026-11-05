// Virtual Pet Chibi Kitten (Tamagotchi) - Arduino Nano + OLED SSD1306 128x64 I2C.
// Library yang dibutuhkan: U8g2 (install melalui Arduino Library Manager).
// Konfigurasi pin, teks banner, dan timing dapat diubah di config.h.
#include "app.h"

void setup() {
  app::begin();
}

void loop() {
  app::loop();
}
