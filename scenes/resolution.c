// scenes/resolution.c
#include "resolution.h"
#include "../defines.h"
#include <raylib.h>

ResolutionConfig g_resolution = { AR_16_9, WM_WINDOWED };

static const char *ratioNames[AR_COUNT] = { "4:3", "3:2", "16:9", "21:9", "32:9" };
static const char *modeNames[WM_COUNT]  = { "Windowed", "Borderless", "Fullscreen" };
static const int   baseWidths[AR_COUNT] = {
    AR_4_3_W, AR_3_2_W, AR_16_9_W, AR_21_9_W, AR_32_9_W
};

const char *resolution_ratio_name(AspectRatio ratio) { return ratioNames[ratio]; }
const char *resolution_mode_name(WindowMode mode)    { return modeNames[mode];   }
int         resolution_base_width(AspectRatio ratio) { return baseWidths[ratio]; }

void resolution_init(void) {
    resolution_apply(g_resolution.ratio, g_resolution.mode);
}

void resolution_apply(AspectRatio ratio, WindowMode mode) {
    int baseW = baseWidths[ratio];

    // Salir de cualquier modo especial antes de cambiar
    if (IsWindowFullscreen())       ToggleFullscreen();
    if (IsWindowState(FLAG_WINDOW_UNDECORATED)) ClearWindowState(FLAG_WINDOW_UNDECORATED);

    // Calcular escala: mayor múltiplo entero de BASE_HEIGHT que entre en el monitor
    int monitorH = GetMonitorHeight(GetCurrentMonitor());
    int monitorW = GetMonitorWidth(GetCurrentMonitor());
    int scale    = monitorH / BASE_HEIGHT;
    if (scale < 1) scale = 1;

    int winW = baseW * scale;
    int winH = BASE_HEIGHT * scale;

    // Asegurarse que el ancho también entre (para ratios muy anchos en monitores chicos)
    while (scale > 1 && winW > monitorW) {
        scale--;
        winW = baseW * scale;
        winH = BASE_HEIGHT * scale;
    }

    switch (mode) {
        case WM_WINDOWED:
            SetWindowSize(winW, winH);
            SetWindowPosition(
                (monitorW - winW) / 2,
                              (monitorH - winH) / 2
            );
            break;

        case WM_BORDERLESS:
            SetWindowSize(monitorW, monitorH);
            SetWindowPosition(0, 0);
            SetWindowState(FLAG_WINDOW_UNDECORATED);
            break;

        case WM_FULLSCREEN:
            SetWindowSize(monitorW, monitorH);
            ToggleFullscreen();
            break;

        default: break;
    }

    g_resolution.ratio = ratio;
    g_resolution.mode  = mode;
}
