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
#include "microphone.h"
#include "stm32l4xx.h"
#include "stm32l4xx_nucleo_144.h"



/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define AUDIO_DATA_BUFFER_SIZE ((uint32_t)  32)   /* Size of array m_AudioBuff[] */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Contains ADC data from microphone */
static uint16_t m_AudioBuff[AUDIO_DATA_BUFFER_SIZE];


/* Private function prototypes -----------------------------------------------*/
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

    return success;
}

HAL_StatusTypeDef Mic_Calibrate(void){
    return ADC_CalibrateVRefInt();
}

HAL_StatusTypeDef Mic_StartRecord(void){
    BSP_LED_On(LED2);
    return ADC_StartDma((uint32_t*)m_AudioBuff, AUDIO_DATA_BUFFER_SIZE);

}

HAL_StatusTypeDef Mic_StopRecord(void){
    /* Clear buffer beforehand maybe */
    BSP_LED_Off(LED2);
    return ADC_StopDma();
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
