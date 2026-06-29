#include "main.h"
#include "fdcan.h"
#include "memorymap.h"
#include "gpio.h"
#include "adc.h"
#include "dma.h"

#include "bsp_can.h"
#include "Motor.h"
#include "lcd.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "gimbal_control.h"
#include "gimbal_keys.h"
#include "gimbal_lvgl_ui.h"

#define LCD_BOOT_SELF_TEST 1U

uint16_t adc_val[2];

static GimbalControlState g_ctrl;
static GimbalMitCommand g_cmd;
static uint8_t g_mit_tx_started;
static uint32_t g_mit_pause_until_ms;

void SystemClock_Config(void);

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

static void Motors_BootEnable(void)
{
  DM_Motor_Command(&FDCAN1TxFrame, 0x01, Motor_Enable);
  HAL_Delay(200);

  DM_Motor_Command(&FDCAN2TxFrame, 0x02, Motor_Enable);
  HAL_Delay(200);

  MOTOR_SEND_YAW(&FDCAN1TxFrame, &DM_Motor_Yaw, MIT_Mode,
                 0.0f, 0.0f, g_ctrl.kp, g_ctrl.kd, g_ctrl.torque_ff);
  MOTOR_SEND_PITCH(&FDCAN2TxFrame, &DM_Motor_Pitch, MIT_Mode,
                   0.0f, 0.0f, g_ctrl.kp, g_ctrl.kd, g_ctrl.torque_ff);
}

static void ApplyControlRequests(void)
{
  uint32_t now = HAL_GetTick();

  if (g_ctrl.req_disable_yaw) {
    PauseMitFor(now, 100U);
    if (DM_Motor_Command_Status(&FDCAN1TxFrame, 0x01, Motor_Disable) == HAL_OK) {
      g_ctrl.req_disable_yaw = 0U;
    }
  }
  if (g_ctrl.req_disable_pitch) {
    PauseMitFor(now, 100U);
    if (DM_Motor_Command_Status(&FDCAN2TxFrame, 0x02, Motor_Disable) == HAL_OK) {
      g_ctrl.req_disable_pitch = 0U;
    }
  }
  if (g_ctrl.req_enable_yaw) {
    PauseMitFor(now, 200U);
    if (DM_Motor_Command_Status(&FDCAN1TxFrame, 0x01, Motor_Enable) == HAL_OK) {
      g_ctrl.req_enable_yaw = 0U;
    }
  }
  if (g_ctrl.req_enable_pitch) {
    PauseMitFor(now, 200U);
    if (DM_Motor_Command_Status(&FDCAN2TxFrame, 0x02, Motor_Enable) == HAL_OK) {
      g_ctrl.req_enable_pitch = 0U;
    }
  }
  if (g_ctrl.req_save_zero_yaw) {
    if (DM_Motor_Yaw.Data.State != 0) {
      PauseMitFor(now, 500U);
      if (DM_Motor_Command_Status(&FDCAN1TxFrame, 0x01, Motor_Save_Zero_Position) == HAL_OK) {
        g_ctrl.req_save_zero_yaw = 0U;
      }
    } else {
      g_ctrl.req_save_zero_yaw = 0U;
    }
  }
  if (g_ctrl.req_save_zero_pitch) {
    if (DM_Motor_Pitch.Data.State != 0) {
      PauseMitFor(now, 500U);
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

#if LCD_BOOT_SELF_TEST == 0U
static void DisplayServiceDuringFlush(void)
{
  static uint32_t last_service_ms = 0U;
  uint32_t now = HAL_GetTick();

  if ((now - last_service_ms) >= 1U) {
    last_service_ms = now;
    ApplyControlRequests();
    GimbalControl_Update(&g_ctrl, now, &g_cmd);
    SendMitFrame();
  }
}

static void ManualJogTask(uint32_t now_ms)
{
  static uint32_t last_jog_ms = 0U;
  GimbalKeyId held;

  if (g_ctrl.mode != CTRL_MANUAL || GimbalLvglUi_IsStatusPage() == 0U) {
    last_jog_ms = now_ms;
    return;
  }

  held = GimbalKeys_GetHeldKey();
  if (held == GKEY_NONE || held == GKEY_CENTER) {
    last_jog_ms = now_ms;
    return;
  }

  if ((now_ms - last_jog_ms) < 20U) {
    return;
  }
  last_jog_ms = now_ms;

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
#endif

static void LcdBootSelfTestTask(uint32_t now_ms)
{
  static uint32_t last_change_ms = 0U;
  static uint8_t color_index = 0U;
  static const uint16_t colors[] = {RED, GREEN, BLUE, BLACK, WHITE};

  if ((now_ms - last_change_ms) < 800U) {
    return;
  }

  last_change_ms = now_ms;
  LCD_Fill(0, 0, LCD_W, LCD_H, colors[color_index]);
  color_index++;
  if (color_index >= (uint8_t)(sizeof(colors) / sizeof(colors[0]))) {
    color_index = 0U;
  }
}

int main(void)
{
  uint32_t last_loop_ms;
  uint32_t last_key_ms = 0U;
#if LCD_BOOT_SELF_TEST == 0U
  uint32_t last_lvgl_ms = 0U;
#endif
  GimbalKeyEvent key_event;

  HAL_Init();
  SystemClock_Config();
  HAL_NVIC_SetPriority(SysTick_IRQn, 1U, 0U);

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
#if !USE_ANALOG_SPI
  MX_SPI1_Init();
#endif
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();

  BSP_FDCAN_Init();
  GimbalControl_Init(&g_ctrl);
  GimbalKeys_Init();

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);

  LCD_Init();
#if LCD_BOOT_SELF_TEST == 0U
  lv_init();
  lv_tick_set_cb(HAL_GetTick);
  LvPortDisp_SetServiceCallback(DisplayServiceDuringFlush);
  LvPortDisp_Init();
  LvPortIndev_Init();
  GimbalLvglUi_Init(&g_ctrl, &DM_Motor_Yaw, &DM_Motor_Pitch);
  GimbalLvglUi_Task(HAL_GetTick(), GimbalKeys_GetAdcRaw());
  (void)lv_timer_handler();
#else
  LCD_Fill(0, 0, LCD_W, LCD_H, RED);
#endif

  HAL_Delay(500);
  Motors_BootEnable();
  GimbalControl_Update(&g_ctrl, HAL_GetTick(), &g_cmd);
  g_mit_tx_started = 1U;

  last_loop_ms = HAL_GetTick();

  while (1)
  {
    uint32_t now = HAL_GetTick();

    if ((now - last_loop_ms) >= 1U) {
      last_loop_ms = now;

      if ((now - last_key_ms) >= 10U) {
        last_key_ms = now;
        GimbalKeys_Task(now);
      }

      while (GimbalKeys_GetEvent(&key_event)) {
        GimbalLvglUi_HandleEvent(&key_event, now);
      }

      ApplyControlRequests();
#if LCD_BOOT_SELF_TEST == 0U
      ManualJogTask(now);
#endif
      GimbalControl_Update(&g_ctrl, now, &g_cmd);
      SendMitFrame();

#if LCD_BOOT_SELF_TEST
      LcdBootSelfTestTask(now);
#else
      if (g_ctrl.lcd_on) {
        GimbalLvglUi_Task(now, GimbalKeys_GetAdcRaw());
        if ((now - last_lvgl_ms) >= 20U) {
          last_lvgl_ms = now;
          (void)lv_timer_handler();
        }
      }
#endif
    }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin);
    for (volatile uint32_t i = 0U; i < 1200000U; i++) {
    }
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
}
#endif
