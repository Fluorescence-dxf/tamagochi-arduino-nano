#pragma once
#include <Arduino.h>

// Semua konfigurasi hardware, waktu, dan pesan kustom yang dapat diubah ada di file ini.
namespace cfg {

  // ---- Pin Konfigurasi ----
  constexpr uint8_t BTN_PINS[3] = { 2, 3, 4 };   // Tombol 1 (Kiri), 2 (Tengah), 3 (Kanan) -> Satu kaki ke PIN, kaki lain ke GND
  constexpr uint8_t BUZZER_PIN  = 5;             // Pin Piezo Buzzer (lewat resistor 100-220 ohm ke GND)

  // ---- Layar OLED (SSD1306 128x64 I2C) ----
  // Pin I2C default Arduino Nano: A4 (SDA), A5 (SCL)
  constexpr uint8_t  SCREEN_W = 128;
  constexpr uint8_t  SCREEN_H = 64;
  constexpr uint32_t I2C_HZ   = 400000;

  // ---- Teks Kustom Hadiah / Ulang Tahun ----
  // Banner pesawat akan menampilkan ucapan dan nama kustom (disimpan di PROGMEM).
  // Disarankan maks 10-12 karakter agar pas di tengah banner.
  #define PET_GIFT_GREET "HAPPY BIRTHDAY"
  #define PET_GIFT_NAME  "MY LOVE <3"

  // ---- Waktu & Kontrol Animasi ----
  constexpr uint16_t FRAME_MS      = 66;     // ~15 FPS
  constexpr uint16_t DEBOUNCE_MS   = 25;
  constexpr uint16_t LONG_PRESS_MS = 2000;   // Tahan tombol selama ini = "long press"
  constexpr uint16_t TOAST_MS      = 1200;   // Lama notifikasi singkat (mis. Mute/Unmute)

  // ---- Boot ----
  constexpr uint16_t BOOT_MS = 4400;

  // ---- Debug ----
  constexpr bool SHOW_FPS = false;           // Tampilkan FPS di pojok kiri atas menu utama
}
