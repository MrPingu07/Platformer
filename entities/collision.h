// entities/collision.h
#pragma once
#include <raylib.h>
#include "player.h"
#include "bullet.h"
#include "hittable.h"

void resolve_environment_collisions(Player *p, Rectangle *platforms, int count);
void resolve_rect_collision(Rectangle *rect, float *vy, Rectangle *platforms, int count, bool *onGround);
void resolve_bullet_hittable_collisions(Hittable *targets, int targetCount, Bullet *bullets, int maxBullets);
