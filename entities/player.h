// entities/player.h
#pragma once
#include <stdbool.h>

typedef struct {
    float x, y;
    float vy;
    bool on_ground;
} Player;

Player player_init(float x, float y);
void   player_update(Player *p, float dt);
void   player_render(Player *p);
