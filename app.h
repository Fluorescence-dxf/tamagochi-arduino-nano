#pragma once
#include <Arduino.h>
#include "scenes.h"

// Inti program: menjalankan scene aktif dengan jam frame yang tetap.
namespace app {
  void begin();
  void loop();                // panggil dari loop()
  void go(SceneId id);        // pindah scene (berlaku di awal loop berikutnya)
}
