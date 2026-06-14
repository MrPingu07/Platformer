// main.c
#include <raylib.h>
#include "scene_manager.h"
#include "scene.h"
#include "scenes/game.h"
#include "scene_manager.h"
#include "scenes/menu.h"

int main(void) {
    // Window configuration
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Prototype");
    SetTargetFPS(60);

    // Initialize the starting scene (Game Scene)
    Scene currentScene = game_scene();

    // Main Game Loop
    scene_manager_set(menu_scene());

    while (!WindowShouldClose()) {
        scene_manager_update(GetFrameTime());
        scene_manager_render();
    }

    // De-virtualization and cleanup
    CloseWindow();
    return 0;
}
