// scenes/game.c
#include "game.h"
#include "../entities/player.h"
#include "../entities/enemy.h"
#include "../entities/box.h"
#include <raylib.h>

#define FLOOR_Y       440.0f
#define SCREEN_WIDTH  640
#define PLAYER_SIZE   40
#define JUMP_FORCE -400.0f

// --- Estado de la escena ---
static Player player;
static Enemy  enemies[4];
static int    enemy_count;
static Box    boxes[8];
static int    box_count;
static Rectangle platforms[4];
static int    platform_count;

// --- Init ---
static void game_init(void) {
    player = player_init(100, 300);

    platforms[0] = (Rectangle){200, 350, 150, 20};
    platforms[1] = (Rectangle){400, 280, 120, 20};
    platforms[2] = (Rectangle){100, 200, 100, 20};
    platforms[3] = (Rectangle){50,  150, 80,  20};
    platform_count = 4;

    boxes[0] = box_init(250, 310);
    boxes[1] = box_init(250, 220);
    boxes[2] = box_init(400, 240);
    box_count = 3;

    enemies[0] = enemy_init(300, 400, 100.0f);
    enemy_count = 1;
}

// --- Update ---
static void game_update(float dt) {
    player.on_ground = false;
    player_update(&player, dt);

    // Colisión con plataformas
    for (int i = 0; i < platform_count; i++) {
        bool falling           = player.vy >= 0;
        bool on_platform       = (player.y + PLAYER_SIZE >= platforms[i].y) && (player.y + PLAYER_SIZE <= platforms[i].y + platforms[i].height + 10);
        bool inside_horizontal = (player.x + PLAYER_SIZE > platforms[i].x) && (player.x < platforms[i].x + platforms[i].width);

        if (falling && on_platform && inside_horizontal) {
            player.y         = platforms[i].y - PLAYER_SIZE;
            player.vy        = 0;
            player.on_ground = true;
        }
    }

    // Colisión con suelo
    if (player.y + PLAYER_SIZE >= FLOOR_Y) {
        player.y         = FLOOR_Y - PLAYER_SIZE;
        player.vy        = 0;
        player.on_ground = true;
    }

    // Colisión con cajas
    for (int i = 0; i < box_count; i++) {
        if (boxes[i].broken) continue;
        bool falling           = player.vy >= 0;
        bool on_box            = (player.y + PLAYER_SIZE >= boxes[i].rect.y) && (player.y + PLAYER_SIZE <= boxes[i].rect.y + boxes[i].rect.height + 10);
        bool inside_horizontal = (player.x + PLAYER_SIZE > boxes[i].rect.x) && (player.x < boxes[i].rect.x + boxes[i].rect.width);

        if (falling && on_box && inside_horizontal) {
            player.y         = boxes[i].rect.y - PLAYER_SIZE;
            player.vy        = -400.0f / 1.5f;
            player.on_ground = true;
            boxes[i].broken  = true;
        }
    }

    // Colisión con enemigos
    for (int i = 0; i < enemy_count; i++) {
        if (enemies[i].dead) continue;
        enemy_update(&enemies[i], dt);

        if (CheckCollisionRecs((Rectangle){player.x, player.y, PLAYER_SIZE, PLAYER_SIZE}, enemies[i].rect)) {
            bool stomping = player.vy > 0 && (player.y + PLAYER_SIZE < enemies[i].rect.y + enemies[i].rect.height / 2);

            if (stomping) {
                enemies[i].dead = true;
                player.vy       = -400.0f / 2;
            } else {
                player.x  = 100;
                player.y  = 300;
                player.vy = 0;
            }
        }

    }
// --- Input salto ---
if (IsKeyPressed(KEY_SPACE) && player.on_ground) {
    player.vy       = JUMP_FORCE;
    player.on_ground = false;
    }
}


// --- Render ---
static void game_render(void) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    player_render(&player);

    for (int i = 0; i < platform_count; i++)
        DrawRectangleRec(platforms[i], RAYWHITE);

    for (int i = 0; i < box_count; i++)
        box_render(&boxes[i]);

    for (int i = 0; i < enemy_count; i++)
        enemy_render(&enemies[i]);

    DrawLine(0, (int)FLOOR_Y, SCREEN_WIDTH, (int)FLOOR_Y, RAYWHITE);
    EndDrawing();
}

// --- Scene ---
Scene game_scene(void) {
    game_init();
    return (Scene){ game_update, game_render };
}
