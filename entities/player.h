// entities/player.h
#pragma once
#include <stdbool.h>
#include "bullet.h"

typedef struct {
    float x, y;
    float vy;
    bool on_ground;
    int facing; // 1 para derecha, -1 para izquierda

    //Inventory System
    Weapon inventory[2];
    int current_slot;
} Player;

Player player_init(float x, float y);
void   player_update(Player *p, float dt);
void   player_render(Player *p);
