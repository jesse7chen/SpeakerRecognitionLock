#!/usr/bin/env python
import asyncio
import websockets
import sys
import struct
import numpy as np
import torch
import simpleaudio as sa
import time
import os
from datetime import datetime
from enum import IntEnum
import audioInterface

# Local module which provides function getServerInfo() to return IP and port
import server_info

###### Constants ######
STM32_HEADER_SIZE = 12
SPH0645_SAMP_FREQ = 16000
BASE_PATH = os.path.dirname(os.path.realpath(__file__))

##### Classes/Enums ######
class Command(IntEnum):
    SERVER_MIN_CMD = 0
    SERVER_HEADER_CMD = SERVER_MIN_CMD
    SERVER_START_TRAIN_CMD = 1
    SERVER_TRAIN_CMD = 2
    SERVER_PAYLOAD_CMD = 3
    SERVER_TEST_CMD = 4
    SERVER_VALIDATE_CMD = 5
    SERVER_ERROR_MSG_CMD = 6
    SERVER_MAX_CMD = 7

class Response(IntEnum):
    RESPONSE_MIN = 0
    RESPONSE_SUCCESS = RESPONSE_MIN
    RESPONSE_VALIDATE_TRUE = 1
    RESPONSE_VALIDATE_FALSE = 2
    RESPONSE_ERROR = 3
    RESPONSE_MAX = 4

##### Functions #####

def handleTestCmd(startTime, dataBuff):
    rawDataFilepath = os.path.join(BASE_PATH, "raw_data")

    # Create folder if it hasn't been created already
    try:
        os.mkdir(rawDataFilepath)
    except FileExistsError as e:
        pass

    print("Transfer time: {}s".format(time.time() - startTime))
    # Save raw numpy array
    dt_string = datetime.now().strftime("%d_%m_%Y_%H_%M_%S")
    filename = "raw_data_" + dt_string
    print(filename)
    savePath = os.path.join(rawDataFilepath, filename)
    np.save(savePath, dataBuff)

    # Convert raw data to playable format
    dataBuff = audioInterface.processSimpleAudio(dataBuff)

    play_obj = sa.play_buffer(dataBuff,
                              1, 2, SPH0645_SAMP_FREQ)
    play_obj.wait_done()


def handleStartTrainCmd(dataBuff):
    trainingSetFilePath = os.path.join(BASE_PATH, "training_set")
    embeddingFilePath = os.path.join(trainingSetFilePath, "user.pt")

    # Create folder if it hasn't been created already
    try:
        os.mkdir(trainingSetFilePath)
    except FileExistsError as e:
        pass

    # Delete old contents of training set
    for filename in os.listdir(trainingSetFilePath):
        file_path = os.path.join(trainingSetFilePath, filename)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.unlink(file_path)
        except Exception as e:
            print('Failed to delete %s. Reason: %s' % (file_path, e))

    # Save raw training data for now
    dt_string = datetime.now().strftime("%d_%m_%Y_%H_%M_%S")
    savePath = os.path.join(trainingSetFilePath, "training_data" + dt_string)
    np.save(savePath, dataBuff)

    # Calculate embedding and save it
    emb = audioInterface.encodeAudioData(dataBuff)
    torch.save(emb, embeddingFilePath)


def handleTrainCmd(dataBuff):
    trainingSetFilePath = os.path.join(BASE_PATH, "training_set")
    embeddingFilePath = os.path.join(trainingSetFilePath, "user.pt")
    # Save raw training data as well for now
    dt_string = datetime.now().strftime("%d_%m_%Y_%H_%M_%S")
    savePath = os.path.join(trainingSetFilePath, "training_data" + dt_string)
    np.save(savePath, dataBuff)

    # Calculate new embedding
    emb = audioInterface.encodeAudioData(dataBuff)

    # Calculate number of files in current embedding
    numFiles = 0
    for filename in os.listdir(trainingSetFilePath):
        file_path = os.path.join(trainingSetFilePath, filename)
        if os.path.isfile(file_path) and filename.startswith("training_data"):
            numFiles += 1

    newSize = numFiles + 1
    # Update average embedding
    user_tensor = torch.load(embeddingFilePath)
    new_average = user_tensor + ((emb - user_tensor)/newSize)

    torch.save(new_average, embeddingFilePath)


