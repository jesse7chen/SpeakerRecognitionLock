#ifndef __ADC_H
#define __ADC_H

#include "common.h"
#include "stm32l4xx.h"

#define CALIBRATE_MODE ((uint8_t) 0UL)
#define RECORD_MODE ((uint8_t) 1UL)

HAL_StatusTypeDef ADC_Init(void);
HAL_StatusTypeDef ADC_InitChannels(void);
HAL_StatusTypeDef ADC_CalibrateVRefInt(void);
HAL_StatusTypeDef ADC_StartDma(uint32_t *buff, uint32_t len);
HAL_StatusTypeDef ADC_StopDma(void);
ADC_HandleTypeDef ADC_GetHandle(void);

#endif /* __ADC_H */
