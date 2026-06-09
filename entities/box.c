// entities/box.c
#include "box.h"
#include <raylib.h>

Box box_init(float x, float y) {
    Box b = { {x, y, 40, 40}, false };
    return b;
}

void box_render(Box *b) {
    if (!b->broken) DrawRectangleRec(b->rect, BROWN);
}
