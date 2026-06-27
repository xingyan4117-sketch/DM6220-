#include "gimbal_control.h"
#include <math.h>

#define PI_F             3.1415926f
#define DEG2RAD          (PI_F / 180.0f)
#define HOME_SPEED_DPS   60.0f

static uint32_t mode_start_ms;

static float clampf(float value, float min_value, float max_value)
{
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

static float ramp_to_zero(float value, float max_step)
{
  if (value > max_step) {
    return value - max_step;
  }
  if (value < -max_step) {
    return value + max_step;
  }
  return 0.0f;
}

void GimbalControl_Init(GimbalControlState *state)
{
  if (state == 0) {
    return;
  }
  state->mode = CTRL_MIT_CIRCLE;
  state->kp = 1.0f;
  state->kd = 0.08f;
  state->torque_ff = 0.0f;
  state->yaw_amp_deg = 10.0f;
  state->pitch_amp_deg = 10.0f;
  state->period_ms = 1000U;
  state->yaw_target_rad = 0.0f;
  state->pitch_target_rad = 0.0f;
  state->yaw_soft_zero_rad = 0.0f;
  state->pitch_soft_zero_rad = 0.0f;
  state->motors_enabled = 1U;
  state->yaw_power_on = 1U;
  state->pitch_power_on = 1U;
  state->lcd_on = 1U;
  state->req_enable_yaw = 0U;
  state->req_enable_pitch = 0U;
  state->req_disable_yaw = 0U;
  state->req_disable_pitch = 0U;
  state->req_save_zero_yaw = 0U;
  state->req_save_zero_pitch = 0U;
  state->req_power_apply = 0U;
  mode_start_ms = 0U;
}

void GimbalControl_ClampParams(GimbalControlState *state)
{
  if (state == 0) {
    return;
  }
  state->kp = clampf(state->kp, 0.0f, 50.0f);
  state->kd = clampf(state->kd, 0.0f, 2.0f);
  state->torque_ff = clampf(state->torque_ff, -1.0f, 1.0f);
  state->yaw_amp_deg = clampf(state->yaw_amp_deg, 0.0f, 45.0f);
  state->pitch_amp_deg = clampf(state->pitch_amp_deg, 0.0f, 45.0f);
  if (state->period_ms < 500U) {
    state->period_ms = 500U;
  } else if (state->period_ms > 5000U) {
    state->period_ms = 5000U;
  }
}

void GimbalControl_SetMode(GimbalControlState *state, GimbalCtrlMode mode, uint32_t now_ms)
{
  if (state == 0) {
    return;
  }
  GimbalControl_ClampParams(state);
  state->mode = mode;
  mode_start_ms = now_ms;
}

void GimbalControl_RequestHome(GimbalControlState *state, uint32_t now_ms)
{
  GimbalControl_SetMode(state, CTRL_HOMING, now_ms);
}

void GimbalControl_EStop(GimbalControlState *state)
{
  if (state == 0) {
    return;
  }
  state->mode = CTRL_STOP;
  state->motors_enabled = 0U;
  state->req_disable_yaw = 1U;
  state->req_disable_pitch = 1U;
}

void GimbalControl_ResetDefaults(GimbalControlState *state)
{
  if (state == 0) {
    return;
  }
  state->kp = 1.0f;
  state->kd = 0.08f;
  state->torque_ff = 0.0f;
  state->yaw_amp_deg = 10.0f;
  state->pitch_amp_deg = 10.0f;
  state->period_ms = 1000U;
}

void GimbalControl_AddManualYaw(GimbalControlState *state, float delta_deg)
{
  if (state == 0) {
    return;
  }
  state->yaw_target_rad = clampf(state->yaw_target_rad + delta_deg * DEG2RAD,
                                 -45.0f * DEG2RAD, 45.0f * DEG2RAD);
}

void GimbalControl_AddManualPitch(GimbalControlState *state, float delta_deg)
{
  if (state == 0) {
    return;
  }
  state->pitch_target_rad = clampf(state->pitch_target_rad + delta_deg * DEG2RAD,
                                   -45.0f * DEG2RAD, 45.0f * DEG2RAD);
}

void GimbalControl_SetSoftZero(GimbalControlState *state, float yaw_now_rad, float pitch_now_rad)
{
  if (state == 0) {
    return;
  }
  state->yaw_soft_zero_rad = yaw_now_rad;
  state->pitch_soft_zero_rad = pitch_now_rad;
  state->yaw_target_rad = 0.0f;
  state->pitch_target_rad = 0.0f;
}

void GimbalControl_Update(GimbalControlState *state, uint32_t now_ms, GimbalMitCommand *cmd)
{
  float yaw_pos = state->yaw_target_rad;
  float pitch_pos = state->pitch_target_rad;
  float yaw_vel = 0.0f;
  float pitch_vel = 0.0f;

  if (state == 0 || cmd == 0) {
    return;
  }
  GimbalControl_ClampParams(state);

  if (state->mode == CTRL_MIT_CIRCLE) {
    float period_s = (float)state->period_ms * 0.001f;
    float omega = 2.0f * PI_F / period_s;
    float theta = 2.0f * PI_F * (float)((now_ms - mode_start_ms) % state->period_ms) / (float)state->period_ms;
    float yaw_amp = state->yaw_amp_deg * DEG2RAD;
    float pitch_amp = state->pitch_amp_deg * DEG2RAD;
    yaw_pos = yaw_amp * cosf(theta);
    pitch_pos = pitch_amp * sinf(theta);
    yaw_vel = -yaw_amp * omega * sinf(theta);
    pitch_vel = pitch_amp * omega * cosf(theta);
    state->yaw_target_rad = yaw_pos;
    state->pitch_target_rad = pitch_pos;
  } else if (state->mode == CTRL_HOMING) {
    float step = HOME_SPEED_DPS * DEG2RAD * 0.001f;
    state->yaw_target_rad = ramp_to_zero(state->yaw_target_rad, step);
    state->pitch_target_rad = ramp_to_zero(state->pitch_target_rad, step);
    yaw_pos = state->yaw_target_rad;
    pitch_pos = state->pitch_target_rad;
    if (yaw_pos == 0.0f && pitch_pos == 0.0f) {
      state->mode = CTRL_STOP;
    }
  } else {
    yaw_pos = state->yaw_target_rad;
    pitch_pos = state->pitch_target_rad;
  }

  cmd->yaw_pos_rad = yaw_pos - state->yaw_soft_zero_rad;
  cmd->pitch_pos_rad = pitch_pos - state->pitch_soft_zero_rad;
  cmd->yaw_vel_rad_s = yaw_vel;
  cmd->pitch_vel_rad_s = pitch_vel;
  cmd->kp = state->kp;
  cmd->kd = state->kd;
  cmd->torque_ff = state->torque_ff;
}

const char *GimbalControl_ModeName(GimbalCtrlMode mode)
{
  switch (mode) {
    case CTRL_STOP:
      return "STOP";
    case CTRL_MIT_CIRCLE:
      return "CIRCLE";
    case CTRL_MANUAL:
      return "MANUAL";
    case CTRL_HOMING:
      return "HOME";
    case CTRL_CALIB_CONFIRM:
      return "CALIB";
    default:
      return "UNK";
  }
}
