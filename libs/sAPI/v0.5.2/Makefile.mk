# sAPI (rtos) library Makefile

PROJECT = libsapi

###############################################################################
#
# sAPI library source code.
#
SRC += $(wildcard ./abstract_modules/src/*.c)
SRC += $(wildcard ./base/src/*.c)
SRC += $(wildcard ./board/src/*.c)
SRC += $(wildcard ./external_pheripherals/src/*.c)
SRC += $(wildcard ./soc/core/src/*.c)
SRC += $(wildcard ./soc/peripherals/src/*.c)
SRC += $(wildcard ./soc/peripherals/usb/device/src/*.c)
SRC += $(wildcard ./sys_newlib/src/*.c)
OBJECTS = $(SRC:.c=.o)

###############################################################################
#
# Paths to the required headers.
#
INCLUDE_PATHS += -I./abstract_modules/inc/
INCLUDE_PATHS += -I./base/inc/
INCLUDE_PATHS += -I./board/inc/
INCLUDE_PATHS += -I./soc/core/inc/
INCLUDE_PATHS += -I./soc/peripherals/inc/
INCLUDE_PATHS += -I./soc/peripherals/usb/device/inc/

INCLUDE_PATHS  += -I./external_peripherals/inc/
INCLUDE_PATHS  += -I./external_peripherals/display/fonts/greek_chars_5x7/inc
INCLUDE_PATHS  += -I./external_peripherals/display/fonts/icon_chars_5x7/inc
INCLUDE_PATHS  += -I./external_peripherals/display/fonts/inc
INCLUDE_PATHS  += -I./external_peripherals/display/lcd/inc
INCLUDE_PATHS  += -I./external_peripherals/display/led_segments/7segment/inc
INCLUDE_PATHS  += -I./external_peripherals/imu/mpu60X0/inc
INCLUDE_PATHS  += -I./external_peripherals/imu/mpu9250/inc
INCLUDE_PATHS  += -I./external_peripherals/keypad/inc/
INCLUDE_PATHS  += -I./external_peripherals/led_rgb/inc/
INCLUDE_PATHS  += -I./external_peripherals/magnetometer/hmc5883l/inc/
INCLUDE_PATHS  += -I./external_peripherals/magnetometer/qmc5883l/inc/
INCLUDE_PATHS  += -I./external_peripherals/memory/eeprom/inc/
INCLUDE_PATHS  += -I./external_peripherals/motor/servo/inc/
INCLUDE_PATHS  += -I./external_peripherals/temperature_humudity/dht11/inc
INCLUDE_PATHS  += -I./external_peripherals/ultrasonic/hcsr04/inc/
INCLUDE_PATHS  += -I./external_peripherals/wifi/esp8266_at/inc/

LPC_INCLUDE_PATH += -I../../LPCOpen/v3.02/boards/edu_ciaa_nxp/inc/
LPC_INCLUDE_PATH += -I../../LPCOpen/v3.02/boards/inc/
LPC_INCLUDE_PATH += -I../../LPCOpen/v3.02/lpc_chip_43xx/inc/
LPC_INCLUDE_PATH += -I../../LPCOpen/v3.02/lpc_chip_43xx/usbd_rom/
LPC_INCLUDE_PATH += -I../../LPCOpen/v3.02/cmsis_core/inc/

# freertos
FREERTOS_INCLUDE_PATHS += -I../../FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/include
FREERTOS_INCLUDE_PATHS += -I../../FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3

# app
APP_INCLUDE_PATHS += -I../../../examples/edu-ciaa-nxp/
APP_INCLUDE_PATHS += -I../../../examples/edu-ciaa-nxp/$(APP_NAME)/

INCLUDE_PATHS += $(LPC_INCLUDE_PATH)
INCLUDE_PATHS += $(FREERTOS_INCLUDE_PATHS)
INCLUDE_PATHS += $(APP_INCLUDE_PATHS)

###############################################################################
#
# Tracealyzer sources, include paths and symbols.
#
ifeq ($(TZ), 1)
  CC_SYMBOLS += -DTRACEALYZER
  ifeq ($(TRACEALIZER_VERSION_NUMBER), v3.3.1)
    INCLUDE_PATHS += -I../../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/include
    INCLUDE_PATHS += -I../../Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/config
    CC_SYMBOLS += -DTRACEALYZER_v3_3_1
  endif
endif

###############################################################################
#
# Flags and symbols required by the sAPI library.
#
CC_SYMBOLS += -DTICK_OVER_RTOS
CC_SYMBOLS += -DUSE_FREERTOS

###############################################################################
#
# Flags and symbols required by the linker.
#
AR_FLAGS = -r

###############################################################################
#
# Rules used to build sAPI.
#
all: $(PROJECT).a

clean:
	+@echo "[sAPI] Cleaning sAPI object files..."
	@rm -f $(PROJECT).a $(OBJECTS) $(DEPS)

.c.o:
	+@echo "[sAPI] Compile: $<"
	@$(CC) $(CPU) $(COMMON_FLAGS) $(C_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<

$(PROJECT).a: $(OBJECTS)
	+@echo "[sAPI] Linking: $@"
	@$(AR) $(AR_FLAGS) $@ $^ -c

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)
