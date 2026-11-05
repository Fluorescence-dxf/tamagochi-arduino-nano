#include "buttons.h"
#include "config.h"

namespace {

constexpr uint8_t N = (uint8_t)Btn::COUNT;

struct State {
  bool down;          // keadaan tombol yang sudah diterima (setelah debounce)
  bool ePress;
  bool eRelease;
  uint32_t changedAt; // kapan keadaan terakhir berubah
};

State st[N];

}  // namespace

void buttons::begin() {
  uint32_t now = millis();
  for (uint8_t i = 0; i < N; i++) {
    pinMode(cfg::BTN_PINS[i], INPUT_PULLUP);
    st[i] = State{};
    st[i].changedAt = now - cfg::DEBOUNCE_MS;   // perubahan pertama langsung diterima
  }
}

// Debounce gaya "lockout": perubahan pertama diterima SEGERA, lalu perubahan
// berikutnya diabaikan selama DEBOUNCE_MS. Klik singkat tidak perlu bertahan
// lama untuk terbaca (cara lama menunggu tombol stabil dulu).
void buttons::update(uint32_t now) {
  for (uint8_t i = 0; i < N; i++) {
    State& s = st[i];
    bool raw = digitalRead(cfg::BTN_PINS[i]) == LOW;

    if (raw != s.down && (now - s.changedAt) >= cfg::DEBOUNCE_MS) {
      s.down = raw;
      s.changedAt = now;
      if (raw) {
        s.ePress = true;
      } else {
        s.eRelease = true;
      }
    }
  }
}

void buttons::clearEdges() {
  for (uint8_t i = 0; i < N; i++) {
    st[i].ePress = false;
    st[i].eRelease = false;
  }
}

bool buttons::held(Btn b)        { return st[(uint8_t)b].down; }
bool buttons::pressed(Btn b)     { return st[(uint8_t)b].ePress; }
bool buttons::released(Btn b)    { return st[(uint8_t)b].eRelease; }

bool buttons::anyPressed() {
  for (uint8_t i = 0; i < N; i++) if (st[i].ePress) return true;
  return false;
}
