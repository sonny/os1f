BOARD := DISCOVERY
#BOARD := NUCLEO
PROJ := OS-CMSIS

ARM-PATH  := /opt/arm/toolchain
#OOCD-PATH := /opt/openocd

# COMMANDS
CC      := $(ARM-PATH)/bin/arm-none-eabi-gcc
GDB     := $(ARM-PATH)/bin/arm-none-eabi-gdb-py
OBJDUMP := $(ARM-PATH)/bin/arm-none-eabi-objdump
SIZE    := $(ARM-PATH)/bin/arm-none-eabi-size
OPENOCD := openocd #$(OOCD-PATH)/bin/openocd
XTERM   := xterm

OUT := BUILD
INC_DIRS :=
SRC_DIRS :=
SRCS :=
OBJS :=
DEPS := 

-include sources_auto.mk

FINAL := $(OUT)/$(PROJ)
ELF   := $(FINAL).elf

CFLAGS := -mcpu=cortex-m7 -mthumb -Og -g3 -Wextra -std=c11
CFLAGS += -fmessage-length=0 -fsigned-char -ffunction-sections
CFLAGS += -fdata-sections -ffreestanding -fno-move-loop-invariants
##CFLAGS += -flto

# TODO: implement non-printf trace functions
DEFINES := -DDEBUG -DTRACE -DSTM32F746xx 
#DEFINEs += -DOS_USE_TRACE_ITM
DEFINES += -DOS_USE_SEMIHOSTING -DOS_USE_TRACE_SEMIHOSTING_DEBUG

ifeq ($(BOARD),DISCOVERY)
DEFINES += -DBOARD_DISCOVERY
else
DEFINES += -DBOARD_NUCLEO
endif

INCLUDES := $(addprefix -I, $(INC_DIRS))
LDFLAGS += -Wl,--gc-sections,--print-memory-usage -z defs 
LDFLAGS += -Lldscripts -T mem.ld -T sections.ld -T libs.ld -nostartfiles --specs=nano.specs -lc -lg
## for semihosting
LDFLAGS += --specs=rdimon.specs -lrdimon

vpath %.c $(SRC_DIRS)
vpath %.S $(SRC_DIRS)
vpath %.h $(INC_DIRS)

all: $(FINAL).siz $(FINAL).disass;

analyze: CFLAGS += -fstack-usage
analyze: $(OBJS)

## disassembly
$(OUT)/%.disass: $(ELF)
	$(OBJDUMP) -dS $< > $@

## Print Size
$(OUT)/%.siz: $(ELF)
	$(SIZE) --format=berkeley $<

## Check for missing symbols
link-check: LDFLAGS := --specs=nosys.specs
link-check: $(OBJS)
	$(CC) $(LDFLAGS) -o $(OUT)/link-check $(OBJS) 

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
