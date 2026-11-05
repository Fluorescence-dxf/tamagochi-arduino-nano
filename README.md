# 🐾 Virtual Pet Chibi Kitten (Tamagotchi)

Sebuah proyek firmware **Virtual Pet / Tamagotchi Kucing Chibi** interaktif berbasis mikrokontroler **Arduino Nano (ATmega328P)** dengan layar **OLED SSD1306 128x64 (I2C)**, 3 tombol kontrol, dan piezo buzzer. Dibuat sebagai hadiah spesial yang personal, ekspresif, dan menyenangkan!

---

## ✨ Fitur Utama

- **Animasi Chibi Prosedural**:
  - Menggambar pose kucing (duduk, bernapas, mengedipkan mata, telinga berkedut, mengibaskan ekor, tidur meringkuk) tanpa memakan memori bitmap besar.
  - Memori sangat efisien (arsitektur kurva tepi siluet di RAM/Flash minim).
- **Interaksi & Kebutuhan Kucing**:
  - **Makan (Feed)**: Memberi makan saat kucing lapar atau mengeong.
  - **Mood & Emosi**: Indikator mood 3-hati di layar.
  - **Evolusi (Grow)**: Setelah makan beberapa kali, kitten akan berevolusi menjadi wujud yang lebih besar.
- **2 Mini-Game Seru**:
  - 🍎 **Catch Game**: Tangkap makanan/buah yang jatuh dan hindari benda berbahaya.
  - 🏃 **Run Game**: Kucing berlari melompati rintangan (obstacle dodge runner).
- **Scene Kejutan (Hadiah / Ulang Tahun)**:
  - Tekan tombol spesial untuk memunculkan pertunjukan kembang api dengan lagu *Happy Birthday* 8-bit tone buzzer.
  - Pesawat penarik banner melintas membawa ucapan spesial dan nama penerima yang dapat Anda kustomisasi sesuka hati di [config.h](config.h).
- **Audio & Kontrol Fleksibel**:
  - SFX non-blocking (Meong, Melodi Ulang Tahun, Suara Game Over, Dll).
  - Fitur **Mute / Unmute** instan dengan menahan tombol 1 & 3 bersamaan.

---

## 🛠️ Kebutuhan Komponen (Hardware)

Proyek ini sengaja dirancang sederhana agar mudah dirakit tanpa PCB khusus (bisa menggunakan breadboard atau perfboard):

| Komponen | Spesifikasi / Catatan | Jumlah |
| :--- | :--- | :---: |
| **Mikrokontroler** | Arduino Nano V3 (ATmega328P, 5V/16MHz) | 1 |
| **Display** | Monokrom OLED 0.96" SSD1306 128x64 (I2C) | 1 |
| **Tombol** | Tactile Push Button switch | 3 |
| **Audio** | Piezo Buzzer (Passive) | 1 |
| **Resistor** | Resistor 100Ω – 220Ω (pembatas arus buzzer) | 1 |
| **Kabel & Daya** | Kabel Jumper & Kabel Mini USB / Baterai | Secukupnya |

---

## 🔌 Skematik & Panduan Wiring

| Pin Komponen | Pin Arduino Nano | Keterangan |
| :--- | :--- | :--- |
| **OLED VCC** | `5V` (atau `3.3V`) | Sumber daya modul OLED |
| **OLED GND** | `GND` | Ground display |
| **OLED SCL** | `A5` | I2C Clock (Hardware I2C AVR) |
| **OLED SDA** | `A4` | I2C Data (Hardware I2C AVR) |
| **Tombol 1 (Kiri)** | `D2` | Kaki tombol satu ke pin D2, kaki lainnya ke **GND** |
| **Tombol 2 (Tengah)**| `D3` | Kaki tombol satu ke pin D3, kaki lainnya ke **GND** |
| **Tombol 3 (Kanan)**| `D4` | Kaki tombol satu ke pin D4, kaki lainnya ke **GND** |
| **Piezo Buzzer (+)** | `D5` | Pin D5 melalui Resistor 100-220Ω ke kaki positif buzzer |
| **Piezo Buzzer (-)** | `GND` | Kaki negatif buzzer langsung ke Ground |

