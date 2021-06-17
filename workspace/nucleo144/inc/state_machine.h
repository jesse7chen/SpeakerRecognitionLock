#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H

#include "fsm_evt_queue.h"
#include "stm32l4xx.h"

typedef struct Fsm Fsm;
typedef bool (*State)(Fsm *, FSM_EVT_T const *);

struct Fsm
{
    State state__;
};

#define FsmCtor_(me_, init_) ((me_)->state__ = (State)(init_))
#define FsmInit(me_, e_)     (*(me_)->state__)((me_), (e_))
#define FsmDispatch(me_, e_) (*(me_)->state__)((me_), (e_))
#define FsmTran_(me_, targ_) ((me_)->state__ = (State)(targ_))

HAL_StatusTypeDef StateMachine_Init(void);
void StateMachine_Run(void);

#endif /* __STATE_MACHINE_H */
