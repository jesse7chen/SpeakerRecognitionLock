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
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32l4xx_it.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef             adc_h;
extern GPIO_InitTypeDef              test_pin;

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
void DMA1_Channel1_IRQHandler(void)
{
	/* TODO: See if the following line of code is necessary or not? Says to do in the documentation but example code doesn't have it */
	//HAL_ADC_IRQHandler(&adc_h);
    /* For debug purposes, toggle this pin */
    // HAL_GPIO_TogglePin(GPIOC, test_pin.Pin);
	HAL_DMA_IRQHandler(adc_h.DMA_Handle);
}
