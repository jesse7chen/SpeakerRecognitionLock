/*
 * events.c
 *
 *  Created on: March 7, 2020
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Event functionality implemented as counting semaphores
 */

#include "events.h"
#include <string.h>

static uint8_t m_eventTable[EVENT_MAX];

void eventInit(void){
    // Reset event table
    memset(m_eventTable, 0, sizeof(uint8_t)*EVENT_MAX);
}

void eventSet(EVENT_T event){
    // Range check event
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        m_eventTable[event]++;
    }
}

void eventClear(EVENT_T event){
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        m_eventTable[event] = 0;
    }
}

uint8_t eventGet(EVENT_T event){
    uint8_t numEvents = 0;
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        numEvents = m_eventTable[event];
    }
    else
    {
        // TODO: Implement proper error handling
        return -1;
    }

    return numEvents;
}

uint8_t eventGetAndDecrement(EVENT_T event){
    uint8_t numEvents = 0;
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        if(m_eventTable[event] > 0){
            numEvents = m_eventTable[event]--;
        }
    }
    else
    {
        // TODO: Implement proper error handling
        return -1;
    }

    return numEvents;
}

uint8_t eventGetAndClear(EVENT_T event){
    uint8_t numEvents = 0;
    if(event >= EVENT_MIN && event < EVENT_MAX)
    {
        numEvents = m_eventTable[event];
        m_eventTable[event] = 0;
    }
    else
    {
        // TODO: Implement proper error handling
        return -1;
    }

    return numEvents;
}
