// entities/enemy.c
#include "enemy.h"

Enemy enemy_init(float x, float y, float speedX) {
    return (Enemy){
        .rect = (Rectangle){ x, y, 40.0f, 40.0f },
        .speedX = speedX,
        .isDead = false
    };
}

void enemy_update(Enemy *e, Player *p, float dt) {
    if (e->isDead) return;

    // 1. Horizontal Patrol Movement Vector Translation
    e->rect.x += e->speedX * dt;

    // Screen boundary bounce logic (640 is the current layout width)
    if (e->rect.x <= 0.0f || e->rect.x + e->rect.width >= 640.0f) {
        e->speedX *= -1.0f;
    }

    // 2. Active Intersector Collision Matrix Resolution
    Rectangle playerRect = { p->x, p->y, 40.0f, 40.0f }; // Standardized player bounds size Matrix

    if (CheckCollisionRecs(playerRect, e->rect)) {
        // Evaluate if the incoming landing vector qualifies as a vertical stomp action
        // Player must be falling (vy > 0) and hitting the top section of the enemy collider
        bool isStomping = (p->vy > 0.0f) && (p->y + 40.0f < e->rect.y + 20.0f);

        if (isStomping) {
            e->isDead = true;
            p->vy = -400.0f / 2.0f; // Execute fixed upward bounce kickback impulse
        } else {
            // Damage Consequence Fallback: Perform a soft reset on player spatial coordinates
            p->x = 100.0f;
            p->y = 300.0f;
            p->vy = 0.0f;
        }
    }
}

void enemy_render(const Enemy *e) {
    if (e->isDead) return;
    DrawRectangleRec(e->rect, RED);
}
