/*
 * bluetooth.c
 *
 *  Created on: Feb 20, 2020
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Bluetooth functionality from Adafruit's Bluefruit LE SPI Friend
 */
///* Includes ------------------------------------------------------------------*/
#include "bluetooth.h"
#include "stm32l4xx_nucleo_144.h"
#include "stm32l4xx_hal.h"

/*  Statics */
static SPI_HandleTypeDef* bleSPI;


 /* SPI Hardware requirements
 The SPI clock should run <=4MHz
 A 100us delay should be added between the moment that the CS line is asserted, and before any data is transmitted on the SPI bus
 The CS line must remain asserted for the entire packet, rather than toggling CS every byte
 The CS line can however be deasserted and then reasserted between individual SDEP packets (of up to 20 bytes each).
 The SPI commands must be setup to transmit MSB (most significant bit) first (not LSB first)
 */


// TODO: Check if BLE has been initiated before running any commands

HAL_StatusTypeDef BLE_Init(SPI_HandleTypeDef* hspi){

  bleSPI = hspi;
  // Probably should architect this so it's not a hard coded SPI line
  // Drive CS line high to de-assert line
  SPIx__CS_HIGH();

  return HAL_OK;
}

HAL_StatusTypeDef BLE_WriteUART(char* s, uint8_t len){
    // Write a string to bluetooth UART TX
    return BLE_SendMultiSDEP(COMMAND_MSG_TYPE, SDEP_CMDTYPE_BLE_UARTTX, len, (uint8_t*)s);
}

HAL_StatusTypeDef BLE_SendAT(char* cmd, uint8_t len){
    return BLE_SendSDEP(COMMAND_MSG_TYPE, SDEP_CMDTYPE_AT_WRAPPER, len, (uint8_t*)cmd);
}

HAL_StatusTypeDef BLE_SendSDEP(uint8_t msgType, uint16_t cmdID, uint8_t len, uint8_t* payload){

  HAL_StatusTypeDef ret = HAL_ERROR;
  // Binary AND with 0x7F since MSB of len is reserved for a flag
  uint8_t numPackets = (len) & 0x7F;
  uint8_t cmdIDLow = (cmdID) & 0x00FF;
  uint8_t cmdIDHigh = (cmdID) >> 8;

  // Drive CS low to enable bluetooth chip
  SPIx__CS_LOW();
  // TODO: Modify this so that we just do one SPI transmit - aka combine all payloads together (but does this require dynamic allocation?)
  // Wait 100us except the smallest time unit HAL_DELAY takes is ms, so we'll try that for now
  HAL_Delay(1);
  // Write message type
  ret |= HAL_SPI_Transmit(bleSPI, &msgType, (uint16_t)1, BLE_TIMEOUT);
  while(HAL_SPI_GetState(bleSPI) != HAL_SPI_STATE_READY);

  // Write cmdID, lowest byte first
  ret |= HAL_SPI_Transmit(bleSPI, &cmdIDLow, (uint16_t)1, BLE_TIMEOUT);
  while(HAL_SPI_GetState(bleSPI) != HAL_SPI_STATE_READY);
  ret |= HAL_SPI_Transmit(bleSPI, &cmdIDHigh, (uint16_t)1, BLE_TIMEOUT);
  while(HAL_SPI_GetState(bleSPI) != HAL_SPI_STATE_READY);

  // Write payload length
  ret |= HAL_SPI_Transmit(bleSPI, &len, (uint16_t)1, BLE_TIMEOUT);
  while(HAL_SPI_GetState(bleSPI) != HAL_SPI_STATE_READY);


  ret |= HAL_SPI_Transmit(bleSPI, payload, (uint16_t)numPackets, BLE_TIMEOUT);
  while(HAL_SPI_GetState(bleSPI) != HAL_SPI_STATE_READY);

  // Wait a bit before driving line high again, perhaps to get bytes on MOSI?
  HAL_Delay(1);

  // Disable CS in between packets
  SPIx__CS_HIGH();

  return ret;
}

HAL_StatusTypeDef BLE_SendMultiSDEP(uint8_t msgType, uint16_t cmdID, uint8_t len, uint8_t* payload){
  uint8_t packetLen;
  // Make this an int so that it can decrement less than 0
  int8_t bytesLeft = len;
  HAL_StatusTypeDef ret = HAL_ERROR;

  while(bytesLeft > 0){
    packetLen = bytesLeft;
    if(packetLen > MAX_SDEP_PACKET_SIZE){
       // If length greater than 16, set len to 16 and set flag to show that more packets are coming
       packetLen = (MAX_SDEP_PACKET_SIZE | (1UL << 7));
    }
    // Wait 4ms before sending packet. Not sure why, but it works!
    HAL_Delay(4);
    // payload+(len-bytesLeft) is to pass the pointer incremeted by our current position in the payload
    ret |= BLE_SendSDEP(msgType, cmdID, packetLen, payload+(len-bytesLeft));
    bytesLeft -= MAX_SDEP_PACKET_SIZE;
  }

  return ret;

}
