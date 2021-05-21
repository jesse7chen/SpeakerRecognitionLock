# SpeakerRecognitionLock
Personal project to develop a lock that opens when a specific speaker says "Open Sesame"

* Utilizes STM32L4R5ZI (Arm Cortex-M4) interfacing with microphone, external buttons/LEDs, Bluetooth development board, and ESP8266 WiFi chip
* Audio data relayed over to ESP8266 over WiFi to local server to train model and check inputs

## Requirements
* CMake v3.15.3 or greater
* [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)
    * Used for flashing STM32
* Various python packages
    * To install: python -m pip install -r requirements.txt

## Help
* To build STM32 code: python workspace/nucleo144/build.py --clean
* To flash STM32 code: python workspace/nucleo144/flash.py --clean
    * May need to update filepath to STM32_Programmer_CLI in flash.py
    * Will automatically flash latest built files
