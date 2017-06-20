#
# Modify example to build in Makefile.mine
#
-include Makefile.mine

###############################################################################
# Boiler-plate

# cross-platform directory manipulation
ifeq ($(shell echo $$OS),$$OS)
    MAKEDIR = if not exist "$(1)" mkdir "$(1)"
    RM = rmdir /S /Q "$(1)"
else
    MAKEDIR = '$(SHELL)' -c "mkdir -p \"$(1)\""
    RM = '$(SHELL)' -c "rm -rf \"$(1)\""
endif

############################################################################### 
GCC_BIN =
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
SIZE    = $(GCC_BIN)arm-none-eabi-size

MAKE_DIR = $(PWD)
FREERTOS_DIR = $(MAKE_DIR)/libs/FreeRTOS 
EXAMPLES_DIR = $(MAKE_DIR)/examples
INCLUDE_DIR = $(MAKE_DIR)/include

MAKE_FLAGS += --no-print-directory

export AS CC CPP LD OBJCOPY SIZE FREERTOS_KERNEL_VERSION_NUMBER TRACEALIZER_VERSION_NUMBER MAKEDIR RM DEBUG TARGET

$(APP_NAME):
	+@echo "-- Target: $(TARGET)"
	+@echo "-- Building $(APP_NAME)"
	@$(MAKE) $(MAKE_FLAGS) -C examples/$(TARGET)/ -f Makefile.mk APP_NAME=$(APP_NAME)

$(APP_NAME)_clean:
	+@echo "-- Cleaning $(APP_NAME)"	
	@$(MAKE) $(MAKE_FLAGS) -C examples/$(TARGET)/ -f Makefile.mk clean APP_NAME=$(APP_NAME)

all: $(APP_NAME)

clean: $(APP_NAME)_clean

###
# CIAA
#
OOCD=C:\\Users\\fep\\Documents\\bin\\openocd-0.10.0\\bin\\openocd

openocd:
	$(Q)$(OOCD) -f board/edu-ciaa-nxp/ciaa-nxp.cfg

debug: $(APP_NAME)
	$(Q)$(GDB) $< -ex "target remote :3333" -ex "mon reset halt" -ex "load" -ex "continue"

run: $(APP_NAME)
	$(Q)$(GDB) $< -batch -ex "target remote :3333" -ex "mon reset halt" -ex "load" -ex "mon reset run" -ex "quit"

download: $(APP_NAME)
	$(Q)$(OOCD) -f board/edu-ciaa-nxp/ciaa-nxp.cfg \
		-c "init" \
		-c "halt 0" \
		-c "flash write_image erase unlock build/$(APP_NAME).bin 0x1A000000 bin" \
		-c "reset run" \
		-c "shutdown"

erase:
	$(Q)$(OOCD) -f ciaa-nxp.cfg \
		-c "init" -c "halt 0" -c "flash erase_sector 0 0 last" -c "shutdown"

