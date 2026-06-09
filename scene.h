// scene.h
#pragma once

typedef struct {
    void (*update)(float dt);
    void (*render)(void);
} Scene;
