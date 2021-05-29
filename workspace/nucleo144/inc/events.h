#ifndef __EVENTS_H
#define __EVENTS_H

#include "common.h"

typedef enum EVENT_T
{
    EVENT_MIN = 0,
    EVENT_USER_BUTTON_PRESS = EVENT_MIN,
    EVENT_MAX
} EVENT_T;

void Event_Init(void);
void Event_Set(EVENT_T event);
void Event_Clear(EVENT_T event);
uint8_t Event_Get(EVENT_T event);
uint8_t Event_GetAndDecrement(EVENT_T event);
uint8_t Event_GetAndClear(EVENT_T event);

#endif /* __EVENTS_H */
