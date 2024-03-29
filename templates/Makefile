#######################
# Project configuration
#######################
# resulting binary name
PROJECT = template

FAMILY=STM32F0
# check libopencm3/ld/devices.data for proper values
# m0, m0plus, m3, m4, m7
CPU = cortex-m0
# STM32F0 to F2, L0 to L1, G0
FPU = -msoft-float
# STM32F3 to F4, L4
#FPU = -mfloat-abi=hard -mfpu=fpv4-sp-d16
# STM32F7
#FPU = -mfloat-abi=hard -mfpu=fpv5-sp-d16

# MCU definition for ADC mcu specific code
MCU = STM32F051

# sources directory
SRCDIR = src
# path to AFW library
AFW = ../external/AFW

# linker script (generics in libopencm3/lib/stm32)
LDSCRIPT = $(PROJECT).ld
# STM32CubeMX project to generate board_gpio.h file
MX_PROJECT = $(PROJECT).ioc

# source files to include in build
SOURCES = $(wildcard $(SRCDIR)/*.c)

AFW_SOURCES = $(wildcard $(AFW)/sources/hal/*.c) \
	      $(AFW)/sources/modules/log.c \
	      $(AFW)/sources/modules/config.c \
	      $(AFW)/sources/modules/fw.c \
	      $(AFW)/sources/utils/crc.c \
	      $(AFW)/sources/utils/time.c \
	      $(AFW)/sources/utils/string.c \
	      $(AFW)/sources/utils/ringbuf.c \
	      $(AFW)/sources/drivers/ms5607.c \
	      $(AFW)/sources/drivers/rn4871.c \
	      $(AFW)/sources/drivers/temperature.c

INCLUDES = $(SRCDIR) \
	   $(AFW)/sources

#######################
# Directories and stuff
#######################
BOOTLOADER = ../$(PROJECT)-bl/$(PROJECT)-bl.bin
BUILD_DIR = bin
MX2BOARD = python $(AFW)/tools/mx2board.py
MX_PATH = /opt/stm32cubemx
BOARD_GPIO_FILE = $(SRCDIR)/board_gpio.h
CODECHECK = cppcheck
OPENCM3_DIR = $(AFW)/sources/external/libopencm3

CODECHECK_FLAGS = --enable=warning,performance -j 4 \
    $(addprefix -I, $(INCLUDES)) \
    --template=gcc --suppress=missingIncludeSystem \
    --suppress=unusedFunction --suppress=unmatchedSuppression \
    --suppress=missingInclude --suppress=va_list_usedBeforeStarted \
    --platform=unix32 -q

SOURCES += $(subst $(AFW)/,,$(AFW_SOURCES))
OBJS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

CODECHECK_FILES	= $(SOURCES) \
		  $(shell find $(INCLUDES) -name '*.h' | grep -v external | grep -v tests)

#######################
# Build configuration
#######################
CSTD = -std=gnu11
OPT = -O0
OPENCM3_LIB = opencm3_$(shell echo $(FAMILY) | tr '[:upper:]' '[:lower:]')
OPENCM3_LIB_PATH = $(OPENCM3_DIR)/lib/lib$(OPENCM3_LIB).a
ARCH_FLAGS = -mthumb $(FPU) -mcpu=$(CPU) -D$(FAMILY) -D$(MCU)
CFLAGS = -Wall -Wextra -Wstrict-prototypes -Wundef -Wshadow -Wredundant-decls \
	-Wno-missing-field-initializers -Wmissing-prototypes -pedantic \
	-fno-common -ffunction-sections -fdata-sections  \
	-Wimplicit-function-declaration  \
	--specs=nano.specs --specs=nosys.specs \
	$(CSTD) $(OPT) $(ARCH_FLAGS) $(addprefix -I, $(INCLUDES)) \
	-I $(OPENCM3_DIR)/include -g

LDFLAGS = -T$(LDSCRIPT) -L$(OPENCM3_DIR)/lib $(ARCH_FLAGS) -nostartfiles \
	  --specs=nano.specs -Wl,--gc-sections -Wl,-Map=$(PROJECT).map -lc
LDLIBS += -lopencm3_stm32f0 \
	  -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

PREFIX	= arm-none-eabi-
CC	= $(PREFIX)gcc
CXX	:= $(PREFIX)g++
LD	:= $(PREFIX)gcc
AR	:= $(PREFIX)ar
AS	:= $(PREFIX)as
OBJCOPY	:= $(PREFIX)objcopy
OBJDUMP	:= $(PREFIX)objdump
GDB	:= $(PREFIX)gdb
SIZE	:= $(PREFIX)size

#######################
# Build rules
#######################
all: codecheck $(OPENCM3_LIB_PATH) $(PROJECT).elf fwu debug image size

size: $(PROJECT).elf
	$(SIZE) $<
	
debug: $(PROJECT)_debug.elf
image: $(PROJECT)_image.bin
fwu: $(PROJECT)_fwu.bin

$(OPENCM3_LIB_PATH):
	$(MAKE) -C $(AFW)/sources/external/libopencm3

# generate pinmux
$(BOARD_GPIO_FILE): $(MX_PROJECT)
	$(MX2BOARD) -p $(MX_PROJECT) -m $(MX_PATH) -o $(BOARD_GPIO_FILE)

# Need a special rule to have a bin dir
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MD -o $@ -c $<

$(BUILD_DIR)/%.o: $(AFW)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MD -o $@ -c $<

$(PROJECT).elf: $(BOARD_GPIO_FILE) $(OBJS) $(LDSCRIPT)
	$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary $(PROJECT).elf $(PROJECT).bin

# Debug elf with bootloader included
$(PROJECT)_debug.elf: $(PROJECT).elf $(PROJECT).bin $(BOOTLOADER)
	$(AFW)/tools/fw.py --hdr $(PROJECT).bin $(PROJECT)_hdr.bin
	cp $< $@
	$(OBJCOPY) --add-section .header=$(PROJECT)_hdr.bin --set-section-flags .header=alloc,load,data --change-section-address .header=0x08000800 $@ 2> /dev/null
	$(OBJCOPY) --add-section .header2=$(PROJECT)_hdr.bin --set-section-flags .header2=alloc,load,data --change-section-address .header2=0x8008000 $@ 2> /dev/null
	$(OBJCOPY) --add-section .bootloader=$(BOOTLOADER) --set-section-flags .bootloader=alloc,load,data --change-section-address .bootloader=0x08000000 $@ 2> /dev/null

# flashable firmware with bootloader
$(PROJECT)_image.bin: $(PROJECT).bin $(BOOTLOADER)
	$(AFW)/tools/fw.py $< $@ --bl $(BOOTLOADER)

# image for firmware update
$(PROJECT)_fwu.bin: $(PROJECT).bin
	$(AFW)/tools/fw.py $< $@ --raw

flash: $(PROJECT).bin
	st-flash write $< 0x8000000

clean:
	@rm -rf $(BUILD_DIR) *.elf *.bin *.map

codecheck:
	@$(CODECHECK) $(CODECHECK_FLAGS) $(CODECHECK_FILES)

.PHONY: all clean flash fwu image debug codecheck
-include $(OBJS:.o=.d)