> [!NOTE]
> Firmware memanfaatkan fitur **Internal Pull-Up Resistor** pada pin tombol D2, D3, dan D4, sehingga Anda **tidak memerlukan** resistor pull-up eksternal untuk ketiga tombol.

---

## 🎮 Cara Bermain & Navigasi

- **Di Menu Utama (Home)**:
  - **Tombol 1 (Kiri)**: Berinteraksi / Memberi Makan (*Feed*) jika kucing lapar.
  - **Tombol 2 (Tengah)**: Membuka Submenu Mini-Game (*Catch* / *Run*). (Hanya bisa dibuka jika kitten sudah kenyang).
  - **Tombol 3 (Kanan)**: Membuka **Scene Kejutan Ulang Tahun** (Pesawat & Banner).
  - **Tahan Tombol 1 + Tombol 3 Bersamaan**: Toggle **Mute / Unmute** suara.
- **Di Dalam Game Catch / Run**:
  - Gunakan tombol arah / lompat untuk mengendalikan kucing.

---

## ⚙️ Kustomisasi Hadiah (Nama & Pesan)

Untuk mengganti teks ucapan dan nama pasangan/orang tersayang di banner pesawat:
1. Buka file [config.h](config.h).
2. Ubah baris konfigurasi berikut:
   ```cpp
   #define PET_GIFT_GREET "HAPPY BIRTHDAY"  // Teks pembuka banner
   #define PET_GIFT_NAME  "MY LOVE <3"       // Teks nama penerima (maks ~10-12 karakter)
   ```
3. Simpan dan upload kembali ke Arduino Nano Anda!

---

## 💻 Panduan Instalasi & Flashing

1. **Unduh Software**:
   - Pasang [Arduino IDE](https://www.arduino.cc/en/software) (versi 2.x disarankan).
2. **Pasang Library U8g2**:
   - Di Arduino IDE, buka menu **Tools > Manage Libraries...** (atau `Ctrl + Shift + I`).
   - Cari **`U8g2`** oleh *oliver*.
   - Klik **Install**.
3. **Buka Proyek**:
   - Buka file [tamagochi-arduino-nano.ino](tamagochi-arduino-nano.ino).
4. **Pengaturan Board**:
   - Buka menu **Tools > Board > Arduino AVR Boards > Arduino Nano**.
   - Buka menu **Tools > Processor > ATmega328P** (jika clone Arduino Nano tidak terdeteksi saat upload, pilih **ATmega328P (Old Bootloader)**).
   - Pilih Port COM yang sesuai di **Tools > Port**.
5. **Upload**:
   - Klik tombol **Upload** (tanda panah ke kanan) atau tekan `Ctrl + U`.

---

## 📂 Struktur Repositori

```text
├── assets/             # Referensi sprite sheets dan asset visual
├── anim_config.h       # Timing animasi dan frame-rate
├── app.cpp / app.h     # State machine dan scene dispatcher
├── buttons.cpp / .h    # Input handling, debouncing, & chord combo
├── config.h            # Konfigurasi Pin, Teks Banner Hadiah, & Layar
├── game_common.cpp/.h  # Helper render & score
├── gfx.cpp / gfx.h     # Driver U8g2 low-RAM page buffer wrapper
├── kitten.cpp / .h     # Procedural chibi kitten renderer & poses
├── pet.cpp / pet.h     # Mood, feeding count, & state logic
├── sound.cpp / sound.h # Tone sequencer, SFX, & melodi
├── scene_*.cpp         # Logika layar terpisah (Boot, Home, Feed, Catch, Run, Bday, Evolve)
├── tamagochi-arduino-nano.ino # Arduino main sketch entry
└── README.md           # Dokumentasi proyek
```

---

## 📄 Lisensi
Proyek ini dirilis di bawah lisensi [MIT License](LICENSE) - bebas digunakan, dimodifikasi, dan dibagikan kembali untuk keperluan personal maupun edukasi.
