/*
 * TestState.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - The testing state (allows free form sending of audio data)
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
#include "TestState.h"

/* Private functions ---------------------------------------------------------*/
static bool RecordState(Fsm* me, FSM_EVT_T const *event);
static bool ProcessingState(Fsm* me, FSM_EVT_T const *event);

bool TestState_Standby(Fsm* me, FSM_EVT_T const *event) {
    State newState = &TestState_Standby;
    bool eventHandled = true;

    switch(event->id)
    {
        case FSM_EVT_USER_BUTTON_PRESS:
            if (Mic_StartRecord() == HAL_OK) {
                newState = &RecordState;
            }
            else {
                newState = &ErrorState;
            }
            break;

        default:
            eventHandled = false;
            break;
    }

    FsmTran_(me, newState);
    return eventHandled;
}

static bool RecordState(Fsm* me, FSM_EVT_T const *event) {
    State newState = &RecordState;
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
                if(Server_StartAudioTx(SERVER_TEST_CMD, size, (uint8_t*)audioData) == true) {
                    newState = &ProcessingState;
                }
                else {
                    newState = &ErrorState;
                }
            }
            else {
                newState = &ErrorState;
            }
            break;

        default:
            eventHandled = false;
            break;
    }

    FsmTran_(me, newState);
    return eventHandled;
}

static bool ProcessingState(Fsm* me, FSM_EVT_T const *event) {
    State newState = &ProcessingState;
    bool eventHandled = true;

    switch(event->id)
    {
        case FSM_EVT_AUDIO_TRANSFER_DONE:
            newState = &TestState_Standby;

        default:
            eventHandled = false;
            break;
    }

    FsmTran_(me, newState);
    return eventHandled;
}
