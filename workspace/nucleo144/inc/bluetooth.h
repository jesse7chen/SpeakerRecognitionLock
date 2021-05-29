#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

// BLE SPI timeout in ms
#define BLE_TIMEOUT                              (500)
#define MAX_SDEP_PACKET_SIZE                     (16)

#define SDEP_CMDTYPE_INITIALIZE                  (0xBEEF)
#define SDEP_CMDTYPE_AT_WRAPPER                  (0x0A00)
#define SDEP_CMDTYPE_BLE_UARTTX                  (0x0A01)
#define SDEP_CMDTYPE_BLE_UARTRX                  (0x0A02)

#define COMMAND_MSG_TYPE                         (0x10)
#define RESPONSE_MSG_TYPE                        (0x20)
#define ALERT_MSG_TYPE                           (0x40)
#define ERROR_MSG_TYPE                           (0x80)

#define AT_SWITCH_MODE                           "+++"
#define AT_SWITCH_MODE_LEN                       (3)

#define AT_WRITE_UART_TEST                       "AT+BLEUARTTX=A\n"
#define AT_WRITE_UART_TEST_LEN                   (15)
#define AT_LED_OFF                               "AT+HWMODELED=1\n"
#define AT_LED_OFF_LEN                           (15)

///* Includes ------------------------------------------------------------------*/
// This file also includes "stm32l4xx_hal.h" and "stm32l4r5xx.h"
// "stm32l4xx_hal.h" includes "stm32l4xx_hal_conf.h" which includes every single HAL module that is enabled (or at least their header files)
// Every single HAL module header enabled includes "stm32l4xx_hal_def.h" which includes "stm32l4xx.h", stm32_hal_legacy.h, and <stddef.h>
// For some reason including "stm32l4xx_hal_def.h" at this point gets rid of those symbols for every other module that tries to import it
// It seems like none of these module C files actually import their own header files. "stm32l4xx_hal_uart.c" only imports "stm32l4xx_hal.h", which as stated above
// imports "stm32l4xx_hal_conf.h" and every single HAL module header that is enabled which then imports stm32l4xx_hal_def.h

// So when we finally reach that C files that are failing, they go to import stm32l4xx_hal_def.h.
// Still really have no idea what is going on in the linker when it fails there

#include "stm32l4xx.h"
// For some reason including the bottom stm32l4xx_hal_spi.h or stm32l4xx_hal_def.h instead of stm32l4xx.h breaks literally everything.
// Interestingly if leave these at the bottom and uncomment them while leaving the top uncommented, nothing breaks. But if you move
// One of these statements above #include "stm32l4xx.h", then everything breaks.

#include "stm32l4xx_hal_spi.h"
// #include "stm32l4xx_hal_def.h"
#include "common.h"

//TODO: Look into making some kind of static SPI_HandleTypeDef for bluetooth, so we don't have to pass in this stuff
// Perhaps make it part of the init function?

HAL_StatusTypeDef bleInit(SPI_HandleTypeDef* hspi);
HAL_StatusTypeDef bleWriteUART(char* s, uint8_t len);
HAL_StatusTypeDef bleSendAT(char* cmd, uint8_t len);
HAL_StatusTypeDef sendSDEP(uint8_t msgType, uint16_t cmdID, uint8_t len, uint8_t* payload);
HAL_StatusTypeDef sendMultiSDEP(uint8_t msgType, uint16_t cmdID, uint8_t len, uint8_t* payload);

#endif /* __BLUETOOTH_H */
