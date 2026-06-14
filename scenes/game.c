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

// --- Scene State (Private Module Storage) ---
static int       levelWidth  = 0;
static int       levelHeight = 0;
static Camera2D  camera = { 0 };
static Player    player;
static Runner    runners[MAX_RUNNERS];
static int       runnerCount;
static Box       boxes[MAX_BOXES];
static int       boxCount;
static PlatformTile platforms[MAX_PLATFORMS];
static int       platformCount;
static Bullet    gameBullets[MAX_BULLETS];
static Drop      gameDrops[MAX_DROPS];
static bool      aliveSnapshot[MAX_RUNNERS];
static int       killCount = 0;
static char      levelNext[64];
static char      levelTileset[64];
static ExitTile  levelExits[MAX_EXITS];
static int       levelExitCount = 0;
static int       movingPlatformCount = 0;
static MovingPlatform movingPlatforms[MAX_MOVING_PLATFORMS];
//

// --- Scene Lifecycle: Init ---
static void game_init(const char *levelFile) {
    // Reset all entity counters before loading
    platformCount = 0;
    boxCount      = 0;
    runnerCount   = 0;
    killCount     = 0;

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
    player        = data.player;
    platformCount = data.platformCount;
    boxCount      = data.boxCount;
    runnerCount   = data.runnerCount;
    levelWidth    = data.levelWidth;
    levelHeight   = data.levelHeight;
    for (int i = 0; i < platformCount; i++) platforms[i] = data.platforms[i];
    for (int i = 0; i < boxCount;      i++) boxes[i]     = data.boxes[i];
    for (int i = 0; i < runnerCount;   i++) runners[i]   = data.runners[i];

    snprintf(levelTileset, sizeof(levelTileset), "%s", data.tileset);
    levelExitCount = data.exitCount;
    for (int i = 0; i < levelExitCount; i++) levelExits[i] = data.exits[i];

    movingPlatformCount = data.movingPlatformCount;
    for (int i = 0; i < movingPlatformCount; i++) movingPlatforms[i] = data.movingPlatforms[i];

    camera_init(&camera, (Vector2){ player.x + 20.0f, player.y + 20.0f });
}

// --- Scene Lifecycle: Update ---
static void game_update(float dt) {
    // 1. Aplicar delta del frame ANTERIOR antes de resetear
    if (player.groundPlatformIndex >= platformCount && player.groundPlatformIndex != -1) {
        int mi = player.groundPlatformIndex - platformCount;
        player.x += movingPlatforms[mi].delta.x;
        player.y += movingPlatforms[mi].delta.y;
    }

    // 2. Reset, luego mover plataformas y reconstruir rects
    player.onGround = false;
    player.groundPlatformIndex = -1;
    moving_platforms_update(movingPlatforms, movingPlatformCount, dt);
    Rectangle platformRects[MAX_PLATFORMS];
    for (int i = 0; i < platformCount; i++) platformRects[i] = platforms[i].rect;
    for (int i = 0; i < movingPlatformCount; i++)
        platformRects[platformCount + i] = movingPlatforms[i].rect;
    int totalPlatforms = platformCount + movingPlatformCount;

    // 3. Física y colisión setean groundPlatformIndex para el próximo frame
    player_update(&player, dt);
    resolve_environment_collisions(&player, platformRects, totalPlatforms);

    //Handle Movement
    player_handle_movement(&player, levelWidth, dt);

    //Handle Combat
    player_handle_combat(&player, gameBullets, MAX_BULLETS, dt);
    bullets_update(gameBullets, MAX_BULLETS, dt);

    //Drops system
    drops_update(gameDrops, MAX_DROPS, platformRects, totalPlatforms, levelHeight, dt);
    drops_collect(gameDrops, MAX_DROPS, &player);

    for (int i = 0; i < runnerCount; i++) aliveSnapshot[i] = false;

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

    Rectangle logicBounds = camera_get_logic_bounds(&camera);

    for (int i = 0; i < runnerCount; i++) {
        if (!CheckCollisionRecs(runners[i].rect, logicBounds)) continue;
        runner_update(&runners[i], &player, dt, (float)levelWidth, platformRects, platformCount);

        if (aliveSnapshot[i] && runners[i].isDead) {
            killCount++;
            drops_try_spawn(gameDrops, MAX_DROPS,
                            runners[i].rect.x + runners[i].rect.width / 2.0f,
                            runners[i].rect.y,
                            killCount);
        }

        if (runners[i].rect.y > levelHeight)
            runners[i].isDead = true;

        runners[i].onGround = false;
        resolve_rect_collision(&runners[i].rect, &runners[i].vy, platformRects, platformCount, &runners[i].onGround);
        for (int j = 0; j < boxCount; j++) {
            if (boxes[j].isBroken) continue;
            resolve_rect_collision(&runners[i].rect, &runners[i].vy, &boxes[j].rect, 1, &runners[i].onGround);
        }
    }

    //boxes system
    for (int i = 0; i < boxCount; i++) box_update(&boxes[i], &player);

    //Level loading system. needs a better explanation.
    for (int i = 0; i < levelExitCount; i++) {
        Rectangle playerRect = { player.x, player.y, TILE_SIZE, TILE_SIZE };
        if (CheckCollisionRecs(playerRect, levelExits[i].rect)) {
            game_init(levelExits[i].destination);
            return;
        }
    }

    camera_update(&camera, (Vector2){ player.x + 20.0f, player.y + 20.0f }, levelWidth, levelHeight, dt);
}

// --- Scene Lifecycle: Render ---
static void game_render(void) {
    BeginDrawing();
    BeginMode2D(camera);
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
    Rectangle viewport = camera_get_viewport(&camera);

    for (int i = 0; i < runnerCount; i++) {
        if (!CheckCollisionRecs(runners[i].rect, viewport)) continue;
        runner_render(&runners[i]);
    }

    //Render Exit
    for (int i = 0; i < levelExitCount; i++)
        DrawRectangleRec(levelExits[i].rect, BLACK);

    //Render Player
    player_render(&player);
    EndMode2D();
    // UI Overlay
    Weapon currentWeapon = player.inventory[player.currentSlot];
    DrawText(TextFormat("WEAPON: %s", currentWeapon.name), 20, 20, 20, RAYWHITE);
    DrawText("A/D: Move  W: Jump  SPACE: Fire  Q: Swap", 20, 450, 10, LIGHTGRAY);

    //DEBUG
    #ifdef DEBUG
    Rectangle logicBounds = camera_get_logic_bounds(&camera);
    int rendered = 0, active = 0;
    for (int i = 0; i < runnerCount; i++) {
        if (runners[i].isDead) continue;
        if (CheckCollisionRecs(runners[i].rect, viewport))    rendered++;
        if (CheckCollisionRecs(runners[i].rect, logicBounds)) active++;
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
