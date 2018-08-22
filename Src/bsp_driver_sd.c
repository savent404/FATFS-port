#include "bsp_driver_sd.h"

extern SD_HandleTypeDef hsd;

uint8_t BSP_SD_Init(void)
{
    uint8_t sd_state = MSD_OK;
    /* Check if the SD card is plugged in the slot */
    if (BSP_SD_IsDetected() != SD_PRESENT) {
        return MSD_ERROR;
    }
    /* HAL SD initialization */
    sd_state = HAL_SD_Init(&hsd);

    return sd_state;
}

uint8_t BSP_SD_ITConfig(void)
{
    return (uint8_t)0;
}

void BSP_SD_DetectIT(void)
{
}

__weak void BSP_SD_DetectCallback(void)
{
}

uint8_t BSP_SD_ReadBlocks(uint32_t* pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_ReadBlocks(&hsd, (uint8_t*)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

uint8_t BSP_SD_WriteBlocks(uint32_t* pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_WriteBlocks(&hsd, (uint8_t*)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

uint8_t BSP_SD_ReadBlocks_DMA(uint32_t* pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
    uint8_t sd_state = MSD_OK;

    /* Read block(s) in DMA transfer mode */
    if (HAL_SD_ReadBlocks_DMA(&hsd, (uint8_t*)pData, ReadAddr, NumOfBlocks) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

uint8_t BSP_SD_WriteBlocks_DMA(uint32_t* pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{
    uint8_t sd_state = MSD_OK;

    /* Write block(s) in DMA transfer mode */
    if (HAL_SD_WriteBlocks_DMA(&hsd, (uint8_t*)pData, WriteAddr, NumOfBlocks) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr)
{
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_Erase(&hsd, StartAddr, EndAddr) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

void BSP_SD_IRQHandler(void)
{
    HAL_SD_IRQHandler(&hsd);
}

void BSP_SD_DMA_Tx_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hsd.hdmatx);
}

void BSP_SD_DMA_Rx_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hsd.hdmarx);
}

uint8_t BSP_SD_GetCardState(void)
{
    return ((HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef* CardInfo)
{
    /* Get SD card Information */
    HAL_SD_GetCardInfo(&hsd, CardInfo);
}

uint8_t BSP_SD_IsDetected(void)
{
    __IO uint8_t status = SD_PRESENT;

    return status;
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef* hsd)
{
    BSP_SD_ReadCpltCallback();
}
void HAL_SD_TxCpltCallback(SD_HandleTypeDef* hsd)
{
    BSP_SD_WriteCpltCallback();
}
void HAL_SD_ErrorCallback(SD_HandleTypeDef* hsd)
{
    //     error("SDIO error:%d\r\n", hsd->ErrorCode);
}
