/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_hal_conf.h
  * @author  MCD应用团队
  * @brief   HAL配置文件。
  ******************************************************************************
  * @attention
  *
  * 版权所有 (c) 2017 STMicroelectronics.
  * 保留所有权利。
  *
  * 本软件的许可条款可以在软件组件根目录的 LICENSE 文件中找到。
  * 如果没有附带 LICENSE 文件，则按 AS-IS 方式提供。
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* 防止递归包含 -----------------------------------------------------------------*/
#ifndef STM32H7xx_HAL_CONF_H
#define STM32H7xx_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* 导出类型 --------------------------------------------------------------------*/
/* 导出常量 --------------------------------------------------------------------*/

/* ########################## 模块选择 ############################## */
/**
  * @brief 这是HAL驱动中要使用的模块列表
  */
#define HAL_MODULE_ENABLED

#define HAL_ADC_MODULE_ENABLED
#define HAL_FDCAN_MODULE_ENABLED
/* #define HAL_FMAC_MODULE_ENABLED   */
/* #define HAL_CEC_MODULE_ENABLED   */
/* #define HAL_COMP_MODULE_ENABLED   */
/* #define HAL_CORDIC_MODULE_ENABLED   */
/* #define HAL_CRC_MODULE_ENABLED   */
/* #define HAL_CRYP_MODULE_ENABLED   */
/* #define HAL_DAC_MODULE_ENABLED   */
/* #define HAL_DCMI_MODULE_ENABLED   */
/* #define HAL_DMA2D_MODULE_ENABLED   */
/* #define HAL_ETH_MODULE_ENABLED   */
/* #define HAL_ETH_LEGACY_MODULE_ENABLED   */
/* #define HAL_NAND_MODULE_ENABLED   */
/* #define HAL_NOR_MODULE_ENABLED   */
/* #define HAL_OTFDEC_MODULE_ENABLED   */
/* #define HAL_SRAM_MODULE_ENABLED   */
/* #define HAL_SDRAM_MODULE_ENABLED   */
/* #define HAL_HASH_MODULE_ENABLED   */
/* #define HAL_HRTIM_MODULE_ENABLED   */
/* #define HAL_HSEM_MODULE_ENABLED   */
/* #define HAL_GFXMMU_MODULE_ENABLED   */
/* #define HAL_JPEG_MODULE_ENABLED   */
/* #define HAL_OPAMP_MODULE_ENABLED   */
/* #define HAL_OSPI_MODULE_ENABLED   */
/* #define HAL_XSPI_MODULE_ENABLED   */
/* #define HAL_I2S_MODULE_ENABLED   */
/* #define HAL_SMBUS_MODULE_ENABLED   */
/* #define HAL_IWDG_MODULE_ENABLED   */
/* #define HAL_LPTIM_MODULE_ENABLED   */
/* #define HAL_LTDC_MODULE_ENABLED   */
/* #define HAL_XSPI_MODULE_ENABLED   */
/* #define HAL_RAMECC_MODULE_ENABLED   */
/* #define HAL_RNG_MODULE_ENABLED   */
/* #define HAL_RTC_MODULE_ENABLED   */
/* #define HAL_SAI_MODULE_ENABLED   */
/* #define HAL_SD_MODULE_ENABLED   */
/* #define HAL_MMC_MODULE_ENABLED   */
/* #define HAL_SPDIFRX_MODULE_ENABLED   */
#define HAL_SPI_MODULE_ENABLED
/* #define HAL_SWPMI_MODULE_ENABLED   */
/* #define HAL_TIM_MODULE_ENABLED   */
/* #define HAL_UART_MODULE_ENABLED   */
/* #define HAL_USART_MODULE_ENABLED   */
/* #define HAL_IRDA_MODULE_ENABLED   */
/* #define HAL_SMARTCARD_MODULE_ENABLED   */
/* #define HAL_WWDG_MODULE_ENABLED   */
/* #define HAL_PCD_MODULE_ENABLED   */
/* #define HAL_HCD_MODULE_ENABLED   */
/* #define HAL_DFSDM_MODULE_ENABLED   */
/* #define HAL_DSI_MODULE_ENABLED   */
/* #define HAL_JPEG_MODULE_ENABLED   */
/* #define HAL_MDIOS_MODULE_ENABLED   */
/* #define HAL_PSSI_MODULE_ENABLED   */
/* #define HAL_DTS_MODULE_ENABLED   */
#define HAL_GPIO_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_MDMA_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_HSEM_MODULE_ENABLED

