#pragma once
#include "scene.h"

// ============================================================
//  CARA MENAMBAH SCENE BARU (misalnya game atau animasi ulang tahun)
//
//   1. Buat file scene_xxx.cpp. Salin scene_home.cpp sebagai contoh.
//      Di akhir file: const Scene sceneXxx = { enter, update, draw, nullptr };
//   2. Di bawah ini: tambah nama di enum SceneId dan satu baris "extern const Scene".
//   3. Di scenes.cpp: tambah &sceneXxx di SCENES (urutannya harus sama dengan enum).
//
//  Pindah scene dari mana saja:  app::go(SCENE_XXX);
// ============================================================

enum SceneId : int8_t {
  SCENE_BOOT,
  SCENE_HOME,
  SCENE_FEED,
  SCENE_BIRTHDAY,
  SCENE_CATCH,
  SCENE_RUN,
  SCENE_EVOLVE,
  SCENE_COUNT
};

extern const Scene sceneBoot;
extern const Scene sceneHome;
extern const Scene sceneFeed;
extern const Scene sceneBirthday;
extern const Scene sceneCatch;
extern const Scene sceneRun;
extern const Scene sceneEvolve;

extern const Scene* const SCENES[SCENE_COUNT];
