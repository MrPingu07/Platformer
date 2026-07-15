// entities/player.c
#include "player.h"
#include "bullet.h"
#include "../defines.h"
#include <raylib.h>

#define GRAVITY    (TILE_SIZE * 20.0f)
#define JUMP_FORCE (TILE_SIZE * -10.0f)

Player player_init(float x, float y) {
    Player p = {x, y, 0, false, 1};
    p.spawnX = x;
    p.spawnY = y;
    p.prevY = y;
    p.prevFrameX = x;
    p.prevFrameY = y;
    p.inventory[0] = weapon_create(WEAPON_SEMIAUTO);
    p.inventory[1] = weapon_create(WEAPON_COUNT);  // empty slot
    p.currentSlot = 0;
    p.groundPlatformIndex = -1;
    return p;
}

void player_respawn(Player *p) {
    p->x  = p->spawnX;
    p->y  = p->spawnY;
    p->vx = 0.0f;
    p->vy = 0.0f;
    p->prevY = p->spawnY;
    p->prevFrameX = p->spawnX;
    p->prevFrameY = p->spawnY;
    p->onGround  = false;
    p->currentSlot   = 0;
    p->groundPlatformIndex = -1;
    p->inventory[0]  = weapon_create(WEAPON_SEMIAUTO);
    p->inventory[1]  = weapon_create(WEAPON_COUNT);
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

    if (IsKeyPressed(KEY_Q) && p->inventory[1].type != WEAPON_COUNT)
        p->currentSlot = (p->currentSlot + 1) % 2;

    bool wantsToFire = currentWeapon->isAutomatic
    ? IsKeyDown(KEY_SPACE)
    : IsKeyPressed(KEY_SPACE);

    if (wantsToFire && currentWeapon->cooldown <= 0.0f) {
        p->aimDir = resolve_aim_direction(p);
        Vector2 fireOrigin;
        // Muzzle origin offset shifts to bottom-center when crouching
        if (p->isCrouching) {
            fireOrigin = (Vector2){ p->x + TILE_SIZE * 0.5f, p->y + TILE_SIZE * 0.75f };
        } else {
            fireOrigin = (Vector2){ p->x + TILE_SIZE * 0.5f, p->y + TILE_SIZE * 0.5f };
        }

        currentWeapon->fireFunc(fireOrigin, p->aimDir, bullets, maxBullets);
        currentWeapon->cooldown = currentWeapon->fireRate;
    }
}

// Apply gravity accumulation and integrate vertical position
void player_apply_gravity(Player *p, float dt) {
    p->vy += GRAVITY * dt;
    if (p->vy > TILE_SIZE * 50.0f) p->vy = TILE_SIZE * 50.0f;
}

void player_integrate_y(Player *p, float dt) {
    p->y += p->vy * dt;
}

void player_render(Player *p) {
    // Render crouched hitbox: shorter, slightly wider
    if (p->isCrouching) {
        DrawRectangle(
            (int)(p->x - TILE_SIZE * 0.125f),
                      (int)(p->y + TILE_SIZE * 0.5f),
                      (int)(TILE_SIZE * 1.25f),
                      (int)(TILE_SIZE * 0.5f),
                      BLUE
        );
    } else {
        DrawRectangle((int)p->x, (int)p->y, (int)TILE_SIZE, (int)TILE_SIZE, BLUE);
    }
}

void player_handle_input(Player *p, float dt) {
    float speed = p->isCrouching ? MOVE_SPEED * 0.5f : MOVE_SPEED;
    bool lockX = IsKeyDown(KEY_W) && p->onGround &&
    !IsKeyDown(KEY_A) && !IsKeyDown(KEY_D);

    if (!lockX) {
        float accel = p->onGround ? ACCELERATION : AIR_ACCELERATION;

        if (IsKeyDown(KEY_D)) { p->vx += accel * dt; p->facing =  1; }
        else if (IsKeyDown(KEY_A)) { p->vx -= accel * dt; p->facing = -1; }

        if (p->vx >  speed) p->vx =  speed;
        if (p->vx < -speed) p->vx = -speed;
    }

    if (!IsKeyDown(KEY_A) && !IsKeyDown(KEY_D)) {
        float friction = p->onGround ? FRICTION : AIR_FRICTION;
        if (p->vx > 0.0f) { p->vx -= friction * dt; if (p->vx < 0.0f) p->vx = 0.0f; }
        if (p->vx < 0.0f) { p->vx += friction * dt; if (p->vx > 0.0f) p->vx = 0.0f; }
    }

    p->isCrouching = IsKeyDown(KEY_S) && p->onGround;

    if (IsKeyPressed(KEY_W) && p->onGround && !p->isCrouching) {
        p->vy = JUMP_FORCE;
        p->onGround = false;
    }
}

void player_integrate_x(Player *p, int levelWidth, float dt) {
    p->x += p->vx * dt;
    if (p->x < 0.0f)                    { p->x = 0.0f;                    p->vx = 0.0f; }
    if (p->x + TILE_SIZE > levelWidth)  { p->x = levelWidth - TILE_SIZE;  p->vx = 0.0f; }
}
