/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : main.c文件的头文件。
  *                   此文件包含应用程序的公共定义。
  ******************************************************************************
  * @attention
  *
  * 版权所有 (c) 2024 STMicroelectronics.
  * 保留所有权利。
  *
  * 本软件的许可条款可以在软件组件根目录的 LICENSE 文件中找到。
  * 如果没有附带 LICENSE 文件，则按 AS-IS 方式提供。
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* 防止递归包含 -----------------------------------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 包含头文件 ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "spi.h"   // SPI1 用于 LCD

/* 私有头文件包含 --------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* 导出类型 --------------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* 导出常量 --------------------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* 导出宏 ----------------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* 导出函数原型 ----------------------------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* 私有宏定义 ------------------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* ========= LCD ST7789V 引脚定义 ========= */
#define LCD_CS_Pin    GPIO_PIN_15
#define LCD_CS_GPIO_Port GPIOE
#define LCD_BLK_Pin   GPIO_PIN_10
#define LCD_BLK_GPIO_Port GPIOB
#define LCD_RES_Pin   GPIO_PIN_11
#define LCD_RES_GPIO_Port GPIOB
#define LCD_DC_Pin     GPIO_PIN_10
#define LCD_DC_GPIO_Port  GPIOD

/* ==== 软件 SPI 引脚 (GPIO 位操作) ==== */
#define LCD_SCK_Pin     GPIO_PIN_3    /* PB3 → 软件SCK */
#define LCD_SCK_GPIO_Port GPIOB
#define LCD_SDA_Pin     GPIO_PIN_7    /* PD7 → 软件MOSI */
#define LCD_SDA_GPIO_Port GPIOD

/* ========= 电机电源控制引脚 ========= */
#define MOTOR_PWR_YAW_Pin   GPIO_PIN_14   /* PC14 → YAW 电机电源 */
#define MOTOR_PWR_PITCH_Pin GPIO_PIN_13   /* PC13 → PITCH 电机电源 */
#define MOTOR_PWR_GPIO_Port  GPIOC
#define PWM_5V_EN_Pin       GPIO_PIN_15   /* PC15 -> PWM connector 5V enable */
#define PWM_5V_EN_GPIO_Port GPIOC

/* USER CODE END Private defines */

/* ========= 零位保存控制 =========
 * 1 = 上电时发送 Save_Zero，把当前位置存为零点（第一次烧录用）
 * 0 = 上电不发送 Save_Zero，使用 flash 中已保存的零位（正常工作用）
 * 流程：设为1 → 烧录 → 上电 → 零位已保存 → 设为0 → 重新烧录
 */
#define SAVE_ZERO_ON_BOOT  0

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
