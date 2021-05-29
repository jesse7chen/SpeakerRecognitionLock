#ifndef __MICROPHONE_H
#define __MICROPHONE_H

#include "stm32l4xx.h"
#include "common.h"

bool Mic_Init(void);
HAL_StatusTypeDef Mic_Calibrate(void);
HAL_StatusTypeDef Mic_StartRecord(void);
HAL_StatusTypeDef Mic_StopRecord(void);

#endif /* __MICROPHONE_H */
