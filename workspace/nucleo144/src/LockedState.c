/*
 * LockedState.c
 *
 *      Author: jessechen
 *      Brief:
 *          - State that manages behavior while device is locked
 */
//

#include "ErrorState.h"
#include "ESP8266.h"
#include "events.h"
#include "fsm_evt_queue.h"
#include "microphone.h"
#include "server.h"
#include "state_machine.h"
#include "stm32l4xx_nucleo_144.h"
#include "LockedState.h"
#include "UnlockedState.h"

/* Private functions ---------------------------------------------------------*/
static bool LockedState_Standby(Fsm* me, FSM_EVT_T const *event);
static bool LockedState_Record(Fsm* me, FSM_EVT_T const *event);
static bool LockedState_Processing(Fsm* me, FSM_EVT_T const *event);

bool LockedState_Entry(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &LockedState_Standby;

    BSP_LED_On(LED_LOCKED);
    FsmTran_(me, nextState);

    return FsmDispatch(me, event);
}

static bool LockedState_Standby(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &LockedState_Standby;
    bool eventHandled = true;

    switch(event->id)
    {
        case FSM_EVT_USER_BUTTON_PRESS:
            if (Mic_StartRecord() == HAL_OK) {
                nextState = &LockedState_Record;
            }
            else {
                nextState = &ErrorState;
            }
            break;

        default:
            eventHandled = false;
            break;
    }

    FsmTran_(me, nextState);
    return eventHandled;
}

static bool LockedState_Record(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &LockedState_Record;
    bool eventHandled = true;

    uint32_t size = 0;
    AUDIO_SIZE_T* audioData;

    switch(event->id)
    {
        // Intentional fall through
        case FSM_EVT_AUDIO_RECORD_DONE:
        case FSM_EVT_USER_BUTTON_PRESS:
            if (Mic_StopRecord() == HAL_OK) {
                audioData = Mic_GetAudioData(&size);
                if(Server_StartAudioTx(SERVER_VALIDATE_CMD, size, (uint8_t*)audioData) == true) {
                    nextState = &LockedState_Processing;
                }
                else {
                    nextState = &ErrorState;
                }
            }
            else {
                nextState = &ErrorState;
            }
            break;

        default:
            eventHandled = false;
            break;
    }

    FsmTran_(me, nextState);
    return eventHandled;
}

static bool LockedState_Processing(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &LockedState_Processing;
    bool eventHandled = true;

    switch(event->id)
    {
        case FSM_EVT_VALIDATE_TRUE:
            BSP_LED_Off(LED_LOCKED);
            nextState = &UnlockedState_Entry;
            break;
        case FSM_EVT_VALIDATE_FALSE:
            nextState = &LockedState_Standby;
            break;
        default:
            eventHandled = false;
            break;
    }

    FsmTran_(me, nextState);
    return eventHandled;
}
