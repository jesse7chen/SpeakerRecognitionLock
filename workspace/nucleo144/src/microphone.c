/*
 * microphone.c
 *
 *  Created on: March 6, 2020
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - microphone recording functionality
 */
//

/* Includes ------------------------------------------------------------------*/
#include "adc.h"
#include "error.h"
#include "events.h"
#include "microphone.h"
#include "stm32l4xx.h"
#include "stm32l4xx_nucleo_144.h"
#include <string.h>



/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint16_t m_DmaBuff1[DMA_BUFFER_LEN];
static uint16_t m_DmaBuff2[DMA_BUFFER_LEN];
static uint16_t m_AudioBuff[AUDIO_DATA_BUFFER_LEN];

static uint16_t* m_CurrDma = m_DmaBuff1;
static uint16_t m_AudioBuffTransferCount = 0;
static DMA_HandleTypeDef m_AudioBuffDmaHandle;

/* Private function prototypes -----------------------------------------------*/
static void Mic_AudioBuffDmaXferComplete(DMA_HandleTypeDef* hdma);

/* Private functions ---------------------------------------------------------*/
bool Mic_Init(void){
    bool success = true;

    success = (ADC_Init() == HAL_OK);

    if(success == true){
        success = (ADC_InitChannels() == HAL_OK);
    }

    if(success == true){
        success = (ADC_CalibrateVRefInt() == HAL_OK);
    }

    if(success == true){
        __HAL_RCC_DMA2_CLK_ENABLE();

        // Configure DMA to main buffer
        m_AudioBuffDmaHandle.Instance = AUDIO_BUFF_DMA_INSTANCE;
        m_AudioBuffDmaHandle.Init.Request = DMA_REQUEST_MEM2MEM;
        m_AudioBuffDmaHandle.Init.Direction = DMA_MEMORY_TO_MEMORY;
        m_AudioBuffDmaHandle.Init.PeriphInc = DMA_PINC_ENABLE;
        m_AudioBuffDmaHandle.Init.MemInc = DMA_MINC_ENABLE;
        // Highest resolution of ADC is 12 bits, so the data width of each transfer can be a halfword (16 bits)
        m_AudioBuffDmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        m_AudioBuffDmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        m_AudioBuffDmaHandle.Init.Mode = DMA_NORMAL;
        m_AudioBuffDmaHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
        // De-init before init
        HAL_DMA_DeInit(&m_AudioBuffDmaHandle);
        success = (HAL_DMA_Init(&m_AudioBuffDmaHandle) == HAL_OK);

        /* NVIC configuration DMA interrupt */
        HAL_NVIC_SetPriority(AUDIO_BUFF_DMA_IRQn, AUDIO_BUFF_DMA_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(AUDIO_BUFF_DMA_IRQn);

        // Register callback functions
        HAL_DMA_RegisterCallback(&m_AudioBuffDmaHandle, HAL_DMA_XFER_CPLT_CB_ID, Mic_AudioBuffDmaXferComplete);
    }

    return success;
}

HAL_StatusTypeDef Mic_Calibrate(void){
    return ADC_CalibrateVRefInt();
}

HAL_StatusTypeDef Mic_StartRecord(void){
    // Reset buffers and metadata
    m_AudioBuffTransferCount = 0;
    memset((uint8_t*)m_AudioBuff, 0, sizeof(m_AudioBuff));
    memset((uint8_t*)m_DmaBuff1, 0, sizeof(DMA_BUFFER_LEN));
    memset((uint8_t*)m_DmaBuff2, 0, sizeof(DMA_BUFFER_LEN));

    // Start recording data
    BSP_LED_On(LED2);
    return ADC_StartDma((uint32_t*)m_CurrDma, DMA_BUFFER_LEN);
}

HAL_StatusTypeDef Mic_StopRecord(void){
    /* Clear buffer beforehand maybe */
    BSP_LED_Off(LED2);
    return ADC_StopDma();
}

uint16_t* Mic_GetAudioData(uint32_t* size){
    *size = sizeof(m_AudioBuff);
    return m_AudioBuff;
}

void Mic_ConvCompleteCallback(ADC_HandleTypeDef* hadc){
    // Check if previous DMA transfer has already completed successfully
    if(Event_Get(EVENT_AUDIO_DMA_IN_PROGRESS) == true){
        // Transfer to m_AudioBuff has not yet finished, signal error
        Error_Handler();
    }

    // Increment number of transfers so far
    m_AudioBuffTransferCount++;

    // Calculate where in main buffer to transfer
    uint32_t buffIdx = DMA_BUFFER_LEN * (m_AudioBuffTransferCount - 1);
    if(buffIdx >= AUDIO_DATA_BUFFER_LEN){
        Error_Handler();
    }

    Event_Set(EVENT_AUDIO_DMA_IN_PROGRESS);

    // Start DMA transfer to main buffer
    if(HAL_DMA_Start_IT(&m_AudioBuffDmaHandle, (uint32_t)m_CurrDma,
                        (uint32_t)&m_AudioBuff[buffIdx], DMA_BUFFER_LEN) != HAL_OK){
        Error_Handler();
    }

    // Check if we need to kickoff another ADC read
    if(m_AudioBuffTransferCount < AUDIO_DATA_BUFFER_EXPECTED_TRANSFERS){
        // Switch ping pong buffer
        if(m_CurrDma == m_DmaBuff1){
            m_CurrDma = m_DmaBuff2;
        }
        else{
            m_CurrDma = m_DmaBuff1;
        }

        // It seems like the ADC-DMA needs to be explicitly stopped
        // after conversion to be restarted for some reason, even if it
        // is done converting
        // Todo: Modify HAL_DMA or HAL_ADC to be able to switch buffers at
        // end of transfer without overhead of calling stop function
        if (ADC_StopDma() != HAL_OK){
            Error_Handler();
        }
        if(ADC_StartDma((uint32_t*)m_CurrDma, DMA_BUFFER_LEN) != HAL_OK){
            Error_Handler();
        }
    }
}

uint16_t* Mic_GetTestData(uint32_t* size){
    for(uint32_t i = 0; i < AUDIO_DATA_BUFFER_LEN; i++){
        m_AudioBuff[i] = i;
    }

    *size = sizeof(m_AudioBuff);
    return m_AudioBuff;
}

DMA_HandleTypeDef* Mic_GetAudioBuffDmaHandle(void){
    return &m_AudioBuffDmaHandle;
}

static void Mic_AudioBuffDmaXferComplete(DMA_HandleTypeDef* hdma){
    // Clear audio transfer event
    if(Event_GetAndClear(EVENT_AUDIO_DMA_IN_PROGRESS) == false){
        // No event set for some reason when there should be
        Error_Handler();
    }
    // Check if this is the final transfer
    if(m_AudioBuffTransferCount >= AUDIO_DATA_BUFFER_EXPECTED_TRANSFERS){
        Event_Set(EVENT_AUDIO_RECORD_DONE);
    }
}
