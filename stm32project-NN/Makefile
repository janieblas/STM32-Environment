# Makefile to compile and link code for STM32F446RE with CMSIS-NN

# Compiler and options configuration
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Compiler flags
CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
                -Wall -Wextra -O2 -g3 -ffunction-sections -fdata-sections

# Assembler flags
ASFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# Linker flags (added -lm for math library and -u _printf_float for float support)
LDFLAGS = -T Ld/STM32F446RETX_FLASH.ld -mcpu=cortex-m4 -mthumb \
                -mfloat-abi=hard -mfpu=fpv4-sp-d16 -specs=nano.specs \
                -Wl,--gc-sections -Wl,--no-warn-rwx-segments -lm \
                -u _printf_float

# Include directories
CPPFLAGS = -D STM32F446xx -D USE_HAL_DRIVER -D ARM_MATH_CM4 \
                -I Inc \
                -I Drivers/CMSIS/cmsis-device-f4/Include \
                -I Drivers/CMSIS/Core/Include \
                -I Drivers/stm32f4xx-hal-driver/Inc \
                -I Drivers/CMSIS-NN/Include \
                -I Drivers/CMSIS-NN/Include/Internal \
                -I Drivers/CMSIS-DSP/Include

# Source files
SRCS = 	Src/main.c \
		Src/stm32f4xx_hal_msp.c \
		Src/stm32f4xx_it.c \
		Drivers/CMSIS/cmsis-device-f4/Source/system_stm32f4xx.c \
		Src/User/syscalls.c \
		Src/User/sysmem.c 

# HAL Driver source files (based on hal_config.h enabled modules)
HAL_SRCS = Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_rcc.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_rcc_ex.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_cortex.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_gpio.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_uart.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_flash.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_flash_ex.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_flash_ramfunc.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_dma.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_dma_ex.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_pwr.c \
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_pwr_ex.c

# CMSIS-NN source files - Only the functions we need for XOR network
CMSIS_NN_SRCS = Drivers/CMSIS-NN/Source/NNSupportFunctions/arm_nn_vec_mat_mult_t_s8.c \
                Drivers/CMSIS-NN/Source/NNSupportFunctions/arm_q7_to_q15_with_offset.c \
                Drivers/CMSIS-NN/Source/NNSupportFunctions/arm_nn_mat_mult_nt_t_s8.c \
                Drivers/CMSIS-NN/Source/BasicMathFunctions/arm_elementwise_add_s8.c \
                Drivers/CMSIS-NN/Source/BasicMathFunctions/arm_elementwise_mul_s8.c \
				Drivers/CMSIS-NN/Source/FullyConnectedFunctions/arm_fully_connected_s8.c 
# If you want to add more CMSIS-NN functions later, uncomment these:
# Drivers/CMSIS-NN/Source/FullyConnectedFunctions/arm_fully_connected_s8.c \
# Drivers/CMSIS-NN/Source/ActivationFunctions/arm_relu_q7.c \
# Drivers/CMSIS-NN/Source/SoftmaxFunctions/arm_softmax_s8.c \

# Combine all source files
ALL_SRCS = $(SRCS) $(HAL_SRCS) $(CMSIS_NN_SRCS)

# Assembly source files
ASRCS = Startup/startup_stm32f446retx.s

# Object files
OBJS = $(ASRCS:.s=.o) $(ALL_SRCS:.c=.o)

# Executable name
TARGET = stm32f4xxx_project
ELF_FILE = $(TARGET).elf
HEX_FILE = $(TARGET).hex
BIN_FILE = $(TARGET).bin

# Default target
all: $(ELF_FILE) $(HEX_FILE) $(BIN_FILE) size

# Link object files to create ELF
$(ELF_FILE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

# Compile C source files
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Assemble assembly source files
%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

# Generate HEX file
$(HEX_FILE): $(ELF_FILE)
	$(OBJCOPY) -O ihex $< $@

# Generate BIN file
$(BIN_FILE): $(ELF_FILE)
	$(OBJCOPY) -O binary $< $@

# Display size information
size: $(ELF_FILE)
	$(SIZE) $<

# Clean generated files
clean:
	rm -f $(OBJS) $(ELF_FILE) $(HEX_FILE) $(BIN_FILE)

# Flash using OpenOCD
flash: $(HEX_FILE)
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
	-c "program $(HEX_FILE) verify reset exit"

# Flash using ST-Link utility (alternative)
flash-stlink: $(HEX_FILE)
	st-flash --format ihex write $(HEX_FILE)

# Complete build and flash
ex-flash: clean all flash

# Debug target
debug: $(ELF_FILE)
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg &
	arm-none-eabi-gdb $(ELF_FILE) -ex "target remote :3333"

# Kill OpenOCD using saved PID
kill-debug:
	@if [ -f .openocd.pid ]; then \
		echo "Killing OpenOCD (PID: $$(cat .openocd.pid))..."; \
		kill $$(cat .openocd.pid) 2>/dev/null || echo "Process already terminated"; \
		rm -f .openocd.pid; \
	else \
		echo "Killing any OpenOCD process..."; \
		pkill openocd || echo "OpenOCD was not running"; \
	fi
	@echo "Port 3333 freed"

# Clean up any leftover processes or files
clean-debug: kill-debug
	@rm -f .openocd.pid
	@echo "Complete cleanup done"

# Check if port 3333 is in use
check-debug:
	@echo "Checking port 3333..."
	@lsof -i :3333 || echo "Port 3333 is free"
    
# Phony targets
.PHONY: all clean flash flash-stlink ex-flash debug size