// scenes/camera.c
#include "camera.h"
#include "../defines.h"

#define CAMERA_LERP     6.0f

void camera_init(Camera2D *cam, Vector2 playerCenter) {
    cam->target   = playerCenter;
    cam->offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    cam->rotation = 0.0f;
    cam->zoom = 40.0f / TILE_SIZE;
}

void camera_update(Camera2D *cam, Vector2 playerCenter, int levelWidth, int levelHeight, float dt) {
    float screenW = cam->offset.x * 2.0f;
    float screenH = cam->offset.y * 2.0f;
    float deadzoneLeft   = screenW * 0.48f;
    float deadzoneRight  = screenW * 0.52f;
    float deadzoneTop    = screenH * 0.48f;
    float deadzoneBottom = screenH * 0.52f;

    float playerScreenX = (playerCenter.x - cam->target.x) * cam->zoom + cam->offset.x;
    float targetX = cam->target.x;
    if (playerScreenX < deadzoneLeft)  targetX += (playerScreenX - deadzoneLeft)  / cam->zoom;
    if (playerScreenX > deadzoneRight) targetX += (playerScreenX - deadzoneRight) / cam->zoom;
    cam->target.x += (targetX - cam->target.x) * CAMERA_LERP * dt;

    float playerScreenY = (playerCenter.y - cam->target.y) * cam->zoom + cam->offset.y;
    float targetY = cam->target.y;
    if (playerScreenY < deadzoneTop)    targetY += (playerScreenY - deadzoneTop)    / cam->zoom;
    if (playerScreenY > deadzoneBottom) targetY += (playerScreenY - deadzoneBottom) / cam->zoom;
    cam->target.y += (targetY - cam->target.y) * CAMERA_LERP * dt;

    float halfW = cam->offset.x / cam->zoom;
    float halfH = cam->offset.y / cam->zoom;
    if (cam->target.x < halfW)               cam->target.x = halfW;
    if (cam->target.x > levelWidth  - halfW) cam->target.x = levelWidth  - halfW;
    if (cam->target.y > levelHeight - halfH) cam->target.y = levelHeight - halfH;
}

Rectangle camera_get_viewport(const Camera2D *cam) {
    return (Rectangle){
        cam->target.x - cam->offset.x / cam->zoom,
        cam->target.y - cam->offset.y / cam->zoom,
        cam->offset.x * 2.0f / cam->zoom,
        cam->offset.y * 2.0f / cam->zoom
    };
}

Rectangle camera_get_logic_bounds(const Camera2D *cam) {
    float w = (cam->offset.x * 2.0f / cam->zoom) * 3.0f;
    float h = (cam->offset.y * 2.0f / cam->zoom) * 3.0f;
    return (Rectangle){
        cam->target.x - w / 2.0f,
        cam->target.y - h / 2.0f,
        w,
        h
    };
}
