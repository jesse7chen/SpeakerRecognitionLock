#ifndef __SERVER_H
#define __SERVER_H

#include "common.h"

#define MAX_SERVER_PACKET_SIZE 4096

typedef struct __attribute__((packed)) SERVER_HEADER_T
{
    uint8_t version; // Version of communication packet being sent
    uint8_t cmd; // Command being sent
    uint16_t payloadLength; // Length of payload (number of uint8_t chunks)
    uint16_t packetNum;
    uint16_t totalPackets;
    uint16_t reserved[2];
} SERVER_HEADER_T;

typedef struct __attribute__((packed)) SERVER_PACKET_T
{
    uint8_t sync; // Sync byte
    SERVER_HEADER_T header;
    uint8_t buffer[MAX_SERVER_PACKET_SIZE];
    // uint8_t crc; Todo: Implement CRC if necessary, may be too much overhead though
} SERVER_PACKET_T;

typedef struct SERVER_BUFFER_T
{
    uint32_t size;
    uint8_t* buffer;
} SERVER_BUFFER_T;

typedef enum SERVER_CMD_T
{
    SERVER_MIN_CMD = 0,
    SERVER_HEADER_CMD = SERVER_MIN_CMD,
    SERVER_START_TRAIN_CMD,
    SERVER_TRAIN_CMD,
    SERVER_PAYLOAD_CMD,
    SERVER_TEST_CMD,
    SERVER_VALIDATE_CMD,
    SERVER_ERROR_MSG_CMD,
    SERVER_MAX_CMD
} SERVER_CMD_T;

typedef enum SERVER_RESPONSE_T
{
    SERVER_RESPONSE_MIN = 0,
    SERVER_RESPONSE_SUCCESS = SERVER_RESPONSE_MIN,
    SERVER_RESPONSE_VALIDATE_TRUE,
    SERVER_RESPONSE_VALIDATE_FALSE,
    SERVER_RESPONSE_ERROR,
    SERVER_RESPONSE_MAX
} SERVER_RESPONSE_T;

bool Server_Init(void);
void Server_Update(void);

bool Server_StartAudioTx(SERVER_CMD_T cmd, uint32_t transferSize, uint8_t* txData);
bool Server_TransmitPacket(SERVER_CMD_T cmd, uint32_t payloadSize, uint16_t packetNum, uint8_t* txData);
bool Server_SendErrorMsg(uint32_t transferSize, uint8_t* txData);
bool Server_TransmitTest(void);

#endif /* __SERVER_H */
