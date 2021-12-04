# Build for edu-ciaa-nxp

BUILD_DIR = ../../build
SAPI_VERSION = v0.5.2
LPC_VERSION = v3.02

###############################################################################
#
# Source code.
#
SRC += $(wildcard $(APP_NAME)/*.c)
SRC += $(wildcard common/*.c)
SRC += $(wildcard ../utils/*.c)

OBJECTS = $(SRC:.c=.o)

###############################################################################
#
# Paths to the required headers.
#
# lpc
LPC_INCLUDE_PATH += -I../../libs/LPCOpen/v3.02/boards/edu_ciaa_nxp/inc/
LPC_INCLUDE_PATH += -I../../libs/LPCOpen/v3.02/boards/inc/
LPC_INCLUDE_PATH += -I../../libs/LPCOpen/v3.02/lpc_chip_43xx/inc/
LPC_INCLUDE_PATH += -I../../libs/LPCOpen/v3.02/lpc_chip_43xx/usbd_rom/
LPC_INCLUDE_PATH += -I../../libs/LPCOpen/v3.02/cmsis_core/inc

# freertos
FREERTOS_INCLUDE_PATH += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/include
FREERTOS_INCLUDE_PATH += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM4F

# sapi library
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/abstract_modules/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/base/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/board/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/display/fonts/greek_chars_5x7/inc
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/display/fonts/icon_chars_5x7/inc
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/display/fonts/inc
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/display/lcd/inc
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/display/led_segments/7segment/inc
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/imu/mpu60X0/inc
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/imu/mpu9250/inc
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/keypad/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/led_rgb/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/magnetometer/hmc5883l/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/magnetometer/qmc5883l/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/memory/eeprom/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/motor/servo/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/temperature_humudity/dht11/inc
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/ultrasonic/hcsr04/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/external_peripherals/wifi/esp8266_at/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/soc/core/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/soc/peripherals/inc/
SAPI_INCLUDE_PATH  += -I../../libs/sAPI/$(SAPI_VERSION)/soc/peripherals/usb/device/inc/

# applicaton
INCLUDE_PATHS += -I.
INCLUDE_PATHS += -I./$(APP_NAME)
INCLUDE_PATHS += -I./common
INCLUDE_PATHS += -I./../utils
INCLUDE_PATHS += -I../../slack/
INCLUDE_PATHS += -I../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)
INCLUDE_PATHS += $(LPC_INCLUDE_PATH)
INCLUDE_PATHS += $(SAPI_INCLUDE_PATH)
INCLUDE_PATHS += $(FREERTOS_INCLUDE_PATH)

###############################################################################
#
# Paths to the required libraries (*.a files)
#
SAPI_LIBRARY_PATH = ../../libs/sAPI/$(SAPI_VERSION)
FREERTOS_LIBRARY_PATH = ../../libs/FreeRTOS
LPC_LIBRARY_PATH = ../../libs/LPCOpen/$(LPC_VERSION)
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
LINKER_SCRIPT = ../../board/edu-ciaa-nxp/ldscript/edu_ciaa_lpc4337.ld

###############################################################################
#
# Tracealyzer sources, include paths and symbols.
#
ifeq ($(TZ), 1)
  CC_SYMBOLS += -DTRACEALYZER
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
CPU += -mcpu=cortex-m4
CPU += -mthumb
CPU += -mfloat-abi=hard
CPU += -mfpu=fpv4-sp-d16
CPU += -specs=nano.specs
CPU += -specs=nosys.specs

CC_FLAGS += $(CPU)

CC_SYMBOLS += -DCORE_M4
CC_SYMBOLS += -D__USE_LPCOPEN
CC_SYMBOLS += -DTARGET_EDU_CIAA_NXP

###############################################################################
#
# Symbols required by the sAPI
#
CC_SYMBOLS += -DBOARD=edu_ciaa_nxp
CC_SYMBOLS += -DCHIP_LPC43XX

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
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk clean APP_DIR=$(APP_NAME) USE_SLACK=1 TZ=$(TZ) TEST=0
	@rm -f $(BUILD_DIR)/$(EXAMPLE).bin $(BUILD_DIR)/$(EXAMPLE).elf $(OBJECTS) $(DEPS)

.c.o:
	+@echo "[App] Compile: $<"
	@$(CC) $(COMMON_FLAGS) $(C_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<	

$(BUILD_DIR)/$(EXAMPLE).elf: $(OBJECTS)
	@$(MAKE) $(MAKE_FLAGS) -C $(LPC_LIBRARY_PATH) -f Makefile.mk
	@$(MAKE) $(MAKE_FLAGS) -C $(SAPI_LIBRARY_PATH) -f Makefile.mk
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
