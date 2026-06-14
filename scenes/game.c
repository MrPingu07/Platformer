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
#include <stdio.h>
#include "level_loader.h"
#include "moving_platform.h"
#include "game_state.h"

// --- Scene State (Private Module Storage) ---
static Box       boxes[MAX_BOXES];
static int       boxCount;
static PlatformTile platforms[MAX_PLATFORMS];
static int       platformCount;
static Bullet    gameBullets[MAX_BULLETS];
static Drop      gameDrops[MAX_DROPS];
static char      levelNext[64];
static char      levelTileset[64];
static ExitTile  levelExits[MAX_EXITS];
static int       levelExitCount = 0;
static int       movingPlatformCount = 0;
static MovingPlatform movingPlatforms[MAX_MOVING_PLATFORMS];
static GameState game;
//

// --- Scene Lifecycle: Init ---
static void game_init(const char *levelFile) {
    // Reset all entity counters before loading
    platformCount = 0;
    boxCount      = 0;
    game.runnerCount   = 0;
    game.killCount     = 0;

    // Reset projectile pool
    for (int i = 0; i < MAX_BULLETS; i++) {
        gameBullets[i].isActive = false;
    }
    for (int i = 0; i < MAX_DROPS; i++) {
        gameDrops[i].isCollected = true;
    }

    // Load level data from external text file
    LevelData data = load_level(TextFormat("assets/levels/%s", levelFile));
    TraceLog(LOG_INFO, "Loading level: assets/levels/%s", levelFile);
    game.player        = data.player;
    platformCount = data.platformCount;
    boxCount      = data.boxCount;
    game.runnerCount = data.runnerCount;
    game.levelWidth  = data.levelWidth;
    game.levelHeight = data.levelHeight;
    for (int i = 0; i < platformCount; i++) platforms[i] = data.platforms[i];
    for (int i = 0; i < boxCount;      i++) boxes[i]     = data.boxes[i];
    for (int i = 0; i < game.runnerCount; i++) game.runners[i] = data.runners[i];

    snprintf(levelTileset, sizeof(levelTileset), "%s", data.tileset);
    levelExitCount = data.exitCount;
    for (int i = 0; i < levelExitCount; i++) levelExits[i] = data.exits[i];

    movingPlatformCount = data.movingPlatformCount;
    for (int i = 0; i < movingPlatformCount; i++) movingPlatforms[i] = data.movingPlatforms[i];

    camera_init(
        &game.camera,
        (Vector2){ game.player.x + 20.0f, game.player.y + 20.0f }
    );
}

