// scenes/resolution.h
#pragma once
#include <stdbool.h>

typedef enum {
    AR_4_3 = 0,
    AR_3_2,
    AR_16_9,
    AR_21_9,
    AR_32_9,
    AR_COUNT
} AspectRatio;

typedef enum {
    WM_WINDOWED = 0,
    WM_BORDERLESS,
    WM_FULLSCREEN,
    WM_COUNT
} WindowMode;

typedef struct {
    AspectRatio ratio;
    WindowMode  mode;
} ResolutionConfig;

extern ResolutionConfig g_resolution;

void resolution_init(void);   // llamar una vez en main antes del game loop
void resolution_apply(AspectRatio ratio, WindowMode mode);

const char *resolution_ratio_name(AspectRatio ratio);
const char *resolution_mode_name(WindowMode mode);
int         resolution_base_width(AspectRatio ratio);
