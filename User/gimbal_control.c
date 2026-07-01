#include "gimbal_control.h"
#include <math.h>

#define PI_F             3.1415926f
#define DEG2RAD          (PI_F / 180.0f)
#define HOME_SPEED_DPS   60.0f
#define CIRCLE_RAMP_MS   1000U

static uint32_t mode_start_ms;
static const uint8_t square_order[GIMBAL_SQUARE_VERTEX_COUNT] = {0U, 1U, 2U, 3U};

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
  state->mode = CTRL_HOMING;
  state->kp = 2.0f;
  state->kd = 0.25f;
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
  GimbalControl_SquareClear(state);
  state->square_edge_ms = GIMBAL_SQUARE_EDGE_MS_DEFAULT;
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
  state->kp = 2.0f;
  state->kd = 0.25f;
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
  state->yaw_target_rad += delta_deg * DEG2RAD;
}

void GimbalControl_AddManualPitch(GimbalControlState *state, float delta_deg)
{
  if (state == 0) {
    return;
  }
  state->pitch_target_rad += delta_deg * DEG2RAD;
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

void GimbalControl_SquareCaptureVertex(GimbalControlState *state, uint8_t index)
{
  if (state == 0 || index >= GIMBAL_SQUARE_VERTEX_COUNT) {
    return;
  }

  state->square_vertices[index].yaw_rad = state->yaw_target_rad;
  state->square_vertices[index].pitch_rad = state->pitch_target_rad;
  state->square_vertex_valid[index] = 1U;
  state->square_calib_index = index;
}

void GimbalControl_SquareClear(GimbalControlState *state)
{
  uint8_t i;

  if (state == 0) {
    return;
  }

  for (i = 0U; i < GIMBAL_SQUARE_VERTEX_COUNT; i++) {
    state->square_vertices[i].yaw_rad = 0.0f;
    state->square_vertices[i].pitch_rad = 0.0f;
    state->square_vertex_valid[i] = 0U;
  }
  state->square_calib_index = 0U;
  state->square_edge_index = 0U;
  state->square_edge_ms = GIMBAL_SQUARE_EDGE_MS_DEFAULT;
  state->square_edge_elapsed_ms = 0U;
}

uint8_t GimbalControl_SquareCanRun(const GimbalControlState *state)
{
  uint8_t i;

  if (state == 0) {
    return 0U;
  }

  for (i = 0U; i < GIMBAL_SQUARE_VERTEX_COUNT; i++) {
    if (state->square_vertex_valid[i] == 0U) {
      return 0U;
    }
  }
  return 1U;
}

void GimbalControl_SquareRun(GimbalControlState *state, uint32_t now_ms)
{
  if (state == 0 || GimbalControl_SquareCanRun(state) == 0U) {
    return;
  }

  state->square_edge_index = 0U;
  state->square_edge_elapsed_ms = 0U;
  state->yaw_target_rad = state->square_vertices[0].yaw_rad;
  state->pitch_target_rad = state->square_vertices[0].pitch_rad;
  GimbalControl_SetMode(state, CTRL_SQUARE_RUN, now_ms);
}

void GimbalControl_SquarePause(GimbalControlState *state)
{
  if (state == 0 || state->mode != CTRL_SQUARE_RUN) {
    return;
  }

  state->mode = CTRL_SQUARE_PAUSE;
}

void GimbalControl_SquareResume(GimbalControlState *state, uint32_t now_ms)
{
  if (state == 0 || state->mode != CTRL_SQUARE_PAUSE || GimbalControl_SquareCanRun(state) == 0U) {
    return;
  }

  GimbalControl_SetMode(state, CTRL_SQUARE_RUN, now_ms);
}

void GimbalControl_SquareReset(GimbalControlState *state, uint32_t now_ms)
{
  if (state == 0 || GimbalControl_SquareCanRun(state) == 0U) {
    return;
  }

  state->square_edge_index = 0U;
  state->square_edge_elapsed_ms = 0U;
  state->yaw_target_rad = state->square_vertices[0].yaw_rad;
  state->pitch_target_rad = state->square_vertices[0].pitch_rad;
  GimbalControl_SetMode(state, CTRL_STOP, now_ms);
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
    uint32_t elapsed_ms = now_ms - mode_start_ms;
    float period_s = (float)state->period_ms * 0.001f;
    float omega = 2.0f * PI_F / period_s;
    float theta = 2.0f * PI_F * (float)(elapsed_ms % state->period_ms) / (float)state->period_ms;
    float yaw_amp = state->yaw_amp_deg * DEG2RAD;
    float pitch_amp = state->pitch_amp_deg * DEG2RAD;
    float circle_scale = 1.0f;

    if (elapsed_ms < CIRCLE_RAMP_MS) {
      circle_scale = (float)elapsed_ms / (float)CIRCLE_RAMP_MS;
    }

    yaw_pos = yaw_amp * circle_scale * cosf(theta);
    pitch_pos = pitch_amp * circle_scale * sinf(theta);
    yaw_vel = -yaw_amp * circle_scale * omega * sinf(theta);
    pitch_vel = pitch_amp * circle_scale * omega * cosf(theta);
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
  } else if (state->mode == CTRL_SQUARE_RUN && GimbalControl_SquareCanRun(state)) {
    uint8_t from_index = square_order[state->square_edge_index];
    uint8_t to_index = square_order[(state->square_edge_index + 1U) % GIMBAL_SQUARE_VERTEX_COUNT];
    float t;

    if (state->square_edge_ms == 0U) {
      state->square_edge_ms = GIMBAL_SQUARE_EDGE_MS_DEFAULT;
    }

    state->square_edge_elapsed_ms++;
    if (state->square_edge_elapsed_ms >= state->square_edge_ms) {
      state->square_edge_elapsed_ms = 0U;
      state->square_edge_index = (uint8_t)((state->square_edge_index + 1U) % GIMBAL_SQUARE_VERTEX_COUNT);
      from_index = square_order[state->square_edge_index];
      to_index = square_order[(state->square_edge_index + 1U) % GIMBAL_SQUARE_VERTEX_COUNT];
    }

    t = (float)state->square_edge_elapsed_ms / (float)state->square_edge_ms;
    yaw_pos = state->square_vertices[from_index].yaw_rad +
              (state->square_vertices[to_index].yaw_rad - state->square_vertices[from_index].yaw_rad) * t;
    pitch_pos = state->square_vertices[from_index].pitch_rad +
                (state->square_vertices[to_index].pitch_rad - state->square_vertices[from_index].pitch_rad) * t;
    state->yaw_target_rad = yaw_pos;
    state->pitch_target_rad = pitch_pos;
  } else if (state->mode == CTRL_SQUARE_PAUSE) {
    yaw_pos = state->yaw_target_rad;
    pitch_pos = state->pitch_target_rad;
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
    case CTRL_SQUARE_CALIB:
      return "SQCAL";
    case CTRL_SQUARE_RUN:
      return "SQRUN";
    case CTRL_SQUARE_PAUSE:
      return "SQPAU";
    default:
      return "UNK";
  }
}
