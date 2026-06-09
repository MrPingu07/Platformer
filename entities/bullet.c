// ==========================================
// FILE: entities/bullet.c
// ==========================================
#include "bullet.h"
#include <math.h>

// --- Utilidad para buscar un slot libre en el array de la escena ---
void spawn_bullet(Bullet* bullets, int max_bullets, Vector2 pos, Vector2 vel, float radius, float lifetime, Color color) {
    for (int i = 0; i < max_bullets; i++) {
        if (!bullets[i].active) {
            bullets[i].position = pos;
            bullets[i].velocity = vel;
            bullets[i].radius = radius;
            bullets[i].color = color;
            bullets[i].lifetime = lifetime;
            bullets[i].active = true;
            return;
        }
    }
}

// =================================================================
// SHOOTING BEHAVIORS
// =================================================================

static void fire_semiauto(Vector2 origin, int dir, Bullet* bullets, int max) {
    Vector2 vel = { dir * 600.0f, 0.0f }; // Rápida y recta
    spawn_bullet(bullets, max, origin, vel, 4.0f, 2.0f, YELLOW);
}

static void fire_fullauto(Vector2 origin, int dir, Bullet* bullets, int max) {
    // Simmilar to Semiauto, but whith a slight imperfection in the Y axis
    float random_y_spread = (float)GetRandomValue(-30, 30);
    Vector2 vel = { dir * 550.0f, random_y_spread };
    spawn_bullet(bullets, max, origin, vel, 4.0f, 1.5f, ORANGE);
}

static void fire_shotgun(Vector2 origin, int dir, Bullet* bullets, int max) {
    // Shoots 3 spread bullets
    float base_speed = 500.0f;

    // Straight bullet
    spawn_bullet(bullets, max, origin, (Vector2){ dir * base_speed, 0.0f }, 4.0f, 0.5f, GOLD);
    // Upwards-angled bullet
    spawn_bullet(bullets, max, origin, (Vector2){ dir * base_speed, -150.0f }, 4.0f, 0.5f, GOLD);
    // Downwards angled bullet
    spawn_bullet(bullets, max, origin, (Vector2){ dir * base_speed, 150.0f }, 4.0f, 0.5f, GOLD);
}

static void fire_flamethrower(Vector2 origin, int dir, Bullet* bullets, int max) {
    // Flamethrower, slow bursts with a lot of spread that die quickly.
    float speed_x = dir * (float)GetRandomValue(200, 300);
    float speed_y = (float)GetRandomValue(-80, 80);
    float size = (float)GetRandomValue(6, 12); // Fire is bigger than bullets

    spawn_bullet(bullets, max, origin, (Vector2){ speed_x, speed_y }, size, 0.4f, RED);
}

// =================================================================

// Weapons
Weapon weapon_create(WeaponType type) {
    Weapon w;
    w.type = type;
    w.cooldown = 0.0f;

    switch (type) {
        case WEAPON_SEMIAUTO:
            w.name = "Semiauto";
            w.fire_rate = 0.25f;
            w.is_automatic = false;
            w.fire_func = fire_semiauto;
            break;
        case WEAPON_SHOTGUN:
            w.name = "Shotgun";
            w.fire_rate = 0.6f;
            w.is_automatic = false;
            w.fire_func = fire_shotgun;
            break;
        case WEAPON_FULLAUTO:
            w.name = "Full-Auto";
            w.fire_rate = 0.1f;
            w.is_automatic = true;
            w.fire_func = fire_fullauto;
            break;
        case WEAPON_FLAMETHROWER:
            w.name = "Lanzallamas";
            w.fire_rate = 0.05f; // Muy rápido por ser partículas de fuego
            w.is_automatic = true;
            w.fire_func = fire_flamethrower;
            break;
    }
    return w;
}

// Update bullets in the air
void bullets_update(Bullet* bullets, int max_bullets, float dt) {
    for (int i = 0; i < max_bullets; i++) {
        if (!bullets[i].active) continue;

        bullets[i].position.x += bullets[i].velocity.x * dt;
        bullets[i].position.y += bullets[i].velocity.y * dt;
        bullets[i].lifetime -= dt;

        if (bullets[i].lifetime <= 0) {
            bullets[i].active = false;
        }
    }
}

// Draw the bullets
void bullets_render(Bullet* bullets, int max_bullets) {
    for (int i = 0; i < max_bullets; i++) {
        if (bullets[i].active) {
            DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
        }
    }
}
