#include "cmsis.h"
#include "sd_diskio.h"

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

void SDIO_IO_Init(void)
{
    hsd.Instance = SDIO;
    hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv = 50;

    __HAL_RCC_DMA2_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
    NVIC_SetVector(DMA2_Stream3_IRQn, (int)BSP_SD_DMA_Rx_IRQHandler);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
    NVIC_SetVector(DMA2_Stream6_IRQn, (int)BSP_SD_DMA_Tx_IRQHandler);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
    HAL_NVIC_SetPriority(SDIO_IRQn, 5, 0);
    NVIC_SetVector(SDIO_IRQn, (int)BSP_SD_IRQHandler);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);
}

void HAL_SD_MspInit(SD_HandleTypeDef* hsd)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if (hsd->Instance == SDIO) {
        __HAL_RCC_SDIO_CLK_ENABLE();
        /**GPIO RCC enable */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**SDIO GPIO Configuration    
        PA6     ------> SDIO_CMD
        PB15     ------> SDIO_CK
        PC8     ------> SDIO_D0 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        hdma_sdio_rx.Instance = DMA2_Stream3;
        hdma_sdio_rx.Init.Channel = DMA_CHANNEL_4;
        hdma_sdio_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_sdio_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_sdio_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_sdio_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_sdio_rx.Init.Mode = DMA_PFCTRL;
        hdma_sdio_rx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_sdio_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
        hdma_sdio_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        hdma_sdio_rx.Init.MemBurst = DMA_MBURST_INC4;
        hdma_sdio_rx.Init.PeriphBurst = DMA_PBURST_INC4;
        HAL_DMA_Init(&hdma_sdio_rx);
        __HAL_LINKDMA(hsd, hdmarx, hdma_sdio_rx);

        hdma_sdio_tx.Instance = DMA2_Stream6;
        hdma_sdio_tx.Init.Channel = DMA_CHANNEL_4;
        hdma_sdio_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_sdio_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_sdio_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_sdio_tx.Init.Mode = DMA_PFCTRL;
        hdma_sdio_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_sdio_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
        hdma_sdio_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        hdma_sdio_tx.Init.MemBurst = DMA_MBURST_SINGLE;
        hdma_sdio_tx.Init.PeriphBurst = DMA_PBURST_INC4;
        HAL_DMA_Init(&hdma_sdio_tx);

        __HAL_LINKDMA(hsd, hdmatx, hdma_sdio_tx);
    }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* hsd)
{
    if (hsd->Instance == SDIO) {
        __HAL_RCC_SDIO_CLK_DISABLE();
        /**SDIO GPIO Configuration    
        PA6     ------> SDIO_CMD
        PB15     ------> SDIO_CK
        PC8     ------> SDIO_D0 
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8);
        HAL_DMA_DeInit(hsd->hdmarx);
        HAL_DMA_DeInit(hsd->hdmatx);
    }
}
