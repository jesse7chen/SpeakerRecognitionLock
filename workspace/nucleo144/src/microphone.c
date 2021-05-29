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
#include "microphone.h"
#include "stm32l4xx_nucleo_144.h"



/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define AUDIO_DATA_BUFFER_SIZE ((uint32_t)  32)   /* Size of array m_AudioBuff[] */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef adc_h;
/* Contains ADC data from microphone */
static uint16_t m_AudioBuff[AUDIO_DATA_BUFFER_SIZE];


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


HAL_StatusTypeDef Mic_StartRecord(void){
    BSP_LED_On(LED2);
    return HAL_ADC_Start_DMA(&adc_h, (uint32_t*)m_AudioBuff, AUDIO_DATA_BUFFER_SIZE);

}

HAL_StatusTypeDef Mic_StopRecord(void){
    /* Clear buffer beforehand maybe */
    BSP_LED_Off(LED2);
    return HAL_ADC_Stop_DMA(&adc_h);
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
