// entities/collision.c
#include "../defines.h"
#include "hittable.h"
#include "player.h"

void resolve_environment_collisions(Player *p, Rectangle *platforms, int count) {
    float playerH = p->isCrouching ? TILE_SIZE * 0.5f  : TILE_SIZE;
    float playerW = p->isCrouching ? TILE_SIZE * 1.25f : TILE_SIZE;
    float offsetX = p->isCrouching ? -TILE_SIZE * 0.125f : 0.0f;

    for (int i = 0; i < count; i++) {
        Rectangle playerRect = { p->x + offsetX, p->y + (TILE_SIZE - playerH), playerW, playerH };

        if (p->vy < 0.0f) continue;
        if (!CheckCollisionRecs(playerRect, platforms[i])) continue;
        if (p->prevY + TILE_SIZE > platforms[i].y) continue;

        p->y = platforms[i].y - playerH - (TILE_SIZE - playerH);
        p->vy = 0.0f;
        p->onGround = true;
        p->groundPlatformIndex = i;
    }
}

// AGREGAR función nueva para plataformas móviles:
void resolve_moving_platform_collisions(Player *p, Rectangle *platforms, int count, int indexOffset) {
    float playerH = p->isCrouching ? TILE_SIZE * 0.5f  : TILE_SIZE;
    float playerW = p->isCrouching ? TILE_SIZE * 1.25f : TILE_SIZE;
    float offsetX = p->isCrouching ? -TILE_SIZE * 0.125f : 0.0f;

    for (int i = 0; i < count; i++) {
        Rectangle playerRect = { p->x + offsetX, p->y + (TILE_SIZE - playerH), playerW, playerH };
        if (p->vy >= 0.0f &&
            CheckCollisionRecs(playerRect, platforms[i]) &&
            (playerRect.y + playerH <= platforms[i].y + TILE_SIZE * 0.5f))
        {
            p->y = platforms[i].y - playerH - (TILE_SIZE - playerH);
            p->vy = 0.0f;
            p->onGround = true;
            p->groundPlatformIndex = indexOffset + i;
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

void resolve_horizontal_collisions(Player *p, Rectangle *platforms, int count) {
    float playerH = p->isCrouching ? TILE_SIZE * 0.5f  : TILE_SIZE;
    float playerW = p->isCrouching ? TILE_SIZE * 1.25f : TILE_SIZE;
    float offsetX = p->isCrouching ? -TILE_SIZE * 0.125f : 0.0f;
    Rectangle playerRect = { p->x + offsetX, p->y + (TILE_SIZE - playerH), playerW, playerH };

    for (int i = 0; i < count; i++) {
        float t = 3.0f;

        // Buscar vecinos en la misma fila
        bool hasLeft = false, hasRight = false;
        for (int j = 0; j < count; j++) {
            if (j == i) continue;
            if (platforms[j].y != platforms[i].y) continue;
            if (platforms[j].x + TILE_SIZE == platforms[i].x) hasLeft  = true;
            if (platforms[j].x - TILE_SIZE == platforms[i].x) hasRight = true;
        }

        if (!hasLeft) {
            Rectangle leftWall = { platforms[i].x - t, platforms[i].y, t, TILE_SIZE };
            if (CheckCollisionRecs(playerRect, leftWall) && p->vx > 0.0f) {
                p->x = platforms[i].x - playerW - offsetX;
                if (p->onGround) p->vx = 0.0f;
            }
        }
        if (!hasRight) {
            Rectangle rightWall = { platforms[i].x + platforms[i].width, platforms[i].y, t, TILE_SIZE };
            if (CheckCollisionRecs(playerRect, rightWall) && p->vx < 0.0f) {
                p->x = platforms[i].x + platforms[i].width - offsetX;
                if (p->onGround) p->vx = 0.0f;
            }
        }
    }
}

