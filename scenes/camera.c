// scenes/camera.c
#include "camera.h"

#define DEADZONE_LEFT   280.0f
#define DEADZONE_RIGHT  310.0f
#define DEADZONE_TOP    180.0f
#define DEADZONE_BOTTOM 300.0f
#define CAMERA_LERP     6.0f

void camera_init(Camera2D *cam, Vector2 playerCenter) {
    cam->target   = playerCenter;
    cam->offset   = (Vector2){ 640.0f / 2.0f, 480.0f / 2.0f };
    cam->rotation = 0.0f;
    cam->zoom     = 1.0f;
}

void camera_update(Camera2D *cam, Vector2 playerCenter, int levelWidth, int levelHeight, float dt) {
    float playerScreenX = playerCenter.x - cam->target.x + cam->offset.x;
    float targetX = cam->target.x;
    if (playerScreenX < DEADZONE_LEFT)  targetX += (playerScreenX - DEADZONE_LEFT);
    if (playerScreenX > DEADZONE_RIGHT) targetX += (playerScreenX - DEADZONE_RIGHT);
    cam->target.x += (targetX - cam->target.x) * CAMERA_LERP * dt;

    float playerScreenY = playerCenter.y - cam->target.y + cam->offset.y;
    float targetY = cam->target.y;
    if (playerScreenY < DEADZONE_TOP)    targetY += (playerScreenY - DEADZONE_TOP);
    if (playerScreenY > DEADZONE_BOTTOM) targetY += (playerScreenY - DEADZONE_BOTTOM);
    cam->target.y += (targetY - cam->target.y) * CAMERA_LERP * dt;

    float halfW = 640.0f / 2.0f;
    float halfH = 480.0f / 2.0f;
    if (cam->target.x < halfW)               cam->target.x = halfW;
    if (cam->target.x > levelWidth  - halfW) cam->target.x = levelWidth  - halfW;
    if (cam->target.y < halfH)               cam->target.y = halfH;
    if (cam->target.y > levelHeight - halfH) cam->target.y = levelHeight - halfH;
}

Rectangle camera_get_viewport(const Camera2D *cam) {
    return (Rectangle){
        cam->target.x - cam->offset.x,
        cam->target.y - cam->offset.y,
        cam->offset.x * 2.0f,
        cam->offset.y * 2.0f
    };
}

Rectangle camera_get_logic_bounds(const Camera2D *cam) {
    float w = cam->offset.x * 2.0f * 3.0f;
    float h = cam->offset.y * 2.0f * 3.0f;
    return (Rectangle){
        cam->target.x - w / 2.0f,
        cam->target.y - h / 2.0f,
        w,
        h
    };
}
