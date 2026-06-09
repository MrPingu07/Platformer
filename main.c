// main.c
#include <raylib.h>
#include "scene.h"
#include "scenes/game.h"

int main(void) {
    // Window configuration
    const int screenWidth = 640;
    const int screenHeight = 480;
    InitWindow(screenWidth, screenHeight, "Prototype");
    SetTargetFPS(60);

    // Initialize the starting scene (Game Scene)
    Scene currentScene = game_scene();

    // Main Game Loop
    while (!WindowShouldClose()) {
        // 1. Update scene logic passing Delta Time
        currentScene.update(GetFrameTime());

        // 2. Render the scene
        currentScene.render();
    }

    // De-virtualization and cleanup
    CloseWindow();
    return 0;
}
