// scenes/game.c
#include "game.h"
#include <raylib.h>
#include "game_init.h"
#include "game_render.h"
#include "game_state.h"
#include "game_update.h"

// --- Scene State (Private Module Storage) ---
static GameState game;
//

// --- Scene Lifecycle: Init ---
static void game_init(const char *levelFile) {
    game_init_world(&game, levelFile);
}

// --- Scene Lifecycle: Update ---

static void game_update(float dt) {
    const char *next = game_update_world(&game, dt);
    if (next) game_init(next);
}

// --- Scene Lifecycle: Render ---
static void game_render(void) {
    game_render_world(&game);
}

// --- Entry Point ---
Scene game_scene(void) {
    game_init("level1.txt");
    return (Scene){ game_update, game_render };
}

Scene game_scene_from(const char *levelFile) {
    game_init(levelFile);
    return (Scene){ game_update, game_render };
}
