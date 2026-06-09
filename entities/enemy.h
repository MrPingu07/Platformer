// entities/enemy.h
#pragma once

#include <raylib.h>
#include "player.h"

// Represents a patrolling enemy entity
typedef struct {
    Rectangle rect;
    float speedX;
    bool isDead;
} Enemy;

// Function declarations
Enemy enemy_init(float x, float y, float speedX);
void enemy_update(Enemy *e, Player *p, float dt);
void enemy_render(const Enemy *e);
