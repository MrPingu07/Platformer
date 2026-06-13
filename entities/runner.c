// entities/runner.c
#include "runner.h"
#include "../defines.h"
#include <math.h>

#define EDGE_RAY_LENGTH (TILE_SIZE * 2.0f)
#define EDGE_RAY_STEPS  8

Runner runner_init(float x, float y, float speedX) {
    return (Runner){
        .rect = (Rectangle){ x, y, TILE_SIZE, TILE_SIZE },
        .speedX = speedX,
        .vy = 0.0f,
        .facing  = speedX >= 0.0f ? 1 : -1,
        .health  = 100.0f,
        .playerDetected = false,
        .lastKnownPos   = (Vector2){ 0.0f, 0.0f },
        .memoryTimer    = 0.0f,
        .hasMemory      = false,
        .onGround       = false,
        .isAggro        = false,
        .detectTimer    = 0.0f,
        .isDead = false
    };
}

bool runner_check_edge(Runner *r, Rectangle *platforms, int platformCount) {
    float startX = r->facing == 1 ? r->rect.x + r->rect.width : r->rect.x;
    float startY = r->rect.y;

    float dirX = r->facing * 0.707f;
    float dirY = 0.707f;

    for (int s = 1; s <= EDGE_RAY_STEPS; s++) {
        float t = (EDGE_RAY_LENGTH / EDGE_RAY_STEPS) * s;
        float px = startX + dirX * t;
        float py = startY + dirY * t;

        for (int i = 0; i < platformCount; i++) {
            if (CheckCollisionPointRec((Vector2){px, py}, platforms[i]))
                return false; // Hay suelo, no hay caida
        }
    }
    return true; // Floor not found. Fall inminent
}

static bool runner_detect_player(Runner *r, Player *p) {
    float dx = (p->x + 20.0f) - (r->rect.x + r->rect.width / 2.0f);
    float dy = (p->y + 20.0f) - (r->rect.y + r->rect.height / 2.0f);
    float dist = sqrtf(dx * dx + dy * dy);
    float range = r->isAggro ? RUNNER_DETECT_RANGE * 2.0f : RUNNER_DETECT_RANGE;
    return dist <= range;
}

void runner_update(Runner *r, Player *p, float dt, float levelWidth, Rectangle *platforms, int platformCount) {
    if (r->isDead) return;

    bool wasDetected = r->playerDetected;
    r->playerDetected = runner_detect_player(r, p);

    // Jugador detectado: actualizar ultima posicion conocida y resetear memoria
    if (r->playerDetected) {
        r->lastKnownPos = (Vector2){ p->x, p->y };
        r->memoryTimer  = RUNNER_MEMORY_TIME;
        r->hasMemory    = true;
    }

    // Flanco de subida: acaba de detectar al jugador, freeze
    if (r->playerDetected && !wasDetected)
        r->detectTimer = 1.0f;

    // Freeze de realization
    if (r->detectTimer > 0.0f) {
        r->detectTimer -= dt;
        r->vy += TILE_SIZE * 80.0f * dt;
        r->rect.y += r->vy * dt;
        return;
    }

    // Tick de memoria
    if (r->hasMemory && !r->playerDetected) {
        r->memoryTimer -= dt;
        if (r->memoryTimer <= 0.0f)
            r->hasMemory = false;
    }

    if (r->detectTimer <= 0.0f && r->playerDetected)
        r->isAggro = true;
    if (!r->playerDetected && !r->hasMemory)
        r->isAggro = false;

    if (r->playerDetected || r->hasMemory) {
        // Target: jugador real o ultima posicion conocida
        float targetX = r->playerDetected ? p->x : r->lastKnownPos.x;
        float targetY = r->playerDetected ? p->y : r->lastKnownPos.y;

        float dx = targetX - r->rect.x;
        float dy = targetY - r->rect.y;

        r->speedX = dx > 0.0f ? RUNNER_SPEED * 1.5f : -RUNNER_SPEED * 1.5f;
        r->facing = dx > 0.0f ? 1 : -1;

        if (!r->playerDetected && r->hasMemory && fabsf(dx) < 20.0f)
            r->hasMemory = false;

        if (targetY < r->rect.y - 20.0f && r->onGround)
            r->vy = -(TILE_SIZE * 12.5f);

    } else {
        // Modo patrulla
        if (runner_check_edge(r, platforms, platformCount)) {
            r->facing *= -1;
            r->speedX *= -1;
        }
    }

    r->rect.x += r->speedX * dt;
    r->vy += (TILE_SIZE * 20.0f) * dt;
    r->rect.y += r->vy * dt;

    if (r->rect.x <= 0.0f || r->rect.x + r->rect.width >= levelWidth)
        r->speedX *= -1.0f;

    Rectangle playerRect = { p->x, p->y, TILE_SIZE, TILE_SIZE };
    if (CheckCollisionRecs(playerRect, r->rect)) {
        bool isStomping = (p->vy > 0.0f) && (p->y + TILE_SIZE < r->rect.y + TILE_SIZE * 0.5f);
        if (isStomping) {
            r->isDead = true;
            p->vy = -(TILE_SIZE * 10.0f) * 0.5f;
        } else {
            player_respawn(p);  // sustituye las 3 líneas hardcodeadas
        }
    }
}

void runner_render(const Runner *r) {
    if (r->isDead) return;

    // Color segun estado
    Color bodyColor = RED;
    if (r->detectTimer > 0.0f)        bodyColor = YELLOW;
    else if (r->playerDetected)        bodyColor = ORANGE;
    else if (r->hasMemory)             bodyColor = PURPLE;

    DrawRectangleRec(r->rect, bodyColor);

    // Debug raycast
#ifdef DEBUG
    float startX = r->facing == 1 ? r->rect.x + r->rect.width : r->rect.x;
    float startY = r->rect.y;
    float endX   = startX + r->facing * 0.707f * EDGE_RAY_LENGTH;
    float endY   = startY + 0.707f * EDGE_RAY_LENGTH;
    DrawLineV((Vector2){startX, startY}, (Vector2){endX, endY}, GREEN);

    // Debug detection range
    float displayRange = r->isAggro ? RUNNER_DETECT_RANGE * 2.0f : RUNNER_DETECT_RANGE;
    DrawCircleLines(
        (int)(r->rect.x + r->rect.width  / 2.0f),
                    (int)(r->rect.y + r->rect.height / 2.0f),
                    displayRange, YELLOW
    );

    // Debug texto
    int tx = (int)r->rect.x;
    int ty = (int)r->rect.y - 40;

    //draw health
    DrawText(TextFormat("HP: %.0f", r->health), tx, ty - 12, 10, GREEN);

    if (r->detectTimer > 0.0f) {
        DrawText(TextFormat("FREEZE %.1f", r->detectTimer), tx, ty, 10, YELLOW);
    } else if (r->playerDetected) {
        DrawText("AGRO", tx, ty, 10, ORANGE);
    } else if (r->hasMemory) {
        DrawText(TextFormat("MEM %.1f | %.0f,%.0f", r->memoryTimer, r->lastKnownPos.x, r->lastKnownPos.y), tx, ty, 10, PURPLE);
    } else {
        DrawText("PATROL", tx, ty, 10, LIGHTGRAY);
    }
#endif
}
