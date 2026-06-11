// entities/collision.c
#include "collision.h"

void resolve_environment_collisions(Player *p, Rectangle *platforms, int count) {
    float playerH = p->isCrouching ? 20.0f : 40.0f;
    float playerW = p->isCrouching ? 50.0f : 40.0f;
    float offsetX = p->isCrouching ? -5.0f : 0.0f;

    for (int i = 0; i < count; i++) {
        Rectangle playerRect = { p->x + offsetX, p->y + (40.0f - playerH), playerW, playerH };
        if (p->vy >= 0.0f &&
            CheckCollisionRecs(playerRect, platforms[i]) &&
            (playerRect.y + playerH <= platforms[i].y + 20))
        {
            p->y = platforms[i].y - playerH - (40.0f - playerH);
            p->vy = 0;
            p->onGround = true;
        }
    }
}

void resolve_rect_collision(Rectangle *rect, float *vy, Rectangle *platforms, int count, bool *onGround) {
    for (int i = 0; i < count; i++) {
        if (*vy >= 0.0f &&
            CheckCollisionRecs(*rect, platforms[i]) &&
            (rect->y + rect->height <= platforms[i].y + 20.0f))
        {
            rect->y = platforms[i].y - rect->height;
            *vy = 0.0f;
            if (onGround) *onGround = true;
        }
    }
}

void resolve_bullet_hittable_collisions(Hittable *targets, int targetCount, Bullet *bullets, int maxBullets) {
    for (int i = 0; i < targetCount; i++) {
        if (*targets[i].isDead) continue;
        for (int j = 0; j < maxBullets; j++) {
            if (!bullets[j].isActive) continue;
            if (CheckCollisionCircleRec(bullets[j].position, bullets[j].radius, *targets[i].rect)) {
                *targets[i].health -= bullets[j].damage;
                bullets[j].isActive = false;
                if (*targets[i].health <= 0.0f)
                    *targets[i].isDead = true;
            }
        }
    }
}
