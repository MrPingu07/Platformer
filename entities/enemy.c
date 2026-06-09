// entities/enemy.c
#include "enemy.h"
#include <raylib.h>

#define SCREEN_WIDTH 640

Enemy enemy_init(float x, float y, float vx) {
    Enemy e = { {x, y, 40, 40}, vx, false };
    return e;
}

void enemy_update(Enemy *e, float dt) {
    if (e->dead) return;

    e->rect.x += e->vx * dt;

    if (e->rect.x <= 0 || e->rect.x + e->rect.width >= SCREEN_WIDTH) {
        e->vx *= -1;
    }
}

void enemy_render(Enemy *e) {
    if (!e->dead) DrawRectangleRec(e->rect, RED);
}
