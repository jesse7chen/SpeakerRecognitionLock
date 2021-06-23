/*
 * InitState.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - State initialization
 */
//

#include "error.h"
#include "FlashDriver.h"
#include "fsm_evt_queue.h"
#include "InitState.h"
#include "LockedState.h"
#include "stm32l4xx_nucleo_144.h"
#include "TestState.h"
#include "TrainState.h"

/* Private functions ---------------------------------------------------------*/

bool InitState(Fsm* me, FSM_EVT_T const *event) {
    State nextState = 0;

    // Check if device is trained or not already
    if(FlashDriver_GetTrainedFlag() == false) {
        nextState = &TrainState_Entry;
    }
    else {
        nextState = &LockedState_Entry;
    }

    FsmTran_(me, nextState);
    return true;
}
