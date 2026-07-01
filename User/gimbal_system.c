#include "gimbal_system.h"

#include "main.h"
#include "bsp_can.h"
#include "Motor.h"
#include "lcd.h"
#include "gimbal_control.h"
#include "gimbal_event.h"
#include "gimbal_keys.h"
#include "gimbal_laser.h"
#include "gimbal_menu.h"

static GimbalControlState g_ctrl;
static GimbalMitCommand g_cmd;
static GimbalMenu g_menu;
static GimbalSystemState g_state;
static uint8_t g_mit_tx_started;
static uint32_t g_mit_pause_until_ms;
static uint32_t g_state_deadline_ms;
static uint32_t g_last_lcd_ms;
static uint32_t g_last_jog_ms;

static uint8_t HasPendingMotorCommand(void)
{
  return (g_ctrl.req_disable_yaw || g_ctrl.req_disable_pitch ||
          g_ctrl.req_enable_yaw || g_ctrl.req_enable_pitch ||
          g_ctrl.req_save_zero_yaw || g_ctrl.req_save_zero_pitch) ? 1U : 0U;
}

static void PauseMitFor(uint32_t now_ms, uint32_t duration_ms)
{
  uint32_t until = now_ms + duration_ms;

  if ((int32_t)(until - g_mit_pause_until_ms) > 0) {
    g_mit_pause_until_ms = until;
  }
}

static void SendMitFrame(void)
{
  if (g_mit_tx_started == 0U || g_ctrl.motors_enabled == 0U) {
    return;
  }
  if (HasPendingMotorCommand()) {
    return;
  }
  if ((int32_t)(HAL_GetTick() - g_mit_pause_until_ms) < 0) {
    return;
  }

  MOTOR_SEND_YAW(&FDCAN1TxFrame, &DM_Motor_Yaw, MIT_Mode,
                 g_cmd.yaw_pos_rad, g_cmd.yaw_vel_rad_s,
                 g_cmd.kp, g_cmd.kd, g_cmd.torque_ff);
  MOTOR_SEND_PITCH(&FDCAN2TxFrame, &DM_Motor_Pitch, MIT_Mode,
                   g_cmd.pitch_pos_rad, g_cmd.pitch_vel_rad_s,
                   g_cmd.kp, g_cmd.kd, g_cmd.torque_ff);
}

