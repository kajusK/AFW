###########################################
# Jakub Kaderka 2020
###########################################
#resulting bin
TARGET		= run_tests

#dirs
SOURCES = ../sources
EXTERNAL = $(SOURCES)/external

OBJDIR		= obj
UNITY_DIR	= $(EXTERNAL)/Unity
SOURCES_DIRS	= $(UNITY_DIR)/src $(UNITY_DIR)/extras/fixture/src src

#additional build flags
OPT		:= -O0
CSTD		= -std=c11
INCLUDES	= \
	-I $(SOURCES) \
	-I src \
	-I $(UNITY_DIR)/src \
	-I $(UNITY_DIR)/extras/fixture/src \
	-I $(UNITY_DIR)/extras/memory/src
BUILD_FLAGS	= -fprofile-arcs -ftest-coverage
DEFS		= -DUNITY_FLOAT_VERBOSE

#files
C_SRC		:= $(shell find $(SOURCES_DIRS) -name '*.c')
C_SRC		:= $(subst ../, , $(C_SRC))
OBJS		:= $(addprefix $(OBJDIR)/, $(C_SRC:.c=.o))
DEPS		:= $(addprefix $(OBJDIR)/, $(C_SRC:.c=.d))
DIRS		:= $(shell find $(SOURCES_DIRS) -type d)
DIRS		:= $(addprefix $(OBJDIR)/, $(subst ../, , $(DIRS)))

$(info $$DEPS is [${DEPS}])

#toolchain
PREFIX		=
#toolchain definitions
CC		:= $(PREFIX)gcc
CPP		:= $(PREFIX)g++
LD		:= $(PREFIX)gcc
AR		:= $(PREFIX)ar
AS		:= $(PREFIX)as
OBJCOPY		:= $(PREFIX)objcopy
OBJDUMP		:= $(PREFIX)objdump
GDB		:= $(PREFIX)gdb
SIZE		:= $(PREFIX)size

###############################################################################
# C flags
CFLAGS	+= $(OPT) $(CSTD) $(ARCH_FLAGS)
CFLAGS	+= $(DEFS)
CFLAGS	+= $(BUILD_FLAGS)
CFLAGS	+= $(INCLUDES)
CFLAGS	+= -g
CFLAGS	+= -Wall -Wextra -pedantic -Wundef -Wshadow
CFLAGS	+= -Wmissing-prototypes -Wstrict-prototypes
CFLAGS	+= -Wno-missing-field-initializers
CFLAGS	+= -fno-common -ffunction-sections -fdata-sections
CFLAGS	+= -Wno-unused-function

# Linker flags
LDFLAGS		+= $(BUILD_FLAGS) -lm -fprofile-arcs --coverage

all: $(TARGET) tests coverage

$(DIRS):
	mkdir -pv $(DIRS)

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -MM -MT '$@ $(basename $@).d' $< > $(basename $@).d

# workaround for directories with path starting with ../
$(OBJDIR)/%.o : ../%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -MM -MT '$@ $(basename $@).d' $< > $(basename $@).d

$(TARGET): $(DIRS) $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

tests: $(TARGET)
	./$< -v

coverage: tests
	mkdir -p coverage
	gcovr --html --html-details -o coverage/index.html -s -u -r "." -e ".*/tests/.*" -e ".*/external/.*"

clean:
	$(RM) -r $(OBJDIR) $(TARGET) coverage *.pbm ramdisk.img

.PHONY : clean all

-include $(DEPS)
