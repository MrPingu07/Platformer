//game_update.c
#include "game_update.h"
#include "camera.h"
#include "moving_platform.h"
#include "level_loader.h"
#include "../entities/player.h"
#include "../entities/runner.h"
#include "../entities/box.h"
#include "../entities/bullet.h"
#include "../entities/collision.h"
#include "../entities/hittable.h"
#include "../entities/drop.h"
#include <raylib.h>
#include <string.h>

const char *game_update_world(GameState *game, float dt) {
    if (game->player.groundPlatformIndex >= game->platformCount && game->player.groundPlatformIndex != -1) {
        int mi = game->player.groundPlatformIndex - game->platformCount;
        game->player.x += game->movingPlatforms[mi].delta.x;
        game->player.y += game->movingPlatforms[mi].delta.y;
    }

    game->player.onGround = false;
    game->player.groundPlatformIndex = -1;
    moving_platforms_update(game->movingPlatforms, game->movingPlatformCount, dt);

    Rectangle platformRects[MAX_PLATFORMS];
    for (int i = 0; i < game->platformCount; i++) platformRects[i] = game->platforms[i].rect;
    for (int i = 0; i < game->movingPlatformCount; i++)
        platformRects[game->platformCount + i] = game->movingPlatforms[i].rect;
    int totalPlatforms = game->platformCount + game->movingPlatformCount;

    player_update(&game->player, dt);

    resolve_environment_collisions(&game->player, platformRects, totalPlatforms);
    if (game->player.y > game->levelHeight) {
        player_respawn(&game->player);
    }

    player_handle_movement(&game->player, game->levelWidth, dt);
    player_handle_combat(&game->player, game->bullets, MAX_BULLETS, dt);
    bullets_update(game->bullets, MAX_BULLETS, dt);

    drops_update(game->drops, MAX_DROPS, platformRects, totalPlatforms, game->levelHeight, dt);
    drops_collect(game->drops, MAX_DROPS, &game->player);

    for (int i = 0; i < game->runnerCount; i++)
        game->aliveSnapshot[i] = !game->runners[i].isDead;

    Hittable targets[255];
    int targetCount = 0;
    for (int i = 0; i < game->runnerCount; i++) {
        targets[targetCount++] = (Hittable){
            .rect   = &game->runners[i].rect,
            .health = &game->runners[i].health,
            .isDead = &game->runners[i].isDead
        };
    }
    resolve_bullet_hittable_collisions(targets, targetCount, game->bullets, MAX_BULLETS);

    Rectangle logicBounds = camera_get_logic_bounds(&game->camera);
    for (int i = 0; i < game->runnerCount; i++) {
        if (!CheckCollisionRecs(game->runners[i].rect, logicBounds)) continue;
        runner_update(&game->runners[i], &game->player, dt, (float)game->levelWidth, platformRects, totalPlatforms);

        if (game->aliveSnapshot[i] && game->runners[i].isDead) {
            game->killCount++;
            drops_try_spawn(game->drops, MAX_DROPS,
                            game->runners[i].rect.x + game->runners[i].rect.width / 2.0f,
                            game->runners[i].rect.y,
                            game->killCount);
        }

        if (game->runners[i].rect.y > game->levelHeight)
            game->runners[i].isDead = true;

        game->runners[i].onGround = false;
        resolve_rect_collision(&game->runners[i].rect, &game->runners[i].vy, platformRects, game->platformCount, &game->runners[i].onGround);
        for (int j = 0; j < game->boxCount; j++) {
            if (game->boxes[j].isBroken) continue;
            resolve_rect_collision(&game->runners[i].rect, &game->runners[i].vy, &game->boxes[j].rect, 1, &game->runners[i].onGround);
        }
    }

    for (int i = 0; i < game->exitCount; i++) {
        Rectangle playerRect = { game->player.x, game->player.y, TILE_SIZE, TILE_SIZE };
        if (CheckCollisionRecs(playerRect, game->exits[i].rect)) {
            bool conditionMet = true;
            if (strcmp(game->winCondition, "kill_all") == 0) {
                conditionMet = true;
                for (int j = 0; j < game->runnerCount; j++)
                    if (!game->runners[j].isDead) { conditionMet = false; break; }
            }
            if (conditionMet)
                return game->exits[i].destination;
        }
    }



    camera_update(&game->camera, (Vector2){ game->player.x + 20.0f, game->player.y + 20.0f }, game->levelWidth, game->levelHeight, dt);
    return 0;
}
