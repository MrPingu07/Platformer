// scene.h
#pragma once

// The Scene struct acts as an interface.
// Any scene (e.g., game, menu) must return this structure filled with its own functions.
typedef struct {
    void (*update)(float dt); // Pointer to the scene's update logic loop
    void (*render)(void);     // Pointer to the scene's drawing loop
} Scene;
