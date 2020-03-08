#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H


#include "stm32l4xx.h"

typedef enum
{
    stateReset = 0,
    stateStandby,
    stateRec,
    stateProc,
    stateCal,
    stateErr
} sm_state_t;

HAL_StatusTypeDef smInit(sm_state_t* state);
void smRun(sm_state_t* state);

#endif /* __STATE_MACHINE_H */
