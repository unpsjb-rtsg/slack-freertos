# Build for mbed lpc1768

BUILD_DIR = ../../build

###############################################################################
#
# Other object files required.
#
SYS_OBJECTS += ../../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/board.o
SYS_OBJECTS += ../../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/cmsis_nvic.o
SYS_OBJECTS += ../../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/retarget.o
SYS_OBJECTS += ../../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/startup_LPC17xx.o
SYS_OBJECTS += ../../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/system_LPC17xx.o

###############################################################################
#
# Paths to the required headers.
#
# mbed
MBED_INCLUDE_PATHS += -I../../../board/lpc1768
MBED_INCLUDE_PATHS += -I../../../board/lpc1768/TARGET_LPC1768
MBED_INCLUDE_PATHS += -I../../../board/lpc1768/TARGET_LPC1768/TARGET_NXP
MBED_INCLUDE_PATHS += -I../../../board/lpc1768/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/
MBED_INCLUDE_PATHS += -I../../../board/lpc1768/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/device
MBED_INCLUDE_PATHS += -I../../../board/lpc1768/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/TARGET_MBED_LPC1768

# freertos
FREERTOS_INCLUDE_PATHS += -I../../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/include
FREERTOS_INCLUDE_PATHS += -I../../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3

# application
INCLUDE_PATHS += -I.
INCLUDE_PATHS += -I../../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)
INCLUDE_PATHS += $(FREERTOS_INCLUDE_PATHS) 
INCLUDE_PATHS += $(MBED_INCLUDE_PATHS) 

###############################################################################
#
# Paths to the required libraries (*.a files).
#
FREERTOS_LIBRARY_PATH = ../../../libs/FreeRTOS
LIBRARY_PATHS += -L../../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM 
LIBRARY_PATHS += -L$(FREERTOS_LIBRARY_PATH)
LIBRARIES = -lmbed -lfreertos

###############################################################################
#
# Linker script used to build the binary.
#
LINKER_SCRIPT = ../../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/LPC1768.ld

###############################################################################
#
# Flags and symbols required by the compiler.
#
CPU = -mcpu=cortex-m3 -mthumb
CC_FLAGS += $(CPU)
CC_SYMBOLS += -DTARGET_LPC1768
CC_SYMBOLS += -DTARGET_M3
CC_SYMBOLS += -DTARGET_NXP
CC_SYMBOLS += -DTARGET_LPC176X
CC_SYMBOLS += -DTARGET_MBED_LPC1768
CC_SYMBOLS += -DTOOLCHAIN_GCC_ARM
CC_SYMBOLS += -DTOOLCHAIN_GCC
CC_SYMBOLS += -D__CORTEX_M3
CC_SYMBOLS += -DARM_MATH_CM3
CC_SYMBOLS += -D__MBED__=1 
CC_SYMBOLS += -DBATCH_TEST=$(BATCH_TEST) 
CC_SYMBOLS += -DMAX_PRIO=$(MAX_PRIO)
CC_SYMBOLS += -DTASK_COUNT_PARAM=$(TASK_COUNT_PARAM)
CC_SYMBOLS += -DRELEASE_COUNT_PARAM=$(RELEASE_COUNT_PARAM)
CC_SYMBOLS += -DSLACK=$(SLACK)
CC_SYMBOLS += -DSLACK_K=$(SLACK_K)
CC_SYMBOLS += -DSLACK_METHOD=$(SLACK_METHOD)
CC_SYMBOLS += -DFREERTOS_VERSION=$(FREERTOS_KERNEL_VERSION_NUMBER_MAJOR)
CC_SYMBOLS += -DMAX_PRIO=$(MAX_PRIO)
CC_SYMBOLS += -DKERNEL_TEST=$(KERNEL_TEST)

###############################################################################
#
# Flags and symbols required by the linker.
#
LD_FLAGS = $(CPU) -Wl,--gc-sections -u _printf_float -u _scanf_float
LD_SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys

# Replace these functions
ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v10.4.1)
WRAP = -Wl,--wrap=vTaskDelayUntil -Wl,--wrap=xTaskIncrementTick
endif
ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v10.3.1)
WRAP = -Wl,--wrap=vTaskDelayUntil -Wl,--wrap=xTaskIncrementTick
endif
ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), v10.2.1)
WRAP = -Wl,--wrap=vTaskDelayUntil -Wl,--wrap=xTaskIncrementTick
endif

export CPU CC_SYMBOLS MBED_INCLUDE_PATHS

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
ELFS = $(OBJS:.o=.elf)
BINS = $(OBJS:.o=.bin)

###############################################################################
#
# Rules to build the example program.
#
all: freertos $(ELFS) $(BINS)
    
clean:
	+@echo "Cleaning binary files..."
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk clean APP_DIR=$(APP_NAME) USE_SLACK=1 TZ=$(TZ)	
	@$(RM) -f $(OBJS) $(ELFS) $(BINS) $(DEPS)	
    
freertos:
	+@echo "[FreeRTOS] Building FreeRTOS $(FREERTOS_KERNEL_VERSION_NUMBER) library..."    
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk APP_DIR=$(APP_NAME) USE_SLACK=1 TZ=$(TZ) TEST_INCLUDE_PATHS=$(INCLUDE_PATHS) TEST=1 KERNEL_TEST=$(KERNEL_TEST) TASK_COUNT_PARAM=$(TASK_COUNT_PARAM)
	+@echo "[FreeRTOS] Done!"

slack_test.o:
	+@echo "[App] Compile: $<"
	@$(CC)  $(COMMON_FLAGS) $(C_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ ./../../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)/slack_tests.c

.cpp.o:
	+@echo "[App] Compile: $<"
	@$(CPP) $(COMMON_FLAGS) $(CPP_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<
    
%.elf: %.o $(SYS_OBJECTS) slack_test.o
	@$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(LD_SYS_LIBS) $(WRAP)
    
%.bin: %.elf
	@$(OBJCOPY) -S -O binary $< $@

DEPS = $(OBJS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
