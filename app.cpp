#include "app.h"
#include "config.h"
#include "buttons.h"
#include "sound.h"
#include "gfx.h"

namespace {

const Scene* cur = nullptr;
int8_t pending = -1;
uint32_t sceneStart = 0;
uint32_t lastFrame = 0;

// Baca tombol dan jalankan suara. Dipanggil tiap loop() DAN di antara halaman
// layar saat menggambar, supaya klik singkat tidak terlewat.
void pump() {
  uint32_t now = millis();
  buttons::update(now);
  sound::update(now);
}

void applyPending(uint32_t now) {
  if (cur && cur->leave) cur->leave();
  cur = SCENES[pending];
  pending = -1;
  sceneStart = now;
  if (cur->enter) cur->enter();
}

}  // namespace

void app::begin() {
  buttons::begin();
  sound::begin();
  gfx::begin();
  gfx::setPageHook(pump);

  uint32_t now = millis();
  pending = SCENE_BOOT;
  applyPending(now);
  lastFrame = now;
}

void app::go(SceneId id) { pending = id; }

void app::loop() {
  pump();

  uint32_t now = millis();
  if (pending >= 0) applyPending(now);

  uint32_t dt = now - lastFrame;
  if (dt < cfg::FRAME_MS) return;
  lastFrame = now;

  uint32_t t = now - sceneStart;
  if (cur->update) cur->update(dt > 0xFFFF ? 0xFFFF : (uint16_t)dt, t);
  buttons::clearEdges();
  gfx::render(*cur, t);
}
