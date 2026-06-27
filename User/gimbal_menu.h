#ifndef GIMBAL_MENU_H
#define GIMBAL_MENU_H

#include "gimbal_control.h"
#include "gimbal_keys.h"
#include "Motor.h"

typedef void (*GimbalMenu_ServiceCallback)(void);

typedef struct {
  uint8_t dummy;
} GimbalMenu;

void GimbalMenu_Init(GimbalMenu *menu);
void GimbalMenu_SetServiceCallback(GimbalMenu_ServiceCallback callback);
void GimbalMenu_RequestRedraw(void);
uint8_t GimbalMenu_NeedsRedraw(void);
uint8_t GimbalMenu_IsStatusPage(void);
void GimbalMenu_MarkRendered(void);
void GimbalMenu_HandleEvent(GimbalMenu *menu,
                            GimbalControlState *ctrl,
                            const GimbalKeyEvent *event,
                            uint32_t now_ms,
                            const DM_Motor_Info_Typedef *yaw,
                            const DM_Motor_Info_Typedef *pitch);
void GimbalMenu_Render(GimbalMenu *menu,
                       const GimbalControlState *ctrl,
                       const DM_Motor_Info_Typedef *yaw,
                       const DM_Motor_Info_Typedef *pitch,
                       uint16_t key_adc);

#endif
