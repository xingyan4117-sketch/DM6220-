#ifndef GIMBAL_LASER_H
#define GIMBAL_LASER_H

#include "main.h"
#include <stdint.h>

#define GIMBAL_LASER_BRIGHTNESS_DEFAULT 50U

void GimbalLaser_Init(void);
void GimbalLaser_SetEnabled(uint8_t enabled);
void GimbalLaser_SetBrightness(uint8_t percent);
uint8_t GimbalLaser_IsEnabled(void);
uint8_t GimbalLaser_GetBrightness(void);

#endif
