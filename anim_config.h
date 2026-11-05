#pragma once
#include <Arduino.h>

// Angka animasi dan tampilan kitten. Ubah di sini tanpa menyentuh kode lain.
// (Pin, frame rate, dan waktu tombol tetap ada di config.h.)
namespace anim {

  // ---- Posisi kitten ----
  constexpr uint8_t KITTEN_CENTER_X = 64;   // tengah badan di layar
  constexpr uint8_t KITTEN_GROUND_Y = 52;   // baris paling bawah kaki (garis bar ada di y=54)
  constexpr uint8_t KITTEN_LEGS_MIN = 4;    // tinggi kaki saat napas turun (napas naik = +1)

  constexpr uint8_t KITTEN_BIG_EXTRA  = 10;   // lebar tambahan wujud besar (genap supaya wajah simetris)

  // ---- Idle ----
  constexpr uint16_t IDLE_BREATH_MS       = 900;    // napas naik/turun bergantian
  constexpr uint16_t IDLE_TAIL_STEP_MS    = 450;    // ekor ganti posisi tiap ini
  constexpr uint16_t IDLE_BLINK_PERIOD_MS = 3300;   // kedip tiap ini
  constexpr uint16_t IDLE_BLINK_MS        = 130;    // lama mata tertutup
  constexpr uint16_t IDLE_LOOK_STEP_MS    = 1700;   // mata melirik ganti arah tiap ini
  constexpr uint16_t IDLE_EAR_PERIOD_MS   = 7500;   // telinga berkedut tiap ini
  constexpr uint16_t IDLE_EAR_AT_MS       = 6600;   // ... pada detik ke sekian dalam periode
  constexpr uint16_t IDLE_EAR_MS          = 150;    // ... selama ini

  // ---- Idle aksi acak (menu utama): menguap, melirik, kibas ekor, melompat ----
  constexpr uint16_t IDLE_ACT_FIRST_MS    = 4000;   // aksi pertama setelah menu muncul
  constexpr uint16_t IDLE_ACT_MIN_MS      = 5000;   // jeda acak antar aksi: min ...
  constexpr uint16_t IDLE_ACT_MAX_MS      = 11000;  // ... dan maks
  constexpr uint16_t IDLE_ACT_RETRY_MS    = 1000;   // kalau sedang tidak boleh (lapar/submenu/tombol), coba lagi
  constexpr uint16_t IDLE_YAWN_MS         = 1400;   // menguap (mulut kecil -> lebar -> kecil)
  constexpr uint16_t IDLE_GLANCE_STEP_MS  = 400;    // melirik kiri-kanan, tiap langkah (6 langkah)
  constexpr uint16_t IDLE_WAG_MS          = 1500;   // kibas ekor cepat
  constexpr uint16_t IDLE_WAG_STEP_MS     = 90;
  constexpr uint16_t IDLE_HOP_MS          = 450;    // satu lompatan
  constexpr uint8_t  IDLE_HOP_COUNT       = 2;
  constexpr uint8_t  IDLE_HOP_PEAK        = 5;      // tinggi lompatan (px)
  constexpr uint8_t  IDLE_RNG_PIN         = A7;     // pin analog mengambang, sumber acak awal

  // ---- Mute: tahan Tombol 1 + Tombol 3 bersamaan ----
  constexpr uint16_t MUTE_CHORD_MS        = 1000;   // disarankan 800-2000

  // ---- Boot: tidur -> mata terbuka -> bangun -> mengeong -> idle sampai cfg::BOOT_MS ----
  constexpr uint16_t BOOT_SLEEP_MS  = 1500;   // tidur meringkuk dengan Z melayang
  constexpr uint16_t BOOT_WAKE_MS   = 600;    // mata terbuka, masih meringkuk
  constexpr uint16_t BOOT_SQUASH_MS = 120;    // gepeng sesaat saat bangun duduk
  constexpr uint16_t BOOT_MEOW_MS   = 1400;   // duduk dan mengeong (balon MEONG!)
  constexpr uint16_t BOOT_BREATH_MS = 700;    // napas saat tidur
  constexpr uint16_t BOOT_ZZZ_MS    = 1430;   // satu siklus Z melayang
  constexpr uint16_t BOOT_MOUTH_MS  = 160;    // mulut buka/tutup saat mengeong
  constexpr uint16_t BOOT_TAIL_MS   = 80;     // ekor kibas cepat saat mengeong