/* ########################## 振荡器值适配 ####################*/
/**
  * @brief 调整应用中使用的外部高速振荡器(HSE)的值。
  *        此值由RCC HAL模块用于计算系统频率
  *        (当HSE用作系统时钟源时，直接或通过PLL)。
  */
#if !defined  (HSE_VALUE)
#define HSE_VALUE    (24000000UL) /*!< 外部振荡器的值，单位Hz : FPGA情况下固定为60MHz */
#endif /* HSE_VALUE */

#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    (100UL)   /*!< HSE启动超时时间，单位ms */
#endif /* HSE_STARTUP_TIMEOUT */

/**
  * @brief 内部振荡器(CSI)默认值。
  *        此值是复位后的默认CSI值。
  */
#if !defined  (CSI_VALUE)
  #define CSI_VALUE    (4000000UL) /*!< 内部振荡器的值，单位Hz*/
#endif /* CSI_VALUE */

/**
  * @brief 内部高速振荡器(HSI)值。
  *        此值由RCC HAL模块用于计算系统频率
  *        (当HSI用作系统时钟源时，直接或通过PLL)。
  */
#if !defined  (HSI_VALUE)
  #define HSI_VALUE    (64000000UL) /*!< 内部振荡器的值，单位Hz*/
#endif /* HSI_VALUE */

/**
  * @brief 外部低速振荡器(LSE)值。
  *        此值由UART、RTC HAL模块用于计算系统频率
  */
#if !defined  (LSE_VALUE)
  #define LSE_VALUE    (32768UL) /*!< 外部振荡器的值，单位Hz*/
#endif /* LSE_VALUE */

#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    (5000UL)   /*!< LSE启动超时时间，单位ms */
#endif /* LSE_STARTUP_TIMEOUT */

#if !defined  (LSI_VALUE)
  #define LSI_VALUE  (32000UL)              /*!< LSI典型值，单位Hz*/
#endif /* LSI_VALUE */                      /*!< 内部低速振荡器的值，单位Hz
                                              实际值可能因电压和温度变化而变化。*/

/**
  * @brief I2S外设的外部时钟源
  *        此值由I2S HAL模块用于计算I2S时钟源
  *        频率，此源直接通过I2S_CKIN引脚输入。
  */
#if !defined  (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE    12288000UL /*!< 外部时钟的值，单位Hz*/
#endif /* EXTERNAL_CLOCK_VALUE */

/* 提示: 为避免每次需要使用不同的HSE时修改此文件，
   ===  您可以在工具链编译器预处理器中定义HSE值。 */

/* ########################### 系统配置 ######################### */
/**
  * @brief 这是HAL系统配置部分
  */
#define  VDD_VALUE                    (3300UL) /*!< VDD的值，单位mv */
#define  TICK_INT_PRIORITY            (15UL) /*!< tick中断优先级 */
#define  USE_RTOS                     0
#define  USE_SD_TRANSCEIVER           0U               /*!< 使用uSD收发器 */
#define  USE_SPI_CRC	              0U               /*!< 在SPI中使用CRC */

