################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HAL_Driver/Src/Legacy/stm32l4xx_hal_can.c 

OBJS += \
./HAL_Driver/Src/Legacy/stm32l4xx_hal_can.o 

C_DEPS += \
./HAL_Driver/Src/Legacy/stm32l4xx_hal_can.d 


# Each subdirectory must supply rules for building sources it contributes
HAL_Driver/Src/Legacy/%.o: ../HAL_Driver/Src/Legacy/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32L4PLUS -DSTM32L4R5ZITx -DNUCLEO_L4R5ZI -DDEBUG -DSTM32L4R5xx -DUSE_HAL_DRIVER -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/HAL_Driver/Inc/Legacy" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/inc" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/CMSIS/device" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/CMSIS/core" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/Utilities/STM32L4xx_Nucleo_144" -I"/Users/jessechen/Documents/SpeakerRecognitionLock/workspace/nucleo144/HAL_Driver/Inc" -O0 -g3 -Wall -fmessage-length=0 -v -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


