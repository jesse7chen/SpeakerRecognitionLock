#ifndef __ESP8266_H
#define __ESP8266_H

#include "common.h"
#include "stm32l4xx.h"

#define MAX_ESP8266_PACKET_LENGTH 2048
#define MAX_ESP8266_PACKET_SIZE ( MAX_ESP8266_PACKET_LENGTH*sizeof(uint16_t) )


// Size of units of data transfer over SPI
typedef uint8_t ESP8266_SPI_DATA_SIZE_T;

bool ESP8266_Init(void);
bool ESP8266_TransmitData(uint8_t* txData, uint16_t size);
SPI_HandleTypeDef* ESP8266_GetSpiHandle(void);
void ESP8266_SpiTxCpltCallback(SPI_HandleTypeDef *hspi);


#endif /* __ESP8266_H */