#define  USE_HAL_ADC_REGISTER_CALLBACKS     0U /* ADC注册回调禁用     */
#define  USE_HAL_CEC_REGISTER_CALLBACKS     0U /* CEC注册回调禁用     */
#define  USE_HAL_COMP_REGISTER_CALLBACKS    0U /* COMP注册回调禁用    */
#define  USE_HAL_CORDIC_REGISTER_CALLBACKS  0U /* CORDIC注册回调禁用  */
#define  USE_HAL_CRYP_REGISTER_CALLBACKS    0U /* CRYP注册回调禁用    */
#define  USE_HAL_DAC_REGISTER_CALLBACKS     0U /* DAC注册回调禁用     */
#define  USE_HAL_DCMI_REGISTER_CALLBACKS    0U /* DCMI注册回调禁用    */
#define  USE_HAL_DFSDM_REGISTER_CALLBACKS   0U /* DFSDM注册回调禁用   */
#define  USE_HAL_DMA2D_REGISTER_CALLBACKS   0U /* DMA2D注册回调禁用   */
#define  USE_HAL_DSI_REGISTER_CALLBACKS     0U /* DSI注册回调禁用     */
#define  USE_HAL_DTS_REGISTER_CALLBACKS     0U /* DTS注册回调禁用     */
#define  USE_HAL_ETH_REGISTER_CALLBACKS     0U /* ETH注册回调禁用     */
#define  USE_HAL_FDCAN_REGISTER_CALLBACKS   0U /* FDCAN注册回调禁用   */
#define  USE_HAL_FMAC_REGISTER_CALLBACKS    0U /* FMAC注册回调禁用  */
#define  USE_HAL_NAND_REGISTER_CALLBACKS    0U /* NAND注册回调禁用    */
#define  USE_HAL_NOR_REGISTER_CALLBACKS     0U /* NOR注册回调禁用     */
#define  USE_HAL_SDRAM_REGISTER_CALLBACKS   0U /* SDRAM注册回调禁用   */
#define  USE_HAL_SRAM_REGISTER_CALLBACKS    0U /* SRAM注册回调禁用    */
#define  USE_HAL_HASH_REGISTER_CALLBACKS    0U /* HASH注册回调禁用    */
#define  USE_HAL_HCD_REGISTER_CALLBACKS     0U /* HCD注册回调禁用     */
#define  USE_HAL_GFXMMU_REGISTER_CALLBACKS  0U /* GFXMMU注册回调禁用  */
#define  USE_HAL_HRTIM_REGISTER_CALLBACKS   0U /* HRTIM注册回调禁用   */
#define  USE_HAL_I2C_REGISTER_CALLBACKS     0U /* I2C注册回调禁用     */
#define  USE_HAL_I2S_REGISTER_CALLBACKS     0U /* I2S注册回调禁用     */
#define  USE_HAL_IRDA_REGISTER_CALLBACKS    0U /* IRDA注册回调禁用    */
#define  USE_HAL_JPEG_REGISTER_CALLBACKS    0U /* JPEG注册回调禁用    */
#define  USE_HAL_LPTIM_REGISTER_CALLBACKS   0U /* LPTIM注册回调禁用   */
#define  USE_HAL_LTDC_REGISTER_CALLBACKS    0U /* LTDC注册回调禁用    */
#define  USE_HAL_MDIOS_REGISTER_CALLBACKS   0U /* MDIO注册回调禁用    */
#define  USE_HAL_MMC_REGISTER_CALLBACKS     0U /* MMC注册回调禁用     */
#define  USE_HAL_OPAMP_REGISTER_CALLBACKS   0U /* MDIO注册回调禁用    */
#define  USE_HAL_OSPI_REGISTER_CALLBACKS    0U /* OSPI注册回调禁用    */
#define  USE_HAL_OTFDEC_REGISTER_CALLBACKS  0U /* OTFDEC注册回调禁用  */
#define  USE_HAL_PCD_REGISTER_CALLBACKS     0U /* PCD注册回调禁用     */
#define  USE_HAL_QSPI_REGISTER_CALLBACKS    0U /* QSPI注册回调禁用    */
#define  USE_HAL_RNG_REGISTER_CALLBACKS     0U /* RNG注册回调禁用     */
#define  USE_HAL_RTC_REGISTER_CALLBACKS     0U /* RTC注册回调禁用     */
#define  USE_HAL_SAI_REGISTER_CALLBACKS     0U /* SAI注册回调禁用     */
#define  USE_HAL_SD_REGISTER_CALLBACKS      0U /* SD注册回调禁用      */
#define  USE_HAL_SMARTCARD_REGISTER_CALLBACKS  0U /* SMARTCARD注册回调禁用 */
#define  USE_HAL_SPDIFRX_REGISTER_CALLBACKS 0U /* SPDIFRX注册回调禁用 */
#define  USE_HAL_SMBUS_REGISTER_CALLBACKS   0U /* SMBUS注册回调禁用   */
#define  USE_HAL_SPI_REGISTER_CALLBACKS     0U /* SPI注册回调禁用     */
#define  USE_HAL_SWPMI_REGISTER_CALLBACKS   0U /* SWPMI注册回调禁用   */
#define  USE_HAL_TIM_REGISTER_CALLBACKS     0U /* TIM注册回调禁用     */
#define  USE_HAL_UART_REGISTER_CALLBACKS    0U /* UART注册回调禁用    */
#define  USE_HAL_USART_REGISTER_CALLBACKS   0U /* USART注册回调禁用   */
#define  USE_HAL_WWDG_REGISTER_CALLBACKS    0U /* WWDG注册回调禁用    */