// --- Scene Lifecycle: Update ---
static void game_update(float dt) {
    // 1. Aplicar delta del frame ANTERIOR antes de resetear
    if (game.player.groundPlatformIndex >= platformCount && game.player.groundPlatformIndex != -1) {
        int mi = game.player.groundPlatformIndex - platformCount;
        game.player.x += movingPlatforms[mi].delta.x;
        game.player.y += movingPlatforms[mi].delta.y;
    }

    // 2. Reset, luego mover plataformas y reconstruir rects
    game.player.onGround = false;
    game.player.groundPlatformIndex = -1;
    moving_platforms_update(movingPlatforms, movingPlatformCount, dt);
    Rectangle platformRects[MAX_PLATFORMS];
    for (int i = 0; i < platformCount; i++) platformRects[i] = platforms[i].rect;
    for (int i = 0; i < movingPlatformCount; i++)
        platformRects[platformCount + i] = movingPlatforms[i].rect;
    int totalPlatforms = platformCount + movingPlatformCount;

    // 3. Física y colisión setean groundPlatformIndex para el próximo frame
    player_update(&game.player, dt);
    resolve_environment_collisions(&game.player, platformRects, totalPlatforms);

    //Handle Movement
    player_handle_movement(&game.player, game.levelWidth, dt);

    //Handle Combat
    player_handle_combat(&game.player, gameBullets, MAX_BULLETS, dt);
    bullets_update(gameBullets, MAX_BULLETS, dt);

    //Drops system
    drops_update(gameDrops, MAX_DROPS, platformRects, totalPlatforms, game.levelHeight, dt);
    drops_collect(gameDrops, MAX_DROPS, &game.player);

    for (int i = 0; i < game.runnerCount; i++) game.aliveSnapshot[i] = false;

    for (int i = 0; i < game.runnerCount; i++)
        game.aliveSnapshot[i] = !game.runners[i].isDead;

    Hittable targets[255];
    int targetCount = 0;
    for (int i = 0; i < game.runnerCount; i++) {
        targets[targetCount++] = (Hittable){
            .rect    = &game.runners[i].rect,
            .health  = &game.runners[i].health,
            .isDead  = &game.runners[i].isDead
        };
    }
    resolve_bullet_hittable_collisions(targets, targetCount, gameBullets, MAX_BULLETS);

    Rectangle logicBounds = camera_get_logic_bounds(&game.camera);

    for (int i = 0; i < game.runnerCount; i++) {
        if (!CheckCollisionRecs(game.runners[i].rect, logicBounds)) continue;
        runner_update(&game.runners[i], &game.player, dt, (float)game.levelWidth, platformRects, platformCount);

        if (game.aliveSnapshot[i] && game.runners[i].isDead) {
            game.killCount++;
            drops_try_spawn(gameDrops, MAX_DROPS,
                            game.runners[i].rect.x + game.runners[i].rect.width / 2.0f,
                            game.runners[i].rect.y,
                            game.killCount);
        }

        if (game.runners[i].rect.y > game.levelHeight)
            game.runners[i].isDead = true;

        game.runners[i].onGround = false;
        resolve_rect_collision(&game.runners[i].rect, &game.runners[i].vy, platformRects, platformCount, &game.runners[i].onGround);
        for (int j = 0; j < boxCount; j++) {
            if (boxes[j].isBroken) continue;
            resolve_rect_collision(&game.runners[i].rect, &game.runners[i].vy, &boxes[j].rect, 1, &game.runners[i].onGround);
        }
    }

    //boxes system
    for (int i = 0; i < boxCount; i++) box_update(&boxes[i], &game.player);

    //Level loading system. needs a better explanation.
    for (int i = 0; i < levelExitCount; i++) {
        Rectangle playerRect = { game.player.x, game.player.y, TILE_SIZE, TILE_SIZE };
        if (CheckCollisionRecs(playerRect, levelExits[i].rect)) {
            game_init(levelExits[i].destination);
            return;
        }
    }

    camera_update(&game.camera, (Vector2){ game.player.x + 20.0f, game.player.y + 20.0f }, game.levelWidth, game.levelHeight, dt);
}

// --- Scene Lifecycle: Render ---
static void game_render(void) {
    BeginDrawing();
    BeginMode2D(game.camera);
    ClearBackground(DARKGRAY);
    //Render platforms
    for (int i = 0; i < platformCount; i++) DrawRectangleRec(platforms[i].rect, GRAY);
    for (int i = 0; i < movingPlatformCount; i++)
        DrawRectangleRec(movingPlatforms[i].rect, GREEN);
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
    Rectangle viewport = camera_get_viewport(&game.camera);

    for (int i = 0; i < game.runnerCount; i++) {
        if (!CheckCollisionRecs(game.runners[i].rect, viewport)) continue;
        runner_render(&game.runners[i]);
    }

    //Render Exit
    for (int i = 0; i < levelExitCount; i++)
        DrawRectangleRec(levelExits[i].rect, BLACK);

    //Render Player
    player_render(&game.player);
    EndMode2D();
    // UI Overlay
    Weapon currentWeapon = game.player.inventory[game.player.currentSlot];
    DrawText(TextFormat("WEAPON: %s", currentWeapon.name), 20, 20, 20, RAYWHITE);
    DrawText("A/D: Move  W: Jump  SPACE: Fire  Q: Swap", 20, 450, 10, LIGHTGRAY);

    //DEBUG
    #ifdef DEBUG
    Rectangle logicBounds = camera_get_logic_bounds(&game.camera);
    int rendered = 0, active = 0;
    for (int i = 0; i < game.runnerCount; i++) {
        if (game.runners[i].isDead) continue;
        if (CheckCollisionRecs(game.runners[i].rect, viewport))    rendered++;
        if (CheckCollisionRecs(game.runners[i].rect, logicBounds)) active++;
    }
    DrawText(TextFormat("RUNNERS  render:%d  logic:%d  total:%d", rendered, active, runnerCount), 20, 40, 14, LIME);
    DrawText(TextFormat("LEVEL  next:%s  tileset:%s", levelNext, levelTileset), 20, 56, 14, LIME);
    #endif//End DEBUG
    EndDrawing();
}

// --- Entry Point ---
Scene game_scene(void) {
    game_init("level1.txt");
    return (Scene){ game_update, game_render };
}
