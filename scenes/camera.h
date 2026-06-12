// scenes/camera.h
#pragma once
#include <raylib.h>

void camera_init(Camera2D *cam, Vector2 playerCenter);
void camera_update(Camera2D *cam, Vector2 playerCenter, int levelWidth, int levelHeight, float dt);
Rectangle camera_get_viewport(const Camera2D *cam);
Rectangle camera_get_logic_bounds(const Camera2D *cam);
