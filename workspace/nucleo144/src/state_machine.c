/*
 * state_machine.c
 *
 *  Created on: March 7, 2020
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - finite state machine functionality
 */
//

/* Includes ------------------------------------------------------------------*/
#include "state_machine.h"
#include "stm32l4xx_nucleo_144.h"
#include "adc.h"
#include "microphone.h"


/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile uint8_t ubUserButtonClickEvent = RESET;  /* Event detection: Set after User Button interrupt */

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

HAL_StatusTypeDef smInit(sm_state_t* state){
  /* Might want to do something with this eventually */
  return HAL_OK;
}

void smRun(sm_state_t* state){

  switch(*state){

    /* Reset state */
    case stateReset:
      *state = stateStandby;
      break;

    /* Standby state */
    case stateStandby:
      if (ubUserButtonClickEvent == SET){

        ubUserButtonClickEvent = RESET;
        if (startRecord() == HAL_OK){
          *state = stateRec;
        }
        else{
          *state = stateErr;
        }

      }
      break;

    /* Recording state */
    case stateRec:
    if (ubUserButtonClickEvent == SET){

      ubUserButtonClickEvent = RESET;
      if (stopRecord() == HAL_OK){
        *state = stateProc;
      }
      else{
        *state = stateErr;
      }

    }
      break;

    /* Processing state */
    case stateProc:
      /* Temporary for now */
      *state = stateStandby;
      break;

    /* Calibration state */
    case stateCal:

      if (calibrateVRefInt() == HAL_OK){
        *state = stateStandby;
      }
      else{
        /* If calibration doesn't succeed, go to error state */
        *state = stateErr;
      }
      break;

    /* Error state */
    case stateErr:
      /* Turn LED3 on - this should have initialized already */
      BSP_LED_On(LED3);
      while (1)
      {
      }
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == USER_BUTTON_PIN)
  {
    /* Set variable to report push button event to main program */
    ubUserButtonClickEvent = SET;
  }
}
