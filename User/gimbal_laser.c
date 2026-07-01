#include "gimbal_laser.h"
#include "tim.h"

static uint8_t laser_enabled;
static uint8_t laser_brightness = GIMBAL_LASER_BRIGHTNESS_DEFAULT;

static uint32_t Laser_CompareFromPercent(uint8_t percent)
{
  uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim1);

  if (percent > 100U) {
    percent = 100U;
  }

  return ((period + 1U) * percent) / 100U;
}

static void Laser_ApplyOutput(void)
{
  uint8_t output = laser_enabled ? laser_brightness : 0U;

  if (laser_enabled) {
    HAL_GPIO_WritePin(PWM_5V_EN_GPIO_Port, PWM_5V_EN_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, Laser_CompareFromPercent(output));
  } else {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0U);
    HAL_GPIO_WritePin(PWM_5V_EN_GPIO_Port, PWM_5V_EN_Pin, GPIO_PIN_RESET);
  }
}

void GimbalLaser_Init(void)
{
  laser_enabled = 0U;
  laser_brightness = GIMBAL_LASER_BRIGHTNESS_DEFAULT;
  (void)HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  Laser_ApplyOutput();
}

void GimbalLaser_SetEnabled(uint8_t enabled)
{
  laser_enabled = enabled ? 1U : 0U;
  Laser_ApplyOutput();
}

void GimbalLaser_SetBrightness(uint8_t percent)
{
  if (percent > 100U) {
    percent = 100U;
  }
  laser_brightness = percent;
  Laser_ApplyOutput();
}

uint8_t GimbalLaser_IsEnabled(void)
{
  return laser_enabled;
}

uint8_t GimbalLaser_GetBrightness(void)
{
  return laser_brightness;
}
