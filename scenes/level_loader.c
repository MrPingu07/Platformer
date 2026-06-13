// scenes/level_loader.c
#include "level_loader.h"
#include "../entities/runner.h"
#include "../defines.h"
#include <raylib.h>
#include <stdio.h>

static int GRID_COLS = 0;
static int GRID_ROWS = 0;

static void measure_level(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;

    int cols = 0, maxCols = 0, rows = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\r') continue;
        if (ch == '\n') {
            if (cols > 0) {
                if (cols > maxCols) maxCols = cols;
                rows++;
            }
            cols = 0;
        } else {
            cols++;
        }
    }
    if (cols > 0) {
        if (cols > maxCols) maxCols = cols;
        rows++;
    }

    fclose(f);
    GRID_COLS = maxCols;
    GRID_ROWS = rows;
}

LevelData load_level(const char *filename) {
    LevelData data = { 0 };
    bool playerSpawned = false;

    measure_level(filename);

    data.levelWidth  = GRID_COLS * (int)TILE_SIZE;
    data.levelHeight = GRID_ROWS * (int)TILE_SIZE;

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        data.player = player_init(0.0f, 0.0f);
        return data;
    }

    int row = 0, col = 0;

    while (row < GRID_ROWS) {
        int ch = fgetc(file);
        if (ch == EOF) break;
        if (ch == '\n' || ch == '\r') continue;

        float posX = col * TILE_SIZE;
        float posY = row * TILE_SIZE;

        switch (ch) {
            case 'P':
                data.player = player_init(posX, posY);
                playerSpawned = true;
                break;
            case '#':
            case '=':
                if (data.platformCount < MAX_PLATFORMS) {
                    data.platforms[data.platformCount++] = (Rectangle){ posX, posY, TILE_SIZE, TILE_SIZE };
                }
                break;
            case 'B':
                if (data.boxCount < MAX_BOXES) {
                    data.boxes[data.boxCount++] = box_init(posX, posY);
                }
                break;
            case 'R':
                if (data.runnerCount < MAX_RUNNERS) {
                    data.runners[data.runnerCount++] = runner_init(posX, posY, RUNNER_SPEED);
                }
                break;
            case '.':
            default:
                break;
        }

        col++;
        if (col >= GRID_COLS) {
            col = 0;
            row++;
        }

    }

    if (!playerSpawned)
        data.player = player_init(0.0f, 0.0f);

    fclose(file);
    return data;
}
