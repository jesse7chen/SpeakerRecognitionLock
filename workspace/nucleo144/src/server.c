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
#include "fsm_evt_queue.h"
#include <limits.h>
#include "microphone.h"
#include "server.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_nucleo_144.h"
#include <string.h>

/* Defines ------------------------------------------------------------*/
#define SERVER_RESPONSE_MSG_SIZE 1

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
static uint8_t m_RxBuffer[SERVER_RESPONSE_MSG_SIZE];

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
                payloadSize = m_TxBuffer.size - buffIdx;
            }
            else{
                payloadSize = MAX_SERVER_PACKET_SIZE;
            }

            // Range check that buffer idx + payload size does not go past end of buffer
            if(buffIdx + payloadSize > m_TxBuffer.size){
                Error_Handler();
            }

            if(Server_TransmitPacket(m_TxPacket.header.cmd , payloadSize, m_TxPacket.header.packetNum,
                                     &m_TxBuffer.buffer[buffIdx]) == false){
                Event_Set(EVENT_AUDIO_TRANSFER_ERROR);
            }
        }
        // Data transfer done, wait for response from server
        else {
            if(ESP8266_ReceiveData(m_RxBuffer, sizeof(m_RxBuffer)) == false) {
                Event_Set(EVENT_AUDIO_TRANSFER_ERROR);
            }
        }
    }
    else if(Event_GetAndClear(EVENT_SERVER_DATA_RX)) {

        FSM_EVT_T event = {
            .id = FSM_EVT_ERROR,
            .size = 0,
            .data = NULL
        };

        // Check response from server
        switch(m_RxBuffer[0]) {
            case SERVER_RESPONSE_VALIDATE_TRUE:
                event.id = FSM_EVT_VALIDATE_TRUE;
                break;
            case SERVER_RESPONSE_VALIDATE_FALSE:
                event.id = FSM_EVT_VALIDATE_FALSE;
                break;
            case SERVER_RESPONSE_SUCCESS:
                event.id = FSM_EVT_AUDIO_TRANSFER_DONE;
                break;
            case SERVER_RESPONSE_ERROR:
                break;
            default:
                break;
        }

        // Notify state machine that audio is finished transferring
        FSM_EVT_QUEUE_Push(event);

        // Reset buffers
        memset(&m_TxPacket, 0, sizeof(m_TxPacket));
        memset(&m_RxBuffer, 0, sizeof(m_RxBuffer));
        m_TxBuffer.buffer = NULL;
        m_TxBuffer.size = 0;
    }
}

bool Server_StartAudioTx(SERVER_CMD_T cmd, uint32_t transferSize, uint8_t* txData){
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

    m_TxPacket.header.cmd = cmd;
    m_TxPacket.header.totalPackets = totalPackets;
    // Calculate payload size
    uint32_t payloadSize = (transferSize >= MAX_SERVER_PACKET_SIZE)
                            ? MAX_SERVER_PACKET_SIZE : transferSize;


    return Server_TransmitPacket(cmd, payloadSize, 1, m_TxBuffer.buffer);
}

bool Server_SendErrorMsg(uint32_t transferSize, uint8_t* txData) {
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

    m_TxPacket.header.cmd = SERVER_ERROR_MSG_CMD;
    m_TxPacket.header.totalPackets = totalPackets;
    // Calculate payload size
    uint32_t payloadSize = (transferSize >= MAX_SERVER_PACKET_SIZE)
                            ? MAX_SERVER_PACKET_SIZE : transferSize;


    return Server_TransmitPacket(m_TxPacket.header.cmd, payloadSize, 1, m_TxBuffer.buffer);
}

bool Server_TransmitPacket(SERVER_CMD_T cmd, uint32_t payloadSize, uint16_t packetNum, uint8_t* txData){
    // Check for NULL pointer or oversized payload
    if(txData == NULL || payloadSize > sizeof(m_TxPacket.buffer))
    {
        return false;
    }
    // Range check command
    if(cmd >= SERVER_MAX_CMD) {
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
