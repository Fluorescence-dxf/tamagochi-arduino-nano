#pragma once
#include <Arduino.h>

// Jangan beri nama B1/B2/...: itu makro bawaan Arduino (angka biner) dan membuat compile gagal.
enum class Btn : uint8_t { BTN1, BTN2, BTN3, COUNT };

// Modul tombol: debounce + deteksi tekan dan lepas (tahan lama/kombinasi dihitung di scene).
// "Edge" (pressed/released) bernilai true sekali, sampai frame berikutnya.
namespace buttons {
  void begin();
  void update(uint32_t now);   // panggil tiap loop()
  void clearEdges();           // dipanggil app setelah update scene

  bool held(Btn b);            // sedang ditekan
  bool pressed(Btn b);         // baru saja ditekan
  bool released(Btn b);        // baru saja dilepas
  bool anyPressed();           // salah satu tombol baru ditekan
}
