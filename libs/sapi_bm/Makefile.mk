# sAPI library Makefile

GCC_BIN ?= $(GCC_BIN_PATH)

PROJECT = libsapi

############################################################################### 
AR = $(GCC_BIN)arm-none-eabi-ar
CC = $(GCC_BIN)arm-none-eabi-gcc

DEFINES += CORE_M4
DEFINES += __USE_LPCOPEN

SRC += $(wildcard ./src/*.c)

OBJECTS = $(SRC:.c=.o)
DEPS = $(SRC:.c=.d)

INCLUDE_PATHS += -I./inc/
INCLUDE_PATHS += $(LPC_INCLUDE_PATH)

CC_FLAGS += $(CFLAGS)
CC_FLAGS += -c -fmessage-length=0 -fno-exceptions -ffunction-sections -fdata-sections -fno-builtin
CC_FLAGS += -MMD

AR_FLAGS = -r

all: $(PROJECT).a

clean:
	+@echo "Cleaning sAPI object files..."
	@rm -f $(PROJECT).bin $(PROJECT).a $(OBJECTS) $(DEPS)

.c.o:
	+@echo "Compile: $<"
	@$(CC) $(CC_FLAGS) $(INCLUDE_PATHS) -o $@ $<

$(PROJECT).a: $(OBJECTS)
	+@echo "Linking: $@"
	@$(AR) $(AR_FLAGS) $@ $^ -c

DEPS = $(OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
