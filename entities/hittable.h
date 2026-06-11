// entities/hittable.h
#pragma once
#include <raylib.h>
#include <stdbool.h>

// Generic damageable entity interface.
// Any entity that can receive damage exposes this structure.
typedef struct {
    Rectangle *rect;
    float     *health;
    bool      *isDead;
} Hittable;
