// entities/enemy.h
#pragma once

#include <stdbool.h>
#include <raylib.h>

typedef struct {
    Rectangle rect;
    float vx;
    bool dead;
} Enemy;

Enemy enemy_init(float x, float y, float vx);
void  enemy_update(Enemy *e, float dt);
void  enemy_render(Enemy *e);
