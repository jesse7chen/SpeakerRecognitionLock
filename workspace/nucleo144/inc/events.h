#ifndef __EVENTS_H
#define __EVENTS_H

#include "common.h"

typedef enum EVENT_T
{
    EVENT_MIN = 0,
    EVENT_USER_BUTTON_PRESS = EVENT_MIN,
    EVENT_MAX
} EVENT_T;

void eventInit(void);
void eventSet(EVENT_T event);
void eventClear(EVENT_T event);
uint8_t eventGet(EVENT_T event);
uint8_t eventGetAndDecrement(EVENT_T event);
uint8_t eventGetAndClear(EVENT_T event);

#endif /* __EVENTS_H */
