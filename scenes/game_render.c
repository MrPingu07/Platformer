#include "game_render.h"
#include "camera.h"
#include "../entities/player.h"
#include "../entities/runner.h"
#include "../entities/box.h"
#include "../entities/bullet.h"
#include "../entities/drop.h"
#include <raylib.h>
#include <string.h>

void game_render_world(GameState *game) {
    BeginDrawing();
    BeginMode2D(game->camera);
    ClearBackground(DARKGRAY);

    for (int i = 0; i < game->platformCount; i++) DrawRectangleRec(game->platforms[i].rect, GRAY);
    for (int i = 0; i < game->movingPlatformCount; i++) DrawRectangleRec(game->movingPlatforms[i].rect, GREEN);

    for (int i = 0; i < game->boxCount; i++) box_render(&game->boxes[i]);

    bullets_render(game->bullets, MAX_BULLETS);
    drops_render(game->drops, MAX_DROPS);

    Rectangle viewport = camera_get_viewport(&game->camera);
    for (int i = 0; i < game->runnerCount; i++) {
        if (!CheckCollisionRecs(game->runners[i].rect, viewport)) continue;
        runner_render(&game->runners[i]);
    }

    for (int i = 0; i < game->exitCount; i++)
        DrawRectangleRec(game->exits[i].rect, BLACK);

    for (int i = 0; i < game->exitCount; i++) {
        bool conditionMet = true;
        if (strcmp(game->winCondition, "kill_all") == 0)
            for (int j = 0; j < game->runnerCount; j++)
                if (!game->runners[j].isDead) { conditionMet = false; break; }
                DrawRectangleRec(game->exits[i].rect, conditionMet ? WHITE : RED);
    }

    player_render(&game->player);
    EndMode2D();

    Weapon currentWeapon = game->player.inventory[game->player.currentSlot];
    DrawText(TextFormat("WEAPON: %s", currentWeapon.name), 20, 20, 20, RAYWHITE);
    DrawText("A/D: Move  W: Jump  SPACE: Fire  Q: Swap", 20, 450, 10, LIGHTGRAY);

    #ifdef DEBUG
    Rectangle logicBounds = camera_get_logic_bounds(&game->camera);
    int rendered = 0, active = 0;
    for (int i = 0; i < game->runnerCount; i++) {
        if (game->runners[i].isDead) continue;
        if (CheckCollisionRecs(game->runners[i].rect, viewport))    rendered++;
        if (CheckCollisionRecs(game->runners[i].rect, logicBounds)) active++;
    }
    DrawText(TextFormat("RUNNERS  render:%d  logic:%d  total:%d", rendered, active, game->runnerCount), 20, 40, 14, LIME);
    DrawText(TextFormat("TILESET: %s", game->levelTileset), 20, 56, 14, LIME);
    #endif

    EndDrawing();
}
