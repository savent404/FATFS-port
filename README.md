# FATFS Port Note

## How to use
main.cpp:
``` CPP
#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

extern "C" void SDIO_IO_Init();
char sd_path[4];

FATFS fatfs;

int main() {
    FATFS_LinkDriver(&SD_Driver, sd_path);
    SDIO_IO_Init();
    f_mount(&fatfs, sd_path, 1);
    while (1)
        ;
}
```
# How to test
``` bash
mbed test -n fatfs*
```

# Note
In sd_diskio.c, a constructor function called SD_LinkDriver is implement.
## Port Note
STM32 DMA should enable Tx&Rx both, then should enable SDIO IRQHandle. DMA Tx Mode should be like this:
``` C
hdma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
hdma_sdio_tx.Init.MemBurst = DMA_MBURST_SINGLE;
```