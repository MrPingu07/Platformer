// main.c
#include <raylib.h>
#include "scene_manager.h"
#include "scenes/menu.h"
#include "scenes/resolution.h"

int main(void) {
    // Window configuration
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Prototype");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    resolution_init();

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