  // ---- Makan: kunigiri jatuh -> 3 gigitan -> senang + hati melayang ----
  constexpr uint8_t  FEEDS_TO_GROW        = 2;      // makan sebanyak ini dalam 1 boot -> kitten besar (1 per game)
  constexpr uint16_t FEED_DROP_MS         = 500;    // kunigiri jatuh dari atas
  constexpr uint8_t  FEED_BITES           = 3;      // jumlah gigitan sampai habis
  constexpr uint16_t FEED_BITE_MS         = 450;    // satu siklus gigit + kunyah
  constexpr uint16_t FEED_BITE_CUT_MS     = 150;    // kapan gigitan terjadi dalam siklus
  constexpr uint16_t FEED_CHEW_MS         = 100;    // mulut buka/tutup saat mengunyah
  constexpr uint16_t FEED_DONE_MS         = 1400;   // lama fase senang sebelum kembali ke home
  constexpr uint16_t FEED_HOP_MS          = 300;    // satu lompatan kecil (2x)
  constexpr uint8_t  FEED_HOP_PX          = 4;      // tinggi lompatan
  constexpr uint16_t FEED_HEART_GAP_MS    = 250;    // jeda antar hati melayang
  constexpr uint16_t FEED_HEART_RISE_MS   = 900;
  constexpr uint8_t  FEED_HEART_RISE_PX   = 20;
  constexpr uint8_t  FEED_HEART_Y         = 30;     // posisi awal hati
  constexpr uint16_t FEED_SPARKLE_MS      = 120;    // kelip kilau (petunjuk makan ke-4)

  // ---- Ulang tahun (30 dtk, selaras dengan lagu: 2 putaran x 15 dtk) ----
  constexpr uint16_t BDAY_MS             = 30000;  // total (samakan dengan panjang lagu di sound.cpp)
  constexpr uint16_t BDAY_SKIP_LOCK_MS   = 1000;   // tombol baru bisa melewati setelah ini
  constexpr uint16_t BDAY_PLANE_IN_AT    = 15000;  // pesawat mulai masuk (awal putaran lagu ke-2)
  constexpr uint16_t BDAY_PLANE_IN_MS    = 3000;   // lama masuk
  constexpr uint16_t BDAY_FLIP_AT        = 22200;  // banner berbalik menampilkan nama custom ("dear [name]")
  constexpr uint16_t BDAY_FLIP_MS        = 600;    // lama berbalik
  constexpr uint16_t BDAY_PLANE_OUT_AT   = 26000;  // pesawat pergi
  constexpr uint16_t BDAY_PLANE_OUT_MS   = 3000;   // lama pergi
  constexpr uint16_t BDAY_FINALE_AT      = 27500;  // hati melayang + mood penuh
  constexpr uint16_t BDAY_BOB_MS         = 500;    // pesawat naik-turun 1 px
  constexpr uint16_t BDAY_SING_MS        = 280;    // mulut kitten buka/tutup mengikuti lagu
  constexpr uint16_t BDAY_HOP_MS         = 900;    // jeda antar lompatan kecil
  constexpr uint16_t BDAY_HOP_UP_MS      = 300;    // lama satu lompatan
  constexpr uint8_t  BDAY_HOP_PX         = 3;
  constexpr uint16_t BDAY_HEART_GAP_MS   = 250;
  constexpr uint16_t BDAY_HEART_RISE_MS  = 900;
  constexpr uint16_t BDAY_HEART_PERIOD_MS = 1500;  // hati mengulang
  constexpr uint8_t  BDAY_HEART_RISE_PX  = 20;
  constexpr uint8_t  BDAY_HEART_Y        = 30;
  // Kembang api (di belakang kitten)
  constexpr uint16_t BDAY_FW_FIRST_MS    = 300;    // letusan pertama
  constexpr uint16_t BDAY_FW_GAP_MS      = 1300;   // jarak antar letusan
  constexpr uint8_t  BDAY_FW_COUNT       = 23;
  constexpr uint16_t BDAY_FW_LIFE_MS     = 1500;   // umur satu letusan
  constexpr uint8_t  BDAY_FW_RADIUS_MIN  = 12;     // radius = MIN + 0..2 * STEP
  constexpr uint8_t  BDAY_FW_RADIUS_STEP = 3;
  constexpr uint8_t  BDAY_FW_GRAVITY_PX  = 8;      // percikan jatuh sejauh ini di akhir umur

  // ---- Lapar: setelah game kitten lelah dan minta makan ----
  constexpr bool     HUNGRY_AT_BOOT        = false;  // true = uji makan tanpa main game dulu
  constexpr uint16_t HUNGRY_FIRST_MEOW_MS  = 800;    // meong pertama setelah kembali ke home
  constexpr uint16_t HUNGRY_MEOW_PERIOD_MS = 7000;   // lalu meong tiap ini
  constexpr uint16_t HUNGRY_BOB_MS         = 500;    // ikon kunigiri di balon naik-turun 1 px

