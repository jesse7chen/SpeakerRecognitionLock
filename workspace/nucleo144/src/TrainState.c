/*
 * TrainState.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Sending over audio data to be trained on
 */
//

#include "ErrorState.h"
#include "ESP8266.h"
#include "error.h"
#include "events.h"
#include "FlashDriver.h"
#include "fsm_evt_queue.h"
#include "LockedState.h"
#include "microphone.h"
#include "server.h"
#include "state_machine.h"
#include "stm32l4xx_nucleo_144.h"
#include "TrainState.h"

/* Constants -----------------------------------------------------------------*/
static const uint8_t REQUIRED_TRAIN_CYCLES = 3;

/* Private variables ---------------------------------------------------------*/
static uint8_t m_trainCycles = 0;

/* Private function prototypes -----------------------------------------------*/
static bool TrainState_Standby(Fsm* me, FSM_EVT_T const *event);
static bool TrainState_Record(Fsm* me, FSM_EVT_T const *event);
static bool TrainState_Processing(Fsm* me, FSM_EVT_T const *event);

bool TrainState_Entry(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &TrainState_Standby;

    // Init data and then pass on event to standby state
    m_trainCycles = 0;
    FsmTran_(me, nextState);

    return FsmDispatch(me, event);
}

static bool TrainState_Standby(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &TrainState_Standby;
    bool eventHandled = true;

    switch(event->id)
    {
        case FSM_EVT_USER_BUTTON_PRESS:
            if (Mic_StartRecord() == HAL_OK) {
                nextState = &TrainState_Record;
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

static bool TrainState_Record(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &TrainState_Record;
    bool eventHandled = true;

    uint32_t size = 0;
    AUDIO_SIZE_T* audioData;
    SERVER_CMD_T cmd = SERVER_MAX_CMD;

    switch(event->id)
    {
        // Intentional fall through
        case FSM_EVT_AUDIO_RECORD_DONE:
        case FSM_EVT_USER_BUTTON_PRESS:
            if (Mic_StopRecord() == HAL_OK) {
                audioData = Mic_GetAudioData(&size);

                if(m_trainCycles == 0) {
                     cmd = SERVER_START_TRAIN_CMD;
                }
                else {
                    cmd = SERVER_TRAIN_CMD;
                }
                if(Server_StartAudioTx(cmd, size, (uint8_t*)audioData) == true) {
                    nextState = &TrainState_Processing;
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

static bool TrainState_Processing(Fsm* me, FSM_EVT_T const *event) {
    State nextState = &TrainState_Processing;
    bool eventHandled = true;

    switch(event->id)
    {
        case FSM_EVT_AUDIO_TRANSFER_DONE:
            m_trainCycles++;

            if(m_trainCycles < REQUIRED_TRAIN_CYCLES) {
                nextState = &TrainState_Standby;
            }
            else {
                // Write trained flag if it hasn't been written yet
                if( FlashDriver_GetTrainedFlag() == false) {
                    if( FlashDriver_SetTrainedFlag() == true) {
                        nextState = &LockedState_Entry;
                    }
                    else {
                        nextState = &ErrorState;
                    }
                }
                else {
                    nextState = &LockedState_Entry;
                }
            }
            break;

        default:
            eventHandled = false;
            break;
    }

    FsmTran_(me, nextState);
    return eventHandled;
}
