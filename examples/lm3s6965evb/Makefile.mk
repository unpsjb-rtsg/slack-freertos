# Build for lm3s6965evb

BUILD_DIR = ../../build

###############################################################################
#
# Source code.
#
SRC += $(wildcard ./$(APP_NAME)/*.c)

OBJECTS = $(SRC:.c=.o)

###############################################################################
#
# Paths to the required headers.
#
# board drivers
DRIVERS += -I../../board/lm3s6965evb/drivers 

# freertos
FREERTOS_INCLUDE_PATH += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/include
FREERTOS_INCLUDE_PATH += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3

# applicaton
INCLUDE_PATHS += -I.
INCLUDE_PATHS += -I./$(APP_NAME)
INCLUDE_PATHS += -I./../utils
INCLUDE_PATHS += -I../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)
INCLUDE_PATHS += $(FREERTOS_INCLUDE_PATH)
INCLUDE_PATHS += $(DRIVERS)

###############################################################################
#
# Paths to the required libraries (*.a files)
#
FREERTOS_LIBRARY_PATH = ../../libs/FreeRTOS
LIBRARY_PATHS += -L$(FREERTOS_LIBRARY_PATH)
LIBRARY_PATHS += -L../../board/lm3s6965evb/drivers/arm-none-eabi-gcc
LIBRARIES += -lfreertos
LIBRARIES += -ldriver
LIBRARIES += -lgr


###############################################################################
#
# Linker script used to build the binary.
#
LINKER_SCRIPT = ../../board/lm3s6965evb/linker.ld

###############################################################################
#
# Tracealyzer sources, include paths and symbols.
#
ifeq ($(TZ), 1)
  CC_SYMBOLS += -DTZ=1
  ifeq ($(TRACEALIZER_VERSION_NUMBER), v3.3.1)
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/include
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/config
    CC_SYMBOLS += -DTRACEALYZER_v3_3_1
  endif
endif

###############################################################################
#
# Flags and symbols required by the compiler.
#
CPU += -mcpu=cortex-m3
CPU += -mthumb

CC_FLAGS += $(CPU)
CC_SYMBOLS += -DTARGET_LM3S6965EVB
CC_SYMBOLS += -DTARGET_M3

###############################################################################
#
# Flags and symbols required by the linker
#
LD_FLAGS += $(CPU)
LD_FLAGS += -nostartfiles
LD_FLAGS += -Wl,-gc-sections
LD_FLAGS += $(foreach l, $(LIBS), -l$(l))

# Replace these functions
ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), 10.0.1)
WRAP = -Wl,--wrap=vTaskDelayUntil -Wl,--wrap=xTaskIncrementTick
endif
ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), 10.4.1)
WRAP = -Wl,--wrap=vTaskDelayUntil -Wl,--wrap=xTaskIncrementTick
endif
ifeq ($(FREERTOS_KERNEL_VERSION_NUMBER), 10.4.6)
WRAP = -Wl,--wrap=xTaskDelayUntil -Wl,--wrap=xTaskIncrementTick
endif

###############################################################################
#
# Export variables to be used by others Makefile.mk files.
#
export TARGET APP_NAME CC_FLAGS CC_SYMBOLS

###############################################################################
#
# Rules used to build the example programs.
#
all: $(BUILD_DIR)/$(EXAMPLE).bin size

clean:
	+@echo "Cleaning $(TARGET) files..."
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk clean APP_DIR=$(APP_NAME) USE_SLACK=1 TZ=$(TZ) TEST=0
	@rm -f $(BUILD_DIR)/$(EXAMPLE).bin $(BUILD_DIR)/$(EXAMPLE).elf $(OBJECTS) $(DEPS)

.c.o:
	+@echo "[App] Compile: $<"
	@$(CC) $(COMMON_FLAGS) $(C_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<	

$(BUILD_DIR)/$(EXAMPLE).elf: $(OBJECTS)
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk APP_DIR=$(APP_NAME) USE_SLACK=1 TZ=$(TZ) TEST=0
	+@echo "[App] Linking: $@"
	@$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(WRAP)

$(BUILD_DIR)/$(EXAMPLE).bin: $(BUILD_DIR)/$(EXAMPLE).elf
	+@echo "[App] Binary: $@"
	@$(OBJCOPY) -O binary $< $@
	
size: $(BUILD_DIR)/$(EXAMPLE).elf
	$(SIZE) $<

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)
