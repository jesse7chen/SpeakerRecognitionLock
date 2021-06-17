/*
 * InitState.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - State initialization
 */
//

#include "fsm_evt_queue.h"
#include "InitState.h"
#include "TestState.h"

/* Private functions ---------------------------------------------------------*/

bool InitState(Fsm* me, FSM_EVT_T const *event) {
    FsmTran_(me, &TestState_Standby);
    return true;
}
