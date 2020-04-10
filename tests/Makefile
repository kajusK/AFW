###############################################################################
# Jakub Kaderka 2020
###############################################################################

#######################
# Project configuration
#######################
# resulting binary name
PROJECT = run_tests

# sources directory
SRCDIR = src
UNITY_DIR = ../sources/external/Unity

# source files to include in build
SOURCES = $(wildcard $(SRCDIR)/*.c) \
	  $(wildcard $(SRCDIR)/*/*.c) \
	  $(wildcard $(UNITY_DIR)/src/*.c) \
	  $(wildcard $(UNITY_DIR)/extras/fixture/src/*.c)

INCLUDES = $(SRCDIR) \
	   ../sources \
	   $(UNITY_DIR)/src \
	   $(UNITY_DIR)/extras/fixture/src \
	   $(UNITY_DIR)/extras/memory/src

#######################
# Directories and stuff
#######################
BUILD_DIR = bin
OPENCM3_DIR = ../sources/external/libopencm3
SOURCES := $(subst ../,,$(SOURCES))
OBJS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

#######################
# Build configuration
#######################
CSTD = -std=gnu11
OPT = -O0
CFLAGS = -Wall -Wextra -Wstrict-prototypes -Wundef -Wshadow -Wredundant-decls \
	-Wno-missing-field-initializers -Wmissing-prototypes -pedantic \
	-fno-common -ffunction-sections -fdata-sections  \
	-Wno-missing-field-initializers -Wno-unused-function \
	-Wno-redundant-decls \
	-Wimplicit-function-declaration  -fprofile-arcs -ftest-coverage \
	-DUNITY_FLOAT_VERBOSE \
	$(CSTD) $(OPT) $(addprefix -I, $(INCLUDES)) \
	-I $(OPENCM3_DIR)/include
LDFLAGS = $(BUILD_FLAGS) -lm -fprofile-arcs --coverage

PREFIX	=
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
all: $(PROJECT) tests coverage

# Need a special rule to have a bin dir
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MD -o $@ -c $<

$(BUILD_DIR)/%.o: ../%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MD -o $@ -c $<

$(PROJECT): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

tests: $(PROJECT)
	./$< -v

coverage: tests
	mkdir -p coverage
	gcovr --html --html-details -o coverage/index.html -s -u -r "." -e ".*/tests/.*" -e ".*/external/.*"

clean:
	@rm -rf $(BUILD_DIR) $(PROJECT) coverage *.pbm ramdisk.img

.PHONY: all clean flash
-include $(OBJS:.o=.d)