/* ########################### 以太网配置 ######################### */
#define ETH_TX_DESC_CNT         4U  /* 以太网Tx DMA描述符数量 */
#define ETH_RX_DESC_CNT         4U  /* 以太网Rx DMA描述符数量 */

#define ETH_MAC_ADDR0    (0x02UL)
#define ETH_MAC_ADDR1    (0x00UL)
#define ETH_MAC_ADDR2    (0x00UL)
#define ETH_MAC_ADDR3    (0x00UL)
#define ETH_MAC_ADDR4    (0x00UL)
#define ETH_MAC_ADDR5    (0x00UL)

/* ########################## 断言选择 ############################## */
/**
  * @brief 取消注释下面的行以展开HAL驱动代码中的"assert_param"宏
  */
/* #define USE_FULL_ASSERT    1U */

/* 包含头文件 ------------------------------------------------------------------*/
/**
  * @brief 包含模块的头文件
  */

#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32h7xx_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32h7xx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32h7xx_hal_dma.h"
#endif /* HAL_DMA_MODULE_ENABLED */

#ifdef HAL_MDMA_MODULE_ENABLED
 #include "stm32h7xx_hal_mdma.h"
#endif /* HAL_MDMA_MODULE_ENABLED */

#ifdef HAL_HASH_MODULE_ENABLED
  #include "stm32h7xx_hal_hash.h"
#endif /* HAL_HASH_MODULE_ENABLED */

#ifdef HAL_DCMI_MODULE_ENABLED
  #include "stm32h7xx_hal_dcmi.h"
#endif /* HAL_DCMI_MODULE_ENABLED */

#ifdef HAL_DMA2D_MODULE_ENABLED
  #include "stm32h7xx_hal_dma2d.h"
#endif /* HAL_DMA2D_MODULE_ENABLED */

#ifdef HAL_DSI_MODULE_ENABLED
  #include "stm32h7xx_hal_dsi.h"
#endif /* HAL_DSI_MODULE_ENABLED */

#ifdef HAL_DFSDM_MODULE_ENABLED
  #include "stm32h7xx_hal_dfsdm.h"
#endif /* HAL_DFSDM_MODULE_ENABLED */

#ifdef HAL_DTS_MODULE_ENABLED
 #include "stm32h7xx_hal_dts.h"
#endif /* HAL_DTS_MODULE_ENABLED */

#ifdef HAL_ETH_MODULE_ENABLED
  #include "stm32h7xx_hal_eth.h"
#endif /* HAL_ETH_MODULE_ENABLED */

#ifdef HAL_ETH_LEGACY_MODULE_ENABLED
  #include "stm32h7xx_hal_eth_legacy.h"
#endif /* HAL_ETH_LEGACY_MODULE_ENABLED */

#ifdef HAL_EXTI_MODULE_ENABLED
  #include "stm32h7xx_hal_exti.h"
#endif /* HAL_EXTI_MODULE_ENABLED */

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32h7xx_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */

#ifdef HAL_ADC_MODULE_ENABLED
  #include "stm32h7xx_hal_adc.h"
#endif /* HAL_ADC_MODULE_ENABLED */

#ifdef HAL_FDCAN_MODULE_ENABLED
  #include "stm32h7xx_hal_fdcan.h"
#endif /* HAL_FDCAN_MODULE_ENABLED */

#ifdef HAL_CEC_MODULE_ENABLED
  #include "stm32h7xx_hal_cec.h"
#endif /* HAL_CEC_MODULE_ENABLED */

