/*
 * events.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Event functionality implemented as counting semaphores
 */

#include "events.h"
#include "stm32l4xx_hal.h"
#include <string.h>

static uint8_t m_eventTable[EVENT_MAX];

// TODO: Implement nested critical sections for accessing m_eventTable

void Event_Init(void){
    // Reset event table
    __disable_irq();
    memset(m_eventTable, 0, sizeof(uint8_t)*EVENT_MAX);
    __enable_irq();
}

void Event_Set(EVENT_T event){
    // Range check event
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        __disable_irq();
        m_eventTable[event]++;
        __enable_irq();
    }
}

void Event_Clear(EVENT_T event){
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        __disable_irq();
        m_eventTable[event] = 0;
        __enable_irq();
    }
}

uint8_t Event_Get(EVENT_T event){
    uint8_t numEvents = 0;
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        __disable_irq();
        numEvents = m_eventTable[event];
        __enable_irq();
    }
    else
    {
        // TODO: Implement proper error handling
        return 0;
    }

    return numEvents;
}

uint8_t Event_GetAndDecrement(EVENT_T event){
    uint8_t numEvents = 0;
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        __disable_irq();
        if(m_eventTable[event] > 0){
            numEvents = m_eventTable[event]--;
        }
        __enable_irq();
    }
    else
    {
        // TODO: Implement proper error handling
        return 0;
    }

    return numEvents;
}

uint8_t Event_GetAndClear(EVENT_T event){
    uint8_t numEvents = 0;
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        __disable_irq();
        numEvents = m_eventTable[event];
        m_eventTable[event] = 0;
        __enable_irq();
    }
    else
    {
        // TODO: Implement proper error handling
        return 0;
    }

    return numEvents;
}
