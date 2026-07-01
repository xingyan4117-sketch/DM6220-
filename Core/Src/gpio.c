/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();   /* 可控输出: PC13(OUT2) + PC14(OUT1) */

  /* ---- 可控输出1: PC14 → YAW电机(CAN1) 电源使能 ---- */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin   = GPIO_PIN_14;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);  /* 默认开启 */

  /* ---- 可控输出2: PC13 → PITCH电机(CAN2) 电源使能 ---- */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);  /* 默认开启 */

  GPIO_InitStruct.Pin = PWM_5V_EN_Pin;
  HAL_GPIO_Init(PWM_5V_EN_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(PWM_5V_EN_GPIO_Port, PWM_5V_EN_Pin, GPIO_PIN_RESET);

  /* ---- LCD GPIO 初始化 ---- */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* LCD_CS: PE15, 推挽输出, 默认高(不选中) */
  GPIO_InitStruct.Pin  = LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

  /* LCD_BLK: PB10, 推挽输出, 默认高(开背光) */
  GPIO_InitStruct.Pin  = LCD_BLK_Pin;
  HAL_GPIO_Init(LCD_BLK_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_RESET);

  /* LCD_RES: PB11, 推挽输出 */
  GPIO_InitStruct.Pin  = LCD_RES_Pin;
  HAL_GPIO_Init(LCD_RES_GPIO_Port, &GPIO_InitStruct);

  /* LCD_DC: PD10, 推挽输出 */
  GPIO_InitStruct.Pin  = LCD_DC_Pin;
  HAL_GPIO_Init(LCD_DC_GPIO_Port, &GPIO_InitStruct);

  /* ---- 软件 SPI (GPIO 位操作) SCK=PB3, MOSI=PD7 ---- */
  GPIO_InitStruct.Pin  = LCD_SCK_Pin;   /* PB3 → 软件 SCK */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  /* 高速翻转 */
  HAL_GPIO_Init(LCD_SCK_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin  = LCD_SDA_Pin;   /* PD7 → 软件 MOSI */
  HAL_GPIO_Init(LCD_SDA_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
