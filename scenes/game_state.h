// scenes/game_state.h
#pragma once

#include <stdbool.h>
#include <raylib.h>

#include "../entities/player.h"
#include "../entities/runner.h"
#include "../entities/box.h"
#include "../entities/bullet.h"
#include "../entities/drop.h"

#include "level_loader.h"
#include "moving_platform.h"

typedef struct {

    int levelWidth;
    int levelHeight;

    Camera2D camera;

    Player player;

    Runner runners[MAX_RUNNERS];
    int runnerCount;

    Box boxes[MAX_BOXES];
    int boxCount;

    PlatformTile platforms[MAX_PLATFORMS];
    int platformCount;

    Bullet bullets[MAX_BULLETS];

    Drop drops[MAX_DROPS];

    bool aliveSnapshot[MAX_RUNNERS];

    int killCount;

    char levelTileset[64];

    ExitTile exits[MAX_EXITS];
    int exitCount;

    MovingPlatform movingPlatforms[MAX_MOVING_PLATFORMS];
    int movingPlatformCount;

} GameState;
