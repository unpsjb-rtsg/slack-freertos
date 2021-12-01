# LPCopen library Makefile

PROJECT = liblpc

###############################################################################
#
# LPCOpen library source code.
#
SRC += $(wildcard ./boards/edu_ciaa_nxp/src/*.c)
SRC += $(wildcard ./cmsis_core/src/*.c)
SRC += $(wildcard ./lpc_chip_43xx/src/*.c)
SRC += $(wildcard ./lpc_startup/src/*.c)
OBJECTS = $(SRC:.c=.o)

###############################################################################
#
# Paths to the required headers.
#
INCLUDE_PATHS += -I./boards/edu_ciaa_nxp/inc/
INCLUDE_PATHS += -I./boards/inc/
INCLUDE_PATHS += -I./lpc_chip_43xx/inc/
INCLUDE_PATHS += -I./lpc_chip_43xx/usbd_rom/
INCLUDE_PATHS += -I./cmsis_core/inc/

###############################################################################
#
# Flags and symbols required by the compiler.
#
CC_SYMBOLS += -DCHIP_LPC43XX

###############################################################################
#
# Flags and symbols required by the linker.
#
AR_FLAGS = -r

###############################################################################
#
# Rules used to build the LPCOpen library.
#
all: $(PROJECT).a

clean:
	+@echo "[LPCOpen] Cleaning LPCopen object files..."
	@rm -f $(PROJECT).a $(OBJECTS) $(DEPS)

.c.o:
	+@echo "[LPCOpen] Compile: $<"
	@$(CC) $(CPU) $(COMMON_FLAGS) $(C_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<

$(PROJECT).a: $(OBJECTS)
	+@echo "[LPCOpen] Linking: $@"
	@$(AR) $(AR_FLAGS) $@ $^ -c

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)
