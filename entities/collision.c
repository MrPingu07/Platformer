// entities/collision.c
#include "../defines.h"
#include "hittable.h"
#include "player.h"
#include <math.h>
#include "../scenes/moving_platform.h"

void resolve_environment_collisions(Player *p, Rectangle *platforms, int count) {
    float playerH = p->isCrouching ? TILE_SIZE * 0.5f  : TILE_SIZE;
    float playerW = p->isCrouching ? TILE_SIZE * 1.25f : TILE_SIZE;
    float offsetX = p->isCrouching ? -TILE_SIZE * 0.125f : 0.0f;

    for (int i = 0; i < count; i++) {
        Rectangle playerRect = { p->x + offsetX, p->y + (TILE_SIZE - playerH), playerW, playerH };

        if (p->vy < 0.0f) continue;
        if (!CheckCollisionRecs(playerRect, platforms[i])) continue;
        if (p->prevY + TILE_SIZE > platforms[i].y) continue;

        // Debe existir superposición horizontal significativa.
        // Evita aterrizar apoyándose sólo en el costado del tile.
        float overlap =
        fminf(playerRect.x + playerRect.width,
              platforms[i].x + platforms[i].width)
        - fmaxf(playerRect.x,
                platforms[i].x);

        if (overlap < 4.0f)
            continue;

        p->y = platforms[i].y - playerH - (TILE_SIZE - playerH);
        p->vy = 0.0f;
        p->onGround = true;
        p->groundPlatformIndex = i;
    }
}

// función para plataformas móviles:
void resolve_moving_platform_collisions(Player *p, MovingPlatform *platforms, int count, int indexOffset) {
    float playerH = p->isCrouching ? TILE_SIZE * 0.5f  : TILE_SIZE;
    float playerW = p->isCrouching ? TILE_SIZE * 1.25f : TILE_SIZE;
    float offsetX = p->isCrouching ? -TILE_SIZE * 0.125f : 0.0f;

    // --- Pasada 1: resolución vertical para todas las plataformas ---
    for (int i = 0; i < count; i++) {
        Rectangle rect = platforms[i].rect;
        Rectangle playerRect = { p->x + offsetX, p->y + (TILE_SIZE - playerH), playerW, playerH };

        float prevPlatformY = rect.y - platforms[i].delta.y;

        if (p->vy >= 0.0f &&
            CheckCollisionRecs(playerRect, rect) &&
            (playerRect.y + playerH <= rect.y + TILE_SIZE * 0.5f) &&
            (p->prevY + playerH <= prevPlatformY))
        {
            p->y = rect.y - playerH - (TILE_SIZE - playerH);
            p->vy = 0.0f;
            p->onGround = true;
            p->groundPlatformIndex = indexOffset + i;
        }

    }

    // --- Pasada 2: empuje lateral ---
    // Solo aplica si el jugador ya está apoyado en algo. En el aire puede
    // estar atravesando una plataforma one-way desde abajo, y ahí no debe
    // haber ningún contacto lateral, sin importar el solapamiento momentáneo.
    // El caso de colisión lateral en el aire queda pendiente como ticket
    // separado (requiere tracking de overlap entre frames, no resoluble
    // con geometría de un solo frame cuando las plataformas están alineadas).
    if (!p->onGround) return;
    for (int i = 0; i < count; i++) {
        if (p->groundPlatformIndex == indexOffset + i) continue;
        Rectangle rect = platforms[i].rect;
        Rectangle playerRect = { p->x + offsetX, p->y + (TILE_SIZE - playerH), playerW, playerH };
        if (!CheckCollisionRecs(playerRect, rect)) continue;

        // Resolución por penetración mínima: no depende de prevFrameX,
        // así que no se ve afectada por el shift de ride-along de otras
        // plataformas. El lado con menor solapamiento es el lado real
        // de contacto.
        float overlapLeft  = (playerRect.x + playerRect.width) - rect.x;
        float overlapRight = (rect.x + rect.width) - playerRect.x;
        if (overlapLeft < overlapRight) {
            p->x = rect.x - playerW - offsetX;
            if (p->vx > 0.0f) p->vx = 0.0f;
        } else {
            p->x = rect.x + rect.width - offsetX;
            if (p->vx < 0.0f) p->vx = 0.0f;
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

        // Buscar vecinos en la misma fila
        bool hasLeft = false, hasRight = false;
        for (int j = 0; j < count; j++) {
            if (j == i) continue;
            if (platforms[j].y != platforms[i].y) continue;
            if (platforms[j].x + TILE_SIZE == platforms[i].x) hasLeft  = true;
            if (platforms[j].x - TILE_SIZE == platforms[i].x) hasRight = true;
        }

        float dx = p->x - p->prevFrameX;

        Rectangle prevRect = {
            p->prevFrameX + offsetX,
            p->y + (TILE_SIZE - playerH),
            playerW,
            playerH
        };

        if (!hasLeft) {
            float wallX = platforms[i].x;

            float prevRight = p->prevFrameX + offsetX + playerW;
            float currRight = p->x + offsetX + playerW;

            if (prevRight <= wallX &&
                currRight > wallX &&
                CheckCollisionRecs(playerRect, platforms[i]))
            {
                p->x = wallX - playerW - offsetX;
                if (p->vx > 0.0f) p->vx = 0.0f;
            }
        }

        if (!hasRight) {
            float wallX = platforms[i].x + platforms[i].width;

            float prevLeft = p->prevFrameX + offsetX;
            float currLeft = p->x + offsetX;

            if (prevLeft >= wallX &&
                currLeft < wallX &&
                CheckCollisionRecs(playerRect, platforms[i]))
            {
                p->x = wallX - offsetX;
                if (p->vx < 0.0f) p->vx = 0.0f;
            }
        }
    }
}

// Returns true when the player has become buried inside level geometry.
// This is considered an invalid gameplay state.
bool player_is_buried(Player *p, Rectangle *platforms, int count) {
    float playerH = p->isCrouching ? TILE_SIZE * 0.5f  : TILE_SIZE;
    float playerW = p->isCrouching ? TILE_SIZE * 1.25f : TILE_SIZE;
    float offsetX = p->isCrouching ? -TILE_SIZE * 0.125f : 0.0f;

    Rectangle playerRect = {
        p->x + offsetX,
        p->y + (TILE_SIZE - playerH),
        playerW,
        playerH
    };

    Rectangle leftProbe = {
        playerRect.x - 1.0f,
        playerRect.y + 1.0f,
        1.0f,
        playerRect.height - 2.0f
    };

    Rectangle rightProbe = {
        playerRect.x + playerRect.width,
        playerRect.y + 1.0f,
        1.0f,
        playerRect.height - 2.0f
    };

    Rectangle topProbe = {
        playerRect.x + 1.0f,
        playerRect.y - 1.0f,
        playerRect.width - 2.0f,
        1.0f
    };

    Rectangle bottomProbe = {
        playerRect.x + 1.0f,
        playerRect.y + playerRect.height,
        playerRect.width - 2.0f,
        1.0f
    };

    bool left = false;
    bool right = false;
    bool top = false;
    bool bottom = false;

    for (int i = 0; i < count; i++) {
        if (!left   && CheckCollisionRecs(leftProbe,   platforms[i])) left = true;
        if (!right  && CheckCollisionRecs(rightProbe,  platforms[i])) right = true;
        if (!top    && CheckCollisionRecs(topProbe,    platforms[i])) top = true;
        if (!bottom && CheckCollisionRecs(bottomProbe, platforms[i])) bottom = true;
    }

    return left && right && top && bottom;
}
