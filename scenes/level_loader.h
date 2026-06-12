// scenes/level_loader.h
#pragma once

#include "../entities/player.h"
#include "../entities/runner.h"
#include "../entities/box.h"

#define MAX_PLATFORMS 2048
#define MAX_BOXES     2048
#define MAX_RUNNERS   2048

typedef struct {
    Rectangle platforms[MAX_PLATFORMS];
    int       platformCount;
    Box       boxes[MAX_BOXES];
    int       boxCount;
    Runner    runners[MAX_RUNNERS];
    int       runnerCount;
    Player    player;
    int       levelWidth;
    int       levelHeight;
} LevelData;

LevelData load_level(const char *filename);
