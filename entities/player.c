// entities/player.c
#include "player.h"
#include <raylib.h>

#define GRAVITY    800.0f
#define SPEED      300.0f
#define JUMP_FORCE -400.0f

Player player_init(float x, float y) {
    Player p = {x, y, 0, false, 1};
    p.inventory[0] = weapon_create(WEAPON_SEMIAUTO);
    p.inventory[1] = weapon_create(WEAPON_SHOTGUN);
    p.current_slot = 0;
    return p;
}

void player_update(Player *p, float dt) {

    p->vy += GRAVITY * dt;
    p->y  += p->vy * dt;

    if (IsKeyDown(KEY_D)) { p->x += SPEED * dt; p->facing = 1; }
    if (IsKeyDown(KEY_A)) { p->x -= SPEED * dt; p->facing = -1; }
}

void player_render(Player *p) {
    DrawRectangle((int)p->x, (int)p->y, 40, 40, BLUE);
}
