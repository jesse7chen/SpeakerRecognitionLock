#!/usr/bin/env python

# WS server example

import asyncio
import websockets
import sys
import struct
import numpy as np
import simpleaudio as sa
import samplerate
import time
import os
from datetime import datetime

# Local module which provides function getServerInfo() to return IP and port
import server_info

STM32_HEADER_SIZE = 12

# MAX9814 settings
MAX_12BIT_NUM = 4096
originalSampFreq = 45941.85 #Hz
desiredSampFreq = 44100 #Hz
# Output of microphone is 2 Vpp on a 1.25 V bias
# ADC measurements output range is 0-3.3V
DC_BIAS = (1.25/3.3)*4095

# SPH0645 settings
samplingFreq = 16000


base_path = os.path.dirname(os.path.realpath(__file__))
rawDataFilepath = os.path.join(base_path, "raw_data")

def sign_extend(value, bits):
    sign_bit = 1 << (bits - 1)
    return (value & (sign_bit - 1)) - (value & sign_bit)

def unpackHeader(data):
    # Unpack header
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

        print("Data length: {}".format(len(data)))

        # Unpack header
        header = unpackHeader(data)

        # For 32 bits audio samples
        numUnits = int(header["payloadSize"]/4)

        # Allocate buffer if this is the first packet
        if(header["packetNum"] == 1):
            buffSize = header["totalPackets"]*numUnits
            audioBuff = np.zeros(buffSize, dtype=np.int32)

        # Unpack data
        dataStruct = struct.Struct(str(numUnits)+'i')
        unpacked_data = dataStruct.unpack(data[STM32_HEADER_SIZE:])
        raw_audio = np.array(unpacked_data).astype(np.int32)

        # Load data into audio buffer
        bufferIdxStart = (header["packetNum"] - 1)*numUnits
        bufferIdxEnd = header["packetNum"]*numUnits
        audioBuff[bufferIdxStart:bufferIdxEnd] = raw_audio

        retVal = "success"
        await websocket.send(retVal)

        if(header["packetNum"] == header["totalPackets"]):
            print("Transfer time: {}s".format(time.time() - startTime))
            # Save raw numpy array
            now = datetime.now()
            dt_string = now.strftime("%d_%m_%Y_%H_%M_%S")
            filename = "raw_data_" + dt_string
            print(filename)
            savePath = os.path.join(rawDataFilepath, filename)
            np.save(savePath, audioBuff)

            # Take out microphone DC bias - ADC only
            # audioBuff = audioBuff - DC_BIAS

            # Convert 18 bit MSB aligned data to regular int32 data
            audioBuff = np.right_shift(audioBuff, 6)
            audioBuff = np.array([sign_extend(x, 18) for x in audioBuff]).astype(np.int32)

            # Convert audio to float
            audioBuff = audioBuff.astype(np.float64)

            # Normalize audio to int16_t range max
            audioBuff *= 32767 / max(abs(audioBuff))

            # Resample data - ADC only
            # ratio = desiredSampFreq/originalSampFreq
            # resampledAudio = samplerate.resample(audioBuff.astype(np.float32), ratio, 'sinc_best')

            # simpleaudio requires input to be signed int16_t
            # play_obj = sa.play_buffer(resampledAudio.astype(np.int16),\
            #                           1, 2, desiredSampFreq)

            play_obj = sa.play_buffer(audioBuff.astype(np.int16),\
                                      1, 2, samplingFreq)
            play_obj.wait_done()

            # Clear audio buffer
            audioBuff.fill(0)
            transferComplete = True
            print("Closing connection")


print("Starting server")

hostIP, hostPort = server_info.getServerInfo()
start_server = websockets.serve(handleData, hostIP, hostPort)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
