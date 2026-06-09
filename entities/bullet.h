// entities/bullet.h
#pragma once
#include <raylib.h>
#include <stdbool.h>

// Buffer constraint for maximum concurrent bullets in the scene
#define MAX_BULLETS 100

// Individual projectile physical state
typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;
    float lifetime; // Time-to-live tracker (seconds) before auto-deactivation
    bool isActive;  // Flag to manage memory pool allocation
} Bullet;

// Weapon identification index
typedef enum {
    WEAPON_SEMIAUTO,
    WEAPON_SHOTGUN,
    WEAPON_FULLAUTO,
    WEAPON_FLAMETHROWER,
    WEAPON_COUNT
} WeaponType;

// Polymorphic Weapon structure
typedef struct Weapon {
    WeaponType type;
    const char* name;
    float fireRate;     // Minimum structural delay between shots (seconds)
    float cooldown;     // Remaining cooldown time allocation per frame
    bool isAutomatic;   // Triggers input check type (Hold vs. Press)

    // Function pointer achieving behavior polymorphism across weapon profiles
    void (*fireFunc)(Vector2 origin, int direction, Bullet* bulletArray, int maxBullets);
} Weapon;

// Projectile system management lifecycle
void bullets_update(Bullet* bullets, int maxBullets, float dt);
void bullets_render(Bullet* bullets, int maxBullets);
void spawn_bullet(Bullet* bullets, int maxBullets, Vector2 pos, Vector2 vel, float radius, float lifetime, Color color);

// Factory function to initialize pre-configured weapon profiles
Weapon weapon_create(WeaponType type);
