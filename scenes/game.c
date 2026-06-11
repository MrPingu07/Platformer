// scenes/game.c
#include "game.h"
#include "../entities/player.h"
#include "../entities/runner.h"
#include "../entities/box.h"
#include "../entities/bullet.h"
#include "../entities/collision.h"
#include "../entities/hittable.h"
#include <raylib.h>
#include <stdio.h>  // Required for file handling (fopen, fgetc, fclose)

// Scene and Grid Constants
#define TILE_SIZE       40.0f
#define FLOOR_Y         440.0f
#define SCREEN_WIDTH    640
#define PLAYER_SIZE     40
#define JUMP_FORCE      -400.0f

// --- Scene State (Private Module Storage) ---
static int       GRID_COLS = 0;
static int       GRID_ROWS = 0;
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

// --- Level Loading Logic ---
// Reads a .txt file and spawns entities based on characters
// Reads an external text asset file and populates the entity arrays based on grid coordinates.
// This implementation uses a continuous stream filter to guarantee that hidden carriage returns
// (\r) and newlines (\n) do not cause column-alignment offsets.

static void measure_level(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;

    int cols = 0, maxCols = 0, rows = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\r') continue;
        if (ch == '\n') {
            if (cols > 0) {           // ignora líneas vacías
                if (cols > maxCols) maxCols = cols;
                rows++;
            }
            cols = 0;
        } else {
            cols++;
        }
    }
    // Última línea sin newline final
    if (cols > 0) {
        if (cols > maxCols) maxCols = cols;
        rows++;
    }

    fclose(f);
    GRID_COLS   = maxCols;
    GRID_ROWS   = rows;
    levelWidth  = maxCols * (int)TILE_SIZE;   // TODO: INSERTAR AQUÍ
    levelHeight = rows    * (int)TILE_SIZE;
}

static void load_level(const char* filename) {
    bool playerSpawned = false;
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        player = player_init(0, 0);
        return;
    }

    // Safety Fallback: If the level file is missing or corrupted,
    // prevent a crash and spawn the player at default safe coordinates.
    if (file == NULL) {
        player = player_init(0, 0);
        return;
    }

    int row = 0;
    int col = 0;

    // Process the file byte-by-byte until we fill our defined 16x12 matrix grid
    while (row < GRID_ROWS) {
        int ch = fgetc(file);

        // Hard break if the file ends prematurely before filling the matrix allocation
        if (ch == EOF) break;

        // CROSS-PLATFORM STRIPPER: Ignore formatting whitespace characters.
        // This bypasses both Linux (\n) and Windows (\r\n) line-ending variations seamlessly.
        if (ch == '\n' || ch == '\r') {
            continue;
        }

        // Calculate explicit 2D space coordinate multipliers based on current grid indexes.
        float posX = col * TILE_SIZE;
        float posY = row * TILE_SIZE;

        // Tile Interpreter Matrix
        switch (ch) {
            case 'P':
                player = player_init(posX, posY);
                playerSpawned = true;
                break;

            case '#': // Static Level Architecture Component
            case '=': // Solid Ground Level Component
                if(platformCount < 255) {
                    platforms[platformCount] = (Rectangle){ posX, posY, TILE_SIZE, TILE_SIZE };
                    platformCount++;
                }
                break;

            case 'B': // Destructible Obstacle Entity
                if (boxCount < 255) {
                    boxes[boxCount] = box_init(posX, posY);
                    boxCount++;
                }
                break;

            case 'E': // Active Patrolling Hazard Entity
                if (runnerCount < 255) {
                    runners[runnerCount] = runner_init(posX, posY, 100.0f);
                    runnerCount++;
                }
                break;

            case '.': // Passable Air / Traversal Buffer
            default:
                break;
        }

        // Advance to the adjacent horizontal column slot
        col++;

        // Row Wrapping Controller: If the column pointer exceeds the screen's maximum
        // horizontal tile count (16), reset the counter and descend to the next vertical line.
        if (col >= GRID_COLS) {
            col = 0;
            row++;
        }
    }

    // Fallback: si no se encontró P en el nivel, spawn en posición segura
    if (!playerSpawned)
        player = player_init(0.0f, 0.0f);

    // Explicitly release the OS file descriptor handle to avoid memory leaks
    fclose(file);
}

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

    // Load level data from external text file
    measure_level(TextFormat("assets/levels/%s", "level1.txt"));
    load_level(TextFormat("assets/levels/%s", "level1.txt"));

    camera.target   = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera.offset   = (Vector2){ 640.0f / 2.0f, 480.0f / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom     = 1.0f;
}