  // ---- Game (umum: layar hasil dipakai semua game) ----
  constexpr uint16_t GAME_RESULT_MS     = 3500;   // layar hasil (kitten kelelahan)
  constexpr uint16_t GAME_RESULT_LOCK_MS = 1200;  // tombol baru bisa melewati hasil setelah ini
  constexpr uint8_t  GAME_SCORE_GOOD    = 10;     // skor segini -> mood +1
  constexpr uint16_t GAME_PANT_MS       = 350;    // napas terengah di layar hasil
  constexpr uint16_t GAME_SWEAT_MS      = 150;    // tetes keringat turun

  constexpr uint8_t  GAME_BOMB_PENALTY    = 2;      // skor berkurang kalau kena bom
  constexpr uint16_t GAME_HIT_BLINK_MS    = 700;    // kitten berkedip setelah kena bom
  constexpr uint8_t  GAME_BLINK_STEP_MS   = 70;
  constexpr uint16_t GAME_SPARK_MS        = 120;    // sumbu bom berkedip

  // ---- Game 1: tangkap hati (Tombol 1 = kiri, Tombol 3 = kanan) ----
  constexpr uint16_t CATCH_INTRO_MS      = 1800;   // petunjuk tombol sebelum mulai
  constexpr uint16_t CATCH_MS            = 30000;  // lama permainan
  constexpr uint8_t  CATCH_BASKET_SPEED  = 80;     // px per detik
  constexpr uint8_t  CATCH_FALL_MIN      = 26;     // kecepatan jatuh hati (px/dtk): awal
  constexpr uint8_t  CATCH_FALL_MAX      = 48;     // ... sampai akhir permainan
  constexpr uint16_t CATCH_SPAWN_START_MS = 800;   // jarak munculnya hati: awal
  constexpr uint16_t CATCH_SPAWN_END_MS  = 500;    // ... sampai akhir permainan
  constexpr uint8_t  CATCH_MAX_HEARTS    = 6;      // benda (hati + bom) di layar bersamaan
  constexpr uint8_t  CATCH_BOMB_PERCENT  = 25;     // peluang benda berupa bom

  // ---- Game 2: lari 3 jalur (Tombol 1 = jalur kiri, Tombol 3 = jalur kanan) ----
  constexpr uint16_t RUN_INTRO_MS        = 1800;
  constexpr uint16_t RUN_MS              = 30000;
  constexpr uint8_t  RUN_FALL_MIN        = 40;     // kecepatan dunia bergerak (px/dtk): awal
  constexpr uint8_t  RUN_FALL_MAX        = 80;     // ... sampai akhir
  constexpr uint16_t RUN_SPAWN_START_MS  = 750;    // jarak munculnya benda: awal
  constexpr uint16_t RUN_SPAWN_END_MS    = 450;    // ... sampai akhir
  constexpr uint8_t  RUN_MAX_ITEMS       = 5;
  constexpr uint8_t  RUN_BOMB_PERCENT    = 30;     // peluang benda berupa bom
    constexpr uint16_t RUN_SLIDE_SPEED     = 400;    // px/dtk pindah jalur
  constexpr uint16_t RUN_BOB_MS          = 150;    // kitten naik-turun 1 px saat lari

  // ---- Submenu GAME di home ----
  constexpr uint16_t HOME_PICK_MS        = 5000;   // submenu pilih game menutup sendiri setelah ini

  // ---- Evolusi: getar + kilau berkumpul -> kilat putih -> melebar ----
  constexpr uint16_t EVOLVE_SHAKE_MS     = 1600;
  constexpr uint16_t EVOLVE_FLASH_MS     = 400;
  constexpr uint16_t EVOLVE_GROW_MS      = 700;
  constexpr uint16_t EVOLVE_SHOW_MS      = 1800;   // pamer wujud besar sebelum kembali ke home
  constexpr uint8_t  EVOLVE_SHAKE_STEP_MS = 60;
  constexpr uint8_t  EVOLVE_SPARKLE_R0   = 34;     // radius kilau: awal berkumpul / akhir menyebar
  constexpr uint8_t  EVOLVE_SPARKLE_R1   = 8;      // ... titik kumpul
  constexpr uint8_t  EVOLVE_CENTER_Y     = 30;     // pusat kilau (sekitar wajah)

  // ---- Mood (ikon hati di kiri atas) ----
  constexpr uint8_t MOOD_MAX   = 3;
  constexpr uint8_t MOOD_START = 2;                 // mood awal tiap boot
  constexpr uint8_t MOOD_X     = 1;
  constexpr uint8_t MOOD_Y     = 1;
  constexpr uint8_t MOOD_STEP  = 9;                 // jarak antar hati
}
