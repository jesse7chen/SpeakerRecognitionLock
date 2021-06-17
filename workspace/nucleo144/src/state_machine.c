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
#include "ErrorState.h"
#include "ESP8266.h"
#include "events.h"
#include "fsm_evt_queue.h"
#include "InitState.h"
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
static Fsm m_StateMachine;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

HAL_StatusTypeDef StateMachine_Init(void){
  FsmCtor_(&m_StateMachine, &InitState);
  FsmInit(&m_StateMachine, 0);
  return HAL_OK;
}

void StateMachine_Run(void) {
    // Check if there are events to service
    while(FSM_EVT_QUEUE_IsEmpty() == false) {
        // Grab data from queue
        FSM_EVT_T event;

        if(FSM_EVT_QUEUE_Pop(&event) == true) {
            // Check if event is not handled - my attempt at implementing
            // "Programming by difference"
            if(FsmDispatch(&m_StateMachine, &event) == false) {
                switch(event.id) {
                    case FSM_EVT_ERROR:
                        FsmTran_(&m_StateMachine, &ErrorState);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
