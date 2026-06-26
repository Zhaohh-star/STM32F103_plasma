################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ACS712.c \
../Core/Src/JX7101S.c \
../Core/Src/adc.c \
../Core/Src/current_filter.c \
../Core/Src/dma.c \
../Core/Src/foot_pedal_hal.c \
../Core/Src/gear_switch_hal.c \
../Core/Src/gpio.c \
../Core/Src/main.c \
../Core/Src/pulse_control_hal.c \
../Core/Src/pwm_hal.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/tim.c \
../Core/Src/tjc_usart_hmi.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/ACS712.o \
./Core/Src/JX7101S.o \
./Core/Src/adc.o \
./Core/Src/current_filter.o \
./Core/Src/dma.o \
./Core/Src/foot_pedal_hal.o \
./Core/Src/gear_switch_hal.o \
./Core/Src/gpio.o \
./Core/Src/main.o \
./Core/Src/pulse_control_hal.o \
./Core/Src/pwm_hal.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/tim.o \
./Core/Src/tjc_usart_hmi.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/ACS712.d \
./Core/Src/JX7101S.d \
./Core/Src/adc.d \
./Core/Src/current_filter.d \
./Core/Src/dma.d \
./Core/Src/foot_pedal_hal.d \
./Core/Src/gear_switch_hal.d \
./Core/Src/gpio.d \
./Core/Src/main.d \
./Core/Src/pulse_control_hal.d \
./Core/Src/pwm_hal.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/tim.d \
./Core/Src/tjc_usart_hmi.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/ACS712.cyclo ./Core/Src/ACS712.d ./Core/Src/ACS712.o ./Core/Src/ACS712.su ./Core/Src/JX7101S.cyclo ./Core/Src/JX7101S.d ./Core/Src/JX7101S.o ./Core/Src/JX7101S.su ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/current_filter.cyclo ./Core/Src/current_filter.d ./Core/Src/current_filter.o ./Core/Src/current_filter.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/foot_pedal_hal.cyclo ./Core/Src/foot_pedal_hal.d ./Core/Src/foot_pedal_hal.o ./Core/Src/foot_pedal_hal.su ./Core/Src/gear_switch_hal.cyclo ./Core/Src/gear_switch_hal.d ./Core/Src/gear_switch_hal.o ./Core/Src/gear_switch_hal.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/pulse_control_hal.cyclo ./Core/Src/pulse_control_hal.d ./Core/Src/pulse_control_hal.o ./Core/Src/pulse_control_hal.su ./Core/Src/pwm_hal.cyclo ./Core/Src/pwm_hal.d ./Core/Src/pwm_hal.o ./Core/Src/pwm_hal.su ./Core/Src/stm32f1xx_hal_msp.cyclo ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_it.cyclo ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f1xx.cyclo ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/tjc_usart_hmi.cyclo ./Core/Src/tjc_usart_hmi.d ./Core/Src/tjc_usart_hmi.o ./Core/Src/tjc_usart_hmi.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

