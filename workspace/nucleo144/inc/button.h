#ifndef __BUTTON_H
#define __BUTTON_H

#include "common.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_tim.h"

bool buttonInit(void);
void buttonDebounceCallback(void);
TIM_HandleTypeDef* getButtonDebounceTmrHandle(void);

#endif /* __BUTTON_H */
