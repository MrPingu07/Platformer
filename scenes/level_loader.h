// scenes/level_loader.h
#pragma once
#include "../entities/player.h"
#include "../entities/runner.h"
#include "../entities/box.h"
#include "moving_platform.h"

#define MAX_PLATFORMS 2048
#define MAX_BOXES     2048
#define MAX_RUNNERS   2048
#define MAX_EXITS     16

typedef struct {
    Rectangle rect;
    int       spriteIndex;
} PlatformTile;

typedef struct {
    Rectangle rect;
    char      destination[64];
} ExitTile;

typedef struct {
    PlatformTile    platforms[MAX_PLATFORMS];
    int             platformCount;
    Box             boxes[MAX_BOXES];
    int             boxCount;
    Runner          runners[MAX_RUNNERS];
    int             runnerCount;
    Player          player;
    int             levelWidth;
    int             levelHeight;
    char            tileset[64];
    ExitTile        exits[MAX_EXITS];
    int             exitCount;
    MovingPlatform  movingPlatforms[MAX_MOVING_PLATFORMS];
    int             movingPlatformCount;
} LevelData;

LevelData load_level(const char *filename);
