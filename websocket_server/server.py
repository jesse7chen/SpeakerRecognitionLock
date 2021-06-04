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

# Local module which provides function getServerInfo() to return IP and port
import server_info

fs = 44100

STM32_HEADER_SIZE = 12

MAX_12BIT_NUM = 4096

originalSampFreq = 45941.85 #Hz
desiredSampFreq = 44100 #Hz

# Output of microphone is 2 Vpp on a 1.25 V bias
# ADC measurements output range is 0-3.3V
DC_BIAS = (1.25/3.3)*4096

bufferSize = 32768*6

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
          "totalPackets": unpackedHeader[4]
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

        # Allocate buffer if this is the first packet
        if(header["packetNum"] == 1):
            buffSize = header["totalPackets"]*header["payloadSize"]
            audioBuff = np.zeros(buffSize, dtype=np.int16)

        # Each audio sample is 16 bits currently
        numUnits = int(header["payloadSize"]/2)

        # Unpack data
        dataStruct = struct.Struct(str(numUnits)+'H')
        unpacked_data = dataStruct.unpack(data[STM32_HEADER_SIZE:])
        raw_audio = np.array(unpacked_data).astype(np.int16)

        # Load data into audio buffer
        bufferIdxStart = (header["packetNum"] - 1)*numUnits
        bufferIdxEnd = header["packetNum"]*numUnits
        audioBuff[bufferIdxStart:bufferIdxEnd] = raw_audio

        retVal = "success"
        await websocket.send(retVal)

        if(header["packetNum"] == header["totalPackets"]):
            print("Transfer time: {}s".format(time.time() - startTime))
            # Take out microphone DC bias
            audioBuff = audioBuff - DC_BIAS

            # Normalize audio to int16_t range max
            audioBuff *= 32767 / max(abs(audioBuff))

            # Resample data
            ratio = desiredSampFreq/originalSampFreq
            resampledAudio = samplerate.resample(audioBuff.astype(np.float32), ratio, 'sinc_best')

            # simpleaudio requires input to be signed int16_t
            play_obj = sa.play_buffer(resampledAudio.astype(np.int16),\
                                      1, 2, desiredSampFreq)
            play_obj.wait_done()

            # Clear audio buffer
            audioBuff.fill(0)
            transferComplete = True


print("Starting server")

hostIP, hostPort = server_info.getServerInfo()
start_server = websockets.serve(handleData, hostIP, hostPort)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
