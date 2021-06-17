/*
 * fsm_evt_queue.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - State machine event queue functionality
 */
//

/* Includes ------------------------------------------------------------------*/
#include "error.h"
#include "fsm_evt_queue.h"
#include "stm32l4xx_nucleo_144.h"


/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define EVT_QUEUE_CAPACITY 64
/* Private variables ---------------------------------------------------------*/
FSM_EVT_QUEUE_INIT(m_EvtQueue, EVT_QUEUE_CAPACITY);

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


bool FSM_EVT_QUEUE_Push(FSM_EVT_T data) {
    bool success = true;

    __disable_irq();
    if(FSM_EVT_QUEUE_IsFull() == true) {
        // For now just go into forever loop if this happens
        Error_Handler();
        success = false;
    }

    if(success == true) {
        uint32_t tail = (m_EvtQueue.count + m_EvtQueue.head) & m_EvtQueue.mask;
        m_EvtQueue.count++;
        m_EvtQueue.buffer[tail] = data;

        if(m_EvtQueue.count > m_EvtQueue.watermark) {
            m_EvtQueue.watermark = m_EvtQueue.count;
        }
    }
    __enable_irq();

    return success;
}

bool FSM_EVT_QUEUE_Pop(FSM_EVT_T* data) {
    bool success = true;

    __disable_irq();

    if(FSM_EVT_QUEUE_IsEmpty() == true) {
        success = false;
    }

    if(success == true) {
        m_EvtQueue.count--;
        *data = m_EvtQueue.buffer[m_EvtQueue.head];
        m_EvtQueue.head = (m_EvtQueue.head + 1) & m_EvtQueue.mask;
    }

    __enable_irq();

    return success;
}

void FSM_EVT_QUEUE_Clear(void) {
    __disable_irq();
    m_EvtQueue.count = 0;
    m_EvtQueue.head = 0;
    __enable_irq();
}

// TODO: Add critical sections to below functions when nested critical sections
// are implemented
bool FSM_EVT_QUEUE_IsFull(void) {
    return (m_EvtQueue.capacity <= m_EvtQueue.count);
}

bool FSM_EVT_QUEUE_IsEmpty(void) {
    return (m_EvtQueue.count == 0);
}
