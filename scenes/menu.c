//menu.c
#include "menu.h"
#include "../scene_manager.h"
#include "level_select.h"
#include "resolution.h"
#include <raylib.h>

typedef enum { MENU_MAIN, MENU_SETTINGS, MENU_RESOLUTION } MenuState;

static MenuState state;
static int mainCursor;
static int settingsCursor;
static AspectRatio pendingRatio;
static WindowMode  pendingMode;
static int         ratioCursor;
static int         modeCursor;

static const char *mainItems[]     = { "Level Select", "Settings" };
static const char *settingsItems[] = { "Resolution", "Return" };
static int mainCount     = 2;
static int settingsCount = 2;

// --- Update ---

static void update_main(void) {
    if (IsKeyPressed(KEY_DOWN)) mainCursor = (mainCursor + 1) % mainCount;
    if (IsKeyPressed(KEY_UP))   mainCursor = (mainCursor - 1 + mainCount) % mainCount;

    if (IsKeyPressed(KEY_ENTER)) {
        if (mainCursor == 0) scene_manager_set(level_select_scene());
        if (mainCursor == 1) { state = MENU_SETTINGS; settingsCursor = 0; }
    }
}

static void update_settings(void) {
    if (IsKeyPressed(KEY_DOWN)) settingsCursor = (settingsCursor + 1) % settingsCount;
    if (IsKeyPressed(KEY_UP))   settingsCursor = (settingsCursor - 1 + settingsCount) % settingsCount;

    if (IsKeyPressed(KEY_ESCAPE)) { state = MENU_MAIN; return; }

    if (IsKeyPressed(KEY_ENTER)) {
        if (settingsCursor == 0) { state = MENU_RESOLUTION; }
        if (settingsCursor == 1)   state = MENU_MAIN;
    }
}

static void update_resolution(void) {
    if (IsKeyPressed(KEY_UP))    ratioCursor = (ratioCursor - 1 + AR_COUNT) % AR_COUNT;
    if (IsKeyPressed(KEY_DOWN))  ratioCursor = (ratioCursor + 1) % AR_COUNT;
    if (IsKeyPressed(KEY_LEFT))  modeCursor  = (modeCursor  - 1 + WM_COUNT) % WM_COUNT;
    if (IsKeyPressed(KEY_RIGHT)) modeCursor  = (modeCursor  + 1) % WM_COUNT;

    if (IsKeyPressed(KEY_ENTER)) {
        pendingRatio = (AspectRatio)ratioCursor;
        pendingMode  = (WindowMode)modeCursor;
        resolution_apply(pendingRatio, pendingMode);
    }

    if (IsKeyPressed(KEY_ESCAPE)) state = MENU_SETTINGS;
}

static void menu_update(float dt) {
    (void)dt;
    if      (state == MENU_MAIN)       update_main();
    else if (state == MENU_SETTINGS)   update_settings();
    else if (state == MENU_RESOLUTION) update_resolution();
}

// --- Render helpers ---

static void draw_menu(const char *title, const char **items, int count, int cursor, int startY) {
    int screenW = GetScreenWidth();
    DrawText(title, screenW / 2 - MeasureText(title, 30) / 2, startY, 30, RAYWHITE);
    for (int i = 0; i < count; i++) {
        Color col      = (i == cursor) ? YELLOW : GRAY;
        const char *prefix = (i == cursor) ? "> " : "  ";
        const char *label  = TextFormat("%s%s", prefix, items[i]);
        DrawText(label, screenW / 2 - MeasureText(label, 20) / 2, startY + 60 + i * 36, 20, col);
    }
}

static void render_resolution(void) {
    int screenW = GetScreenWidth();

    const char *title = "RESOLUTION";
    DrawText(title, screenW / 2 - MeasureText(title, 30) / 2, 100, 30, RAYWHITE);

    const char *arLabel = "ASPECT RATIO  (UP/DOWN)";
    DrawText(arLabel, screenW / 2 - MeasureText(arLabel, 14) / 2, 160, 14, DARKGRAY);
    for (int i = 0; i < AR_COUNT; i++) {
        bool sel = (i == ratioCursor);
        Color col = sel ? YELLOW : GRAY;
        const char *mark  = (i == (int)g_resolution.ratio) ? " *" : "";
        const char *label = TextFormat("%s%s%s",
                                       sel ? "> " : "  ",
                                       resolution_ratio_name((AspectRatio)i),
                                       mark);
        DrawText(label, screenW / 2 - MeasureText(label, 20) / 2, 184 + i * 32, 20, col);
    }

    const char *wmLabel = "WINDOW MODE  (LEFT/RIGHT)";
    int wmY = 184 + AR_COUNT * 32 + 24;
    DrawText(wmLabel, screenW / 2 - MeasureText(wmLabel, 14) / 2, wmY, 14, DARKGRAY);
    for (int i = 0; i < WM_COUNT; i++) {
        bool sel = (i == modeCursor);
        Color col = sel ? YELLOW : GRAY;
        const char *mark  = (i == (int)g_resolution.mode) ? " *" : "";
        const char *label = TextFormat("%s%s%s",
                                       sel ? "> " : "  ",
                                       resolution_mode_name((WindowMode)i),
                                       mark);
        DrawText(label, screenW / 2 - MeasureText(label, 20) / 2, wmY + 24 + i * 32, 20, col);
    }

    const char *hint = "ENTER: apply   ESC: back";
    DrawText(hint, screenW / 2 - MeasureText(hint, 14) / 2, 620, 14, DARKGRAY);
}

static void menu_render(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    if      (state == MENU_MAIN)       draw_menu("PROTOTYPE", mainItems, mainCount, mainCursor, 260);
    else if (state == MENU_SETTINGS)   draw_menu("SETTINGS", settingsItems, settingsCount, settingsCursor, 260);
    else if (state == MENU_RESOLUTION) render_resolution();

    EndDrawing();
}

// --- Entry ---

Scene menu_scene(void) {
    state          = MENU_MAIN;
    mainCursor     = 0;
    settingsCursor = 0;
    ratioCursor    = (int)g_resolution.ratio;
    modeCursor     = (int)g_resolution.mode;
    pendingRatio   = g_resolution.ratio;
    pendingMode    = g_resolution.mode;
    return (Scene){ menu_update, menu_render };
}
