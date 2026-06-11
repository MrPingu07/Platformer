// scenes/camera.h
#pragma once
#include <raylib.h>

void camera_init(Camera2D *cam, Vector2 playerCenter);
void camera_update(Camera2D *cam, Vector2 playerCenter, int levelWidth, int levelHeight, float dt);
