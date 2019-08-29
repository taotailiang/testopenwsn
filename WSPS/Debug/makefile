################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include startup/subdir.mk
-include src/subdir.mk
-include openwsn/projects/common/03oos_openwsn/subdir.mk
-include openwsn/openstack/cross-layers/subdir.mk
-include openwsn/openstack/04-TRAN/subdir.mk
-include openwsn/openstack/03b-IPv6/subdir.mk
-include openwsn/openstack/03a-IPHC/subdir.mk
-include openwsn/openstack/02b-MAChigh/subdir.mk
-include openwsn/openstack/02a-MAClow/subdir.mk
-include openwsn/openstack/subdir.mk
-include openwsn/openapps/userialbridge/subdir.mk
-include openwsn/openapps/uinject/subdir.mk
-include openwsn/openapps/uexpiration_monitor/subdir.mk
-include openwsn/openapps/uexpiration/subdir.mk
-include openwsn/openapps/uecho/subdir.mk
-include openwsn/openapps/rrt/subdir.mk
-include openwsn/openapps/opencoap/subdir.mk
-include openwsn/openapps/cwellknown/subdir.mk
-include openwsn/openapps/cstorm/subdir.mk
-include openwsn/openapps/csensors/subdir.mk
-include openwsn/openapps/cleds/subdir.mk
-include openwsn/openapps/cjoin/subdir.mk
-include openwsn/openapps/cinfo/subdir.mk
-include openwsn/openapps/cexample/subdir.mk
-include openwsn/openapps/c6t/subdir.mk
-include openwsn/openapps/subdir.mk
-include openwsn/kernel/openos/subdir.mk
-include openwsn/drivers/common/subdir.mk
-include openwsn/bsp/chips/at86rf231/subdir.mk
-include openwsn/bsp/chips/S2LP/Src/subdir.mk
-include openwsn/bsp/boards/openmotestm/library/src/subdir.mk
-include openwsn/bsp/boards/openmotestm/configure/subdir.mk
-include openwsn/bsp/boards/openmotestm/subdir.mk
-include openwsn/adc/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: WSPS.elf

# Tool invocations
WSPS.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU G++ Linker'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -T"E:\61_WSPS\WSPS\LinkerScript.ld" -Wl,-Map=output.map -Wl,--gc-sections -fno-exceptions -fno-rtti -o "WSPS.elf" @"objects.list" $(USER_OBJS) $(LIBS) -lm
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

post-build:
	-@echo 'Generating binary and Printing size information:'
	arm-none-eabi-objcopy -O binary "WSPS.elf" "WSPS.bin"
	arm-none-eabi-size "WSPS.elf"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
