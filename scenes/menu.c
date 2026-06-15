#include "menu.h"
#include "../scene_manager.h"
#include "game.h"
#include <raylib.h>

typedef enum { MENU_MAIN, MENU_SETTINGS, MENU_RESOLUTION } MenuState;

typedef struct { int w, h; } Resolution;

static Resolution resolutions[] = {
    { 1280,  720 },
    { 1920, 1080 },
    { 2560, 1440 },
};
static int resCount = 3;

static MenuState state;
static int mainCursor;
static int settingsCursor;
static int resCursor;
static int selectedRes;

static const char *mainItems[]     = { "Play", "Settings" };
static const char *settingsItems[] = { "Resolution", "Return" };
static int mainCount     = 2;
static int settingsCount = 2;

// --- Update ---

static void update_main(void) {
    if (IsKeyPressed(KEY_DOWN)) mainCursor = (mainCursor + 1) % mainCount;
    if (IsKeyPressed(KEY_UP))   mainCursor = (mainCursor - 1 + mainCount) % mainCount;

    if (IsKeyPressed(KEY_ENTER)) {
        if (mainCursor == 0) scene_manager_set(game_scene());
        if (mainCursor == 1) { state = MENU_SETTINGS; settingsCursor = 0; }
    }
}

static void update_settings(void) {
    if (IsKeyPressed(KEY_DOWN)) settingsCursor = (settingsCursor + 1) % settingsCount;
    if (IsKeyPressed(KEY_UP))   settingsCursor = (settingsCursor - 1 + settingsCount) % settingsCount;

    if (IsKeyPressed(KEY_ESCAPE)) { state = MENU_MAIN; return; }

    if (IsKeyPressed(KEY_ENTER)) {
        if (settingsCursor == 0) { state = MENU_RESOLUTION; resCursor = selectedRes; }
        if (settingsCursor == 1)   state = MENU_MAIN;
    }
}

static void update_resolution(void) {
    if (IsKeyPressed(KEY_DOWN)) resCursor = (resCursor + 1) % resCount;
    if (IsKeyPressed(KEY_UP))   resCursor = (resCursor - 1 + resCount) % resCount;

    if (IsKeyPressed(KEY_ENTER)) {
        selectedRes = resCursor;
        // dummy: aquí iría SetWindowSize(resolutions[selectedRes].w, resolutions[selectedRes].h);
        state = MENU_SETTINGS;
    }
    if (IsKeyPressed(KEY_ESCAPE)) state = MENU_SETTINGS;
}

static void menu_update(float dt) {
    (void)dt;
    if (state == MENU_MAIN)       update_main();
    else if (state == MENU_SETTINGS)   update_settings();
    else if (state == MENU_RESOLUTION) update_resolution();
}

// --- Render helpers ---

static void draw_menu(const char *title, const char **items, int count, int cursor, int startY) {
    int screenW = GetScreenWidth();
    DrawText(title, screenW / 2 - MeasureText(title, 30) / 2, startY, 30, RAYWHITE);
    for (int i = 0; i < count; i++) {
        Color col = (i == cursor) ? YELLOW : GRAY;
        const char *prefix = (i == cursor) ? "> " : "  ";
        const char *label  = TextFormat("%s%s", prefix, items[i]);
        DrawText(label, screenW / 2 - MeasureText(label, 20) / 2, startY + 60 + i * 36, 20, col);
    }
}

static void menu_render(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    if (state == MENU_MAIN) {
        draw_menu("PROTOTYPE", mainItems, mainCount, mainCursor, 260);
    }
    else if (state == MENU_SETTINGS) {
        draw_menu("SETTINGS", settingsItems, settingsCount, settingsCursor, 260);
    }
    else if (state == MENU_RESOLUTION) {
        int screenW = GetScreenWidth();
        const char *title = "RESOLUTION";
        DrawText(title, screenW / 2 - MeasureText(title, 30) / 2, 260, 30, RAYWHITE);
        for (int i = 0; i < resCount; i++) {
            Color col = (i == resCursor) ? YELLOW : GRAY;
            const char *prefix = (i == resCursor) ? "> " : "  ";
            const char *label  = TextFormat("%s%dx%d%s",
                                            prefix,
                                            resolutions[i].w, resolutions[i].h,
                                            (i == selectedRes) ? "  *" : "");
            DrawText(label, screenW / 2 - MeasureText(label, 20) / 2, 320 + i * 36, 20, col);
        }
        DrawText("ESC: volver", screenW / 2 - MeasureText("ESC: volver", 14) / 2, 500, 14, DARKGRAY);
    }

    EndDrawing();
}

// --- Entry ---

Scene menu_scene(void) {
    state         = MENU_MAIN;
    mainCursor    = 0;
    settingsCursor = 0;
    resCursor     = 0;
    selectedRes   = 0;
    return (Scene){ menu_update, menu_render };
}
