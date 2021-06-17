/*
 * ErrorState.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Error state handling
 */
//

#include "fsm_evt_queue.h"
#include "InitState.h"
#include "stm32l4xx_nucleo_144.h"

/* Private functions ---------------------------------------------------------*/

bool ErrorState(Fsm* me, FSM_EVT_T const *event) {
    BSP_LED_On(LED3);
    return true;
}
