// entities/drop.h
#pragma once
#include <raylib.h>
#include <stdbool.h>
#include "bullet.h"
#include "player.h"

#define MAX_DROPS 32

typedef struct {
    Vector2 position;
    Vector2 velocity;
    WeaponType weaponType;
    bool isCollected;
    bool onGround;
} Drop;

Drop drop_create(float x, float y);
void drops_update(Drop *drops, int maxDrops, Rectangle *platforms, int platformCount, int levelHeight, float dt);
void drops_render(const Drop *drops, int maxDrops);
bool drops_collect(Drop *drops, int maxDrops, Player *p);
void drops_try_spawn(Drop *drops, int maxDrops, float x, float y, int killCount);
