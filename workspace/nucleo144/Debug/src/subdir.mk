################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/adc.c \
../src/bluetooth.c \
../src/main.c \
../src/stm32l4xx_hal_msp.c \
../src/stm32l4xx_it.c \
../src/syscalls.c \
../src/system_stm32l4xx.c 

OBJS += \
./src/adc.o \
./src/bluetooth.o \
./src/main.o \
./src/stm32l4xx_hal_msp.o \
./src/stm32l4xx_it.o \
./src/syscalls.o \
./src/system_stm32l4xx.o 

C_DEPS += \
./src/adc.d \
./src/bluetooth.d \
./src/main.d \
./src/stm32l4xx_hal_msp.d \
./src/stm32l4xx_it.d \
./src/syscalls.d \
./src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32L4PLUS -DSTM32L4R5ZITx -DNUCLEO_L4R5ZI -DDEBUG -DSTM32L4R5xx -DUSE_HAL_DRIVER -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/HAL_Driver/Inc/Legacy" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/inc" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/CMSIS/device" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/CMSIS/core" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/Utilities/STM32L4xx_Nucleo_144" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/HAL_Driver/Inc" -O0 -g3 -Wall -fmessage-length=0 -v -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


