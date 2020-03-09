# SpeakerRecognitionLock
Personal project to develop a lock that opens when a specific speaker says "Open Sesame"

- Utilizes STM32L4R5ZI (Arm Cortex-M4) interfacing with microphone, external buttons/LEDs, Bluetooth development board, and ESP8266 WiFi chip
- Audio data relayed over to ESP8266 over WiFi to local server to train model and check inputs
