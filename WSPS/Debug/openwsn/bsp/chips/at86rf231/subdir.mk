################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../openwsn/bsp/chips/at86rf231/radio.c 

OBJS += \
./openwsn/bsp/chips/at86rf231/radio.o 

C_DEPS += \
./openwsn/bsp/chips/at86rf231/radio.d 


# Each subdirectory must supply rules for building sources it contributes
openwsn/bsp/chips/at86rf231/%.o: ../openwsn/bsp/chips/at86rf231/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32L4 -DSTM32L431CCTx -DDEBUG -DSTM32L431xx -DUSE_HAL_DRIVER -I"E:/61_WSPS/WSPS/HAL_Driver/Inc/Legacy" -I"E:/61_WSPS/WSPS/openwsn/bsp/boards/openmotestm/configure" -I"E:/61_WSPS/WSPS/openwsn/openstack" -I"E:/61_WSPS/WSPS/openwsn/projects/common/03oos_openwsn" -I"E:/61_WSPS/WSPS/openwsn/kernel" -I"E:/61_WSPS/WSPS/openwsn/openapps/c6t" -I"E:/61_WSPS/WSPS/openwsn/openapps" -I"E:/61_WSPS/WSPS/openwsn/openapps/cexample" -I"E:/61_WSPS/WSPS/openwsn/openapps/cinfo" -I"E:/61_WSPS/WSPS/openwsn/openapps/cjoin" -I"E:/61_WSPS/WSPS/openwsn/openapps/cleds" -I"E:/61_WSPS/WSPS/openwsn/openapps/csensors" -I"E:/61_WSPS/WSPS/openwsn/openapps/cstorm" -I"E:/61_WSPS/WSPS/openwsn/openapps/cwellknown" -I"E:/61_WSPS/WSPS/openwsn/openapps/rrt" -I"E:/61_WSPS/WSPS/openwsn/openapps/uecho" -I"E:/61_WSPS/WSPS/openwsn/openapps/uexpiration" -I"E:/61_WSPS/WSPS/openwsn/openapps/uexpiration_monitor" -I"E:/61_WSPS/WSPS/openwsn/openapps/uinject" -I"E:/61_WSPS/WSPS/openwsn/openapps/userialbridge" -I"E:/61_WSPS/WSPS/openwsn/openapps/opencoap" -I"E:/61_WSPS/WSPS/openwsn/openstack/02a-MAClow" -I"E:/61_WSPS/WSPS/openwsn/openstack/03a-IPHC" -I"E:/61_WSPS/WSPS/openwsn/openstack/03b-IPv6" -I"E:/61_WSPS/WSPS/openwsn/openstack/04-TRAN" -I"E:/61_WSPS/WSPS/openwsn/openstack/cross-layers" -I"E:/61_WSPS/WSPS/openwsn/openstack/02b-MAChigh" -I"E:/61_WSPS/WSPS/openwsn/drivers/common" -I"E:/61_WSPS/WSPS/openwsn/inc" -I"E:/61_WSPS/WSPS/openwsn/bsp/boards/openmotestm" -I"E:/61_WSPS/WSPS/openwsn/bsp/boards" -I"E:/61_WSPS/WSPS/openwsn/bsp/chips/S2LP/Inc" -I"E:/61_WSPS/WSPS/openwsn/bsp/chips/at86rf231" -I"E:/61_WSPS/WSPS/openwsn/bsp/chips/S2LP" -I"E:/61_WSPS/WSPS/openwsn/bsp/boards/openmotestm/library/CMSIS/Device/ST/STM32L4xx/Include" -I"E:/61_WSPS/WSPS/openwsn/bsp/boards/openmotestm/library/CMSIS/Include" -I"E:/61_WSPS/WSPS/openwsn/bsp/boards/openmotestm/library/inc" -I"E:/61_WSPS/WSPS/openwsn/bsp/boards/openmotestm/library/inc/Legacy" -I"E:/61_WSPS/WSPS/inc" -I"E:/61_WSPS/WSPS/CMSIS/device" -I"E:/61_WSPS/WSPS/CMSIS/core" -I"E:/61_WSPS/WSPS/HAL_Driver/Inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


