############################################################################### 
#
# Include the test configuration file.
#
-include Makefile.config

TARGET = lpc1768

############################################################################### 
#
# Compiler executables path and names.
# 
GCC_BIN =
AR      = $(GCC_BIN)arm-none-eabi-ar
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
FREERTOS_DIR = $(MAKE_DIR)/../../../libs/FreeRTOS

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
  COMMON_FLAGS += -ggdb3
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
export AR AS CC CPP LD OBJCOPY SIZE 
export FREERTOS_KERNEL_VERSION_NUMBER TRACEALIZER_VERSION_NUMBER 
export COMMON_FLAGS C_COMMON_FLAGS CPP_COMMON_FLAGS TARGET EXAMPLE TZ
export KERNEL_TEST TASK_COUNT_PARAM RELEASE_COUNT_PARAM SLACK SLACK_K SLACK_METHOD MAX_PRIO TEST_PATH DEBUG

###############################################################################
#
# Rules used to build the programs.
#
all:
	+@echo "-- Target: $(TARGET)"
	+@echo "-- Building: $(APP_NAME)"
	+@echo "-- DEBUG: $(DEBUG)"
	@$(MAKE) $(MAKE_FLAGS) -f Makefile.mk
	+@echo "Done!"

clean:
	@$(MAKE) $(MAKE_FLAGS) -f Makefile.mk clean
