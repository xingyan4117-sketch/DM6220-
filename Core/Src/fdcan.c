/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   此文件提供FDCAN实例的配置代码
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
#include "fdcan.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;   /* 新增 FDCAN2，接 PITCH 电机 */

/* FDCAN1初始化函数 */
void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 3;
  hfdcan1.Init.NominalSyncJumpWidth = 10;
  hfdcan1.Init.NominalTimeSeg1 = 29;
  hfdcan1.Init.NominalTimeSeg2 = 10;
  hfdcan1.Init.DataPrescaler = 3;
  hfdcan1.Init.DataSyncJumpWidth = 10;
  hfdcan1.Init.DataTimeSeg1 = 29;
  hfdcan1.Init.DataTimeSeg2 = 10;
  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 4;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 0;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 0;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 4;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** 初始化外设时钟
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1时钟使能 */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**FDCAN1 GPIO配置
    PD0     ------> FDCAN1_RX
    PD1     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* FDCAN1中断初始化 */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
  else if (fdcanHandle->Instance == FDCAN2)
  {
    /* FDCAN2 时钟（与 FDCAN1 共用 FDCAN 总线时钟，已在 FDCAN1 里使能，此处保险再使能） */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection  = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /**FDCAN2 GPIO 配置
     * PB5  ------> FDCAN2_RX
     * PB6  ------> FDCAN2_TX
     */
    GPIO_InitStruct.Pin       = GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* FDCAN2 中断初始化 */
    HAL_NVIC_SetPriority(FDCAN2_IT0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
  }
}   /* ← HAL_FDCAN_MspInit 结束（补全缺失的函数闭合括号） */

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* 外设时钟禁用 */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO配置
    PD0     ------> FDCAN1_RX
    PD1     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0|GPIO_PIN_1);

    /* FDCAN1中断反初始化 */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* ================================================================
 * FDCAN2 初始化（接 PITCH 电机）PB5=RX, PB6=TX
 * 与 FDCAN1 相同：Classic CAN, 1Mbps
 * CAN 时钟源 = PLL(480MHz) / 预分频3 = 160MHz
 * NominalPrescaler=3, TimeSeg1=29, TimeSeg2=10  → 1Mbps
 * MessageRAMOffset 必须与 FDCAN1 的 RAM 占用不重叠
 * FDCAN1: Offset=0, 使用约 68 words；FDCAN2 从 68 开始
 * ================================================================ */
void MX_FDCAN2_Init(void)
{
    hfdcan2.Instance                  = FDCAN2;
    hfdcan2.Init.FrameFormat          = FDCAN_FRAME_CLASSIC;
    hfdcan2.Init.Mode                 = FDCAN_MODE_NORMAL;
    hfdcan2.Init.AutoRetransmission   = DISABLE;
    hfdcan2.Init.TransmitPause        = DISABLE;
    hfdcan2.Init.ProtocolException    = DISABLE;
    hfdcan2.Init.NominalPrescaler     = 3;
    hfdcan2.Init.NominalSyncJumpWidth = 10;
    hfdcan2.Init.NominalTimeSeg1      = 29;
    hfdcan2.Init.NominalTimeSeg2      = 10;
    hfdcan2.Init.DataPrescaler        = 3;
    hfdcan2.Init.DataSyncJumpWidth    = 10;
    hfdcan2.Init.DataTimeSeg1         = 29;
    hfdcan2.Init.DataTimeSeg2         = 10;
    hfdcan2.Init.MessageRAMOffset     = 68;   /* 紧接 FDCAN1 之后 */
    hfdcan2.Init.StdFiltersNbr        = 1;
    hfdcan2.Init.ExtFiltersNbr        = 0;
    hfdcan2.Init.RxFifo0ElmtsNbr     = 4;
    hfdcan2.Init.RxFifo0ElmtSize     = FDCAN_DATA_BYTES_8;
    hfdcan2.Init.RxFifo1ElmtsNbr     = 0;
    hfdcan2.Init.RxFifo1ElmtSize     = FDCAN_DATA_BYTES_8;
    hfdcan2.Init.RxBuffersNbr        = 0;
    hfdcan2.Init.RxBufferSize        = FDCAN_DATA_BYTES_8;
    hfdcan2.Init.TxEventsNbr         = 0;
    hfdcan2.Init.TxBuffersNbr        = 0;
    hfdcan2.Init.TxFifoQueueElmtsNbr = 4;
    hfdcan2.Init.TxFifoQueueMode     = FDCAN_TX_FIFO_OPERATION;
    hfdcan2.Init.TxElmtSize          = FDCAN_DATA_BYTES_8;
    if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE END 1 */
