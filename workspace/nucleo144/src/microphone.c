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
#include "fsm_evt_queue.h"
#include "microphone.h"
#include "SPH0645.h"
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
#ifdef USE_PING_PONG_BUFFER

static AUDIO_SIZE_T m_DmaBuff1[DMA_BUFFER_LEN];
static AUDIO_SIZE_T m_DmaBuff2[DMA_BUFFER_LEN];
static AUDIO_SIZE_T* m_CurrDma = m_DmaBuff1;
static uint16_t m_AudioBuffTransferCount = 0;

static void Mic_AudioBuffDmaXferComplete(DMA_HandleTypeDef* hdma);

#endif

static DMA_HandleTypeDef m_AudioBuffDmaHandle;
static AUDIO_SIZE_T m_AudioBuff[AUDIO_DATA_BUFFER_LEN];

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
bool Mic_Init(void){
    bool success = true;

    success = SPH0645_Init();

#ifdef USE_PING_PONG_BUFFER
    if(success == true){
        __HAL_RCC_DMAMUX1_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();

        // Configure DMA to main buffer
        m_AudioBuffDmaHandle.Instance = AUDIO_BUFF_DMA_INSTANCE;
        m_AudioBuffDmaHandle.Init.Request = DMA_REQUEST_MEM2MEM;
        m_AudioBuffDmaHandle.Init.Direction = DMA_MEMORY_TO_MEMORY;
        m_AudioBuffDmaHandle.Init.PeriphInc = DMA_PINC_ENABLE;
        m_AudioBuffDmaHandle.Init.MemInc = DMA_MINC_ENABLE;
        // SPH0645 records 24 bit data, need to store in 32 bit word
        m_AudioBuffDmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        m_AudioBuffDmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
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
#endif

    return success;
}

HAL_StatusTypeDef Mic_Calibrate(void){
    return ADC_CalibrateVRefInt();
}

HAL_StatusTypeDef Mic_StartRecord(void){
    // Reset buffers and metadata
    memset((uint8_t*)m_AudioBuff, 0, sizeof(m_AudioBuff));

    // Start recording data
    BSP_LED_On(LED2);

#ifdef USE_PING_PONG_BUFFER
    m_AudioBuffTransferCount = 0;
    memset((uint8_t*)m_DmaBuff1, 0, sizeof(DMA_BUFFER_LEN));
    memset((uint8_t*)m_DmaBuff2, 0, sizeof(DMA_BUFFER_LEN));
    return SPH0645_StartRecord((uint8_t*)m_CurrDma, DMA_BUFFER_LEN);
#else
    return SPH0645_StartRecord((uint8_t*)m_AudioBuff, AUDIO_DATA_BUFFER_LEN);
#endif
}

HAL_StatusTypeDef Mic_StopRecord(void){
    /* Clear buffer beforehand maybe */
    BSP_LED_Off(LED2);
    return SPH0645_StopRecord();
}

AUDIO_SIZE_T* Mic_GetAudioData(uint32_t* size) {

    uint16_t length = 0;

    length = AUDIO_DATA_BUFFER_LEN - SPH0645_GetNumUnitsLeft();
    *size = length * sizeof(AUDIO_SIZE_T);

    // Sanity check size
    if(*size > sizeof(m_AudioBuff)) {
        *size = sizeof(m_AudioBuff);
    }

    return m_AudioBuff;
}

void Mic_RxCompleteCallback(void){

#ifdef USE_PING_PONG_BUFFER

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

        // TODO: See if I2S DMA peripheral also needs to be stopped before starting
        // like ADC does - so far it doesn't seem like it but leaving this in here
        // just in case
        if (SPH0645_StopRecord() != HAL_OK){
            Error_Handler();
        }
        if(SPH0645_StartRecord((uint8_t*)m_CurrDma, DMA_BUFFER_LEN) != HAL_OK){
            Error_Handler();
        }
    }

#else
    FSM_EVT_T event = {
        .id = FSM_EVT_AUDIO_RECORD_DONE,
        .size = 0,
        .data = NULL
    };
    FSM_EVT_QUEUE_Push(event);
#endif
}

AUDIO_SIZE_T* Mic_GetTestData(uint32_t* size){
    for(uint32_t i = 0; i < AUDIO_DATA_BUFFER_LEN; i++){
        m_AudioBuff[i] = i;
    }

    *size = sizeof(m_AudioBuff);
    return m_AudioBuff;
}

DMA_HandleTypeDef* Mic_GetAudioBuffDmaHandle(void){
    return &m_AudioBuffDmaHandle;
}

#ifdef USE_PING_PONG_BUFFER

static void Mic_AudioBuffDmaXferComplete(DMA_HandleTypeDef* hdma){
    // Clear audio transfer event
    if(Event_GetAndClear(EVENT_AUDIO_DMA_IN_PROGRESS) == false){
        // No event set for some reason when there should be
        Error_Handler();
    }
    // Check if this is the final transfer
    if(m_AudioBuffTransferCount >= AUDIO_DATA_BUFFER_EXPECTED_TRANSFERS){
        FSM_EVT_T event = {
            .id = FSM_EVT_AUDIO_RECORD_DONE,
            .size = 0,
            .data = NULL
        };
        FSM_EVT_QUEUE_Push(event);
    }
}
#endif
