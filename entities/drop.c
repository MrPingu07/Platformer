// entities/drop.c
#include "drop.h"
#include "player.h"
#include "../defines.h"
#include <raylib.h>

#define DROP_GRAVITY 800.0f
#define DROP_SIZE    (TILE_SIZE * 1.0f)

static WeaponType roll_weapon_type(void) {
    int roll = GetRandomValue(1, 150);
    if (roll <= 75)  return WEAPON_SHOTGUN;
    if (roll <= 125) return WEAPON_FULLAUTO;
    return WEAPON_FLAMETHROWER;
}

Drop drop_create(float x, float y) {
    return (Drop){
        .position    = (Vector2){ x, y },
        .velocity    = (Vector2){ (float)GetRandomValue(-80, 80), -300.0f },
        .weaponType  = roll_weapon_type(),
        .isCollected = false,
        .onGround    = false
    };
}

void drops_update(Drop *drops, int maxDrops, Rectangle *platforms, int platformCount, int levelHeight, float dt) {
    for (int i = 0; i < maxDrops; i++) {
        if (drops[i].isCollected) continue;
        if (drops[i].onGround) continue;

        if (drops[i].position.y > levelHeight) {
            drops[i].isCollected = true;
            continue;
        }

        drops[i].velocity.y += DROP_GRAVITY * dt;
        drops[i].position.x += drops[i].velocity.x * dt;
        drops[i].position.y += drops[i].velocity.y * dt;

        Rectangle rect = { drops[i].position.x, drops[i].position.y, DROP_SIZE, DROP_SIZE };
        for (int j = 0; j < platformCount; j++) {
            if (drops[i].velocity.y >= 0.0f &&
                CheckCollisionRecs(rect, platforms[j]) &&
                (rect.y + DROP_SIZE <= platforms[j].y + TILE_SIZE * 2.0f))
            {
                drops[i].position.y = platforms[j].y - DROP_SIZE;
                drops[i].velocity.y = 0.0f;
                drops[i].velocity.x = 0.0f;
                drops[i].onGround   = true;
            }
        }
    }
}

void drops_render(const Drop *drops, int maxDrops) {
    for (int i = 0; i < maxDrops; i++) {
        if (drops[i].isCollected) continue;
        DrawRectangle(
            (int)drops[i].position.x,
                      (int)drops[i].position.y,
                      (int)DROP_SIZE, (int)DROP_SIZE,
                      SKYBLUE
        );
        const char *label = "?";
        if      (drops[i].weaponType == WEAPON_SHOTGUN)     label = "SG";
        else if (drops[i].weaponType == WEAPON_FULLAUTO)    label = "FA";
        else if (drops[i].weaponType == WEAPON_FLAMETHROWER) label = "FT";
        DrawText(label, (int)drops[i].position.x + (int)(TILE_SIZE * 0.2f), (int)drops[i].position.y + (int)(TILE_SIZE * 0.4f), 10, WHITE);
    }
}

bool drops_collect(Drop *drops, int maxDrops, Player *p) {
    Rectangle playerRect = { p->x, p->y, TILE_SIZE, TILE_SIZE };
    for (int i = 0; i < maxDrops; i++) {
        if (drops[i].isCollected) continue;
        Rectangle dropRect = { drops[i].position.x, drops[i].position.y, DROP_SIZE, DROP_SIZE };
        if (CheckCollisionRecs(playerRect, dropRect)) {
            p->inventory[1] = weapon_create(drops[i].weaponType);
            drops[i].isCollected = true;
            return true;
        }
    }
    return false;
}

void drops_try_spawn(Drop *drops, int maxDrops, float x, float y, int killCount) {
    int chance = 5 + killCount;
    if (chance > 80) chance = 80;
    if (GetRandomValue(1, 100) > chance) return;
    for (int i = 0; i < maxDrops; i++) {
        if (drops[i].isCollected) {
            drops[i] = drop_create(x, y);
            return;
        }
    }
}
