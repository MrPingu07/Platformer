// entities/player.h
#pragma once

#include <raylib.h>
#include "bullet.h"        // Required for the Weapon type and inventory
#include "../defines.h"

#define MOVE_SPEED       (TILE_SIZE * 7.5f)
#define ACCELERATION     (TILE_SIZE * 50.0f)
#define AIR_ACCELERATION (TILE_SIZE * 10.0f)
#define FRICTION         (TILE_SIZE * 35.0f)
#define AIR_FRICTION     (TILE_SIZE * 7.5f)

// Player entity with physics, state, and inventory management
typedef struct {
    float x, y;          // Position in world space
    float vy;            // Vertical velocity (for gravity and jumping)
    float vx;            // Horizontal velocity (player features momentum)
    float prevFrameX;
    float prevFrameY;
    float spawnX, spawnY; //Spawnwing coordinates
    bool onGround;       // Flag to check if the player is touching a platform
    bool wasGroundedLastFrame;
    float prevY;         // Precious Y coordinates
    int facing;          // Look direction: 1 for Right, -1 for Left

    // Aiming & crouch state
    Vector2 aimDir;   // Normalized direction vector for current aim
    bool isCrouching;
    int  groundPlatformIndex;
    float lateralPushVel;   // Velocidad de corrección lateral gradual (ease in/out)
    float pushTimer;        // Tiempo acumulado tocando una cara lateral (segundos)
    float accelFactor;      // Multiplicador de aceleración por input (1.0 = control total)
    bool  pushLocked;       // true mientras se resuelve un empuje que no debe soltar
    float pushDir;          // Dirección fija del empuje mientras el lock esté activo (-1 / +1)

    // Combat & Inventory system
    Weapon inventory[2]; // Two available weapon slots
    int currentSlot;     // Index of the currently equipped weapon (0 or 1)
} Player;

// Lifecycle functions
Player player_init(float x, float y);
void player_respawn(Player *p);
void player_handle_input(Player *p, float dt);
void player_integrate_x(Player *p, int levelWidth, float dt);
void player_handle_combat(Player *p, Bullet *bullets, int maxBullets, float dt);
void player_apply_gravity(Player *p, float dt);
void player_integrate_y(Player *p, float dt);
void player_render(Player *p);
