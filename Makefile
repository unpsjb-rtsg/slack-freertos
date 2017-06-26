#
# Modify example to build in Makefile.mine
#
-include Makefile.mine

############################################################################### 
#
# Name for the binary, hex and other output build files.
# 
EXAMPLE = $(TARGET)-$(APP_NAME)

############################################################################### 
#
# Compiler executables path and names.
# 
GCC_BIN =
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
SIZE    = $(GCC_BIN)arm-none-eabi-size

############################################################################### 
#
# Paths.
#
MAKE_DIR = $(PWD)
FREERTOS_DIR = $(MAKE_DIR)/libs/FreeRTOS 
EXAMPLES_DIR = $(MAKE_DIR)/examples
INCLUDE_DIR = $(MAKE_DIR)/include

###############################################################################
#
# Common flags and symbols used by the compiler for all the examples.
#
COMMON_FLAGS += -c
COMMON_FLAGS += -g
COMMON_FLAGS += -Wall
COMMON_FLAGS += -fno-common
COMMON_FLAGS += -fmessage-length=0
COMMON_FLAGS += -fno-exceptions
COMMON_FLAGS += -fno-builtin
COMMON_FLAGS += -ffunction-sections
COMMON_FLAGS += -fdata-sections
COMMON_FLAGS += -funsigned-char
COMMON_FLAGS += -fno-delete-null-pointer-checks
COMMON_FLAGS += -fomit-frame-pointer
COMMON_FLAGS += -MMD -MP

ifeq ($(DEBUG), 1)
  COMMON_FLAGS += -DDEBUG
  COMMON_FLAGS += -Og
  COMMON_FLAGS += -ggdb3         # Extra debugging information, for example: including macro definitions.
else
  COMMON_FLAGS += -DNDEBUG -Os  
endif

C_COMMON_FLAGS += -std=gnu99
CPP_COMMON_FLAGS += -fno-rtti
CPP_COMMON_FLAGS += -std=gnu++98

###############################################################################
#
# Make flags.
#
MAKE_FLAGS += --no-print-directory

###############################################################################
#
# Export variables to be used by others Makefile.mk files.
#
export AS CC CPP LD OBJCOPY SIZE FREERTOS_KERNEL_VERSION_NUMBER TRACEALIZER_VERSION_NUMBER MAKEDIR RM DEBUG TARGET COMMON_FLAGS C_COMMON_FLAGS CPP_COMMON_FLAGS EXAMPLE

###############################################################################
#
# Rules used to build the programs.
#
$(APP_NAME):
	+@echo "-- Target: $(TARGET)"
	+@echo "-- Building $(APP_NAME)"
	+@echo "-- DEBUG: $(DEBUG)"
	@$(MAKE) $(MAKE_FLAGS) -C examples/$(TARGET)/ -f Makefile.mk APP_NAME=$(APP_NAME)

$(APP_NAME)_clean:
	+@echo "-- Cleaning $(APP_NAME)"	
	@$(MAKE) $(MAKE_FLAGS) -C examples/$(TARGET)/ -f Makefile.mk clean APP_NAME=$(APP_NAME)

all: $(APP_NAME)

clean: $(APP_NAME)_clean

###############################################################################
#
# Rules for the EDU-CIAA-NXP.
#
OOCD=C:\\Users\\fep\\Documents\\bin\\openocd-0.10.0\\bin\\openocd

openocd:
	$(Q)$(OOCD) -f board/edu-ciaa-nxp/ciaa-nxp.cfg

debug: $(APP_NAME)
	$(Q)$(GDB) $< -ex "target remote :3333" -ex "mon reset halt" -ex "load" -ex "continue"

run: $(APP_NAME)
	$(Q)$(GDB) $< -batch -ex "target remote :3333" -ex "mon reset halt" -ex "load" -ex "mon reset run" -ex "quit"

download: $(APP_NAME)
	$(Q)$(OOCD) -f board/edu-ciaa-nxp/ciaa-nxp.cfg \
		-c "init" \
		-c "halt 0" \
		-c "flash write_image erase unlock build/$(APP_NAME).bin 0x1A000000 bin" \
		-c "reset run" \
		-c "shutdown"

erase:
	$(Q)$(OOCD) -f ciaa-nxp.cfg \
		-c "init" -c "halt 0" -c "flash erase_sector 0 0 last" -c "shutdown"

