// entities/player.c
#include "player.h"
#include "bullet.h"
#include <raylib.h>

#define GRAVITY     800.0f
#define MOVE_SPEED  300.0f
#define JUMP_FORCE  -400.0f

Player player_init(float x, float y) {
    Player p = {x, y, 0, false, 1};
    p.inventory[0] = weapon_create(WEAPON_FULLAUTO);
    p.inventory[1] = weapon_create(WEAPON_FLAMETHROWER);
    p.currentSlot = 0;
    return p;
}

// Resolves aim direction vector from directional input and player state
static Vector2 resolve_aim_direction(Player *p) {
    bool up    = IsKeyDown(KEY_W);
    bool down  = IsKeyDown(KEY_S);
    bool left  = IsKeyDown(KEY_A);
    bool right = IsKeyDown(KEY_D);

    if (up && right) return (Vector2){  1.0f, -1.0f };
    if (up && left)  return (Vector2){ -1.0f, -1.0f };
    if (!p->onGround && down && right) return (Vector2){  1.0f, 1.0f };
    if (!p->onGround && down && left)  return (Vector2){ -1.0f, 1.0f };

    if (up) return (Vector2){ 0.0f, -1.0f };

    if (!p->onGround && down) return (Vector2){ 0.0f, 1.0f };

    // Default: horizontal facing
    return (Vector2){ (float)p->facing, 0.0f };
}

void player_handle_combat(Player *p, Bullet *bullets, int maxBullets, float dt) {
    Weapon *currentWeapon = &p->inventory[p->currentSlot];

    if (currentWeapon->cooldown > 0.0f)
        currentWeapon->cooldown -= dt;

    if (IsKeyPressed(KEY_Q))
        p->currentSlot = (p->currentSlot + 1) % 2;

    bool wantsToFire = currentWeapon->isAutomatic
    ? IsKeyDown(KEY_SPACE)
    : IsKeyPressed(KEY_SPACE);

    if (wantsToFire && currentWeapon->cooldown <= 0.0f) {
        p->aimDir = resolve_aim_direction(p);
        Vector2 fireOrigin;
        // Muzzle origin offset shifts to bottom-center when crouching
        if (p->isCrouching) {
            fireOrigin = (Vector2){ p->x + 20.0f, p->y + 30.0f };
        } else {
            fireOrigin = (Vector2){ p->x + 20.0f, p->y + 20.0f };
        }

        currentWeapon->fireFunc(fireOrigin, p->aimDir, bullets, maxBullets);
        currentWeapon->cooldown = currentWeapon->fireRate;
    }
}

// Apply gravity accumulation and integrate vertical position
void player_update(Player *p, float dt) {
    p->vy += GRAVITY * dt;
    p->y  += p->vy * dt;

    // Halve movement speed while crouching
    float speed = p->isCrouching ? MOVE_SPEED * 0.5f : MOVE_SPEED;

    // Lock horizontal axis when aiming up on ground with no horizontal input
    bool lockX = IsKeyDown(KEY_W) && p->onGround &&
    !IsKeyDown(KEY_A) && !IsKeyDown(KEY_D);

    if (!lockX) {
        if (IsKeyDown(KEY_D)) { p->x += speed * dt; p->facing =  1; }
        if (IsKeyDown(KEY_A)) { p->x -= speed * dt; p->facing = -1; }
    }
}

void player_render(Player *p) {
    // Render crouched hitbox: shorter, slightly wider
    if (p->isCrouching) {
        DrawRectangle((int)p->x - 5, (int)p->y + 20, 50, 20, BLUE);
    } else {
        DrawRectangle((int)p->x, (int)p->y, 40, 40, BLUE);
    }
    DrawText(TextFormat("y:%.0f onGround:%d crouch:%d", p->y, p->onGround, p->isCrouching), 20, 60, 16, YELLOW);
    DrawText(TextFormat("y:%.0f onGround:%d crouch:%d vy:%.0f", p->y, p->onGround, p->isCrouching, p->vy), 20, 60, 16, YELLOW);

}
