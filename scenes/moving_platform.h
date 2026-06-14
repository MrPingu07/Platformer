// scenes/moving_platform.h
#pragma once
#include <raylib.h>

#define MAX_MOVING_PLATFORMS 64

typedef struct {
    Rectangle rect;
    Vector2   pathStart;
    Vector2   pathEnd;
    float     t;
    float     tVelocity;
    int       tDirection;
    float     speed;
    float     accel;
    float     decel;
    int       spriteIndex;
    Vector2   delta;
} MovingPlatform;

void moving_platform_update(MovingPlatform *mp, float dt);
void moving_platforms_update(MovingPlatform *platforms, int count, float dt);
