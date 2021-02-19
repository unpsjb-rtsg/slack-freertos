# Build for mbed lpc1768

BUILD_DIR = ../build

###############################################################################
#
# Board object files required.
#
SYS_OBJECTS += ../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/board.o
SYS_OBJECTS += ../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/cmsis_nvic.o
SYS_OBJECTS += ../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/retarget.o
SYS_OBJECTS += ../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/startup_LPC17xx.o
SYS_OBJECTS += ../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/system_LPC17xx.o

###############################################################################
#
# Paths to the required headers.
#
# mbed
MBED_INCLUDE_PATHS += -I../board/lpc1768
MBED_INCLUDE_PATHS += -I../board/lpc1768/TARGET_LPC1768
MBED_INCLUDE_PATHS += -I../board/lpc1768/TARGET_LPC1768/TARGET_NXP
MBED_INCLUDE_PATHS += -I../board/lpc1768/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/
MBED_INCLUDE_PATHS += -I../board/lpc1768/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/device
MBED_INCLUDE_PATHS += -I../board/lpc1768/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/TARGET_MBED_LPC1768

# freertos
FREERTOS_INCLUDE_PATHS += -I../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/include
FREERTOS_INCLUDE_PATHS += -I../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3

# application
INCLUDE_PATHS += -I.
INCLUDE_PATHS += -I../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)
INCLUDE_PATHS += $(FREERTOS_INCLUDE_PATHS) 
INCLUDE_PATHS += $(MBED_INCLUDE_PATHS) 

###############################################################################
#
# Paths to the required libraries (*.a files).
#
FREERTOS_LIBRARY_PATH = ../libs/FreeRTOS
LIBRARY_PATHS += -L../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM 
LIBRARY_PATHS += -L$(FREERTOS_LIBRARY_PATH)
LIBRARIES = -lmbed -lfreertos

###############################################################################
#
# Linker script used to build the binary.
#
LINKER_SCRIPT = ../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/LPC1768.ld

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
CC_SYMBOLS += -DFREERTOS_KERNEL_VERSION_NUMBER_MAJOR=$(FREERTOS_KERNEL_VERSION_NUMBER_MAJOR)
CC_SYMBOLS += -DMAX_PRIO=$(MAX_PRIO)
CC_SYMBOLS += -DKERNEL_TEST=$(KERNEL_TEST)

###############################################################################
#
# Flags and symbols required by the linker.
#
LD_FLAGS = $(CPU) -Wl,--gc-sections -u _printf_float -u _scanf_float
LD_SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys

ifeq ($(SLACK), 1)
    # Replace these functions
    WRAP = -Wl,--wrap=vTaskDelayUntil -Wl,--wrap=xTaskIncrementTick
endif

###############################################################################
#
# Export symbols required by other makefiles.
#
export CPU CC_SYMBOLS MBED_INCLUDE_PATHS COMMON_FLAGS CC_SYMBOLS DEBUG

# Required by the FreeRTOS makefile
export TEST_PATH TASK_COUNT_PARAM RELEASE_COUNT_PARAM FREERTOS_KERNEL_VERSION_NUMBER FREERTOS_KERNEL_VERSION_NUMBER_MAJOR SLACK_METHOD SLACK_K MAX_PRIO

###############################################################################
#
# Rules to build the example program.
#
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
ELFS = $(OBJS:.o=.elf)
BINS = $(OBJS:.o=.bin)

all: freertos $(ELFS) $(BINS)
    
clean:
	+@echo "[App] Cleaning binary files..."
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk clean TEST=1 USE_SLACK=$(SLACK) TZ=0	
	@$(RM) -f $(OBJS) $(ELFS) $(BINS) $(DEPS)	
    
freertos:
	+@echo "[FreeRTOS] Building FreeRTOS $(FREERTOS_KERNEL_VERSION_NUMBER) library..."    
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk TEST=1 USE_SLACK=$(SLACK) TZ=0
	+@echo "[FreeRTOS] Done!"

.cpp.o:
	+@echo "[App] Compile: $<"
	@$(CPP) $(COMMON_FLAGS) $(CPP_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<	
    
%.elf: %.o $(SYS_OBJECTS)
	@$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(LD_SYS_LIBS) $(WRAP)
    
%.bin: %.elf
	@$(OBJCOPY) -S -O binary $< $@

DEPS = $(OBJS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
