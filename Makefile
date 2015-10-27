#
# Targets:
# Examples (see examples directory):
#   Example1
#   Example2
#   Example3
TARGET=example3

FREERTOS_KERNEL_VERSION_NUMBER=v8.1.2

MAKE_DIR = $(PWD)

GCC_BIN = #/path/to/gcc/bin

############################################################################### 
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy

FREERTOS_DIR := $(MAKE_DIR)/libs/FreeRTOS 
EXAMPLES_DIR := $(MAKE_DIR)/examples
INCLUDE_DIR := $(MAKE_DIR)/include

export AS CC CPP LD OBJCOPY FREERTOS_KERNEL_VERSION_NUMBER

example1:
	@echo " Building example Test1 ... "
	@$(MAKE) -C libs/FreeRTOS/ -f Makefile.mk APP_DIR=../../examples/lpc1768/$(TARGET)/ USE_SLACK=1
	@$(MAKE) -C examples/lpc1768/ -f Makefile.mk TARGET=$(TARGET)
	
example1_clean:
	@echo " Cleaning example Test1 ... "
	@$(MAKE) -C examples/lpc1768/ -f Makefile.mk clean TARGET=$(TARGET)
	@$(MAKE) -C libs/FreeRTOS/ -f Makefile.mk clean USE_SLACK=1
	
example2:
	@echo " Building example Test2 ... "
	@$(MAKE) -C libs/FreeRTOS/ -f Makefile.mk APP_DIR=../../examples/lpc1768/$(TARGET)/ USE_SLACK=1
	@$(MAKE) -C examples/lpc1768/ -f Makefile.mk TARGET=$(TARGET)
	
example2_clean:
	@echo " Cleaning example Test2 ... "
	@$(MAKE) -C examples/lpc1768/ -f Makefile.mk clean TARGET=$(TARGET)
	@$(MAKE) -C libs/FreeRTOS/ -f Makefile.mk clean USE_SLACK=1
	
example3:
	@echo " Building example Test3 ... "
	@$(MAKE) -C libs/FreeRTOS/ -f Makefile.mk APP_DIR=../../examples/lpc1768/$(TARGET)/ USE_SLACK=1 TZ=1
	@$(MAKE) -C examples/lpc1768/ -f Makefile.mk TARGET=$(TARGET) TZ=1
	
example3_clean:
	@echo " Cleaning example Test3 ... "
	@$(MAKE) -C examples/lpc1768/ -f Makefile.mk clean TARGET=$(TARGET) TZ=1
	@$(MAKE) -C libs/FreeRTOS/ -f Makefile.mk clean USE_SLACK=1 TZ=1

all: $(TARGET)
	
clean: $(TARGET)_clean