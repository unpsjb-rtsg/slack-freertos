# sAPI library Makefile

PROJECT = libsapi

###############################################################################
#
# sAPI library source code.
#
SRC += $(wildcard ./src/*.c)
OBJECTS = $(SRC:.c=.o)

###############################################################################
#
# Paths to the required headers.
#
INCLUDE_PATHS += -I./inc/
LPC_INCLUDE_PATH += -I../../LPCOpen/v2.16/lpc_chip_43xx/inc
LPC_INCLUDE_PATH += -I../../LPCOpen/v2.16/lpc_board_ciaa_edu_4337/inc
LPC_INCLUDE_PATH += -I../../LPCOpen/v2.16/lpc_chip_43xx/inc/usbd/
INCLUDE_PATHS += $(LPC_INCLUDE_PATH)

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
