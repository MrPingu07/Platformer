// entities/box.h
#pragma once

#include <raylib.h>
#include "player.h"

// Represents a breakable structural box in the environment
typedef struct {
    Rectangle rect;   // Position and dimensions (X, Y, Width, Height)
    bool isBroken;    // Flag to check if the box has been destroyed
} Box;

// Function declarations (Prototypes)
Box  box_init(float x, float y);
void box_update(Box *b, Player *p);
void box_render(const Box *b);
