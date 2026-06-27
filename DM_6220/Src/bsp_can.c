#include "fdcan.h"
#include "bsp_can.h"
#include "Motor.h"

/* ================================================================
 * FDCAN1 帧定义（接 电机1 / YAW 轴）PD0=RX, PD1=TX
 * ================================================================ */
FDCAN_RxFrame_TypeDef FDCAN1_RxFrame;

FDCAN_TxFrame_TypeDef FDCAN1TxFrame = {
  .hcan = &hfdcan1,
  .Header.IdType              = FDCAN_STANDARD_ID,
  .Header.TxFrameType         = FDCAN_DATA_FRAME,
  .Header.DataLength          = 8,
  .Header.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
  .Header.BitRateSwitch       = FDCAN_BRS_OFF,
  .Header.FDFormat            = FDCAN_CLASSIC_CAN,
  .Header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS,
  .Header.MessageMarker       = 0,
};

/* ================================================================
 * FDCAN2 帧定义（接 电机2 / PITCH 轴）PB5=RX, PB6=TX
 * ================================================================ */
FDCAN_RxFrame_TypeDef FDCAN2_RxFrame;

FDCAN_TxFrame_TypeDef FDCAN2TxFrame = {
  .hcan = &hfdcan2,
  .Header.IdType              = FDCAN_STANDARD_ID,
  .Header.TxFrameType         = FDCAN_DATA_FRAME,
  .Header.DataLength          = 8,
  .Header.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
  .Header.BitRateSwitch       = FDCAN_BRS_OFF,
  .Header.FDFormat            = FDCAN_CLASSIC_CAN,
  .Header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS,
  .Header.MessageMarker       = 0,
};

/* ================================================================
 * BSP 初始化：FDCAN1 + FDCAN2 同时使能
 * ================================================================ */
void BSP_FDCAN_Init(void)
{
    /* ---- FDCAN1 滤波器 ---- */
    FDCAN_FilterTypeDef filter;
    filter.IdType       = FDCAN_STANDARD_ID;
    filter.FilterIndex  = 0;
    filter.FilterType   = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1    = 0x00000000;  /* 不过滤，接收所有 ID */
    filter.FilterID2    = 0x00000000;
    HAL_FDCAN_ConfigFilter(&hfdcan1, &filter);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT,
                                  FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_FDCAN_Start(&hfdcan1);

    /* ---- FDCAN2 滤波器 ---- */
    filter.FilterIndex  = 0;
    HAL_FDCAN_ConfigFilter(&hfdcan2, &filter);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT,
                                  FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_FDCAN_Start(&hfdcan2);
}

/* ================================================================
 * 接收中断回调
 * ================================================================ */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    uint8_t drain_count = 0U;
    (void)RxFifo0ITs;

    if (hfdcan->Instance == FDCAN1) {
        while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0U && drain_count < 4U) {
            if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0,
                                       &FDCAN1_RxFrame.Header, FDCAN1_RxFrame.Data) != HAL_OK) {
                break;
            }
            DM_Motor_Info_Update(FDCAN1_RxFrame.Data, &DM_Motor_Yaw);
            drain_count++;
        }
    } else if (hfdcan->Instance == FDCAN2) {
        while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0U && drain_count < 4U) {
            if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0,
                                       &FDCAN2_RxFrame.Header, FDCAN2_RxFrame.Data) != HAL_OK) {
                break;
            }
            DM_Motor_Info_Update(FDCAN2_RxFrame.Data, &DM_Motor_Pitch);
            drain_count++;
        }
    }
}
