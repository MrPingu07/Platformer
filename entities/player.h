// entities/player.h
#pragma once

#include <raylib.h>
#include "bullet.h" // Required for the Weapon type and inventory

#define MOVE_SPEED       300.0f
#define ACCELERATION     1800.0f
#define AIR_ACCELERATION 400.0f
#define FRICTION         1400.0f
#define AIR_FRICTION     300.0f

// Player entity with physics, state, and inventory management
typedef struct {
    float x, y;          // Position in world space
    float vy;            // Vertical velocity (for gravity and jumping)
    float vx;            // Horizontal velocity (player features momentum)
    float spawnX, spawnY; //Spawnwing coordinates
    bool onGround;       // Flag to check if the player is touching a platform
    int facing;          // Look direction: 1 for Right, -1 for Left

    // Aiming & crouch state
    Vector2 aimDir;   // Normalized direction vector for current aim
    bool isCrouching;

    // Combat & Inventory system
    Weapon inventory[2]; // Two available weapon slots
    int currentSlot;     // Index of the currently equipped weapon (0 or 1)
} Player;

// Lifecycle functions
Player player_init(float x, float y);
void player_respawn(Player *p);
void player_handle_movement(Player *p, int levelWidth, float dt);
void player_handle_combat(Player *p, Bullet *bullets, int maxBullets, float dt);
void   player_update(Player *p, float dt);
void   player_render(Player *p);
