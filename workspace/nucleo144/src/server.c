/*
 * server.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Higher level functionality of communicating with server
 */
//
/* Includes ------------------------------------------------------------------*/
#include "ESP8266.h"
#include "error.h"
#include "events.h"
#include <limits.h>
#include "microphone.h"
#include "server.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_nucleo_144.h"
#include <string.h>

/* Constants -----------------------------------------------------------------*/
static const uint8_t SYNC_BYTE_LENGTH = sizeof(uint8_t);
static const uint8_t SYNC_BYTE = 0xAB;
static const uint8_t HEADER_VERSION = 0x01;

static const uint16_t SERVER_HEADER_LENGTH = sizeof(SERVER_HEADER_T)/sizeof(ESP8266_SPI_DATA_SIZE_T);

// Max payload size is equal to the max number of packets we can send
static const uint32_t MAX_TRANSFER_SIZE = USHRT_MAX*MAX_SERVER_PACKET_SIZE;

/* Private variables ---------------------------------------------------------*/
static SERVER_PACKET_T m_TxPacket;
static SERVER_BUFFER_T m_TxBuffer;

/* Private function prototypes -----------------------------------------------*/


bool Server_Init(void){
    return ESP8266_Init();
}

void Server_Update(void){
    if(Event_GetAndClear(EVENT_AUDIO_PKT_SENT)){
        if (m_TxPacket.header.packetNum < m_TxPacket.header.totalPackets){
            uint32_t buffIdx = 0;
            uint16_t payloadSize = 0;

            // Calculate next buffer portion to send
            buffIdx = m_TxPacket.header.packetNum * MAX_SERVER_PACKET_SIZE;
            // Increment packet number
            m_TxPacket.header.packetNum++;

            // May not need to send a full payload if last packet
            if(m_TxPacket.header.packetNum >= m_TxPacket.header.totalPackets){
                payloadSize = MAX_SERVER_PACKET_SIZE;
            }
            else{
                payloadSize = MAX_SERVER_PACKET_SIZE;
            }

            // Range check that buffer idx + payload size does not go past end of buffer
            if(buffIdx + payloadSize > m_TxBuffer.size){
                Error_Handler();
            }

            if(Server_TransmitPacket(SERVER_PAYLOAD_CMD, payloadSize, m_TxPacket.header.packetNum,
                                     &m_TxBuffer.buffer[buffIdx]) == false){
                Event_Set(EVENT_AUDIO_TRANSFER_ERROR);
            }
        }
        // Data transfer done, reset values
        else{
            memset(&m_TxPacket, 0, sizeof(m_TxPacket));
            m_TxBuffer.buffer = NULL;
            m_TxBuffer.size = 0;
            Event_Set(EVENT_AUDIO_TRANSFER_DONE);
        }
    }
}

bool Server_StartAudioTx(uint32_t transferSize, uint8_t* txData){
    if(txData == NULL || transferSize > MAX_TRANSFER_SIZE){
        return false;
    }

    // Copy metadata into m_TxBuffer
    m_TxBuffer.size = transferSize;
    m_TxBuffer.buffer = txData;

    // Calculate number of packets necessary
    uint16_t totalPackets = ( transferSize/MAX_SERVER_PACKET_SIZE );
    if(transferSize % MAX_SERVER_PACKET_SIZE != 0){
        totalPackets++;
    }

    m_TxPacket.header.totalPackets = totalPackets;
    // Calculate payload size, for now default to the max size
    uint32_t payloadSize = MAX_SERVER_PACKET_SIZE;

    return Server_TransmitPacket(SERVER_PAYLOAD_CMD, payloadSize, 1, m_TxBuffer.buffer);
}

bool Server_TransmitPacket(uint16_t cmd, uint32_t payloadSize, uint16_t packetNum, uint8_t* txData){
    // Check for NULL pointer or oversized payload
    if(txData == NULL || payloadSize > sizeof(m_TxPacket.buffer))
    {
        return false;
    }

    uint16_t payloadLength = 0;
    uint16_t transferLength = 0;

    payloadLength = payloadSize/sizeof(ESP8266_SPI_DATA_SIZE_T);

    // Add sync byte
    m_TxPacket.sync = SYNC_BYTE;

    // Format header
    m_TxPacket.header.version = HEADER_VERSION;
    m_TxPacket.header.cmd = cmd;
    m_TxPacket.header.payloadLength = payloadLength;
    m_TxPacket.header.packetNum = packetNum;
    // Load data
    memcpy((uint8_t*) &m_TxPacket.buffer, txData, payloadSize);

    transferLength = payloadLength + SERVER_HEADER_LENGTH + SYNC_BYTE_LENGTH;

    // Length is used here because HAL_SPI transmits data in terms of length or
    // number of units instead of absolute size. The number of units is dependent
    // on the data width of SPI transfer
    return ESP8266_TransmitData((uint8_t*)&m_TxPacket, transferLength);
}

bool Server_TransmitTest(void){
    uint16_t buff[100];
    char* data = "123456789";
    uint16_t length = strlen(data);

    for(int i = 0; i < length+1; i++){
        buff[i] = (uint16_t)data[i];
    }

    return Server_TransmitPacket(SERVER_TEST_CMD, length+1, 0, (uint8_t*)buff);
}
