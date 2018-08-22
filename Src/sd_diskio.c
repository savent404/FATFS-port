#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "cmsis.h"
#include "cmsis_os2.h"
#include "mbed_rtos_storage.h"
#include "stdlib.h"

#define QUEUE_SIZE         (uint32_t) 10
#define READ_CPLT_MSG      (uint32_t) 1
#define WRITE_CPLT_MSG     (uint32_t) 2

#define SD_TIMEOUT 30 * 1000

#define SD_DEFAULT_BLOCK_SIZE 512

static volatile DSTATUS Stat = STA_NOINIT;

static mbed_rtos_storage_msg_queue_t message_buf;
static osMessageQueueAttr_t message_attr = {0};
static osMessageQueueId_t SDQueueID;

static DSTATUS SD_CheckStatus(BYTE lun);
DSTATUS SD_initialize (BYTE);
DSTATUS SD_status (BYTE);
DRESULT SD_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
  DRESULT SD_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT SD_ioctl (BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */

const Diskio_drvTypeDef  SD_Driver =
{
  SD_initialize,
  SD_status,
  SD_read,
#if  _USE_WRITE == 1
  SD_write,
#endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
  SD_ioctl,
#endif /* _USE_IOCTL == 1 */
};

static DSTATUS SD_CheckStatus(BYTE lun)
{
  Stat = STA_NOINIT;

  if(BSP_SD_GetCardState() == MSD_OK)
  {
    Stat &= ~STA_NOINIT;
  }

  return Stat;
}

DSTATUS SD_initialize(BYTE lun)
{
  Stat = STA_NOINIT;
  /*
   * check that the kernel has been started before continuing
   * as the osMessage API will fail otherwise
   */
  if(osKernelGetState() == osKernelRunning)
  {
#if !defined(DISABLE_SD_INIT)

    if(BSP_SD_Init() == MSD_OK)
    {
      Stat = SD_CheckStatus(lun);
    }

#else
    Stat = SD_CheckStatus(lun);
#endif

    /*
     * if the SD is correctly initialized, create the operation queue
     */

    if (Stat != STA_NOINIT)
    {
      message_attr.name = "SDIO_Message";
      message_attr.cb_mem = &message_buf;
      message_attr.cb_size = sizeof(message_buf);
      message_attr.mq_mem = malloc(sizeof(int)*QUEUE_SIZE+128);
      message_attr.mq_size = sizeof(int)*QUEUE_SIZE + 128;
      SDQueueID = osMessageQueueNew(QUEUE_SIZE, sizeof(int), &message_attr);
    }
  }
  return Stat;
}

DSTATUS SD_status(BYTE lun)
{
  return SD_CheckStatus(lun);
}

DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR;
  osStatus_t status;
  int value;
  uint32_t timer;
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
  uint32_t alignedAddr;
#endif

  if(BSP_SD_ReadBlocks_DMA((uint32_t*)buff,
                           (uint32_t) (sector),
                           count) == MSD_OK)
  {
    /* wait for a message from the queue or a timeout */
    status = osMessageQueueGet(SDQueueID, &value, NULL, SD_TIMEOUT);

    if (status == osOK)
    {
      if (value == READ_CPLT_MSG)
      {
        timer = osKernelGetTickCount() + SD_TIMEOUT;
        /* block until SDIO IP is ready or a timeout occur */
        while(timer > osKernelGetTickCount())
        {
          if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
          {
            res = RES_OK;
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
            /*
               the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
               adjust the address and the D-Cache size to invalidate accordingly.
             */
            alignedAddr = (uint32_t)buff & ~0x1F;
            SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr, count*BLOCKSIZE + ((uint32_t)buff - alignedAddr));
#endif
            break;
          }
        }
      }
    }
  }

  return res;
}


#if _USE_WRITE == 1
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR;
  osStatus_t status;
  int value;
  uint32_t timer;
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
  uint32_t alignedAddr;
  /*
   the SCB_CleanDCache_by_Addr() requires a 32-Byte aligned address
   adjust the address and the D-Cache size to clean accordingly.
   */
  alignedAddr = (uint32_t)buff &  ~0x1F;
  SCB_CleanDCache_by_Addr((uint32_t*)alignedAddr, count*BLOCKSIZE + ((uint32_t)buff - alignedAddr));
#endif

  if(BSP_SD_WriteBlocks_DMA((uint32_t*)buff,
                            (uint32_t) (sector),
                            count) == MSD_OK)
  {
    /* Get the message from the queue */
    status = osMessageQueueGet(SDQueueID, &value, NULL, SD_TIMEOUT);

    if (status == osOK)
    {
      if (value == WRITE_CPLT_MSG)
      {
        timer = osKernelGetTickCount() + SD_TIMEOUT;
        /* block until SDIO IP is ready or a timeout occur */
        while(timer > osKernelGetTickCount())
        {
          if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
          {
            res = RES_OK;
            break;
          }
        }
      }
    }
  }

  return res;
}
#endif /* _USE_WRITE == 1 */

#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;
  BSP_SD_CardInfo CardInfo;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    BSP_SD_GetCardInfo(&CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockNbr;
    res = RES_OK;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    BSP_SD_GetCardInfo(&CardInfo);
    *(WORD*)buff = CardInfo.LogBlockSize;
    res = RES_OK;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    BSP_SD_GetCardInfo(&CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
    res = RES_OK;
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */

void BSP_SD_WriteCpltCallback(void)
{
  /*
   * No need to add an "osKernelRunning()" check here, as the SD_initialize()
   * is always called before any SD_Read()/SD_Write() call
   */
  int value = WRITE_CPLT_MSG;
  osMessageQueuePut(SDQueueID, &value, osPriorityNormal, 0);
}

void BSP_SD_ReadCpltCallback(void)
{
  /*
   * No need to add an "osKernelRunning()" check here, as the SD_initialize()
   * is always called before any SD_Read()/SD_Write() call
   */
  int value = READ_CPLT_MSG;
  osMessageQueuePut(SDQueueID, &value, osPriorityNormal, 0);
}

