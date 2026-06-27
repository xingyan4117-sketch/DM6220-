/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.h
  * @brief   此文件包含中断处理程序的头文件。
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
#ifndef __STM32H7xx_IT_H
#define __STM32H7xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

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
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void FDCAN1_IT0_IRQHandler(void);
void DMA1_Stream0_IRQHandler(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32H7xx_IT_H */
