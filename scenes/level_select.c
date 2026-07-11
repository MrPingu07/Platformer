// scenes/level_select.c
#include "level_select.h"
#include "game.h"
#include "../scene_manager.h"
#include "menu.h"
#include <raylib.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEVELS       256
#define LEVELS_PER_PAGE  8
#define LEVEL_NAME_LEN   64
#define LEVELS_DIR       "assets/levels"

// --- Storage (static, nunca heap) ---
static char  levelNames[MAX_LEVELS][LEVEL_NAME_LEN];
static int   levelCount  = 0;
static int   cursor      = 0;   // índice dentro de la página actual
static int   page        = 0;   // página actual (base 0)
static bool  loaded      = false;

// --- Helpers ---

static int cmp_names(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

static void levels_load(void) {
    levelCount = 0;
    DIR *d = opendir(LEVELS_DIR);
    if (!d) return;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL && levelCount < MAX_LEVELS) {
        const char *name = entry->d_name;
        int len = (int)strlen(name);
        if (len < 5) continue;                          // mínimo "a.txt"
        if (strcmp(name + len - 4, ".txt") != 0) continue;
        strncpy(levelNames[levelCount], name, LEVEL_NAME_LEN - 1);
        levelNames[levelCount][LEVEL_NAME_LEN - 1] = '\0';
        levelCount++;
    }
    closedir(d);
    qsort(levelNames, levelCount, LEVEL_NAME_LEN, cmp_names);
    loaded = true;
}

static void levels_clear(void) {
    // Borra los nombres y resetea contadores.
    // El array es estático, solo pisamos con ceros.
    for (int i = 0; i < levelCount; i++)
        levelNames[i][0] = '\0';
    levelCount = 0;
    cursor     = 0;
    page       = 0;
    loaded     = false;
}

static int page_count(void) {
    if (levelCount == 0) return 1;
    return (levelCount + LEVELS_PER_PAGE - 1) / LEVELS_PER_PAGE;
}

// Índice global del ítem bajo el cursor
static int selected_index(void) {
    return page * LEVELS_PER_PAGE + cursor;
}

// Cuántos ítems hay en la página actual
static int items_on_page(void) {
    int start = page * LEVELS_PER_PAGE;
    int remaining = levelCount - start;
    return remaining < LEVELS_PER_PAGE ? remaining : LEVELS_PER_PAGE;
}

// --- Lifecycle ---

static void level_select_update(float dt) {
    (void)dt;

    int onPage = items_on_page();

    if (IsKeyPressed(KEY_UP))
        cursor = (cursor - 1 + onPage) % onPage;

    if (IsKeyPressed(KEY_DOWN))
        cursor = (cursor + 1) % onPage;

    if (IsKeyPressed(KEY_RIGHT) && page < page_count() - 1) {
        page++;
        cursor = 0;
    }

    if (IsKeyPressed(KEY_LEFT) && page > 0) {
        page--;
        cursor = 0;
    }

    if (IsKeyPressed(KEY_ENTER) && levelCount > 0) {
        char chosen[LEVEL_NAME_LEN];
        strncpy(chosen, levelNames[selected_index()], LEVEL_NAME_LEN - 1);
        chosen[LEVEL_NAME_LEN - 1] = '\0';
        levels_clear();
        scene_manager_set(game_scene_from(chosen));
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        levels_clear();                          // liberar al retroceder
        scene_manager_set(menu_scene());
        return;
    }
}

static void level_select_render(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    int screenW = GetScreenWidth();
    const char *title = "SELECT LEVEL";
    DrawText(title, screenW / 2 - MeasureText(title, 30) / 2, 80, 30, RAYWHITE);

    if (levelCount == 0) {
        const char *msg = "No levels found in assets/levels/";
        DrawText(msg, screenW / 2 - MeasureText(msg, 18) / 2, 260, 18, GRAY);
    } else {
        int start  = page * LEVELS_PER_PAGE;
        int onPage = items_on_page();

        for (int i = 0; i < onPage; i++) {
            bool sel = (i == cursor);
            Color col       = sel ? YELLOW : GRAY;
            const char *prefix = sel ? "> " : "  ";
            const char *label  = TextFormat("%s%s", prefix, levelNames[start + i]);
            int y = 160 + i * 36;
            DrawText(label, screenW / 2 - MeasureText(label, 20) / 2, y, 20, col);
        }

        // Indicador de página
        const char *pageLabel = TextFormat("< Page %d / %d >", page + 1, page_count());
        DrawText(pageLabel,
                 screenW / 2 - MeasureText(pageLabel, 16) / 2,
                 160 + LEVELS_PER_PAGE * 36 + 12,
                 16, DARKGRAY);
    }

    const char *hint = "UP/DOWN: navigate   LEFT/RIGHT: page   ENTER: load   ESC: back";
    DrawText(hint, screenW / 2 - MeasureText(hint, 12) / 2, 660, 12, DARKGRAY);

    EndDrawing();
}

Scene level_select_scene(void) {
    if (!loaded) levels_load();   // scan único por visita
    cursor = 0;
    page   = 0;
    return (Scene){ level_select_update, level_select_render };
}
