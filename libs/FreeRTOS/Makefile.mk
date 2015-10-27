# FreeRTOS library Makefile

GCC_BIN?=$(GCC_BIN_PATH)

PROJECT = libfreertos
OBJECTS = ./queue.o ./list.o ./portable/heap_1.o ./portable/port.o
INCLUDE_PATHS = -I. -I./include -I./portable -I$(APP_DIR)

ifeq ($(USE_SLACK), 1)
  OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/tasks.o
  INCLUDE_PATHS += -I../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)
else
  OBJECTS += ./tasks.o
endif

ifeq ($(TZ), 1)
  OBJECTS += ../Tracealizer/trcBase.o ../Tracealizer/trcHardwarePort.o ../Tracealizer/trcKernel.o ../Tracealizer/trcKernelPortFreeRTOS.o ../Tracealizer/trcUser.o
  MBED_INCLUDE_PATHS = -I../../libs/mbed/TARGET_LPC1768 -I../../libs/mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM -I../../libs/mbed/TARGET_LPC1768/TARGET_NXP -I../../libs/mbed/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X -I../../libs/mbed/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/TARGET_MBED_LPC1768
  INCLUDE_PATHS += -I../Tracealizer/Include -I../Tracealizer/ConfigurationTemplate $(MBED_INCLUDE_PATHS)  
endif

############################################################################### 
AR		= $(GCC_BIN)arm-none-eabi-ar
CC      = $(GCC_BIN)arm-none-eabi-gcc

CPU = -mcpu=cortex-m3 -mthumb
# removed -fno-common
CC_FLAGS = $(CPU) -c -fmessage-length=0 -fno-exceptions -ffunction-sections -fdata-sections -fno-builtin -Wall 
CC_FLAGS += -MMD -MP
CC_SYMBOLS = -D__REDLIB__ -D__CODE_RED -DTARGET_LPC1768 -DTARGET_M3 -DTARGET_NXP -DTARGET_LPC176X -DTARGET_MBED_LPC1768 -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC -D__CORTEX_M3 -DARM_MATH_CM3

# Required for the test (see test directory)
ifeq ($(TEST), 1)
  CC_SYMBOLS +=  -DTASK_CNT=$(TASK_CNT) -DRELEASE_CNT=$(RELEASE_CNT) -DSLACK=$(SLACK) -DSLACK_K=$(SLACK_K) -DSLACK_METHOD=$(SLACK_METHOD)
endif

AR_FLAGS = -r

ifeq ($(DEBUG), 1)
  CC_FLAGS += -Og -g3
  CC_SYMBOLS += -DDEBUG
else
  CC_FLAGS += -Os
  CC_SYMBOLS += -DNDEBUG
endif

all: $(PROJECT).a

clean:
	@$(RM) -f $(PROJECT).bin $(PROJECT).a $(OBJECTS) $(DEPS)
	@echo "Cleaning FreeRTOS object files..."

.c.o:
	@$(CC) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99 $(INCLUDE_PATHS) -o $@ $<
	@echo "CC $<"

$(PROJECT).a: $(OBJECTS) $(SYS_OBJECTS)
	@$(AR) $(AR_FLAGS) $@ $^ -c
	@echo "AR $@"

DEPS = $(OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
