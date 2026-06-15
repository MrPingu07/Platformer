#include "game_init.h"
#include "level_loader.h"
#include "camera.h"
#include "../entities/bullet.h"
#include "../entities/drop.h"
#include <raylib.h>
#include <stdio.h>

void game_init_world(GameState *game, const char *levelFile) {
    game->platformCount = 0;
    game->boxCount      = 0;
    game->runnerCount   = 0;
    game->killCount     = 0;

    for (int i = 0; i < MAX_BULLETS; i++) game->bullets[i].isActive  = false;
    for (int i = 0; i < MAX_DROPS;   i++) game->drops[i].isCollected = true;

    LevelData data = load_level(TextFormat("assets/levels/%s", levelFile));
    TraceLog(LOG_INFO, "Loading level: assets/levels/%s", levelFile);

    game->player        = data.player;
    game->platformCount = data.platformCount;
    game->boxCount      = data.boxCount;
    game->runnerCount   = data.runnerCount;
    game->levelWidth    = data.levelWidth;
    game->levelHeight   = data.levelHeight;

    for (int i = 0; i < game->platformCount;        i++) game->platforms[i]       = data.platforms[i];
    for (int i = 0; i < game->boxCount;             i++) game->boxes[i]           = data.boxes[i];
    for (int i = 0; i < game->runnerCount;          i++) game->runners[i]         = data.runners[i];
    for (int i = 0; i < data.exitCount;             i++) game->exits[i]           = data.exits[i];
    for (int i = 0; i < data.movingPlatformCount;   i++) game->movingPlatforms[i] = data.movingPlatforms[i];

    game->exitCount             = data.exitCount;
    game->movingPlatformCount   = data.movingPlatformCount;

    snprintf(game->levelTileset, sizeof(game->levelTileset), "%s", data.tileset);
    snprintf(game->winCondition, sizeof(game->winCondition), "%s", data.winCondition);


    camera_init(&game->camera, (Vector2){ game->player.x + 20.0f, game->player.y + 20.0f });
}
