# LPCopen library Makefile

PROJECT = liblpc

###############################################################################
#
# LPCOpen library source code.
#
SRC += $(wildcard ./lpc_chip_43xx/src/*.c)
SRC += $(wildcard ./lpc_board_ciaa_edu_4337/src/*.c)
OBJECTS = $(SRC:.c=.o)

###############################################################################
#
# Paths to the required headers.
#
INCLUDE_PATHS += -I./lpc_chip_43xx/inc
INCLUDE_PATHS += -I./lpc_chip_43xx/inc/usbd/
INCLUDE_PATHS += -I./lpc_board_ciaa_edu_4337/inc

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
	+@echo "Cleaning LPCopen object files..."
	@rm -f $(PROJECT).a $(OBJECTS) $(DEPS)

.c.o:
	+@echo "Compile: $<"
	@$(CC) $(CPU) $(COMMON_FLAGS) $(C_COMMON_FLAGS) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<

$(PROJECT).a: $(OBJECTS)
	+@echo "Linking: $@"
	@$(AR) $(AR_FLAGS) $@ $^ -c

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)
