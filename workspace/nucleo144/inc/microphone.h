#ifndef __MICROPHONE_H
#define __MICROPHONE_H

#include "stm32l4xx.h"
#include <stdint.h>

HAL_StatusTypeDef startRecord(void);
HAL_StatusTypeDef stopRecord(void);

#endif /* __MICROPHONE_H */
