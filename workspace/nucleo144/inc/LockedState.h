#ifndef __LOCKED_STATE_H
#define __LOCKED_STATE_H

#include "common.h"
#include "state_machine.h"

bool LockedState_Entry(Fsm* me, FSM_EVT_T const *event);

#endif /* __LOCKED_STATE_H */
