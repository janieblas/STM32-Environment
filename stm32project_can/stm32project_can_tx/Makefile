# Makefile to compile and link code for STM32F446RE

# Board serials
SERIAL_BD1 = 066AFF515049657187181722
SERIAL_BD2 = 0670FF555185754867103047

# Select board - usage: make flash BD=1 or make flash BD=2
ifeq ($(BD),1)
  HLA_SERIAL = $(SERIAL_BD1)
else ifeq ($(BD),2)
  HLA_SERIAL = $(SERIAL_BD2)
else
  HLA_SERIAL =
endif

# Build serial command fragment only if BD was specified
ifneq ($(HLA_SERIAL),)
  STLINK_SERIAL = -c "hla_serial $(HLA_SERIAL)"
else
  STLINK_SERIAL =
endif

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

# Linker flags
LDFLAGS = -T Device/Ld/STM32F446RETX_FLASH.ld -mcpu=cortex-m4 -mthumb \
                -mfloat-abi=hard -mfpu=fpv4-sp-d16 -specs=nano.specs \
                -Wl,--gc-sections -Wl,--no-warn-rwx-segments

# Include directories
CPPFLAGS = -D STM32F446xx -D USE_HAL_DRIVER \
                -I Core/Inc \
				-I App/Inc \
                -I Drivers/CMSIS/cmsis-device-f4/Include \
                -I Drivers/CMSIS/Core/Include \
                -I Drivers/stm32f4xx-hal-driver/Inc

# Source files
SRCS = 	Core/Src/main.c \
		Core/Src/stm32f4xx_hal_msp.c \
		Core/Src/stm32f4xx_it.c \
		App/Src/system_clock.c \
		App/Src/uart_debug.c \
		App/Src/can_bus.c \
		Drivers/CMSIS/cmsis-device-f4/Source/system_stm32f4xx.c \
		Core/Src/User/syscalls.c \
		Core/Src/User/sysmem.c 

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
                Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_pwr_ex.c\
				Drivers/stm32f4xx-hal-driver/Src/stm32f4xx_hal_can.c

# Combine all source files
ALL_SRCS = $(SRCS) $(HAL_SRCS)

# Assembly source files
ASRCS = Device/Startup/startup_stm32f446retx.s

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
ifndef BD
	$(error Por favor especifica la placa: make flash BD=1 o make flash BD=2)
endif
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
	$(STLINK_SERIAL) \
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