#ifndef __ADC_H
#define __ADC_H

#include "common.h"
#include "stm32l4xx.h"

#define CALIBRATE_MODE ((uint8_t) 0UL)
#define RECORD_MODE ((uint8_t) 1UL)

HAL_StatusTypeDef changeADCMode(uint8_t mode);
HAL_StatusTypeDef adcInit(void);
HAL_StatusTypeDef adcChannelsInit(void);
HAL_StatusTypeDef calibrateVRefInt(void);

#endif /* __ADC_H */
