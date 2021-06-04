#ifndef __MICROPHONE_H
#define __MICROPHONE_H

#include "stm32l4xx.h"
#include "common.h"

#define DMA_BUFFER_LEN ((uint32_t) 32768) // Max DMA transfer length is 0xFFFF

#define AUDIO_DATA_BUFFER_EXPECTED_TRANSFERS ( 6 )
#define AUDIO_DATA_BUFFER_LEN ((uint32_t)  DMA_BUFFER_LEN*AUDIO_DATA_BUFFER_EXPECTED_TRANSFERS)

bool Mic_Init(void);
HAL_StatusTypeDef Mic_Calibrate(void);
HAL_StatusTypeDef Mic_StartRecord(void);
HAL_StatusTypeDef Mic_StopRecord(void);
uint16_t* Mic_GetAudioData(uint32_t* size);
uint16_t* Mic_GetTestData(uint32_t* size);

void Mic_ConvCompleteCallback(ADC_HandleTypeDef* hadc);
DMA_HandleTypeDef* Mic_GetAudioBuffDmaHandle(void);

#endif /* __MICROPHONE_H */
