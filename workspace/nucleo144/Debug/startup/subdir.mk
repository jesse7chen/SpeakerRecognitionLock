################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32l4r5xx.s 

OBJS += \
./startup/startup_stm32l4r5xx.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo $(PWD)
	arm-none-eabi-as -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/HAL_Driver/Inc/Legacy" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/inc" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/CMSIS/device" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/CMSIS/core" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/Utilities/STM32L4xx_Nucleo_144" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/HAL_Driver/Inc" -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


