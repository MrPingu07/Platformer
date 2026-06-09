// entities/box.h
#pragma once

#include <stdbool.h>
#include <raylib.h>

typedef struct {
    Rectangle rect;
    bool broken;
} Box;

Box  box_init(float x, float y);
void box_render(Box *b);
