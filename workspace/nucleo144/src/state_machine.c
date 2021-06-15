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
#include "ESP8266.h"
#include "events.h"
#include "microphone.h"
#include "server.h"
#include "state_machine.h"
#include "stm32l4xx_nucleo_144.h"


/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

HAL_StatusTypeDef smInit(sm_state_t* state){
  /* Might want to do something with this eventually */
  return HAL_OK;
}

void smRun(sm_state_t* state){
  uint32_t size = 0;
  AUDIO_SIZE_T* audioData;

  switch(*state){

    /* Reset state */
    case stateReset:
      *state = stateStandby;
      break;

    /* Standby state */
    case stateStandby:
      if (Event_GetAndClear(EVENT_USER_BUTTON_PRESS)){
        if (Mic_StartRecord() == HAL_OK){
          *state = stateRec;
        }
        else{
          *state = stateErr;
        }
      }
      break;

    /* Recording state */
    case stateRec:
        if (Event_GetAndClear(EVENT_USER_BUTTON_PRESS)){
            if (Mic_StopRecord() == HAL_OK){
                audioData = Mic_GetAudioData(&size);
                // audioData = Mic_GetTestData(&size);
                // Start data transfer
                if(Server_StartAudioTx(size, (uint8_t*)audioData) == false){
                    *state = stateErr;
                }
                *state = stateProc;
            }
            else{
                *state = stateErr;
            }
        }
        if(Event_GetAndClear(EVENT_AUDIO_RECORD_DONE)){
            BSP_LED_Off(LED2);
        }

        break;

    /* Processing state */
    case stateProc:
        // Check if processing done
        if(Event_GetAndClear(EVENT_AUDIO_TRANSFER_DONE)){
            // Consume any events that may have been generated but not used
            Event_Clear(EVENT_USER_BUTTON_PRESS);
            *state = stateStandby;
        }
        break;

    /* Calibration state */
    case stateCal:

      if (Mic_Calibrate() == HAL_OK){
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
