################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../openwsn/New0001.c 

OBJS += \
./openwsn/New0001.o 

C_DEPS += \
./openwsn/New0001.d 


# Each subdirectory must supply rules for building sources it contributes
openwsn/%.o: ../openwsn/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32L4 -DSTM32L431CCTx -DDEBUG -DSTM32L431xx -DUSE_HAL_DRIVER -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/HAL_Driver/Inc/Legacy" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/boards/openmotestm/configure" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openstack" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/projects/common/03oos_openwsn" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/kernel" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/c6t" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/cexample" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/cinfo" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/cjoin" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/cleds" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/csensors" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/cstorm" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/cwellknown" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/rrt" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/uecho" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/uexpiration" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/uexpiration_monitor" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/uinject" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/userialbridge" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openapps/opencoap" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openstack/02a-MAClow" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openstack/03a-IPHC" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openstack/03b-IPv6" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openstack/04-TRAN" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openstack/cross-layers" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/openstack/02b-MAChigh" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/drivers/common" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/inc" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/boards/openmotestm" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/boards" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/chips/S2LP/Inc" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/chips/at86rf231" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/chips/S2LP" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/boards/openmotestm/library/CMSIS/Device/ST/STM32L4xx/Include" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/boards/openmotestm/library/CMSIS/Include" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/boards/openmotestm/library/inc" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/openwsn/bsp/boards/openmotestm/library/inc/Legacy" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/inc" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/CMSIS/device" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/CMSIS/core" -I"D:/workspace/1_WIRELESS/2_Progarme/46_WSPS/WSPS/HAL_Driver/Inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


