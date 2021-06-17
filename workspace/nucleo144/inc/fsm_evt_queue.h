#ifndef __FSM_EVT_QUEUE_H
#define __FSM_EVT_QUEUE_H

#include "common.h"

typedef enum
{
    FSM_EVT_MIN = 0,
    FSM_EVT_USER_BUTTON_PRESS = FSM_EVT_MIN,
    FSM_EVT_AUDIO_TRANSFER_DONE,
    FSM_EVT_AUDIO_RECORD_DONE,
    FSM_EVT_ERROR,
    FSM_EVT_MAX
} FSM_EVT_ID_T;

typedef struct FSM_EVT_T {
    FSM_EVT_ID_T id;
    uint32_t size; // Size of data stored in data buffer, if any
    uint8_t* data;
} FSM_EVT_T;

typedef struct FSM_EVT_QUEUE_T {
    FSM_EVT_T* buffer;
    uint32_t capacity;
    uint32_t mask;
    uint32_t count;
    uint32_t head;
    uint32_t watermark;
} FSM_EVT_QUEUE_T;

#define IS_POWER_OF_TWO(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))

#define FSM_EVT_QUEUE_INIT(_name_, _capacity_)                               \
    _Static_assert(IS_POWER_OF_TWO(_capacity_), "Capacity not power of 2");  \
    static FSM_EVT_T _name_##_buf[_capacity_];                               \
    static FSM_EVT_QUEUE_T _name_ = {                                        \
        .buffer = _name_##_buf,                                              \
        .capacity = _capacity_,                                              \
        .mask = _capacity_ - 1,                                              \
        .head = 0,                                                           \
        .count = 0,                                                          \
        .watermark = 0,                                                      \
    }

bool FSM_EVT_QUEUE_Push(FSM_EVT_T data);
bool FSM_EVT_QUEUE_Pop(FSM_EVT_T* data);
void FSM_EVT_QUEUE_Clear(void);
bool FSM_EVT_QUEUE_IsFull(void);
bool FSM_EVT_QUEUE_IsEmpty(void);

#endif /* __FSM_EVT_QUEUE_H */
