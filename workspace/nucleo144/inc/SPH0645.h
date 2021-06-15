#ifndef __SPH0645_H
#define __SPH0645_H

#include "common.h"
#include "stm32l4xx.h"

bool SPH0645_Init(void);
HAL_StatusTypeDef SPH0645_StartRecord(uint8_t *buff, uint16_t size);
HAL_StatusTypeDef SPH0645_StopRecord(void);

void SPH0645_SAIRxCpltCallback(SAI_HandleTypeDef *hsai);
DMA_HandleTypeDef* SPH0645_GetDMAHandle(void);
SAI_HandleTypeDef* SPH0645_GetSAIHandle(void);

#endif /* __SPH0645_H */
