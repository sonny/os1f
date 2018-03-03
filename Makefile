BOARD := DISCOVERY
#BOARD := NUCLEO
FPU := ENABLED
PROJ := OS-CMSIS

ARM-PATH  := /opt/arm/toolchain
#OOCD-PATH := /opt/openocd

# COMMANDS
CC      := $(ARM-PATH)/bin/arm-none-eabi-gcc
GDB     := $(ARM-PATH)/bin/arm-none-eabi-gdb-py
OBJDUMP := $(ARM-PATH)/bin/arm-none-eabi-objdump
SIZE    := $(ARM-PATH)/bin/arm-none-eabi-size
OPENOCD := openocd #$(OOCD-PATH)/bin/openocd

OUT := BUILD
INC_DIRS :=
SRC_DIRS :=
SRCS :=
OBJS :=
DEPS := 

-include make/sources_auto.mk

FINAL  := $(OUT)/$(PROJ)
ELF    := $(FINAL).elf

##==================================================
## DEFINES
##==================================================
# TODO: implement non-printf trace functions
DEFINES := -DSTM32F746xx 
#DEFINEs += -DOS_USE_TRACE_ITM
DEFINES += -DDEBUG -DTRACE 
#DEFINES += -DOS_USE_SEMIHOSTING -DOS_USE_TRACE_SEMIHOSTING_DEBUG
DEFINES += -DOS_USE_VCP
DEFINES += -DOS_USE_LCD

ifeq ($(BOARD),DISCOVERY)
DEFINES += -DBOARD_DISCOVERY
else
DEFINES += -DBOARD_NUCLEO
endif

ARCH := -mcpu=cortex-m7 -mthumb
ifeq ($(FPU),ENABLED)
ARCH += -mfloat-abi=hard -mfpu=fpv5-sp-d16
DEFINES += -DENABLE_FPU
else
ARCH += -mfloat-abi=soft
endif

##==================================================
## CFLAGS
##==================================================
CFLAGS := $(ARCH) -Og -g3 -std=c11
CFLAGS += -Wall -Wextra -Wfatal-errors -pedantic -Wno-unused-parameter
CFLAGS += -fsigned-char -ffunction-sections -fdata-sections -ffreestanding 
##CFLAGS += -flto

##==================================================
## INCLUDES
##==================================================
INCLUDES := $(addprefix -I, $(INC_DIRS))

##==================================================
## PREPROC -- preprocessor flags
##==================================================
PREPROC = $(DEFINES) -MMD -MP -MF$(@:%.o=%.d) -MT$(@)

##==================================================
## LDFLAGS
##==================================================
## NOTE: use deferred assignment here
LDFLAGS = $(ARCH) -Wl,-Map,$(OUT)/$*.map -Wl,--gc-sections,--print-memory-usage
#-z defs 
#LDFLAGS += -Lldscripts -T mem.ld -T sections.ld -T libs.ld -nostartfiles --specs=nano.specs -lc -lg -lm
LDFLAGS += -TDebug_STM32F746NG_FLASH.ld -nostartfiles --specs=nano.specs -lc -lg -lm
## for semihosting
#LDFLAGS += --specs=rdimon.specs -lrdimon

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
	$(CC) -o $@ $(OBJS) $(LDFLAGS) 

## Build Object files from S files
$(OUT)/%.o: %.S 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(PREPROC) $(INCLUDES) -c -o $@ $<

## Build Object files from C files
$(OUT)/%.o: %.c 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(PREPROC) $(INCLUDES) -c -o $@ $<

JUNK := `find . | grep '\~'`
clean:
	@rm -fr $(OUT)
	@rm -f $(JUNK)
	@rm -f src/*.lst

stuff:
	@echo "$(OBJS)"

-include make/openocd.mk
-include $(DEPS)

.SECONDARY:
