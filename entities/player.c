// entities/player.c
#include "player.h"
#include <raylib.h>

// Physics constants
#define GRAVITY     800.0f
#define MOVE_SPEED  300.0f
#define JUMP_FORCE  -400.0f

// Initializes the player with default physics and starting weapons
Player player_init(float x, float y) {
    // Initial state: Standing, looking Right (1)
    Player p = {x, y, 0, false, 1};

    // Populate inventory with starting weapons (Modular system)
    p.inventory[0] = weapon_create(WEAPON_SEMIAUTO);
    p.inventory[1] = weapon_create(WEAPON_SHOTGUN);
    p.currentSlot = 0;

    return p;
}

//Encapsulates weapon swapping, shooting triggers, and cooldown decay
void player_handle_combat(Player *p, Bullet *bullets, int maxBullets, float dt) {
    Weapon *currentWeapon = &p->inventory[p->currentSlot];

    // 1. Cooldown timer countdown
    if (currentWeapon->cooldown > 0.0f) {
        currentWeapon->cooldown -= dt;
    }

    // 2. Weapon Hot-Swapping (Q Key)
    if (IsKeyPressed(KEY_Q)) {
        p->currentSlot = (p->currentSlot + 1) % 2; // Cycle between slot 0 and 1
    }

    // 3. Attack Trigger Execution (SPACE Key)
    // Check if the weapon is automatic (requires holding) vs semi-automatic (requires tapping)
    bool wantsToFire = currentWeapon->isAutomatic ? IsKeyDown(KEY_SPACE) : IsKeyPressed(KEY_SPACE);

    if (wantsToFire && currentWeapon->cooldown <= 0.0f) {
        // Calculate project origin matching player dimensions (offsets can be added here if needed)
        Vector2 fireOrigin = { p->x + 20.0f, p->y + 20.0f };

        // Execute your exact polymorphic function pointer definition
        currentWeapon->fireFunc(fireOrigin, p->facing, bullets, maxBullets);

        // Reset the rate clock back to the weapon default
        currentWeapon->cooldown = currentWeapon->fireRate;
    }
}

// Handles player physics and horizontal movement input
void player_update(Player *p, float dt) {
    // 1. Apply Gravity: Vertical velocity accumulates over time
    p->vy += GRAVITY * dt;
    p->y  += p->vy * dt;

    // 2. Handle Horizontal Movement and update 'facing' direction
    if (IsKeyDown(KEY_D)) {
        p->x += MOVE_SPEED * dt;
        p->facing = 1;  // Facing Right
    }
    if (IsKeyDown(KEY_A)) {
        p->x -= MOVE_SPEED * dt;
        p->facing = -1; // Facing Left
    }
}

// Renders the player sprite
void player_render(Player *p) {
    // Draw the player character
    DrawRectangle((int)p->x, (int)p->y, 40, 40, BLUE); //A 40x40 Blue box
}
