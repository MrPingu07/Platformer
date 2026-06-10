// entities/enemy.h
#pragma once
#include <raylib.h>
#include "player.h"

// Represents a patrolling enemy entity
typedef struct {
    Rectangle rect;
    float speedX;
    float vy;
    float health;
    bool isDead;
} Runner;

// Function declarations
Runner runner_init(float x, float y, float speedX);
void   runner_update(Runner *r, Player *p, float dt);
void   runner_render(const Runner *r);
