// entities/runner.h
#pragma once
#include <raylib.h>
#include "player.h"

#define RUNNER_DETECT_RANGE  200.0f
#define RUNNER_MEMORY_TIME   100.0f
#define RUNNER_SPEED         100.0f

// Represents a patrolling enemy entity
typedef struct {
    Rectangle rect;
    float speedX;
    float vy;
    int facing;
    float health;
    bool playerDetected;
    float detectTimer;
    Vector2 lastKnownPos;   // Ultima posicion conocida del jugador
    float memoryTimer;      // Countdown de 10 segundos
    bool hasMemory;         // Flag: tiene posicion recordada activa
    bool isAggro;
    bool onGround;
    bool isDead;
} Runner;

// Function declarations
Runner runner_init(float x, float y, float speedX);
bool runner_check_edge(Runner *r, Rectangle *platforms, int platformCount);
void runner_update(Runner *r, Player *p, float dt, float levelWidth, Rectangle *platforms, int platformCount);
void runner_render(const Runner *r);
