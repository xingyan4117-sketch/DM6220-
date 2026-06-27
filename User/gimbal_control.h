#ifndef GIMBAL_CONTROL_H
#define GIMBAL_CONTROL_H

#include <stdint.h>

typedef enum {
  CTRL_STOP = 0,
  CTRL_MIT_CIRCLE,
  CTRL_MANUAL,
  CTRL_HOMING,
  CTRL_CALIB_CONFIRM
} GimbalCtrlMode;

typedef struct {
  GimbalCtrlMode mode;
  float kp;
  float kd;
  float torque_ff;
  float yaw_amp_deg;
  float pitch_amp_deg;
  uint16_t period_ms;
  float yaw_target_rad;
  float pitch_target_rad;
  float yaw_soft_zero_rad;
  float pitch_soft_zero_rad;
  uint8_t motors_enabled;
  uint8_t yaw_power_on;
  uint8_t pitch_power_on;
  uint8_t lcd_on;
  uint8_t req_enable_yaw;
  uint8_t req_enable_pitch;
  uint8_t req_disable_yaw;
  uint8_t req_disable_pitch;
  uint8_t req_save_zero_yaw;
  uint8_t req_save_zero_pitch;
  uint8_t req_power_apply;
} GimbalControlState;

typedef struct {
  float yaw_pos_rad;
  float yaw_vel_rad_s;
  float pitch_pos_rad;
  float pitch_vel_rad_s;
  float kp;
  float kd;
  float torque_ff;
} GimbalMitCommand;

void GimbalControl_Init(GimbalControlState *state);
void GimbalControl_SetMode(GimbalControlState *state, GimbalCtrlMode mode, uint32_t now_ms);
void GimbalControl_RequestHome(GimbalControlState *state, uint32_t now_ms);
void GimbalControl_EStop(GimbalControlState *state);
void GimbalControl_ResetDefaults(GimbalControlState *state);
void GimbalControl_AddManualYaw(GimbalControlState *state, float delta_deg);
void GimbalControl_AddManualPitch(GimbalControlState *state, float delta_deg);
void GimbalControl_SetSoftZero(GimbalControlState *state, float yaw_now_rad, float pitch_now_rad);
void GimbalControl_Update(GimbalControlState *state, uint32_t now_ms, GimbalMitCommand *cmd);
void GimbalControl_ClampParams(GimbalControlState *state);
const char *GimbalControl_ModeName(GimbalCtrlMode mode);

#endif
