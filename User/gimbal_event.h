#ifndef GIMBAL_EVENT_H
#define GIMBAL_EVENT_H

#include <stdint.h>

#define GIMBAL_EVT_TICK_1MS          (1UL << 0)
#define GIMBAL_EVT_KEY_SCAN_10MS     (1UL << 1)
#define GIMBAL_EVT_MOTOR_FEEDBACK    (1UL << 2)
#define GIMBAL_EVT_LCD_REFRESH       (1UL << 3)
#define GIMBAL_EVT_ERROR             (1UL << 31)

void GimbalEvent_Init(void);
void GimbalEvent_Push(uint32_t events);
void GimbalEvent_PushFromIsr(uint32_t events);
uint8_t GimbalEvent_Fetch(uint32_t *events);
uint32_t GimbalEvent_Peek(void);

#endif
