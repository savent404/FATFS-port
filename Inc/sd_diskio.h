#ifndef __SD_DISKIO_H
#define __SD_DISKIO_H

#include "bsp_driver_sd.h"
#include "ff_gen_drv.h"

extern const Diskio_drvTypeDef  SD_Driver;
void BSP_SD_WriteCpltCallback(void);
void BSP_SD_ReadCpltCallback(void);
#endif /* __SD_DISKIO_H */
