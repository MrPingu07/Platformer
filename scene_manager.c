//scene-manager.c
#include "scene.h"
#include "scene_manager.h"

static Scene currentScene;

void scene_manager_set(Scene s)        { currentScene = s; }
void scene_manager_update(float dt)    { currentScene.update(dt); }
void scene_manager_render(void)        { currentScene.render(); }
