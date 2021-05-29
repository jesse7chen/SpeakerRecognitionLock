#ifndef __ADC_H
#define __ADC_H

#include "common.h"
#include "stm32l4xx.h"

#define CALIBRATE_MODE ((uint8_t) 0UL)
#define RECORD_MODE ((uint8_t) 1UL)

HAL_StatusTypeDef ADC_ChangeMode(uint8_t mode);
HAL_StatusTypeDef ADC_Init(void);
HAL_StatusTypeDef ADC_InitChannels(void);
HAL_StatusTypeDef ADC_CalibrateVRefInt(void);

#endif /* __ADC_H */
