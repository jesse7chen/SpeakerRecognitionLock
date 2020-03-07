#ifndef __ADC_H
#define __ADC_H

#include "stm32l4xx.h"
#include <stdint.h>

HAL_StatusTypeDef adcInit(void);
HAL_StatusTypeDef adcChannelsInit(void);
HAL_StatusTypeDef calibrateVRefInt(void);



#endif /* __ADC_H */
