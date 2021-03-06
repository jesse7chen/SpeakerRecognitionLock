/*
 * error.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Error handling functionality
 */

#include "error.h"
#include "server.h"
#include "stm32l4xx_nucleo_144.h"

void Error_Handler(void){
  // Turn on LED3 and spin in while loop for now
  BSP_LED_On(LED3);
  while (1)
  {
  }
}

bool Error_SendErrorMsg(char* msg, uint32_t msgSize) {
    return Server_SendErrorMsg(msgSize, (uint8_t*)msg);
}
