#include "scenes.h"

// Urutan HARUS sama dengan enum SceneId di scenes.h
const Scene* const SCENES[SCENE_COUNT] = {
  &sceneBoot,
  &sceneHome,
  &sceneFeed,
  &sceneBirthday,
  &sceneCatch,
  &sceneRun,
  &sceneEvolve,
};
