// ==========================================
// FILE: entities/bullet.h
// ==========================================
#pragma once
#include <raylib.h>
#include <stdbool.h>

#define MAX_BULLETS 100

// El objeto físico de la bala
typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;
    float lifetime; // Kill bullet after X seconds
    bool active;
} Bullet;

// Enumerator for identigying the weapons
typedef enum {
    WEAPON_SEMIAUTO,
    WEAPON_SHOTGUN,
    WEAPON_FULLAUTO,
    WEAPON_FLAMETHROWER,
    WEAPON_COUNT
} WeaponType;

// The Contract of what a weapon is
typedef struct Weapon {
    WeaponType type;
    const char* name;
    float fire_rate;    // Minimum time between shots
    float cooldown;     // Remaining time to be able to fire again
    bool is_automatic;  // Is it shot by keeping it pressed?

    // Unique shooting behavior: Each weapon will implement this function
    void (*fire_func)(Vector2 origin, int direction, Bullet* bullet_array, int max_bullets);
} Weapon;

// Functions for the scene to handle the bullets
void bullets_update(Bullet* bullets, int max_bullets, float dt);
void bullets_render(Bullet* bullets, int max_bullets);
void spawn_bullet(Bullet* bullets, int max_bullets, Vector2 pos, Vector2 vel, float radius, float lifetime, Color color);

// Default weapons generator
Weapon weapon_create(WeaponType type);