static void ApplyControlRequests(uint32_t now_ms)
{
  if (g_ctrl.req_disable_yaw) {
    PauseMitFor(now_ms, 100U);
    if (DM_Motor_Command_Status(&FDCAN1TxFrame, 0x01, Motor_Disable) == HAL_OK) {
      g_ctrl.req_disable_yaw = 0U;
    }
  }
  if (g_ctrl.req_disable_pitch) {
    PauseMitFor(now_ms, 100U);
    if (DM_Motor_Command_Status(&FDCAN2TxFrame, 0x02, Motor_Disable) == HAL_OK) {
      g_ctrl.req_disable_pitch = 0U;
    }
  }
  if (g_ctrl.req_enable_yaw) {
    PauseMitFor(now_ms, 200U);
    if (DM_Motor_Command_Status(&FDCAN1TxFrame, 0x01, Motor_Enable) == HAL_OK) {
      g_ctrl.req_enable_yaw = 0U;
    }
  }
  if (g_ctrl.req_enable_pitch) {
    PauseMitFor(now_ms, 200U);
    if (DM_Motor_Command_Status(&FDCAN2TxFrame, 0x02, Motor_Enable) == HAL_OK) {
      g_ctrl.req_enable_pitch = 0U;
    }
  }
  if (g_ctrl.req_save_zero_yaw) {
    if (DM_Motor_Yaw.Data.State != 0) {
      PauseMitFor(now_ms, 500U);
      if (DM_Motor_Command_Status(&FDCAN1TxFrame, 0x01, Motor_Save_Zero_Position) == HAL_OK) {
        g_ctrl.req_save_zero_yaw = 0U;
      }
    } else {
      g_ctrl.req_save_zero_yaw = 0U;
    }
  }
  if (g_ctrl.req_save_zero_pitch) {
    if (DM_Motor_Pitch.Data.State != 0) {
      PauseMitFor(now_ms, 500U);
      if (DM_Motor_Command_Status(&FDCAN2TxFrame, 0x02, Motor_Save_Zero_Position) == HAL_OK) {
        g_ctrl.req_save_zero_pitch = 0U;
      }
    } else {
      g_ctrl.req_save_zero_pitch = 0U;
    }
  }
  if (g_ctrl.req_power_apply) {
    g_ctrl.req_power_apply = 0U;
    HAL_GPIO_WritePin(MOTOR_PWR_GPIO_Port, MOTOR_PWR_YAW_Pin,
                      g_ctrl.yaw_power_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_PWR_GPIO_Port, MOTOR_PWR_PITCH_Pin,
                      g_ctrl.pitch_power_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

static void ManualJogTask(uint32_t now_ms)
{
  GimbalKeyId held;
  uint8_t manual_active = (g_ctrl.mode == CTRL_MANUAL && GimbalMenu_IsStatusPage() != 0U) ? 1U : 0U;
  uint8_t square_calib_active = (g_ctrl.mode == CTRL_SQUARE_CALIB &&
                                 GimbalMenu_IsSquareCalibActive() != 0U) ? 1U : 0U;

  if (manual_active == 0U && square_calib_active == 0U) {
    g_last_jog_ms = now_ms;
    return;
  }

  held = GimbalKeys_GetHeldKey();
  if (held == GKEY_NONE || held == GKEY_CENTER) {
    g_last_jog_ms = now_ms;
    return;
  }

  if ((now_ms - g_last_jog_ms) < 20U) {
    return;
  }
  g_last_jog_ms = now_ms;

  if (held == GKEY_LEFT) {
    GimbalControl_AddManualYaw(&g_ctrl, 0.5f);
    GimbalKeys_MarkHeldUsed();
  } else if (held == GKEY_RIGHT) {
    GimbalControl_AddManualYaw(&g_ctrl, -0.5f);
    GimbalKeys_MarkHeldUsed();
  } else if (held == GKEY_UP) {
    GimbalControl_AddManualPitch(&g_ctrl, -0.5f);
    GimbalKeys_MarkHeldUsed();
  } else if (held == GKEY_DOWN) {
    GimbalControl_AddManualPitch(&g_ctrl, 0.5f);
    GimbalKeys_MarkHeldUsed();
  }
}

static void MenuServiceDuringDraw(void)
{
  static uint32_t last_service_ms;
  uint32_t now = HAL_GetTick();

  if ((now - last_service_ms) >= 1U) {
    last_service_ms = now;
    GimbalControl_Update(&g_ctrl, now, &g_cmd);
    SendMitFrame();
  }
}

static void DrainKeyEvents(uint32_t now_ms)
{
  GimbalKeyEvent key_event;

  while (GimbalKeys_GetEvent(&key_event)) {
    GimbalMenu_HandleEvent(&g_menu, &g_ctrl, &key_event, now_ms,
                           &DM_Motor_Yaw, &DM_Motor_Pitch);
    if (g_ctrl.motors_enabled == 0U && g_ctrl.mode == CTRL_STOP) {
      g_state = SYS_ESTOP;
    } else if (g_state == SYS_ESTOP && g_ctrl.motors_enabled != 0U) {
      g_state = SYS_RUN;
    }
  }
}

static void BootTick(uint32_t now_ms)
{
  switch (g_state) {
    case SYS_BOOT_DELAY:
      if ((int32_t)(now_ms - g_state_deadline_ms) >= 0) {
        g_state = SYS_BOOT_ENABLE_YAW;
      }
      break;

    case SYS_BOOT_ENABLE_YAW:
      (void)DM_Motor_Command_Status(&FDCAN1TxFrame, 0x01, Motor_Enable);
      g_state_deadline_ms = now_ms + 200U;
      g_state = SYS_BOOT_WAIT_YAW;
      break;

    case SYS_BOOT_WAIT_YAW:
      if ((int32_t)(now_ms - g_state_deadline_ms) >= 0) {
        g_state = SYS_BOOT_ENABLE_PITCH;
      }
      break;

    case SYS_BOOT_ENABLE_PITCH:
      (void)DM_Motor_Command_Status(&FDCAN2TxFrame, 0x02, Motor_Enable);
      g_state_deadline_ms = now_ms + 200U;
      g_state = SYS_BOOT_WAIT_PITCH;
      break;

    case SYS_BOOT_WAIT_PITCH:
      if ((int32_t)(now_ms - g_state_deadline_ms) >= 0) {
        g_state = SYS_BOOT_PRIME_MIT;
      }
      break;

    case SYS_BOOT_PRIME_MIT:
      GimbalControl_Update(&g_ctrl, now_ms, &g_cmd);
      MOTOR_SEND_YAW(&FDCAN1TxFrame, &DM_Motor_Yaw, MIT_Mode,
                     0.0f, 0.0f, g_ctrl.kp, g_ctrl.kd, g_ctrl.torque_ff);
      MOTOR_SEND_PITCH(&FDCAN2TxFrame, &DM_Motor_Pitch, MIT_Mode,
                       0.0f, 0.0f, g_ctrl.kp, g_ctrl.kd, g_ctrl.torque_ff);
      g_mit_tx_started = 1U;
      g_state = SYS_RUN;
      break;

    default:
      break;
  }
}

static void RunTick(uint32_t now_ms)
{
  ApplyControlRequests(now_ms);
  ManualJogTask(now_ms);
  GimbalControl_Update(&g_ctrl, now_ms, &g_cmd);
  SendMitFrame();
}

void GimbalSystem_Init(void)
{
  uint32_t now = HAL_GetTick();

  GimbalControl_Init(&g_ctrl);
  GimbalKeys_Init();
  GimbalLaser_Init();
  GimbalMenu_Init(&g_menu);
  GimbalMenu_SetServiceCallback(MenuServiceDuringDraw);

  g_state = SYS_BOOT_DELAY;
  g_mit_tx_started = 0U;
  g_mit_pause_until_ms = 0U;
  g_state_deadline_ms = now;
  g_last_lcd_ms = now;
  g_last_jog_ms = now;

  while (GimbalMenu_NeedsRedraw()) {
    GimbalMenu_Render(&g_menu, &g_ctrl, &DM_Motor_Yaw, &DM_Motor_Pitch,
                      GimbalKeys_GetAdcRaw());
  }
}

void GimbalSystem_BootStart(void)
{
  g_mit_tx_started = 0U;
  g_state_deadline_ms = HAL_GetTick() + 500U;
  g_state = SYS_BOOT_DELAY;
}

void GimbalSystem_Dispatch(uint32_t events)
{
  uint32_t now = HAL_GetTick();

  if ((events & GIMBAL_EVT_KEY_SCAN_10MS) != 0U) {
    GimbalKeys_Task(now);
    DrainKeyEvents(now);
  }

  if ((events & GIMBAL_EVT_TICK_1MS) != 0U) {
    if (g_state == SYS_RUN || g_state == SYS_ESTOP) {
      RunTick(now);
    } else {
      BootTick(now);
    }
  }
}

void GimbalSystem_IdleRender(void)
{
  uint32_t now = HAL_GetTick();

  if (g_ctrl.lcd_on == 0U) {
    return;
  }

  if (GimbalMenu_NeedsRedraw()) {
    GimbalMenu_Render(&g_menu, &g_ctrl, &DM_Motor_Yaw, &DM_Motor_Pitch,
                      GimbalKeys_GetAdcRaw());
    g_last_lcd_ms = now;
  } else if ((now - g_last_lcd_ms) >= 250U) {
    g_last_lcd_ms = now;
    if (GimbalMenu_IsStatusPage()) {
      GimbalMenu_Render(&g_menu, &g_ctrl, &DM_Motor_Yaw, &DM_Motor_Pitch,
                        GimbalKeys_GetAdcRaw());
    }
  }
}

GimbalSystemState GimbalSystem_GetState(void)
{
  return g_state;
}
