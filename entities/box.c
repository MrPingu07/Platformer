// entities/box.c
#include "box.h"

// Initializes a box at the given coordinates with a fixed 40x40 size
Box box_init(float x, float y) {
    return (Box){
        .rect = (Rectangle){ x, y, 40.0f, 40.0f },
        .isBroken = false
    };
}

void box_update(Box *b, Player *p) {
    // If the box is already destroyed, it has no collision profile left to process
    if (b->isBroken) return;

    Rectangle playerRect = { p->x, p->y, 40.0f, 40.0f };

    // Check if the player interceptor overlaps with the box bounds
    if (p->vy >= 0.0f && CheckCollisionRecs(playerRect, b->rect)) {

        // Stomp validation: Player must be descending and their feet close to the top edge
        bool isStomping = (p->y + 40.0f <= b->rect.y + 10.0f);

        if (isStomping) {
            p->y = b->rect.y - 40.0f;          // Snap player to the top of the box to prevent sinking
            p->vy = -350.0f;             // Apply a scaled upward jump force bounce execution
            p->onGround = true;
            b->isBroken = true;                 // Trigger destruction state
        }
    }
}

// Renders the box only if it hasn't been destroyed yet
void box_render(const Box *b) {
    if (b->isBroken) return;
    DrawRectangleRec(b->rect, BROWN);
    DrawRectangleLinesEx(b->rect, 2.0f, DARKBROWN);
}
