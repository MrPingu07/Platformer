// entities/enemy.c
#include "runner.h"

Runner runner_init(float x, float y, float speedX) {
    return (Runner){
        .rect = (Rectangle){ x, y, 40.0f, 40.0f },
        .speedX = speedX,
        .vy = 0.0f,
        .health  = 100.0f,
        .isDead = false
    };
}

void runner_update(Runner *r, Player *p, float dt) {
    if (r->isDead) return;

    r->rect.x += r->speedX * dt;
    r->vy += 800.0f * dt;
    r->rect.y += r->vy * dt;

    if (r->rect.x <= 0.0f || r->rect.x + r->rect.width >= 640.0f)
        r->speedX *= -1.0f;

    Rectangle playerRect = { p->x, p->y, 40.0f, 40.0f };
    if (CheckCollisionRecs(playerRect, r->rect)) {
        bool isStomping = (p->vy > 0.0f) && (p->y + 40.0f < r->rect.y + 20.0f);
        if (isStomping) {
            r->isDead = true;
            p->vy = -400.0f / 2.0f;
        } else {
            p->x = 100.0f;
            p->y = 300.0f;
            p->vy = 0.0f;
        }
    }
}

void runner_render(const Runner *r) {
    if (r->isDead) return;
    DrawRectangleRec(r->rect, RED);
}
