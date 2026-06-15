// scenes/level_loader.c
#include "level_loader.h"
#include "../entities/runner.h"
#include "../defines.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct { int cols; int rows; } GridSize;

static GridSize measure_level(const char *filename) {
    GridSize size = { 0, 0 };
    FILE *f = fopen(filename, "r");
    if (!f) return size;
    char line[4096];
    bool inGrid = false;
    int maxCols = 0, rows = 0;
    while (fgets(line, sizeof(line), f)) {
        int len = 0;
        while (line[len] && line[len] != '\r' && line[len] != '\n') len++;
        line[len] = '\0';
        if (!inGrid) {
            if (line[0] == '-' && line[1] == '-' && line[2] == '-') inGrid = true;
            continue;
        }
        if (len > maxCols) maxCols = len;
        if (len > 0) rows++;
    }
    fclose(f);
    size.cols = maxCols;
    size.rows = rows;
    return size;
}

LevelData load_level(const char *filename) {
    LevelData data = { 0 };
    bool playerSpawned = false;

    GridSize grid = measure_level(filename);
    data.levelWidth  = grid.cols * (int)TILE_SIZE;
    data.levelHeight = grid.rows * (int)TILE_SIZE;

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        data.player = player_init(0.0f, 0.0f);
        return data;
    }

    char line[4096];
    bool inGrid = false;

    while (!inGrid && fgets(line, sizeof(line), file)) {
        int len = 0;
        while (line[len] && line[len] != '\r' && line[len] != '\n') len++;
        line[len] = '\0';

        if (line[0] == '-' && line[1] == '-' && line[2] == '-') {
            inGrid = true;
            break;
        }

        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = line;
        char *val = eq + 1;

        if (strcmp(key, "tileset") == 0) snprintf(data.tileset, sizeof(data.tileset), "%s", val);
        if (strcmp(key, "win") == 0) snprintf(data.winCondition, sizeof(data.winCondition), "%s", val);

        if (strncmp(key, "exit:", 5) == 0) {
            int idx = atoi(key + 5);
            if (idx >= 0 && idx < MAX_EXITS)
                snprintf(data.exits[idx].destination, 64, "%s", val);
        }

        if (strncmp(key, "moving:", 7) == 0) {
            int idx = atoi(key + 7);
            if (idx >= 0 && idx < MAX_MOVING_PLATFORMS) {
                float dx, dy, speed, accel, decel;
                int spriteIndex;
                sscanf(val, "%f,%f,%f,%f,%f,%d", &dx, &dy, &speed, &accel, &decel, &spriteIndex);
                data.movingPlatforms[idx].pathStart   = (Vector2){ 0.0f, 0.0f };
                data.movingPlatforms[idx].pathEnd     = (Vector2){ dx * TILE_SIZE, dy * TILE_SIZE };
                data.movingPlatforms[idx].speed       = speed;
                data.movingPlatforms[idx].accel       = accel;
                data.movingPlatforms[idx].decel       = decel;
                data.movingPlatforms[idx].spriteIndex = spriteIndex;
                data.movingPlatforms[idx].t           = 0.0f;
                data.movingPlatforms[idx].tVelocity   = 0.0f;
                data.movingPlatforms[idx].tDirection  = 1;
            }
        }
    }

    int row = 0;
    while (row < grid.rows && fgets(line, sizeof(line), file)) {
        int len = 0;
        while (line[len] && line[len] != '\r' && line[len] != '\n') len++;

        int col = 0;
        int i = 0;
        while (i < len && col < grid.cols) {
            unsigned char ch = (unsigned char)line[i];
            float posX = col * TILE_SIZE;
            float posY = row * TILE_SIZE;
            int spriteIndex = -1;

            switch (ch) {
                case 'P':
                    data.player = player_init(posX, posY);
                    playerSpawned = true;
                    break;
                case '5': spriteIndex = 4; break;
                case '8': spriteIndex = 1; break;
                case '2': spriteIndex = 7; break;
                case '4': spriteIndex = 3; break;
                case '6': spriteIndex = 5; break;
                case '7': spriteIndex = 0; break;
                case '9': spriteIndex = 2; break;
                case '1': spriteIndex = 6; break;
                case '3': spriteIndex = 8; break;
                case '=': spriteIndex = 9; break;
                case 'B':
                    if (data.boxCount < MAX_BOXES)
                        data.boxes[data.boxCount++] = box_init(posX, posY);
                break;
                case 'R':
                    if (data.runnerCount < MAX_RUNNERS)
                        data.runners[data.runnerCount++] = runner_init(posX, posY, RUNNER_SPEED);
                break;
                case 'E':
                    if (data.exitCount < MAX_EXITS) {
                        data.exits[data.exitCount].rect = (Rectangle){ posX, posY, TILE_SIZE, TILE_SIZE };
                        data.exitCount++;
                    }
                    break;
                case 'M':
                    if (data.movingPlatformCount < MAX_MOVING_PLATFORMS) {
                        data.movingPlatforms[data.movingPlatformCount].rect = (Rectangle){ posX, posY, TILE_SIZE, TILE_SIZE };
                        data.movingPlatformCount++;
                    }
                    break;
                case '.':
                default:
                    break;
            }

            if (spriteIndex >= 0 && data.platformCount < MAX_PLATFORMS) {
                data.platforms[data.platformCount].rect = (Rectangle){ posX, posY, TILE_SIZE, TILE_SIZE };
                data.platforms[data.platformCount].spriteIndex = spriteIndex;
                data.platformCount++;
            }

            i++;
            col++;
        }
        row++;
    }

    if (!playerSpawned)
        data.player = player_init(0.0f, 0.0f);

    for (int i = 0; i < data.movingPlatformCount; i++) {
        MovingPlatform *mp = &data.movingPlatforms[i];
        mp->pathStart = (Vector2){ mp->rect.x, mp->rect.y };
        bool noPath = (mp->pathEnd.x == 0.0f && mp->pathEnd.y == 0.0f);
        if (noPath) {
            mp->pathEnd     = mp->pathStart;
            mp->speed       = 0.0f;
            mp->spriteIndex = 9;
        } else {
            mp->pathEnd.x += mp->rect.x;
            mp->pathEnd.y += mp->rect.y;
        }
    }

    fclose(file);
    return data;
}
