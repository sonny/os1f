BOARD := DISCOVERY
#BOARD := NUCLEO
PROJ := OS-CMSIS

ARM-PATH  := /opt/arm/toolchain
OOCD-PATH := /opt/openocd

# COMMANDS
CC      := $(ARM-PATH)/bin/arm-none-eabi-gcc
GDB     := $(ARM-PATH)/bin/arm-none-eabi-gdb
OBJDUMP := $(ARM-PATH)/bin/arm-none-eabi-objdump
SIZE    := $(ARM-PATH)/bin/arm-none-eabi-size
OPENOCD := $(OOCD-PATH)/bin/openocd
XTERM   := xterm

OUT := BUILD
INC_DIRS :=
SRC_DIRS :=
SRCS :=
OBJS :=
DEPS := 

-include sources.mk

MKFILES := Makefile sources.mk openocd.mk

FINAL := $(OUT)/$(PROJ)
ELF   := $(FINAL).elf

CFLAGS := -mcpu=cortex-m7 -mthumb -Og -g3 -std=gnu11 -Wextra
CFLAGS += -fmessage-length=0 -fsigned-char -ffunction-sections
CFLAGS += -fdata-sections -ffreestanding -fno-move-loop-invariants -flto

DEFINES := -DDEBUG -DTRACE -DSTM32F746xx

ifeq ($(BOARD),DISCOVERY)
DEFINES += -DBOARD_DISCOVERY
else
DEFINES += -DBOARD_NUCLEO
endif

INCLUDES := $(addprefix -I, $(INC_DIRS))
LDFLAGS := -T mem.ld -T sections.ld -T libs.ld -nostartfiles -Xlinker --gc-sections -Lldscripts  --specs=nano.specs


vpath %.c $(SRC_DIRS)
#vpath %.h $(INC_DIRS)

all: $(FINAL).siz $(FINAL).disass;

## disassembly
$(OUT)/%.disass: $(ELF)
	$(OBJDUMP) -dS $< > $@

## Print Size
$(OUT)/%.siz: $(ELF)
	$(SIZE) --format=berkeley $<

## Build ELF file
$(OUT)/%.elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-Map,$*.map -o $@ $(OBJS) 


## Build Object files from S files
$(OUT)/%.o: %.S 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MP -MF$(@:%.o=%.d) -MT$(@) -c -o $@ $<

## Build Object files from C files
$(OUT)/%.o: %.c 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MP -MF$(@:%.o=%.d) -MT$(@) -c -o $@ $<

JUNK := `find . | grep '\~'`
clean:
	@rm -fr $(OUT)
	@rm -f $(JUNK)
	@rm -f src/*.lst

stuff:
	@echo "$(OBJS)"

-include openocd.mk
-include $(DEPS)

.SECONDARY:
