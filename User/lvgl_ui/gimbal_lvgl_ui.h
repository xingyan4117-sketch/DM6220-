#ifndef GIMBAL_LVGL_UI_H
#define GIMBAL_LVGL_UI_H

#include "gimbal_control.h"
#include "gimbal_keys.h"
#include "Motor.h"
#include "lvgl.h"

void GimbalLvglUi_Init(GimbalControlState *ctrl,
                       DM_Motor_Info_Typedef *yaw,
                       DM_Motor_Info_Typedef *pitch);
void GimbalLvglUi_HandleEvent(const GimbalKeyEvent *event, uint32_t now_ms);
void GimbalLvglUi_Task(uint32_t now_ms, uint16_t key_adc);
uint8_t GimbalLvglUi_IsStatusPage(void);

#endif
