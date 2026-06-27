/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   中断服务程序。
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

/* 包含头文件 ------------------------------------------------------------------*/
#include "main.h"
#include "stm32h7xx_it.h"
/* 私有头文件包含 --------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* 私有类型定义 ----------------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* 私有宏定义 ------------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* 私有宏 ----------------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* 私有变量 --------------------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* 私有函数原型 ----------------------------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* 私有用户代码 ----------------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void fault_blink(void)
{
  while (1)
  {
    HAL_GPIO_TogglePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin);
    for (volatile uint32_t i = 0U; i < 350000U; i++) {
    }
  }
}

/* USER CODE END 0 */

/* 外部变量 --------------------------------------------------------------------*/
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;   /* 新增 FDCAN2 */
extern DMA_HandleTypeDef hdma_adc1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex处理器中断和异常处理程序          */
/******************************************************************************/
/**
  * @brief  此函数处理不可屏蔽中断。
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief  此函数处理硬故障中断。
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  fault_blink();

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief  此函数处理内存管理故障。
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  fault_blink();

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief  此函数处理预取故障、内存访问故障。
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */
  fault_blink();

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief  此函数处理未定义指令或非法状态。
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
  fault_blink();

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief  此函数处理通过SWI指令的系统服务调用。
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief  此函数处理调试监视器。
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief  此函数处理可挂起的系统服务请求。
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief  此函数处理系统滴答定时器。
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx外设中断处理程序                                    */
/* 在此处添加所用外设的中断处理程序。                  */
/* 有关可用外设中断处理程序名称，请参考启动文件(startup_stm32h7xx.s)。                    */
/******************************************************************************/

/**
  * @brief  此函数处理FDCAN1中断0。
  */
void FDCAN1_IT0_IRQHandler(void)
{
  /* USER CODE BEGIN FDCAN1_IT0_IRQn 0 */

  /* USER CODE END FDCAN1_IT0_IRQn 0 */
  HAL_FDCAN_IRQHandler(&hfdcan1);
  /* USER CODE BEGIN FDCAN1_IT0_IRQn 1 */

  /* USER CODE END FDCAN1_IT0_IRQn 1 */
}

void DMA1_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc1);
}

/* USER CODE BEGIN 1 */

/**
  * @brief  此函数处理FDCAN2中断0（接 PITCH 电机）
  */
void FDCAN2_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&hfdcan2);
}

/* USER CODE END 1 */
