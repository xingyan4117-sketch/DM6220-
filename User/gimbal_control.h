#ifndef GIMBAL_CONTROL_H
#define GIMBAL_CONTROL_H

#include <stdint.h>

typedef enum {
  CTRL_STOP = 0,
  CTRL_MIT_CIRCLE,
  CTRL_MANUAL,
  CTRL_HOMING,
  CTRL_CALIB_CONFIRM,
  CTRL_SQUARE_CALIB,
  CTRL_SQUARE_RUN,
  CTRL_SQUARE_PAUSE
} GimbalCtrlMode;

#define GIMBAL_SQUARE_VERTEX_COUNT      4U
#define GIMBAL_SQUARE_EDGE_MS_DEFAULT   2000U

typedef struct {
  float yaw_rad;
  float pitch_rad;
} GimbalSquareVertex;

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
  GimbalSquareVertex square_vertices[GIMBAL_SQUARE_VERTEX_COUNT];
  uint8_t square_vertex_valid[GIMBAL_SQUARE_VERTEX_COUNT];
  uint8_t square_calib_index;
  uint8_t square_edge_index;
  uint16_t square_edge_ms;
  uint16_t square_edge_elapsed_ms;
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
void GimbalControl_SquareCaptureVertex(GimbalControlState *state, uint8_t index);
void GimbalControl_SquareClear(GimbalControlState *state);
uint8_t GimbalControl_SquareCanRun(const GimbalControlState *state);
void GimbalControl_SquareRun(GimbalControlState *state, uint32_t now_ms);
void GimbalControl_SquarePause(GimbalControlState *state);
void GimbalControl_SquareResume(GimbalControlState *state, uint32_t now_ms);
void GimbalControl_SquareReset(GimbalControlState *state, uint32_t now_ms);
void GimbalControl_Update(GimbalControlState *state, uint32_t now_ms, GimbalMitCommand *cmd);
void GimbalControl_ClampParams(GimbalControlState *state);
const char *GimbalControl_ModeName(GimbalCtrlMode mode);

#endif
