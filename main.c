// main.c
#include <raylib.h>
#include "scene.h"
#include "scenes/game.h"

int main(void) {
    InitWindow(640, 480, "Platformer");
    SetTargetFPS(60);

    Scene current = game_scene();

    while (!WindowShouldClose()) {
        current.update(GetFrameTime());
        current.render();
    }

    CloseWindow();
    return 0;
}
