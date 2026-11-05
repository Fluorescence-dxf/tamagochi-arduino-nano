#include "sound.h"
#include "config.h"
#include <avr/pgmspace.h>

namespace {

#define LEN(a) ((uint8_t)(sizeof(a) / sizeof((a)[0])))

// ---- Tabel nada (disimpan di flash). Ubah/tambah di sini. ----
const Note SEQ_CLICK[] PROGMEM = { {1800, 15} };
const Note SEQ_BOOT[]  PROGMEM = { {523, 90}, {659, 90}, {784, 90}, {1047, 200} };
// Meong dua kali: nada naik lalu turun, jeda, ulang.
const Note SEQ_MEONG[] PROGMEM = {
  {620, 60}, {760, 70}, {900, 70}, {780, 70}, {600, 100}, {480, 130},
  {0, 200},
  {660, 60}, {800, 70}, {920, 70}, {780, 70}, {580, 110}, {460, 150},
};
// Dengkur: napas masuk dan keluar.
const Note SEQ_SNORE[] PROGMEM = { {180, 200}, {150, 260}, {0, 140}, {170, 220}, {130, 320} };

// Makan: siulan jatuh, mendarat, gigit (nyam), dan nada senang.
const Note SEQ_FALL[] PROGMEM = { {1400, 30}, {1200, 30}, {1000, 30}, {850, 30}, {700, 30}, {560, 30} };
const Note SEQ_PLOP[] PROGMEM = { {330, 35}, {220, 60} };
const Note SEQ_NOM[]  PROGMEM = { {300, 30}, {0, 15}, {230, 45}, {0, 15}, {280, 30} };
const Note SEQ_YUM[]  PROGMEM = { {659, 80}, {784, 80}, {988, 80}, {1319, 220} };

// Game: hati tertangkap (ting naik) dan kitten kelelahan (turun lesu).
const Note SEQ_CATCH[] PROGMEM = { {1320, 20}, {1760, 35} };
const Note SEQ_TIRED[] PROGMEM = { {392, 120}, {330, 120}, {262, 260} };

const Note SEQ_BONK[]  PROGMEM = { {220, 50}, {150, 90} };   // kena batu

// Evolusi: 16 nada naik (tiap 100 ms, selaras dengan getar 1,6 dtk), lalu fanfare saat kilat.
const Note SEQ_EVOLVE[] PROGMEM = {
  {262, 100}, {277, 100}, {294, 100}, {311, 100}, {330, 100}, {349, 100}, {370, 100}, {392, 100},
  {415, 100}, {440, 100}, {466, 100}, {494, 100}, {523, 100}, {554, 100}, {587, 100}, {622, 100},
  {1047, 130}, {1319, 130}, {1568, 130}, {2093, 450},
};

// ---- Lagu Happy Birthday: 25 ketukan x 600 ms = 15 dtk, diputar 2x = 30 dtk ----
constexpr uint16_t BEAT = 600;                  // tempo: ubah di sini
constexpr uint16_t GAP  = 40;                   // jeda antar dua nada sama ("Hap-py")
constexpr uint16_t HAP  = BEAT * 3 / 4 - GAP;   // "Hap" (titik) + jeda + "py" (pendek) = 1 ketukan
constexpr uint16_t PY   = BEAT / 4;
constexpr uint16_t G4 = 392, A4 = 440, B4 = 494, C5 = 523, D5 = 587, E5 = 659, F5 = 698, G5 = 784;
#define PAIR(n) {n, HAP}, {0, GAP}, {n, PY}
const Note SEQ_BIRTHDAY[] PROGMEM = {
  PAIR(G4), {A4, BEAT}, {G4, BEAT}, {C5, BEAT}, {B4, 2 * BEAT},                    // Happy birthday to you
  PAIR(G4), {A4, BEAT}, {G4, BEAT}, {D5, BEAT}, {C5, 2 * BEAT},                    // Happy birthday to you
  PAIR(G4), {G5, BEAT}, {E5, BEAT}, {C5, BEAT}, {B4, BEAT}, {A4, 2 * BEAT},        // Happy birthday dear [name]
  PAIR(F5), {E5, BEAT}, {C5, BEAT}, {D5, BEAT}, {C5, 2 * BEAT},                    // Happy birthday to you
};

struct Entry {
  const Note* seq;
  uint8_t len;
  uint8_t repeats;   // berapa kali diputar (1 = sekali)
};

// Urutan HARUS sama dengan enum Sfx di sound.h
const Entry TABLE[] = {
  { SEQ_CLICK, LEN(SEQ_CLICK), 1 },
  { SEQ_BOOT,  LEN(SEQ_BOOT), 1 },
  { SEQ_MEONG, LEN(SEQ_MEONG), 1 },
  { SEQ_SNORE, LEN(SEQ_SNORE), 1 },
  { SEQ_FALL,  LEN(SEQ_FALL), 1 },
  { SEQ_PLOP,  LEN(SEQ_PLOP), 1 },
  { SEQ_NOM,   LEN(SEQ_NOM), 1 },
  { SEQ_YUM,   LEN(SEQ_YUM), 1 },
  { SEQ_BIRTHDAY, LEN(SEQ_BIRTHDAY), 2 },
  { SEQ_CATCH, LEN(SEQ_CATCH), 1 },
  { SEQ_TIRED, LEN(SEQ_TIRED), 1 },
  { SEQ_BONK,  LEN(SEQ_BONK), 1 },
  { SEQ_EVOLVE, LEN(SEQ_EVOLVE), 1 },
};

static_assert(sizeof(TABLE) / sizeof(TABLE[0]) == (uint8_t)Sfx::COUNT,
              "TABLE di sound.cpp harus sejumlah enum Sfx");

const Note* curSeq = nullptr;
uint8_t curLen = 0;
uint8_t curIdx = 0;
uint8_t repeatsLeft = 0;
bool active = false;
bool mutedFlag = false;
uint32_t nextAt = 0;
constexpr uint16_t MAX_LAG_MS = 80;

}  // namespace

void sound::begin() {
  pinMode(cfg::BUZZER_PIN, OUTPUT);
  noTone(cfg::BUZZER_PIN);
}

void sound::play(Sfx id) {
  const Entry& e = TABLE[(uint8_t)id];
  curSeq = e.seq;
  curLen = e.len;
  curIdx = 0;
  repeatsLeft = e.repeats;
  active = true;
  nextAt = millis();
}

void sound::stop() {
  active = false;
  noTone(cfg::BUZZER_PIN);
}

void sound::update(uint32_t now) {
  if (!active) return;
  if ((int32_t)(now - nextAt) < 0) return;

  if (curIdx >= curLen) {
    if (repeatsLeft > 1) { repeatsLeft--; curIdx = 0; }
    else { stop(); return; }
  }

  Note n;
  memcpy_P(&n, &curSeq[curIdx], sizeof(n));
  curIdx++;

  if (n.freq > 0 && !mutedFlag) tone(cfg::BUZZER_PIN, n.freq);
  else noTone(cfg::BUZZER_PIN);

  // Jadwal berlanjut dari nada sebelumnya (tidak melenceng). Kalau telat jauh, mulai ulang dari sekarang.
  uint32_t base = ((int32_t)(now - nextAt) > (int32_t)MAX_LAG_MS) ? now : nextAt;
  nextAt = base + n.ms;
}

void sound::setMuted(bool m) {
  mutedFlag = m;
  if (m) noTone(cfg::BUZZER_PIN);
}

bool sound::muted() { return mutedFlag; }

void sound::toggleMute() { setMuted(!mutedFlag); }
