// scenes/moving_platform.c
#include "moving_platform.h"
#include "../defines.h"
#include <raylib.h>
#include <math.h>

// Calcula velocidad máxima de t (en t/s) a partir de speed en tiles/s
static float compute_tspeed(const MovingPlatform *mp) {
    float dx = mp->pathEnd.x - mp->pathStart.x;
    float dy = mp->pathEnd.y - mp->pathStart.y;
    float dist = sqrtf(dx * dx + dy * dy); // distancia en px
    if (dist < 0.001f) return 0.0f;
    // speed está en tiles/s → convertir a px/s → normalizar por dist
    return (mp->speed * TILE_SIZE) / dist;
}

void moving_platform_update(MovingPlatform *mp, float dt) {
    float tSpeed = compute_tspeed(mp);
    if (tSpeed < 0.001f) return; // plataforma estática, sin path
    mp->delta = (Vector2){ 0.0f, 0.0f };
    if (tSpeed < 0.001f) return;

    // Aceleración y deceleración en unidades de tSpeed/s
    // accel/decel son segundos para alcanzar tSpeed → tAccel = tSpeed / accel
    float tAccel = (mp->accel > 0.0f) ? tSpeed / mp->accel : tSpeed * 1000.0f;
    float tDecel = (mp->decel > 0.0f) ? tSpeed / mp->decel : tSpeed * 1000.0f;

    // Distancia de frenado en unidades de t
    // v² = 2·a·d  →  d = v² / (2·a)
    float decelDist = (tDecel > 0.0f) ? (mp->tVelocity * mp->tVelocity) / (2.0f * tDecel) : 0.0f;

    // Distancia restante al extremo actual
    float distToEdge = mp->tDirection > 0 ? (1.0f - mp->t) : mp->t;

    // Zona de frenado
    bool inDecelZone = distToEdge <= decelDist && decelDist > 0.0f;

    if (inDecelZone) {
        mp->tVelocity -= tDecel * dt;
        if (mp->tVelocity < 0.0f) mp->tVelocity = 0.0f;
    } else {
        mp->tVelocity += tAccel * dt;
        if (mp->tVelocity > tSpeed) mp->tVelocity = tSpeed;
    }

    mp->t += mp->tVelocity * mp->tDirection * dt;

    // Llegó al extremo: invertir dirección
    if (mp->t >= 1.0f) {
        mp->t = 1.0f;
        mp->tDirection = -1;
        mp->tVelocity  = 0.0f;
    } else if (mp->t <= 0.0f) {
        mp->t = 0.0f;
        mp->tDirection = 1;
        mp->tVelocity  = 0.0f;
    }

    mp->delta.x = (mp->pathStart.x + (mp->pathEnd.x - mp->pathStart.x) * mp->t) - mp->rect.x;
    mp->delta.y = (mp->pathStart.y + (mp->pathEnd.y - mp->pathStart.y) * mp->t) - mp->rect.y;

    mp->rect.x += mp->delta.x;
    mp->rect.y += mp->delta.y;

}

void moving_platforms_update(MovingPlatform *platforms, int count, float dt) {
    for (int i = 0; i < count; i++)
        moving_platform_update(&platforms[i], dt);
}