def handleValidateCmd(dataBuff):
    trainingSetFilePath = os.path.join(BASE_PATH, "training_set")
    embeddingFilePath = os.path.join(trainingSetFilePath, "user.pt")
    user_tensor = torch.load(embeddingFilePath)

    score, match = audioInterface.validateAudioData(dataBuff, user_tensor)

    print("Score: {}\Match: {}\n".format(score, match))

    return match

def unpackHeader(data):
    headerStruct = struct.Struct('2B5H')
    unpackedHeader = headerStruct.unpack(data[0:STM32_HEADER_SIZE])
    print(unpackedHeader)

    header = {
          "version": unpackedHeader[0],
          "cmd": unpackedHeader[1],
          "payloadSize": unpackedHeader[2],
          "packetNum": unpackedHeader[3],
          "totalPackets": unpackedHeader[4],
          "reserved1": unpackedHeader[5],
          "reserved2": unpackedHeader[6]
    }

    return header

async def handleData(websocket, path):
    transferComplete = False
    startTime = time.time()

    # Get data
    while (transferComplete == False):
        data = await websocket.recv()

        # Unpack header
        header = unpackHeader(data)

        # For 32 bits audio samples
        numUnits = int(header["payloadSize"]/4)

        # Allocate buffer and store command if this is the first packet
        if(header["packetNum"] == 1):
            command = header["cmd"]
            buffSize = header["totalPackets"]*numUnits
            dataBuff = np.zeros(buffSize, dtype=np.int32)

        # Unpack data
        dataStruct = struct.Struct(str(numUnits)+'i')
        unpacked_data = dataStruct.unpack(data[STM32_HEADER_SIZE:])
        raw_data = np.array(unpacked_data).astype(np.int32)

        # Load data into audio buffer
        bufferIdxStart = (header["packetNum"] - 1)*numUnits
        bufferIdxEnd = header["packetNum"]*numUnits
        dataBuff[bufferIdxStart:bufferIdxEnd] = raw_data

        # Tell ESP8266 that data has been transferred
        retVal = "success"
        await websocket.send(retVal)

        if(header["packetNum"] == header["totalPackets"]):
            # Handle data based on command
            if command == Command.SERVER_TEST_CMD:
                print("Test command")
                handleTestCmd(startTime, dataBuff)

            elif command == Command.SERVER_START_TRAIN_CMD:
                print("Start train command")
                handleStartTrainCmd(dataBuff)
                retVal = "response {}".format(Response.RESPONSE_SUCCESS)
                await websocket.send(retVal)

            elif command == Command.SERVER_TRAIN_CMD:
                print("Train command")
                handleTrainCmd(dataBuff)
                retVal = "response {}".format(Response.RESPONSE_SUCCESS)
                await websocket.send(retVal)

            elif command == Command.SERVER_VALIDATE_CMD:
                print("Validate command")
                result = handleValidateCmd(dataBuff)
                response = Response.RESPONSE_VALIDATE_TRUE if result \
                           else Response.RESPONSE_VALIDATE_FALSE
                retVal = "response {}".format(response)
                await websocket.send(retVal)

            elif command == Command.SERVER_ERROR_MSG_CMD:
                print(raw_data)
            else:
                print("Command not handled!")

            # Clear audio buffer
            dataBuff.fill(0)
            transferComplete = True
            print("Closing connection")



if __name__ == "__main__":
    print("Starting server")

    hostIP, hostPort = server_info.getServerInfo()
    start_server = websockets.serve(handleData, hostIP, hostPort)

    asyncio.get_event_loop().run_until_complete(start_server)
    asyncio.get_event_loop().run_forever()
