#ifndef GIMBAL_SYSTEM_H
#define GIMBAL_SYSTEM_H

#include <stdint.h>

typedef enum {
  SYS_BOOT_DELAY = 0,
  SYS_BOOT_ENABLE_YAW,
  SYS_BOOT_WAIT_YAW,
  SYS_BOOT_ENABLE_PITCH,
  SYS_BOOT_WAIT_PITCH,
  SYS_BOOT_PRIME_MIT,
  SYS_RUN,
  SYS_ESTOP
} GimbalSystemState;

void GimbalSystem_Init(void);
void GimbalSystem_BootStart(void);
void GimbalSystem_Dispatch(uint32_t events);
void GimbalSystem_IdleRender(void);
GimbalSystemState GimbalSystem_GetState(void);

#endif
