// scenes/game.c
#include "game.h"
#include "../entities/player.h"
#include "../entities/enemy.h"
#include "../entities/box.h"
#include "../entities/bullet.h"
#include <raylib.h>
#include <stdio.h>  // Required for file handling (fopen, fgetc, fclose)

// Scene and Grid Constants
#define TILE_SIZE       40.0f
#define GRID_COLS       16
#define GRID_ROWS       12
#define FLOOR_Y         440.0f
#define SCREEN_WIDTH    640
#define PLAYER_SIZE     40
#define JUMP_FORCE      -400.0f

// --- Scene State (Private Module Storage) ---
static Player    player;
static Enemy     enemies[255];
static int       enemyCount;
static Box       boxes[255];
static int       boxCount;
static Rectangle platforms[255]; // Increased capacity for tilemap layouts
static int       platformCount;
static Bullet    gameBullets[MAX_BULLETS];

// --- Level Loading Logic ---
// Reads a .txt file and spawns entities based on characters
// Reads an external text asset file and populates the entity arrays based on grid coordinates.
// This implementation uses a continuous stream filter to guarantee that hidden carriage returns
// (\r) and newlines (\n) do not cause column-alignment offsets.
static void load_level(const char* filename) {
    FILE *file = fopen(filename, "r");

    // Safety Fallback: If the level file is missing or corrupted,
    // prevent a crash and spawn the player at default safe coordinates.
    if (file == NULL) {
        player = player_init(100, 300);
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
            case 'P': // Player Start Token
                player = player_init(posX, posY);
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
                if (enemyCount < 255) {
                    enemies[enemyCount] = enemy_init(posX, posY, 100.0f);
                    enemyCount++;
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

    // Explicitly release the OS file descriptor handle to avoid memory leaks
    fclose(file);
}

// --- Scene Lifecycle: Init ---
static void game_init(void) {
    // Reset all entity counters before loading
    platformCount = 0;
    boxCount = 0;
    enemyCount = 0;

    // Reset projectile pool
    for (int i = 0; i < MAX_BULLETS; i++) {
        gameBullets[i].isActive = false;
    }

    // Load level data from external text file
    load_level(TextFormat("assets/levels/%s", "level1.txt"));
}

// Internal helper to handle player interaction with static level geometry (platforms/floor)
static void resolve_environment_collisions(Player *p, Rectangle *worldPlatforms, int count) {
    for (int i = 0; i < count; i++) {
        // Only check for landing if the player is moving downwards (vy >= 0)
        if (p->vy >= 0.0f &&
            CheckCollisionRecs((Rectangle){p->x, p->y, 40, 40}, worldPlatforms[i]) &&
            (p->y + 40 <= worldPlatforms[i].y + 10))
        {
            p->y = worldPlatforms[i].y - 40; // Snap to surface
            p->vy = 0;                       // Stop vertical momentum
            p->onGround = true;              // Reset jump ability
        }
    }
}
// --- Scene Lifecycle: Update ---
static void game_update(float dt) {
    // 1. Reset state for the new frame
    player.onGround = false;

    // 2. Core Entity Updates
    player_update(&player, dt);
    player_handle_combat(&player, gameBullets, MAX_BULLETS, dt);
    bullets_update(gameBullets, MAX_BULLETS, dt);

    // 3. Encapsulated Collision/Logic Passes
    // Each system is now responsible for its own interactions
    for (int i = 0; i < enemyCount; i++) enemy_update(&enemies[i], &player, dt);
    for (int i = 0; i < boxCount; i++)   box_update(&boxes[i], &player);

    // 4. Static Environment Resolution
    resolve_environment_collisions(&player, platforms, platformCount);

    // 5. Input Post-Processing
    if (IsKeyPressed(KEY_W) && player.onGround) {
        player.vy = -400.0f; // JUMP_FORCE
        player.onGround = false;
    }
}

// --- Scene Lifecycle: Render ---
static void game_render(void) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    for (int i = 0; i < platformCount; i++) DrawRectangleRec(platforms[i], GRAY);

    for (int i = 0; i < boxCount; i++) {
        // Each box handles its own state updates and player interaction checks
        box_render(&boxes[i]);
    }

    for (int i = 0; i < enemyCount; i++) {
        enemy_render(&enemies[i]);
    }

    bullets_render(gameBullets, MAX_BULLETS);
    player_render(&player);

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
