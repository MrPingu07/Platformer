// entities/player.h
#pragma once

#include <raylib.h>
#include "bullet.h" // Required for the Weapon type and inventory

// Player entity with physics, state, and inventory management
typedef struct {
    float x, y;          // Position in world space
    float vy;            // Vertical velocity (for gravity and jumping)
    bool onGround;       // Flag to check if the player is touching a platform
    int facing;          // Look direction: 1 for Right, -1 for Left

    // Combat & Inventory system
    Weapon inventory[2]; // Two available weapon slots
    int currentSlot;     // Index of the currently equipped weapon (0 or 1)
} Player;

// Lifecycle functions
Player player_init(float x, float y);
void player_handle_combat(Player *p, Bullet *bullets, int maxBullets, float dt);
void   player_update(Player *p, float dt);
void   player_render(Player *p);
