#ifndef __BUTTON_H
#define __BUTTON_H

#include "common.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_tim.h"

bool Button_Init(void);
void Button_DebounceCallback(void);
TIM_HandleTypeDef* Button_GetDebounceTmrHandle(void);

#endif /* __BUTTON_H */
