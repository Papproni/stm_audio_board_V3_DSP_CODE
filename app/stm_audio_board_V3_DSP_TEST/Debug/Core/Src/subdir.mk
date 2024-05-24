################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/AD1939_driver.c \
../Core/Src/dma.c \
../Core/Src/driver_w25qxx.c \
../Core/Src/effect_manager.c \
../Core/Src/fmc.c \
../Core/Src/gpio.c \
../Core/Src/guitar_effect_delay.c \
../Core/Src/guitar_effect_octave.c \
../Core/Src/is42s16800j.c \
../Core/Src/main.c \
../Core/Src/octospi.c \
../Core/Src/sai.c \
../Core/Src/spi.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32h7xx.c 

OBJS += \
./Core/Src/AD1939_driver.o \
./Core/Src/dma.o \
./Core/Src/driver_w25qxx.o \
./Core/Src/effect_manager.o \
./Core/Src/fmc.o \
./Core/Src/gpio.o \
./Core/Src/guitar_effect_delay.o \
./Core/Src/guitar_effect_octave.o \
./Core/Src/is42s16800j.o \
./Core/Src/main.o \
./Core/Src/octospi.o \
./Core/Src/sai.o \
./Core/Src/spi.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32h7xx.o 

C_DEPS += \
./Core/Src/AD1939_driver.d \
./Core/Src/dma.d \
./Core/Src/driver_w25qxx.d \
./Core/Src/effect_manager.d \
./Core/Src/fmc.d \
./Core/Src/gpio.d \
./Core/Src/guitar_effect_delay.d \
./Core/Src/guitar_effect_octave.d \
./Core/Src/is42s16800j.d \
./Core/Src/main.d \
./Core/Src/octospi.d \
./Core/Src/sai.d \
./Core/Src/spi.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32h7xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"C:/Users/Aron/Documents/GitHub/DSP_CODE/stm_audio_board_V3_DSP_CODE/app/stm_audio_board_V3_DSP_TEST/Drivers/CMSIS/DSP/Include" -I"C:/Users/Aron/Documents/GitHub/DSP_CODE/stm_audio_board_V3_DSP_CODE/app/stm_audio_board_V3_DSP_TEST/Drivers/CMSIS/Include" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Inc -Ofast -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/AD1939_driver.cyclo ./Core/Src/AD1939_driver.d ./Core/Src/AD1939_driver.o ./Core/Src/AD1939_driver.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/driver_w25qxx.cyclo ./Core/Src/driver_w25qxx.d ./Core/Src/driver_w25qxx.o ./Core/Src/driver_w25qxx.su ./Core/Src/effect_manager.cyclo ./Core/Src/effect_manager.d ./Core/Src/effect_manager.o ./Core/Src/effect_manager.su ./Core/Src/fmc.cyclo ./Core/Src/fmc.d ./Core/Src/fmc.o ./Core/Src/fmc.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/guitar_effect_delay.cyclo ./Core/Src/guitar_effect_delay.d ./Core/Src/guitar_effect_delay.o ./Core/Src/guitar_effect_delay.su ./Core/Src/guitar_effect_octave.cyclo ./Core/Src/guitar_effect_octave.d ./Core/Src/guitar_effect_octave.o ./Core/Src/guitar_effect_octave.su ./Core/Src/is42s16800j.cyclo ./Core/Src/is42s16800j.d ./Core/Src/is42s16800j.o ./Core/Src/is42s16800j.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/octospi.cyclo ./Core/Src/octospi.d ./Core/Src/octospi.o ./Core/Src/octospi.su ./Core/Src/sai.cyclo ./Core/Src/sai.d ./Core/Src/sai.o ./Core/Src/sai.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/stm32h7xx_hal_msp.cyclo ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_it.cyclo ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32h7xx.cyclo ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su

.PHONY: clean-Core-2f-Src