// --- Scene Lifecycle: Update ---
static void game_update(float dt) {
    player.onGround = false;
    player_update(&player, dt);
    resolve_environment_collisions(&player, platforms, platformCount);

    // Horizontal movement
    float speed = player.isCrouching ? MOVE_SPEED * 0.5f : MOVE_SPEED;
    bool lockX = IsKeyDown(KEY_W) && player.onGround &&
    !IsKeyDown(KEY_A) && !IsKeyDown(KEY_D);

    if (!lockX) {
        float accel = player.onGround ? ACCELERATION : AIR_ACCELERATION;

        if (IsKeyDown(KEY_D)) { player.vx += accel * dt; player.facing =  1; }
        else if (IsKeyDown(KEY_A)) { player.vx -= accel * dt; player.facing = -1; }

        if (player.vx >  speed) player.vx =  speed;
        if (player.vx < -speed) player.vx = -speed;

        player.x += player.vx * dt;

        if (player.x < 0.0f) { player.x = 0.0f; player.vx = 0.0f; }
        if (player.x + 40.0f > levelWidth) { player.x = levelWidth - 40.0f; player.vx = 0.0f; }
    }

    // Friction
    if (!IsKeyDown(KEY_A) && !IsKeyDown(KEY_D)) {
        float friction = player.onGround ? FRICTION : AIR_FRICTION;
        if (player.vx > 0.0f) { player.vx -= friction * dt; if (player.vx < 0.0f) player.vx = 0.0f; }
        if (player.vx < 0.0f) { player.vx += friction * dt; if (player.vx > 0.0f) player.vx = 0.0f; }
    }

    // Crouch state
    player.isCrouching = IsKeyDown(KEY_S) && player.onGround;
    player_handle_combat(&player, gameBullets, MAX_BULLETS, dt);
    bullets_update(gameBullets, MAX_BULLETS, dt);

    for (int i = 0; i < runnerCount; i++) {
        runner_update(&runners[i], &player, dt, (float)levelWidth, platforms, platformCount);
        runners[i].onGround = false;
        resolve_rect_collision(&runners[i].rect, &runners[i].vy, platforms, platformCount, &runners[i].onGround);
        for (int j = 0; j < boxCount; j++) {
            if (boxes[j].isBroken) continue;
            resolve_rect_collision(&runners[i].rect, &runners[i].vy, &boxes[j].rect, 1, &runners[i].onGround);
        }
    }

    for (int i = 0; i < boxCount; i++) box_update(&boxes[i], &player);

    if (IsKeyPressed(KEY_W) && player.onGround && !player.isCrouching) {
        player.vy = -400.0f;
        player.onGround = false;
    }

    // Deadzone: middle third of the screen
    float playerScreenX = (player.x + 20.0f) - camera.target.x + camera.offset.x;

    float deadzoneLeft  = 280.0f;
    float deadzoneRight = 310.0f;

    float targetX = camera.target.x;

    if (playerScreenX < deadzoneLeft)
        targetX += (playerScreenX - deadzoneLeft);
    if (playerScreenX > deadzoneRight)
        targetX += (playerScreenX - deadzoneRight);

    camera.target.x += (targetX - camera.target.x) * 6.0f * dt;

    // Lerp vertical
    float playerScreenY = (player.y + 20.0f) - camera.target.y + camera.offset.y;

    float deadzoneTop    = 180.0f;
    float deadzoneBottom = 300.0f;

    float targetY = camera.target.y;

    if (playerScreenY < deadzoneTop)
        targetY += (playerScreenY - deadzoneTop);
    if (playerScreenY > deadzoneBottom)
        targetY += (playerScreenY - deadzoneBottom);

    camera.target.y += (targetY - camera.target.y) * 6.0f * dt;

    // Clamp to level borders
    float halfW = 640.0f / 2.0f;
    float halfH = 480.0f / 2.0f;

    if (camera.target.x < halfW)                camera.target.x = halfW;
    if (camera.target.x > levelWidth  - halfW)  camera.target.x = levelWidth  - halfW;
    if (camera.target.y < halfH)                camera.target.y = halfH;
    if (camera.target.y > levelHeight - halfH)  camera.target.y = levelHeight - halfH;

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

    if (player.y > levelHeight + 200.0f) game_init();

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
