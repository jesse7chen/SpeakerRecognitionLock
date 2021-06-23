/*
 * UnlockedState.c
 *
 *      Author: jessechen
 *      Brief:
 *          - State that manages behavior while device is unlocked
 */
//

#include "ErrorState.h"
#include "ESP8266.h"
#include "events.h"
#include "fsm_evt_queue.h"
#include "LockedState.h"
#include "microphone.h"
#include "server.h"
#include "state_machine.h"
#include "stm32l4xx_nucleo_144.h"
#include "TrainState.h"
#include "UnlockedState.h"

/* Private functions ---------------------------------------------------------*/
static bool UnlockedState_Standby(Fsm* me, FSM_EVT_T const *event);

bool UnlockedState_Entry(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &UnlockedState_Standby;

    BSP_LED_On(LED_UNLOCK);
    FsmTran_(me, nextState);

    return FsmDispatch(me, event);
}

bool UnlockedState_Standby(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &UnlockedState_Standby;
    bool eventHandled = true;

    switch(event->id)
    {
        case FSM_EVT_USER_BUTTON_PRESS:
            BSP_LED_Off(LED_UNLOCK);
            nextState = &TrainState_Entry;
            break;

        case FSM_EVT_LOCK_BUTTON_PRESS:
            BSP_LED_Off(LED_UNLOCK);
            nextState = &LockedState_Entry;
            break;

        default:
            eventHandled = false;
            break;
    }

    FsmTran_(me, nextState);
    return eventHandled;
}
