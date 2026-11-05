#pragma once
#include <Arduino.h>

// Satu nada. freq = 0 berarti jeda (diam) selama ms.
struct Note {
  uint16_t freq;
  uint16_t ms;
};

// Daftar efek suara. Untuk menambah efek baru:
//   1. tambah nama di sini
//   2. tambah tabel nada di sound.cpp
//   3. tambah satu baris di TABLE (urutannya harus sama dengan enum ini)
enum class Sfx : uint8_t { CLICK, BOOT, MEONG, SNORE, FALL, PLOP, NOM, YUM, BIRTHDAY, CATCH, TIRED, BONK, EVOLVE, COUNT };

// Modul suara: memutar urutan nada tanpa memblokir program.
namespace sound {
  void begin();
  void update(uint32_t now);   // panggil tiap loop()
  void play(Sfx id);           // memutus suara yang sedang jalan
  void stop();
  void setMuted(bool m);
  bool muted();
  void toggleMute();
}
