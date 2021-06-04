#ifndef __ADC_H
#define __ADC_H

#include "common.h"
#include "stm32l4xx.h"

typedef enum ADC_MODE_T{
    ADC_MODE_MIN = 0,
    ADC_MODE_CALIBRATE = ADC_MODE_MIN,
    ADC_MODE_RECORD,
    ADC_MODE_MAX
} ADC_MODE_T;

HAL_StatusTypeDef ADC_Init(void);
HAL_StatusTypeDef ADC_InitChannels(void);
HAL_StatusTypeDef ADC_CalibrateVRefInt(void);
HAL_StatusTypeDef ADC_StartDma(uint32_t *buff, uint32_t len);
HAL_StatusTypeDef ADC_StopDma(void);
ADC_HandleTypeDef* ADC_GetHandle(void);
DMA_HandleTypeDef* ADC_GetDmaHandle(void);

#endif /* __ADC_H */
