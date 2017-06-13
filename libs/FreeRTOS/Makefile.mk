# FreeRTOS library Makefile

GCC_BIN ?= $(GCC_BIN_PATH)

PROJECT = libfreertos

OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/queue.o 
OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/list.o 
OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/MemMang/heap_1.o 
OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3/port.o

INCLUDE_PATHS += -I./$(FREERTOS_KERNEL_VERSION_NUMBER)
INCLUDE_PATHS += -I./$(FREERTOS_KERNEL_VERSION_NUMBER)/include
INCLUDE_PATHS += -I./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3
INCLUDE_PATHS += -I$(APP_DIR)

ifeq ($(USE_SLACK), 1)
  OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/tasks.o
  ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v9.0.0)
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack.o
  endif
  INCLUDE_PATHS += -I../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)
else
  OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/tasks.o
endif

ifeq ($(TEST), 1)
  ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v9.0.0)
     OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack_tests.o
  endif
endif

ifeq ($(TZ), 1)
  MBED_INCLUDE_PATHS += -I../../libs/mbed/TARGET_LPC1768
  MBED_INCLUDE_PATHS += -I../../libs/mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM
  MBED_INCLUDE_PATHS += -I../../libs/mbed/TARGET_LPC1768/TARGET_NXP
  MBED_INCLUDE_PATHS += -I../../libs/mbed/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X
  MBED_INCLUDE_PATHS += -I../../libs/mbed/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/TARGET_MBED_LPC1768
  INCLUDE_PATHS += $(MBED_INCLUDE_PATHS)
  
  ifeq ($(TRACEALIZER_VERSION_NUMBER), v3.0.2)
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcBase.o
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcHardwarePort.o
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcKernel.o
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcKernelPortFreeRTOS.o
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcUser.o
  
    INCLUDE_PATHS += -I../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/Include
    INCLUDE_PATHS += -I../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/ConfigurationTemplate
    
    CC_SYMBOLS += -DTRACEALYZER_v3_0_2    
  endif
  
  ifeq ($(TRACEALIZER_VERSION_NUMBER), v3.1.3)
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcKernelPort.o
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcSnapshotRecorder.o
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcStreamingRecorder.o
    
    INCLUDE_PATHS += -I../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/config
    INCLUDE_PATHS += -I../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/include
    
    CC_SYMBOLS += -DTRACEALYZER_v3_1_3
  endif  
endif

############################################################################### 
AR = $(GCC_BIN)arm-none-eabi-ar
CC = $(GCC_BIN)arm-none-eabi-gcc

CPU = -mcpu=cortex-m3 -mthumb
# removed -fno-common
CC_FLAGS = $(CPU) -c -fmessage-length=0 -fno-exceptions -ffunction-sections -fdata-sections -fno-builtin -Wall 
CC_FLAGS += -MMD -MP
CC_SYMBOLS += -D__REDLIB__ -D__CODE_RED -DTARGET_LPC1768 -DTARGET_M3 -DTARGET_NXP -DTARGET_LPC176X -DTARGET_MBED_LPC1768 -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC -D__CORTEX_M3 -DARM_MATH_CM3

# Required for the test (see test directory)
ifeq ($(TEST), 1)
  CC_SYMBOLS += -DTASK_COUNT_PARAM=$(TASK_COUNT_PARAM)
  CC_SYMBOLS += -DRELEASE_COUNT_PARAM=$(RELEASE_COUNT_PARAM)
  CC_SYMBOLS += -DSLACK=$(SLACK) 
  CC_SYMBOLS += -DSLACK_K=$(SLACK_K)
  CC_SYMBOLS += -DSLACK_METHOD=$(SLACK_METHOD)
  CC_SYMBOLS += -DFREERTOS_VERSION=$(FREERTOS_VERSION)
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
	+@echo "Cleaning FreeRTOS object files..."
	@rm -f $(PROJECT).bin $(PROJECT).a $(OBJECTS) $(DEPS)	

.c.o:
	+@echo "Compile: $<"
	@$(CC) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99 $(INCLUDE_PATHS) -o $@ $<

$(PROJECT).a: $(OBJECTS) $(SYS_OBJECTS)
	+@echo "Linking: $@"
	@$(AR) $(AR_FLAGS) $@ $^ -c

DEPS = $(OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
