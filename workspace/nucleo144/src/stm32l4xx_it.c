/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "adc.h"
#include "button.h"
#include "ESP8266.h"
#include "microphone.h"
#include "SPH0645.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32l4xx_it.h"
#include "stm32l4xx_nucleo_144.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern GPIO_InitTypeDef              test_pin;
static uint32_t m_ErrorCount = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
#ifdef USE_RTOS_SYSTICK
    osSystickHandler();
#endif
}

/******************************************************************************/
/*                 STM32L4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l4xx.s).                                               */
/******************************************************************************/

/**
* @brief  This function handles DMA1_Channel1_IRQHandler interrupt request.
* @param  None
* @retval None
*/



// DMA1 Channel 1 is Microphone ADC to Memory stream
void DMA1_Channel1_IRQHandler(void)
{
    // Currently this ADC IRQHandler doesn't seem to be necessary, but will
    // leave it in case it has functionality which is needed
    HAL_ADC_IRQHandler(ADC_GetHandle());
	HAL_DMA_IRQHandler(ADC_GetHandle()->DMA_Handle);
}


// DMA1 Channel 2 is SAI Rx interface to SPH0645 microphone
void DMA1_Channel2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(SPH0645_GetDMAHandle());
}

// SPH0645 is only user of SAI1 at the moment
void SAI1_IRQHandler(void)
{
    HAL_SAI_IRQHandler(SPH0645_GetSAIHandle());
}

// DMA2 Channel 1 is for ping pong buffers to main audio buffer
void DMA2_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(Mic_GetAudioBuffDmaHandle());
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if(hsai->Instance == SPH0645_SAI) {
        SPH0645_SAIRxCpltCallback(hsai);
    }
}

/**
  * @brief  This function handles external lines 10 to 15 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(USER_BUTTON_PIN);
}

/**
  * @brief  Handle EXTI interrupt request.
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == USER_BUTTON_PIN)
  {
    Button_DebounceCallback();
  }
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(Button_GetDebounceTmrHandle());
}

void SPI3_IRQHandler(void)
{
    HAL_SPI_IRQHandler(ESP8266_GetSpiHandle());
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
    if(hspi->Instance == ESP8266_SPI3){
        ESP8266_SpiTxCpltCallback(hspi);
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
    if(hspi->Instance == ESP8266_SPI3){
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
    if(hspi->Instance == ESP8266_SPI3){
    }
}

// Todo: Implement specific error handling for SPI errors
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
    if(hspi->Instance == ESP8266_SPI3){
        m_ErrorCount++;
    }
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi){
    if(hspi->Instance == ESP8266_SPI3){
        m_ErrorCount++;
    }
}
