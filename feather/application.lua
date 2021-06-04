--[[

Upon learning that there is data to send I need to:
    Create a TCP connection to the host, perhaps send a GET request with the data in the URI
    I need to make sure to keep medium priority tasks under 2ms and low priority tasks under 15ms...since receiving and sending the data may take longer than this,
    I need to figure out a way to do these reads in multiple chunks

Seems like I can use the MQTT protocol to send messages over TCP/IP
Some other protocols that are common are:
    CoAP (Constrained Application Protocol)
    AMQP (Advanced Message Queuing Protocol)
    DDS (Data Distribution Service)
    STOMP (Simple Text Oriented Messaging Protocol)
    MQTT (Message Queue Telemetry Transport)

]]--

-- May need to add external pullup resisters, depending on pins choosen

-- HostMachineIP and HostMachineWSPort defined in credentials.lua file

-- Change max string size
collectgarbage("setmemlimit", 8192)

-- Static variables
local STM32_HEADER_SIZE_LEN = 12
local INT_PIN = 1

-- Flags
isTransferInProgress = false

-- MQTT Functionality
success_callback = function(client)
    print(string.format("Connected to broker: %s", HostMachineIP))
    client:publish("zz-sensor-readings", "test1234", 0, 0, function(client) print("sent") end)
end

fail_callback = function(client, reason)
    print("failed reason", reason)
end

function MQTT_Connect()
    m:connect(HostMachineIP, 1883, 0, success_callback, fail_callback)
end

function MQTT_Disconnect()
    m:close()
end

-- Utility functions
function StringToUint16(s)

    lowerByte = string.byte(s:sub(1,1))
    upperByte = string.byte(s:sub(2,2))

    return (bit.lshift(upperByte, 8) + lowerByte)
end

-- SPI functionality
function SPI_FetchData()
    -- Sync byte in case noise causes interrupt to trigger
    sync_byte = string.byte(spi.recv(1, 1))

    if(sync_byte == 0xAB) then
        isTransferInProgress = true
        header = spi.recv(1, STM32_HEADER_SIZE_LEN)

        version = string.byte(header:sub(1,1))
        cmd = string.byte(header:sub(2,2))
        payloadLength = StringToUint16(header:sub(3,4))
        packetNum = StringToUint16(header:sub(5,6))

        if payloadLength ~= 0 then
            data = spi.recv(1, payloadLength)
            -- If this is the first packet, start connection
            -- Todo: Add check to make sure that first packet was received and connection
            -- established before sending more packets, otherwise if packet 2 is
            -- received first for some reason, then we will fail
            if packetNum == 1 then
                ws:on("connection", function()
                    ws:send(header..data,2)
                end)
                ws:connect(HostMachineWSPort)
            else
                -- Todo: Error handling when connection is closed unexpectedly
                ws:send(header..data,2)
            end
        end
    end
end

function GPIO_IntCallback()
    -- If transfer is still in progress, set a timer to retry again in a bit
    if(isTransferInProgress == true) then
        -- print("Transfer in progress")
        GPIO_IntTimer:start()
    -- Otherwise handle data
    else
        SPI_FetchData()
    end
end

-- Websocket functionality
function Websocket_CloseCallback(_, status)
    -- print("Transfer finished")
    isTransferInProgress = false
end

function Websocket_ReceiveCallback(_, msg, opcode)
    if(msg == "success") then
        isTransferInProgress = false
    end
end

function init_peripherals()
    -- Use SPI ID 1 for HSPI, since SPI 0 is probably being used to communicate with flash chip
    -- Databits = 8
    -- Clock_divider = 2 (default is 80 MHz)
    -- CLKPolarity is LOW
    -- CLK phase also seems to be low
    spi.setup(1, spi.MASTER, spi.CPOL_LOW, spi.CPHA_LOW, 8, 8)

    gpio.mode(INT_PIN, gpio.INT, gpio.PULLUP)
    gpio.trig(INT_PIN, "down", GPIO_IntCallback)

    ws = websocket.createClient()
    ws:on("close", Websocket_CloseCallback)
    ws:on("receive", Websocket_ReceiveCallback)

    GPIO_IntTimer = tmr.create()
    GPIO_IntTimer:register(50, tmr.ALARM_SEMI, GPIO_IntCallback)

    print("Done with initialization")
end

print("Initializing peripherals")
init_peripherals()
