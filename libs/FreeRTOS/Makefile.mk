# FreeRTOS library Makefile

GCC_BIN ?= $(GCC_BIN_PATH)

PROJECT = libfreertos

###############################################################################
#
# FreeRTOS source code.
#
OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/queue.o 
OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/list.o
OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/timers.o
OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/MemMang/heap_1.o 
ifeq ($(TARGET), lpc1768)
  OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3/port.o
endif
ifeq ($(TARGET), lm3s6965evb)
  OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3/port.o
endif
ifeq ($(TARGET), edu-ciaa-nxp)
  OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM4F/port.o
endif
ifeq ($(TARGET), frdm-k64f)
  OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM4F/port.o
endif

###############################################################################
#
# Paths to the required headers.
#
ifeq ($(TEST), 0)
    INCLUDE_PATHS += -I../../examples/$(TARGET)/
    INCLUDE_PATHS += -I../../examples/$(TARGET)/$(APP_NAME)
else
    INCLUDE_PATHS += -I../../test/$(TEST_PATH)
endif
INCLUDE_PATHS += -I./$(FREERTOS_KERNEL_VERSION_NUMBER)
INCLUDE_PATHS += -I./$(FREERTOS_KERNEL_VERSION_NUMBER)/include
ifeq ($(TARGET), lpc1768)
  INCLUDE_PATHS += -I./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3
endif
ifeq ($(TARGET), lm3s6965evb)
  INCLUDE_PATHS += -I./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3
endif
ifeq ($(TARGET), edu-ciaa-nxp)
  INCLUDE_PATHS += -I./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM4F
  INCLUDE_PATHS += $(LPC_INCLUDE_PATH)
endif
ifeq ($(TARGET), frdm-k64f)
  INCLUDE_PATHS += -I./$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM4F
endif

###############################################################################
#
# Slack Stealing framework source and headers.
#
ifeq ($(USE_SLACK), 1)
  ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v8.1.2)
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/tasks.o
  endif  
  ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v9.0.0)
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/tasks.o
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack.o
  endif
  ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v10.2.1)
    OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/tasks.o
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack.o
  endif
  ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v10.3.1)
    OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/tasks.o
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack.o
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack_algorithms/ss_davis.o
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack_algorithms/ss_fixed.o
    CC_SYMBOLS += -DSS_ALGORITHM=$(SS_ALGORITHM)
  endif
  ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v10.4.1)
    OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/tasks.o
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack.o
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack_algorithms/ss_davis.o
    OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack_algorithms/ss_fixed.o
    CC_SYMBOLS += -DSS_ALGORITHM=$(SS_ALGORITHM)
  endif
  INCLUDE_PATHS += -I../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)  
else
  OBJECTS += ./$(FREERTOS_KERNEL_VERSION_NUMBER)/tasks.o
endif

###############################################################################
#
# Required for tests (see test directory)
#
ifeq ($(TEST), 1)
  ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v9.0.0)
     OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack_tests.o
  endif
  ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v10.4.1)
     OBJECTS += ../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack_tests.o
  endif

  CC_SYMBOLS += -DTASK_COUNT_PARAM=$(TASK_COUNT_PARAM)
  CC_SYMBOLS += -DRELEASE_COUNT_PARAM=$(RELEASE_COUNT_PARAM)
  CC_SYMBOLS += -DSLACK=$(SLACK) 
  CC_SYMBOLS += -DSLACK_K=$(SLACK_K)
  CC_SYMBOLS += -DSLACK_METHOD=$(SLACK_METHOD)
  CC_SYMBOLS += -DFREERTOS_VERSION=$(FREERTOS_VERSION)
endif

###############################################################################
#
# Tracealyzer sources, include paths and symbols.
#
ifeq ($(TZ), 1)
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
    
    ifeq ($(TARGET), frdm-k64f)
      OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/streamports/JLink_RTT/SEGGER_RTT_Printf.o
      OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/streamports/JLink_RTT/SEGGER_RTT.o
      INCLUDE_PATHS += -I../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/streamports/JLink_RTT/include
    endif
    
    CC_SYMBOLS += -DTRACEALYZER_v3_1_3
  endif
  
  ifeq ($(TRACEALIZER_VERSION_NUMBER), v3.3.1)
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcKernelPort.o
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcSnapshotRecorder.o
    OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/trcStreamingRecorder.o
    
    INCLUDE_PATHS += -I../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/config
    INCLUDE_PATHS += -I../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/include
    
    ifeq ($(TARGET), frdm-k64f)      
      OBJECTS += ../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/streamports/JLink_RTT/SEGGER_RTT.o
      INCLUDE_PATHS += -I../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/streamports/JLink_RTT/include
    endif
    
    CC_SYMBOLS += -DTRACEALYZER_v3_3_1
  endif
  
  ifeq ($(TARGET), lpc1768)
    CC_SYMBOLS += -DTARGET_LPC1768
  endif
  ifeq ($(TARGET), frdm-k64f)
    CC_SYMBOLS += -DTARGET_K64F
  endif
  ifeq ($(TARGET), edu-ciaa-nxp)
    #CC_FLAGS = $(CFLAGS)
  endif
  ifeq ($(TARGET), lm3s6965evb)
  	CC_SYMBOLS += -DTARGET_LM3S6965EVB
  endif    
endif

###############################################################################
#
# Flags and symbols required by the linker.
#
AR_FLAGS = -r
CC_SYMBOLS += -DUSE_SLACK=$(USE_SLACK)

###############################################################################
#
# Rules to build FreeRTOS
#
all: $(PROJECT).a	

clean:
	+@echo "[FreeRTOS] Cleaning object files..."
	@rm -f $(PROJECT).bin $(PROJECT).a $(OBJECTS) $(DEPS)	

.c.o:
	+@echo "[FreeRTOS] Compile: $<"    
	@$(CC) $(CPU) $(COMMON_FLAGS) $(C_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<

$(PROJECT).a: $(OBJECTS)
	+@echo "[FreeRTOS] Linking: $@"
	@$(AR) $(AR_FLAGS) $@ $^ -c

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)
