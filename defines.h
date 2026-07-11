// defines.h
#pragma once

#define TILE_SIZE    25.0f
#define CAMERA_ZOOM  (40.0f / TILE_SIZE)

// Resolución base (altura fija, el ancho varía por aspect ratio)
#define BASE_HEIGHT  240

// Aspect ratios disponibles (ancho para BASE_HEIGHT=240)
#define AR_4_3_W     320
#define AR_3_2_W     360
#define AR_16_9_W    426
#define AR_21_9_W    560
#define AR_32_9_W    853