#ifdef HAL_COMP_MODULE_ENABLED
  #include "stm32h7xx_hal_comp.h"
#endif /* HAL_COMP_MODULE_ENABLED */

#ifdef HAL_CORDIC_MODULE_ENABLED
  #include "stm32h7xx_hal_cordic.h"
#endif /* HAL_CORDIC_MODULE_ENABLED */

#ifdef HAL_CRC_MODULE_ENABLED
  #include "stm32h7xx_hal_crc.h"
#endif /* HAL_CRC_MODULE_ENABLED */

#ifdef HAL_CRYP_MODULE_ENABLED
  #include "stm32h7xx_hal_cryp.h"
#endif /* HAL_CRYP_MODULE_ENABLED */

#ifdef HAL_DAC_MODULE_ENABLED
  #include "stm32h7xx_hal_dac.h"
#endif /* HAL_DAC_MODULE_ENABLED */

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32h7xx_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */

#ifdef HAL_GFXMMU_MODULE_ENABLED
  #include "stm32h7xx_hal_gfxmmu.h"
#endif /* HAL_GFXMMU_MODULE_ENABLED */

#ifdef HAL_FMAC_MODULE_ENABLED
  #include "stm32h7xx_hal_fmac.h"
#endif /* HAL_FMAC_MODULE_ENABLED */

#ifdef HAL_HRTIM_MODULE_ENABLED
  #include "stm32h7xx_hal_hrtim.h"
#endif /* HAL_HRTIM_MODULE_ENABLED */

#ifdef HAL_HSEM_MODULE_ENABLED
  #include "stm32h7xx_hal_hsem.h"
#endif /* HAL_HSEM_MODULE_ENABLED */

#ifdef HAL_SRAM_MODULE_ENABLED
  #include "stm32h7xx_hal_sram.h"
#endif /* HAL_SRAM_MODULE_ENABLED */

#ifdef HAL_NOR_MODULE_ENABLED
  #include "stm32h7xx_hal_nor.h"
#endif /* HAL_NOR_MODULE_ENABLED */

#ifdef HAL_NAND_MODULE_ENABLED
  #include "stm32h7xx_hal_nand.h"
#endif /* HAL_NAND_MODULE_ENABLED */

#ifdef HAL_I2C_MODULE_ENABLED
 #include "stm32h7xx_hal_i2c.h"
#endif /* HAL_I2C_MODULE_ENABLED */

#ifdef HAL_I2S_MODULE_ENABLED
 #include "stm32h7xx_hal_i2s.h"
#endif /* HAL_I2S_MODULE_ENABLED */

#ifdef HAL_IWDG_MODULE_ENABLED
 #include "stm32h7xx_hal_iwdg.h"
#endif /* HAL_IWDG_MODULE_ENABLED */

#ifdef HAL_JPEG_MODULE_ENABLED
 #include "stm32h7xx_hal_jpeg.h"
#endif /* HAL_JPEG_MODULE_ENABLED */

#ifdef HAL_MDIOS_MODULE_ENABLED
 #include "stm32h7xx_hal_mdios.h"
#endif /* HAL_MDIOS_MODULE_ENABLED */

#ifdef HAL_MMC_MODULE_ENABLED
 #include "stm32h7xx_hal_mmc.h"
#endif /* HAL_MMC_MODULE_ENABLED */

#ifdef HAL_LPTIM_MODULE_ENABLED
#include "stm32h7xx_hal_lptim.h"
#endif /* HAL_LPTIM_MODULE_ENABLED */

#ifdef HAL_LTDC_MODULE_ENABLED
#include "stm32h7xx_hal_ltdc.h"
#endif /* HAL_LTDC_MODULE_ENABLED */

#ifdef HAL_OPAMP_MODULE_ENABLED
#include "stm32h7xx_hal_opamp.h"
#endif /* HAL_OPAMP_MODULE_ENABLED */

#ifdef HAL_OSPI_MODULE_ENABLED
 #include "stm32h7xx_hal_ospi.h"
#endif /* HAL_OSPI_MODULE_ENABLED */

#ifdef HAL_OTFDEC_MODULE_ENABLED
#include "stm32h7xx_hal_otfdec.h"
#endif /* HAL_OTFDEC_MODULE_ENABLED */

