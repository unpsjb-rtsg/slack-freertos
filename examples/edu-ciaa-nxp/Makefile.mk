# This file is based on the Makefile automagically generated by mbed.org. 
# For more information, see http://mbed.org/handbook/Exporting-to-GCC-ARM-Embedded

BUILD_DIR = ../../build

###############################################################################
#
# Source code.
#
SRC += $(wildcard $(APP_NAME)/*.c)
SRC += $(wildcard utils/*.c)
SRC += $(wildcard common/*.c)

OBJECTS = $(SRC:.c=.o)

###############################################################################
#
# Paths to the required headers.
#
# lpc
LPC_INCLUDE_PATH += -I../../board/edu-ciaa-nxp/lpc/lpc_chip_43xx/inc
LPC_INCLUDE_PATH += -I../../board/edu-ciaa-nxp/lpc/lpc_board_ciaa_edu_4337/inc
LPC_INCLUDE_PATH += -I../../board/edu-ciaa-nxp/lpc/lpc_chip_43xx/inc/usbd/ 

# freertos
FREERTOS_INCLUDE_PATH += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/include
FREERTOS_INCLUDE_PATH += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM4F

# sapi library
SAPI_INCLUDE_PATH += -I../../libs/sapi_bm/inc

# applicaton
INCLUDE_PATHS += -I.
INCLUDE_PATHS += -I./$(APP_NAME)
INCLUDE_PATHS += -I./utils
INCLUDE_PATHS += -I./common
INCLUDE_PATHS += -I../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)
INCLUDE_PATHS += $(LPC_INCLUDE_PATH)
INCLUDE_PATHS += $(SAPI_INCLUDE_PATH)
INCLUDE_PATHS += $(FREERTOS_INCLUDE_PATH)

###############################################################################
#
# Paths to the required libraries (*.a files)
#
SAPI_LIBRARY_PATH = ../../libs/sapi_bm
FREERTOS_LIBRARY_PATH = ../../libs/FreeRTOS
LPC_LIBRARY_PATH = ../../board/edu-ciaa-nxp/lpc/
LIBRARY_PATHS += -L$(FREERTOS_LIBRARY_PATH)
LIBRARY_PATHS += -L$(LPC_LIBRARY_PATH)
LIBRARY_PATHS += -L$(SAPI_LIBRARY_PATH)
LIBRARIES += -lfreertos
LIBRARIES += -llpc
LIBRARIES += -lsapi

###############################################################################
#
# Linker script used to build the binary.
#
LINKER_SCRIPT = ../../board/edu-ciaa-nxp/ldscript/ciaa_lpc4337.ld

###############################################################################
#
# Tracealyzer sources, include paths and symbols.
#
ifeq ($(APP_NAME), example3)
  TZ = 1
  ifeq ($(TRACEALIZER_VERSION_NUMBER), v3.0.2)
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/Include
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/ConfigurationTemplate
    CC_FLAGS += -DTRACEALYZER_v3_0_2
  endif
  ifeq ($(TRACEALIZER_VERSION_NUMBER), v3.1.3)
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/include
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/config
    CC_FLAGS += -DTRACEALYZER_v3_1_3
  endif
else
  TZ = 0
endif

###############################################################################
#
# Flags and symbols required by the compiler.
#
CPU += -mcpu=cortex-m4
CPU += -mthumb
CPU += -mfloat-abi=hard
CPU += -mfpu=fpv4-sp-d16

CC_FLAGS += $(CPU)

CC_SYMBOLS += -DCORE_M4
CC_SYMBOLS += -D__USE_LPCOPEN
CC_SYMBOLS += -DTARGET_EDU_CIAA_NXP

###############################################################################
#
# Flags and symbols required by the linker
#
LD_FLAGS += $(CPU)
LD_FLAGS += -nostartfiles
LD_FLAGS += -Wl,-gc-sections
LD_FLAGS += $(foreach l, $(LIBS), -l$(l))

###############################################################################
#
# Export variables to be used by others Makefile.mk files.
#
export TARGET APP_NAME LPC_INCLUDE_PATH CC_FLAGS CC_SYMBOLS

###############################################################################
#
# Rules used to build the example programs.
#
all: $(BUILD_DIR)/$(EXAMPLE).bin size

clean:
	+@echo "Cleaning $(TARGET) files..."
	@$(MAKE) $(MAKE_FLAGS) -C $(LPC_LIBRARY_PATH) -f Makefile.mk clean
	@$(MAKE) $(MAKE_FLAGS) -C $(SAPI_LIBRARY_PATH) -f Makefile.mk clean
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk clean APP_DIR=$(APP_NAME) USE_SLACK=1 TZ=$(TZ)
	@rm -f $(BUILD_DIR)/$(EXAMPLE).bin $(BUILD_DIR)/$(EXAMPLE).elf $(OBJECTS) $(DEPS)

.c.o:
	+@echo "Compile: $<"
	@$(CC) $(COMMON_FLAGS) $(C_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<	

$(BUILD_DIR)/$(EXAMPLE).elf: $(OBJECTS)
	@$(MAKE) $(MAKE_FLAGS) -C $(LPC_LIBRARY_PATH) -f Makefile.mk
	@$(MAKE) $(MAKE_FLAGS) -C $(SAPI_LIBRARY_PATH) -f Makefile.mk
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk APP_DIR=$(APP_NAME) USE_SLACK=1 TZ=$(TZ)
	+@echo "Linking: $@"
	@$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES)

$(BUILD_DIR)/$(EXAMPLE).bin: $(BUILD_DIR)/$(EXAMPLE).elf
	+@echo "Binary: $@"
	@$(OBJCOPY) -O binary $< $@
	
size: $(BUILD_DIR)/$(EXAMPLE).elf
	$(SIZE) $<

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)