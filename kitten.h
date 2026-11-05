#pragma once
#include <Arduino.h>

// Modul kitten: menggambar kitten chibi dari tabel siluet kecil + bentuk wajah.
// Tidak ada bitmap besar: bentuk badan = satu angka per baris (lihat TOP_EDGE di
// kitten.cpp), jadi mudah diubah dan hemat flash.
//
// Pemakaian di scene:
//   kitten::draw(kitten::idlePose(t));              // idle otomatis dari waktu
//   kitten::Pose p = kitten::basePose();            // atau atur sendiri
//   p.eyes = kitten::Eyes::HAPPY;  p.mouth = kitten::Mouth::OPEN;
//   kitten::draw(p);
namespace kitten {

enum class Eyes  : uint8_t { OPEN, HAPPY, CLOSED, SAD, SLEEP };   // SLEEP = mata terpejam santai (U)
enum class Mouth : uint8_t { W, SMALL_O, OPEN, FROWN };

struct Pose {
  uint8_t centerX;       // tengah badan di layar
  uint8_t groundY;       // baris paling bawah kaki
  uint8_t legRows;       // tinggi kaki (napas, lompat). Normal 4-5
  uint8_t extraCols;     // lebar tambahan di tengah. 0 = kecil, ~10 = wujud besar
  int8_t  tailSwing;     // -3..3 = ekor geser ke kiri/kanan (ujung bergerak paling jauh)
  int8_t  look;          // -1, 0, +1 = arah pupil
  uint8_t earLeftDrop;   // 1 = telinga kiri turun 1 px (berkedut)
  uint8_t earRightDrop;  // 1 = telinga kanan turun 1 px
  Eyes    eyes;
  Mouth   mouth;
};

// Wujud kitten: 0 = kecil, >0 = jumlah kolom tambahan (wujud besar). basePose()/idlePose()
// otomatis memakainya, jadi semua scene ikut berubah setelah evolusi.
uint8_t growth();
void setGrowth(uint8_t extraCols);

Pose basePose();                 // pose duduk standar dengan nilai dari anim_config.h
Pose idlePose(uint32_t t);       // pose idle pada waktu t (ms sejak scene mulai)
void draw(const Pose& p);        // gambar kitten (dipanggil dari fungsi draw scene)

// Pose tidur meringkuk: kepala di kiri, badan oval di kanan, ekor melingkar di depan.
// centerX/groundY seperti Pose. breath = 0 atau 1 (badan naik 1 px saat bernapas).
void drawCurled(uint8_t centerX, uint8_t groundY, uint8_t breath, Eyes eyes, Mouth mouth);

}
