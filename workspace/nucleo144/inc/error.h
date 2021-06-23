#ifndef __ERROR_H
#define __ERROR_H

#include "common.h"

void Error_Handler(void);
bool Error_SendErrorMsg(char* msg, uint32_t msgSize);
#endif /* __ERROR_H */