#ifdef HAL_PSSI_MODULE_ENABLED
 #include "stm32h7xx_hal_pssi.h"
#endif /* HAL_PSSI_MODULE_ENABLED */

#ifdef HAL_PWR_MODULE_ENABLED
 #include "stm32h7xx_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */

#ifdef HAL_QSPI_MODULE_ENABLED
 #include "stm32h7xx_hal_qspi.h"
#endif /* HAL_QSPI_MODULE_ENABLED */

#ifdef HAL_RAMECC_MODULE_ENABLED
 #include "stm32h7xx_hal_ramecc.h"
#endif /* HAL_RAMECC_MODULE_ENABLED */

#ifdef HAL_RNG_MODULE_ENABLED
 #include "stm32h7xx_hal_rng.h"
#endif /* HAL_RNG_MODULE_ENABLED */

#ifdef HAL_RTC_MODULE_ENABLED
 #include "stm32h7xx_hal_rtc.h"
#endif /* HAL_RTC_MODULE_ENABLED */

#ifdef HAL_SAI_MODULE_ENABLED
 #include "stm32h7xx_hal_sai.h"
#endif /* HAL_SAI_MODULE_ENABLED */

#ifdef HAL_SD_MODULE_ENABLED
 #include "stm32h7xx_hal_sd.h"
#endif /* HAL_SD_MODULE_ENABLED */

#ifdef HAL_SDRAM_MODULE_ENABLED
 #include "stm32h7xx_hal_sdram.h"
#endif /* HAL_SDRAM_MODULE_ENABLED */

#ifdef HAL_SPI_MODULE_ENABLED
 #include "stm32h7xx_hal_spi.h"
#endif /* HAL_SPI_MODULE_ENABLED */

#ifdef HAL_SPDIFRX_MODULE_ENABLED
 #include "stm32h7xx_hal_spdifrx.h"
#endif /* HAL_SPDIFRX_MODULE_ENABLED */

#ifdef HAL_SWPMI_MODULE_ENABLED
 #include "stm32h7xx_hal_swpmi.h"
#endif /* HAL_SWPMI_MODULE_ENABLED */

#ifdef HAL_TIM_MODULE_ENABLED
 #include "stm32h7xx_hal_tim.h"
#endif /* HAL_TIM_MODULE_ENABLED */

#ifdef HAL_UART_MODULE_ENABLED
 #include "stm32h7xx_hal_uart.h"
#endif /* HAL_UART_MODULE_ENABLED */

#ifdef HAL_USART_MODULE_ENABLED
 #include "stm32h7xx_hal_usart.h"
#endif /* HAL_USART_MODULE_ENABLED */

#ifdef HAL_IRDA_MODULE_ENABLED
 #include "stm32h7xx_hal_irda.h"
#endif /* HAL_IRDA_MODULE_ENABLED */

#ifdef HAL_SMARTCARD_MODULE_ENABLED
 #include "stm32h7xx_hal_smartcard.h"
#endif /* HAL_SMARTCARD_MODULE_ENABLED */

#ifdef HAL_SMBUS_MODULE_ENABLED
 #include "stm32h7xx_hal_smbus.h"
#endif /* HAL_SMBUS_MODULE_ENABLED */

#ifdef HAL_WWDG_MODULE_ENABLED
 #include "stm32h7xx_hal_wwdg.h"
#endif /* HAL_WWDG_MODULE_ENABLED */

#ifdef HAL_PCD_MODULE_ENABLED
 #include "stm32h7xx_hal_pcd.h"
#endif /* HAL_PCD_MODULE_ENABLED */

#ifdef HAL_HCD_MODULE_ENABLED
 #include "stm32h7xx_hal_hcd.h"
#endif /* HAL_HCD_MODULE_ENABLED */

/* 导出宏 ----------------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  assert_param宏用于函数参数检查。
  * @param  expr: 如果expr为false，它调用assert_failed函数
  *         报告源文件名和失败调用的源行号。
  *         如果expr为true，不返回任何值。
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* 导出函数 ----------------------------------------------------------------------- */
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* STM32H7xx_HAL_CONF_H */
