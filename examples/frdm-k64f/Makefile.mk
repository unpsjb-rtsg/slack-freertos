# This file is based on the Makefile automagically generated by mbed.org. 
# For more information, see http://mbed.org/handbook/Exporting-to-GCC-ARM-Embedded

BUILD_DIR = ../../build

EXAMPLE = $(APP_NAME)

OBJECTS += ./$(EXAMPLE)/main.o 
OBJECTS += ./utils/utils.o
OBJECTS += ./common/common.o

SYS_OBJECTS += $(wildcard ../../board/frdm-k64f/TARGET_K64F/TOOLCHAIN_GCC_ARM/*.o)

MBED_INCLUDE_PATHS += -I../../board/frdm-k64f
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/drivers
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/hal
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/platform
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/TARGET_K64F
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/TARGET_K64F/TARGET_Freescale
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/TARGET_K64F/TARGET_Freescale/TARGET_KSDK2_MCUS/api
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/TARGET_K64F/TARGET_Freescale/TARGET_KSDK2_MCUS/TARGET_MCU_K64F/
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/TARGET_K64F/TARGET_Freescale/TARGET_KSDK2_MCUS/TARGET_MCU_K64F/device
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/TARGET_K64F/TARGET_Freescale/TARGET_KSDK2_MCUS/TARGET_MCU_K64F/drivers
MBED_INCLUDE_PATHS += -I../../board/frdm-k64f/TARGET_K64F/TARGET_Freescale/TARGET_KSDK2_MCUS/TARGET_MCU_K64F/TARGET_FRDM

FREERTOS_INCLUDE_PATHS += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/include
FREERTOS_INCLUDE_PATHS += -I../../libs/FreeRTOS/$(FREERTOS_KERNEL_VERSION_NUMBER)/portable/GCC/ARM_CM4F

INCLUDE_PATHS += -I./$(EXAMPLE)
INCLUDE_PATHS += -I./utils/ 
INCLUDE_PATHS += -I./common/
INCLUDE_PATHS += -I../../slack/$(FREERTOS_KERNEL_VERSION_NUMBER)
INCLUDE_PATHS += $(FREERTOS_INCLUDE_PATHS) 
INCLUDE_PATHS += $(MBED_INCLUDE_PATHS) 

FREERTOS_LIBRARY_PATH = ../../libs/FreeRTOS
LIBRARY_PATHS += -L../../board/frdm-k64f/TARGET_K64F/TOOLCHAIN_GCC_ARM 
LIBRARY_PATHS += -L$(FREERTOS_LIBRARY_PATH)
LIBRARIES = -lmbed -lfreertos

LINKER_SCRIPT = ../../board/frdm-k64f/TARGET_K64F/TOOLCHAIN_GCC_ARM/MK64FN1M0xxx12.ld

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

ifeq ($(HARDFP),1)
	FLOAT_ABI = hard
else
	FLOAT_ABI = softfp
endif

CPU += -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=$(FLOAT_ABI) 
CC_FLAGS += -c -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fmessage-length=0 -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -MMD -fno-delete-null-pointer-checks -fomit-frame-pointer -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -std=gnu99 -D__MBED__=1 -DDEVICE_I2CSLAVE=1 -D__FPU_PRESENT=1 -DTARGET_Freescale -DDEVICE_PORTINOUT=1 -DTARGET_RTOS_M4_M7 -DDEVICE_LOWPOWERTIMER=1 -DDEVICE_RTC=1 -DTOOLCHAIN_object -D__CMSIS_RTOS -DFSL_RTOS_MBED -DDEVICE_STORAGE=1 -DMBEDTLS_ENTROPY_HARDWARE_ALT -DTARGET_KPSDK_MCUS -DTOOLCHAIN_GCC -DTARGET_CORTEX_M -DTARGET_KSDK2_MCUS -DTARGET_LIKE_CORTEX_M4 -DDEVICE_ANALOGOUT=1 -DTARGET_M4 -DTARGET_UVISOR_UNSUPPORTED -DTARGET_K64F -DDEVICE_SERIAL=1 -DMBED_BUILD_TIMESTAMP=1482431125.39 -DDEVICE_INTERRUPTIN=1 -DDEVICE_I2C=1 -DDEVICE_PORTOUT=1 -D__CORTEX_M4 -DDEVICE_STDIO_MESSAGES=1 -DCPU_MK64FN1M0VMD12 -DFEATURE_IPV4=1 -DTARGET_LIKE_MBED -DTARGET_FF_ARDUINO -DTARGET_KPSDK_CODE -DTARGET_RELEASE -DDEVICE_SERIAL_FC=1 -DFEATURE_STORAGE=1 -D__MBED_CMSIS_RTOS_CM -DDEVICE_SLEEP=1 -DTOOLCHAIN_GCC_ARM -DTARGET_FRDM -DDEVICE_SPI=1 -DDEVICE_ERROR_RED=1 -DDEVICE_SPISLAVE=1 -DDEVICE_ANALOGIN=1 -DDEVICE_PWMOUT=1 -DDEVICE_PORTIN=1 -DTARGET_MCU_K64F -DARM_MATH_CM4 -include mbed_config.h -MMD -MP
CPPC_FLAGS += -c -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fmessage-length=0 -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -MMD -fno-delete-null-pointer-checks -fomit-frame-pointer -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -std=gnu++98 -fno-rtti -Wvla -D__MBED__=1 -DDEVICE_I2CSLAVE=1 -D__FPU_PRESENT=1 -DTARGET_Freescale -DDEVICE_PORTINOUT=1 -DTARGET_RTOS_M4_M7 -DDEVICE_LOWPOWERTIMER=1 -DDEVICE_RTC=1 -DTOOLCHAIN_object -D__CMSIS_RTOS -DFSL_RTOS_MBED -DDEVICE_STORAGE=1 -DMBEDTLS_ENTROPY_HARDWARE_ALT -DTARGET_KPSDK_MCUS -DTOOLCHAIN_GCC -DTARGET_CORTEX_M -DTARGET_KSDK2_MCUS -DTARGET_LIKE_CORTEX_M4 -DDEVICE_ANALOGOUT=1 -DTARGET_M4 -DTARGET_UVISOR_UNSUPPORTED -DTARGET_K64F -DDEVICE_SERIAL=1 -DMBED_BUILD_TIMESTAMP=1482431125.39 -DDEVICE_INTERRUPTIN=1 -DDEVICE_I2C=1 -DDEVICE_PORTOUT=1 -D__CORTEX_M4 -DDEVICE_STDIO_MESSAGES=1 -DCPU_MK64FN1M0VMD12 -DFEATURE_IPV4=1 -DTARGET_LIKE_MBED -DTARGET_FF_ARDUINO -DTARGET_KPSDK_CODE -DTARGET_RELEASE -DDEVICE_SERIAL_FC=1 -DFEATURE_STORAGE=1 -D__MBED_CMSIS_RTOS_CM -DDEVICE_SLEEP=1 -DTOOLCHAIN_GCC_ARM -DTARGET_FRDM -DDEVICE_SPI=1 -DDEVICE_ERROR_RED=1 -DDEVICE_SPISLAVE=1 -DDEVICE_ANALOGIN=1 -DDEVICE_PWMOUT=1 -DDEVICE_PORTIN=1 -DTARGET_MCU_K64F -DARM_MATH_CM4 -include mbed_config.h -MMD -MP
ASM_FLAGS += -x assembler-with-cpp -D__CORTEX_M4 -DCPU_MK64FN1M0VMD12 -DARM_MATH_CM4 -D__FPU_PRESENT=1 -D__MBED_CMSIS_RTOS_CM -D__CMSIS_RTOS -DFSL_RTOS_MBED -DMBEDTLS_ENTROPY_HARDWARE_ALT -c -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fmessage-length=0 -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -MMD -fno-delete-null-pointer-checks -fomit-frame-pointer -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Os
CC_SYMBOLS += -D__MBED__=1 -DDEVICE_I2CSLAVE=1 -D__FPU_PRESENT=1 -DTARGET_Freescale -DDEVICE_PORTINOUT=1 -DTARGET_RTOS_M4_M7 -DDEVICE_LOWPOWERTIMER=1 -DDEVICE_RTC=1 -DTOOLCHAIN_object -D__CMSIS_RTOS -DFSL_RTOS_MBED -DDEVICE_STORAGE=1 -DMBEDTLS_ENTROPY_HARDWARE_ALT -DTARGET_KPSDK_MCUS -DTOOLCHAIN_GCC -DTARGET_CORTEX_M -DTARGET_KSDK2_MCUS -DTARGET_LIKE_CORTEX_M4 -DDEVICE_ANALOGOUT=1 -DTARGET_M4 -DTARGET_UVISOR_UNSUPPORTED -DTARGET_K64F -DDEVICE_SERIAL=1 -DMBED_BUILD_TIMESTAMP=1482431125.39 -DDEVICE_INTERRUPTIN=1 -DDEVICE_I2C=1 -DDEVICE_PORTOUT=1 -D__CORTEX_M4 -DDEVICE_STDIO_MESSAGES=1 -DCPU_MK64FN1M0VMD12 -DFEATURE_IPV4=1 -DTARGET_LIKE_MBED -DTARGET_FF_ARDUINO -DTARGET_KPSDK_CODE -DTARGET_RELEASE -DDEVICE_SERIAL_FC=1 -DFEATURE_STORAGE=1 -D__MBED_CMSIS_RTOS_CM -DDEVICE_SLEEP=1 -DTOOLCHAIN_GCC_ARM -DTARGET_FRDM -DDEVICE_SPI=1 -DDEVICE_ERROR_RED=1 -DDEVICE_SPISLAVE=1 -DDEVICE_ANALOGIN=1 -DDEVICE_PWMOUT=1 -DDEVICE_PORTIN=1 -DTARGET_MCU_K64F -DARM_MATH_CM4 

LD_FLAGS += -Wl,--gc-sections -Wl,--wrap,main -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp 
LD_SYS_LIBS += -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys

ifeq ($(DEBUG), 1)
  CC_FLAGS += -DDEBUG -g
  CPPC_FLAGS += -DDEBUG -g
else
  CC_FLAGS += -DNDEBUG -Os
  CPPC_FLAGS += -DNDEBUG -Os
endif

export CPU MBED_INCLUDE_PATHS

all: $(BUILD_DIR)/$(EXAMPLE).bin size

clean:
	@$(MAKE) $(MAKE_FLAGS) -C $(FREERTOS_LIBRARY_PATH) -f Makefile.mk clean APP_DIR=$(EXAMPLE) USE_SLACK=1 TZ=$(TZ)
	+@echo "Cleaning $(TARGET) files..."
	@rm -f $(BUILD_DIR)/$(EXAMPLE).bin $(BUILD_DIR)/$(EXAMPLE).elf $(OBJECTS) $(DEPS)

.cpp.o:
	+@echo "Compile: $<"
	@$(CPP) $(CPPC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<	

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
