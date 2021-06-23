#ifndef __FLASH_DRIVER_H
#define __FLASH_DRIVER_H

#include "common.h"

bool FlashDriver_GetTrainedFlag(void);
bool FlashDriver_SetTrainedFlag(void);
uint32_t FlashDriver_GetErrorCode(void);

#endif /* __FLASH_DRIVER_H */
