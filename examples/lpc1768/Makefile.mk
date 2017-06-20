# This file is based on the Makefile automagically generated by mbed.org. 
# For more information, see http://mbed.org/handbook/Exporting-to-GCC-ARM-Embedded

BUILD_DIR = ../../build

EXAMPLE = $(APP_NAME)

OBJECTS += ./$(EXAMPLE)/main.o 
OBJECTS += ./utils/utils.o
OBJECTS += ./common/common.o

SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/board.o
SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/cmsis_nvic.o
SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/retarget.o
SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/startup_LPC17xx.o
SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/system_LPC17xx.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/analogin_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/analogout_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/can_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/cmsis_nvic.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/ethernet_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/gpio_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/gpio_irq_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/i2c_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/mbed_board.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/mbed_retarget.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/pinmap.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/port_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/pwmout_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/rtc_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/serial_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/sleep.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/spi_api.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/startup_LPC17xx.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/system_LPC17xx.o
#SYS_OBJECTS += ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/us_ticker.o

MBED_INCLUDE_PATHS += -I../../board/lpc1768
#MBED_INCLUDE_PATHS += -I../../board/lpc1768/drivers
#MBED_INCLUDE_PATHS += -I../../board/lpc1768/hal
#MBED_INCLUDE_PATHS += -I../../board/lpc1768/platform

MBED_INCLUDE_PATHS += -I../../board/lpc1768/TARGET_LPC1768
MBED_INCLUDE_PATHS += -I../../board/lpc1768/TARGET_LPC1768/TARGET_NXP
MBED_INCLUDE_PATHS += -I../../board/lpc1768/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/
MBED_INCLUDE_PATHS += -I../../board/lpc1768/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/device
MBED_INCLUDE_PATHS += -I../../board/lpc1768/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/TARGET_MBED_LPC1768

FREERTOS_INCLUDE_PATHS += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/include
FREERTOS_INCLUDE_PATHS += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM3

INCLUDE_PATHS += -I./$(EXAMPLE)
INCLUDE_PATHS += -I./utils/ 
INCLUDE_PATHS += -I./common/
INCLUDE_PATHS += -I../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)
INCLUDE_PATHS += $(FREERTOS_INCLUDE_PATHS) 
INCLUDE_PATHS += $(MBED_INCLUDE_PATHS) 

FREERTOS_LIBRARY_PATH = ../../libs/FreeRTOS
LIBRARY_PATHS += -L../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM 
LIBRARY_PATHS += -L$(FREERTOS_LIBRARY_PATH)
LIBRARIES = -lmbed -lfreertos

LINKER_SCRIPT = ../../board/lpc1768/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/LPC1768.ld

ifeq ($(APP_NAME), example3)
  TZ = 1
  ifeq ($(TRACEALIZER_VERSION_NUMBER), v3.0.2)
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/Include
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/ConfigurationTemplate
    CC_SYMBOLS += -DTRACEALYZER_v3_0_2
  endif
  ifeq ($(TRACEALIZER_VERSION_NUMBER), v3.1.3)
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/include
    INCLUDE_PATHS += -I../../libs/Tracealizer/$(TRACEALIZER_VERSION_NUMBER)/config
    CC_SYMBOLS += -DTRACEALYZER_v3_1_3
  endif
else
  TZ = 0
endif

CPU = -mcpu=cortex-m3 -mthumb

CC_FLAGS += $(CPU)
CC_FLAGS += -c -g -fno-common -fmessage-length=0 -Wall -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer -fno-rtti
CC_FLAGS += -MMD -MP

CC_SYMBOLS += -DTARGET_LPC1768
CC_SYMBOLS += -DTARGET_M3
CC_SYMBOLS += -DTARGET_NXP
CC_SYMBOLS += -DTARGET_LPC176X
CC_SYMBOLS += -DTARGET_MBED_LPC1768
CC_SYMBOLS += -DTOOLCHAIN_GCC_ARM
CC_SYMBOLS += -DTOOLCHAIN_GCC
CC_SYMBOLS += -D__CORTEX_M3
CC_SYMBOLS += -DARM_MATH_CM3
CC_SYMBOLS += -DMBED_BUILD_TIMESTAMP=1414254042.69
CC_SYMBOLS += -D__MBED__=1 
CC_SYMBOLS += -DBATCH_TEST=$(BATCH_TEST) 
CC_SYMBOLS += -DMAX_PRIO=$(MAX_PRIO)

LD_FLAGS = -mcpu=cortex-m3 -mthumb -Wl,--gc-sections -u _printf_float -u _scanf_float
LD_SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys

ifeq ($(DEBUG), 1)
  CC_FLAGS += -DDEBUG -O0
else
  CC_FLAGS += -DNDEBUG -Os
endif

all: $(BUILD_DIR)/$(EXAMPLE).bin size

clean:
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk clean APP_DIR=$(EXAMPLE) USE_SLACK=1 TZ=$(TZ)
	+@echo "Cleaning $(TARGET) files..."
	@rm -f $(BUILD_DIR)/$(EXAMPLE).bin $(BUILD_DIR)/$(EXAMPLE).elf $(OBJECTS) $(DEPS)

install:
	 cp $(BUILD_DIR)/$(EXAMPLE).bin F:\$(EXAMPLE)

.c.o:
	+@echo "Compile: $<"
	@$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<	

.cpp.o:
	+@echo "Compile: $<"
	@$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 $(INCLUDE_PATHS) -o $@ $<	

$(BUILD_DIR)/$(EXAMPLE).elf: $(OBJECTS) $(SYS_OBJECTS)
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk APP_DIR=$(EXAMPLE) USE_SLACK=1 TZ=$(TZ)
	+@echo "Linking: $@"
	@$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(LD_SYS_LIBS)

$(BUILD_DIR)/$(EXAMPLE).bin: $(BUILD_DIR)/$(EXAMPLE).elf
	+@echo "Binary: $@"
	@$(OBJCOPY) -O binary $< $@
	
size: $(BUILD_DIR)/$(EXAMPLE).elf
	$(SIZE) $<

DEPS = $(OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
