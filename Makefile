#
# Modify example to build in Makefile.mine. 
#
-include Makefile.mine

###############################################################################
# Boiler-plate

# cross-platform directory manipulation
ifeq ($(shell echo $$OS),$$OS)
    MAKEDIR = if not exist "$(1)" mkdir "$(1)"
    RM = rmdir /S /Q "$(1)"
else
    MAKEDIR = '$(SHELL)' -c "mkdir -p \"$(1)\""
    RM = '$(SHELL)' -c "rm -rf \"$(1)\""
endif

############################################################################### 
GCC_BIN =
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
SIZE    = $(GCC_BIN)arm-none-eabi-size

MAKE_DIR = $(PWD)
FREERTOS_DIR := $(MAKE_DIR)/libs/FreeRTOS 
EXAMPLES_DIR := $(MAKE_DIR)/examples
INCLUDE_DIR := $(MAKE_DIR)/include

MAKE_FLAGS += --no-print-directory

ifeq ($(TARGET), example3)
TZ = 1
else
TZ = 0
endif

export AS CC CPP LD OBJCOPY SIZE FREERTOS_KERNEL_VERSION_NUMBER TRACEALIZER_VERSION_NUMBER MAKEDIR RM DEBUG

$(TARGET):
	+@echo "-- Building $(TARGET)"
	+@echo "-- Build FreeRTOS library"
	@$(MAKE) $(MAKE_FLAGS) -C libs/FreeRTOS/ -f Makefile.mk APP_DIR=../../examples/lpc1768/$(TARGET)/ USE_SLACK=1 TZ=$(TZ)
	+@echo "-- Build $(TARGET) program"
	@$(MAKE) $(MAKE_FLAGS) -C examples/lpc1768/ -f Makefile.mk TARGET=$(TARGET) TZ=$(TZ)

$(TARGET)_clean:
	+@echo "-- Cleaning $(TARGET)"
	@$(MAKE) $(MAKE_FLAGS) -C libs/FreeRTOS/ -f Makefile.mk clean USE_SLACK=1 TZ=$(TZ)
	@$(MAKE) $(MAKE_FLAGS) -C examples/lpc1768/ -f Makefile.mk clean TARGET=$(TARGET) TZ=$(TZ)

all: $(TARGET)

clean: $(TARGET)_clean
