// entities/bullet.c
#include "bullet.h"

// Loops through the pre-allocated pool to find and initialize an inactive bullet slot
void spawn_bullet(Bullet* bullets, int maxBullets, Vector2 pos, Vector2 vel, float radius, float lifetime, Color color) {
    for (int i = 0; i < maxBullets; i++) {
        if (!bullets[i].isActive) {
            bullets[i].position = pos;
            bullets[i].velocity = vel;
            bullets[i].radius = radius;
            bullets[i].color = color;
            bullets[i].lifetime = lifetime;
            bullets[i].isActive = true; // Allocate slot
            return;
        }
    }
}

// =================================================================
// WEAPON BEHAVIOR IMPLEMENTATIONS (Polymorphic Fire Functions)
// =================================================================

// High-speed, high-precision straight single shot
static void fire_semiauto(Vector2 origin, int dir, Bullet* bullets, int max) {
    Vector2 vel = { dir * 600.0f, 0.0f };
    spawn_bullet(bullets, max, origin, vel, 4.0f, 2.0f, YELLOW);
}

// Continuous fire stream with a minor vertical recoil simulation
static void fire_fullauto(Vector2 origin, int dir, Bullet* bullets, int max) {
    float randomYSpread = (float)GetRandomValue(-30, 30);
    Vector2 vel = { dir * 550.0f, randomYSpread };
    spawn_bullet(bullets, max, origin, vel, 4.0f, 1.5f, ORANGE);
}

// Multi-projectile spread pattern casting 3 distinct angular vectors
static void fire_shotgun(Vector2 origin, int dir, Bullet* bullets, int max) {
    float baseSpeed = 500.0f;

    // Center straight projectile
    spawn_bullet(bullets, max, origin, (Vector2){ dir * baseSpeed, 0.0f }, 4.0f, 0.5f, GOLD);
    // Upward angled projectile
    spawn_bullet(bullets, max, origin, (Vector2){ dir * baseSpeed, -150.0f }, 4.0f, 0.5f, GOLD);
    // Downward angled projectile
    spawn_bullet(bullets, max, origin, (Vector2){ dir * baseSpeed, 150.0f }, 4.0f, 0.5f, GOLD);
}

// High-density, short-range particle simulation with randomized scale outputs
static void fire_flamethrower(Vector2 origin, int dir, Bullet* bullets, int max) {
    float speedX = dir * (float)GetRandomValue(200, 300);
    float speedY = (float)GetRandomValue(-80, 80);
    float randomSize = (float)GetRandomValue(6, 12);

    spawn_bullet(bullets, max, origin, (Vector2){ speedX, speedY }, randomSize, 0.4f, RED);
}

// =================================================================

// Weapon Blueprint Factory: Sets attributes and assigns correct function bindings
Weapon weapon_create(WeaponType type) {
    Weapon w;
    w.type = type;
    w.cooldown = 0.0f;

    switch (type) {
        case WEAPON_SEMIAUTO:
            w.name = "Semi-Auto";
            w.fireRate = 0.25f;
            w.isAutomatic = false;
            w.fireFunc = fire_semiauto;
            break;
        case WEAPON_SHOTGUN:
            w.name = "Shotgun";
            w.fireRate = 0.6f;
            w.isAutomatic = false;
            w.fireFunc = fire_shotgun;
            break;
        case WEAPON_FULLAUTO:
            w.name = "Full-Auto";
            w.fireRate = 0.1f;
            w.isAutomatic = true;
            w.fireFunc = fire_fullauto;
            break;
        case WEAPON_FLAMETHROWER:
            w.name = "Flamethrower";
            w.fireRate = 0.05f;
            w.isAutomatic = true;
            w.fireFunc = fire_flamethrower;
            break;
        default:
            // Fallback default case to silence compiler warnings for meta-values like WEAPON_COUNT
            w.name = "Unknown";
            w.fireRate = 1.0f;
            w.isAutomatic = false;
            w.fireFunc = fire_semiauto;
            break;
    }
    return w;
}

// Updates physical translations and handles decay tracking for active projectiles
void bullets_update(Bullet* bullets, int maxBullets, float dt) {
    for (int i = 0; i < maxBullets; i++) {
        if (!bullets[i].isActive) continue;

        // Linear translation resolution
        bullets[i].position.x += bullets[i].velocity.x * dt;
        bullets[i].position.y += bullets[i].velocity.y * dt;

        // Decay timer evaluation
        bullets[i].lifetime -= dt;
        if (bullets[i].lifetime <= 0.0f) {
            bullets[i].isActive = false; // Deallocate slot for reuse
        }
    }
}

// Iterates through active slots to render circle primitives
void bullets_render(Bullet* bullets, int maxBullets) {
    for (int i = 0; i < maxBullets; i++) {
        if (bullets[i].isActive) {
            DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
        }
    }
}
