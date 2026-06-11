// scenes/game.c
#include "game.h"
#include "../entities/player.h"
#include "../entities/runner.h"
#include "../entities/box.h"
#include "../entities/bullet.h"
#include "../entities/collision.h"
#include "../entities/hittable.h"
#include "../entities/drop.h"
#include "camera.h"
#include <raylib.h>
#include "level_loader.h"

// --- Scene State (Private Module Storage) ---
static int       levelWidth  = 0;
static int       levelHeight = 0;
static Camera2D  camera = { 0 };
static Player    player;
static Runner    runners[255];
static int       runnerCount;
static Box       boxes[255];
static int       boxCount;
static Rectangle platforms[255];
static int       platformCount;
static Bullet    gameBullets[MAX_BULLETS];
static Drop      gameDrops[MAX_DROPS];

// --- Scene Lifecycle: Init ---
static void game_init(void) {
    // Reset all entity counters before loading
    platformCount = 0;
    boxCount = 0;
    runnerCount = 0;

    // Reset projectile pool
    for (int i = 0; i < MAX_BULLETS; i++) {
        gameBullets[i].isActive = false;
    }
    for (int i = 0; i < MAX_DROPS; i++) {
        gameDrops[i].isCollected = true;
    }

    // Load level data from external text file
    LevelData data = load_level(TextFormat("assets/levels/%s", "level1.txt"));
    player        = data.player;
    platformCount = data.platformCount;
    boxCount      = data.boxCount;
    runnerCount   = data.runnerCount;
    levelWidth    = data.levelWidth;
    levelHeight   = data.levelHeight;
    for (int i = 0; i < platformCount; i++) platforms[i] = data.platforms[i];
    for (int i = 0; i < boxCount;      i++) boxes[i]     = data.boxes[i];
    for (int i = 0; i < runnerCount;   i++) runners[i]   = data.runners[i];

    camera_init(&camera, (Vector2){ player.x + 20.0f, player.y + 20.0f });
}

// --- Scene Lifecycle: Update ---
static void game_update(float dt) {
    player.onGround = false;
    player_update(&player, dt);
    resolve_environment_collisions(&player, platforms, platformCount);

    //Handle Movement
    player_handle_movement(&player, levelWidth, dt);

    //Handle Combat
    player_handle_combat(&player, gameBullets, MAX_BULLETS, dt);
    bullets_update(gameBullets, MAX_BULLETS, dt);

    //Drops system
    drops_update(gameDrops, MAX_DROPS, platforms, platformCount, dt);
    drops_collect(gameDrops, MAX_DROPS, &player);

    bool aliveSnapshot[255] = { false };
    for (int i = 0; i < runnerCount; i++)
        aliveSnapshot[i] = !runners[i].isDead;

    Hittable targets[255];
    int targetCount = 0;
    for (int i = 0; i < runnerCount; i++) {
        targets[targetCount++] = (Hittable){
            .rect    = &runners[i].rect,
            .health  = &runners[i].health,
            .isDead  = &runners[i].isDead
        };
    }
    resolve_bullet_hittable_collisions(targets, targetCount, gameBullets, MAX_BULLETS);

    for (int i = 0; i < runnerCount; i++) {
        runner_update(&runners[i], &player, dt, (float)levelWidth, platforms, platformCount);

        if (aliveSnapshot[i] && runners[i].isDead)
            drops_try_spawn(gameDrops, MAX_DROPS,
                            runners[i].rect.x + runners[i].rect.width / 2.0f,
                            runners[i].rect.y);

        runners[i].onGround = false;
        resolve_rect_collision(&runners[i].rect, &runners[i].vy, platforms, platformCount, &runners[i].onGround);
        for (int j = 0; j < boxCount; j++) {
            if (boxes[j].isBroken) continue;
            resolve_rect_collision(&runners[i].rect, &runners[i].vy, &boxes[j].rect, 1, &runners[i].onGround);
        }
    }

    //boxes system
    for (int i = 0; i < boxCount; i++) box_update(&boxes[i], &player);

    camera_update(&camera, (Vector2){ player.x + 20.0f, player.y + 20.0f }, levelWidth, levelHeight, dt);

}

// --- Scene Lifecycle: Render ---
static void game_render(void) {
    BeginDrawing();
    BeginMode2D(camera);
    ClearBackground(DARKGRAY);
    //Render platforms
    for (int i = 0; i < platformCount; i++) DrawRectangleRec(platforms[i], GRAY);
    //Render Boxes
    for (int i = 0; i < boxCount; i++) {
        // Each box handles its own state updates and player interaction checks
        box_render(&boxes[i]);
    }
    //Render bullets
    bullets_render(gameBullets, MAX_BULLETS);

    //Render drops
    drops_render(gameDrops, MAX_DROPS);

    //Render Runner
    for (int i = 0; i < runnerCount; i++) {
        runner_render(&runners[i]);
    }
    //Render Player
    player_render(&player);
    EndMode2D();
    // UI Overlay
    Weapon currentWeapon = player.inventory[player.currentSlot];
    DrawText(TextFormat("WEAPON: %s", currentWeapon.name), 20, 20, 20, RAYWHITE);
    DrawText("A/D: Move  W: Jump  SPACE: Fire  Q: Swap", 20, 450, 10, LIGHTGRAY);
    EndDrawing();
}

// --- Entry Point ---
Scene game_scene(void) {
    game_init();
    return (Scene){ game_update, game_render };
}
