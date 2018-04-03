INC_DIRS := include 
INC_DIRS += os/include
INC_DIRS += board/STM32F7_$(BOARD)/config
INC_DIRS += board/STM32F7_$(BOARD)/include/BSP

INC_DIRS += system/include 
INC_DIRS += system/include/cmsis 
INC_DIRS += system/include/stm32f7xx 

INC_DIRS += system/include/Components
INC_DIRS += Utilities/Fonts

SRC_DIRS := src
SRC_DIRS += os/src
#SRC_DIRS += Utilities/CPU
#SRC_DIRS += Utilities/Fonts
#SRC_DIRS += Utilities/Log
#SRC_DIRS += system/include/Components/exc7200
#SRC_DIRS += system/include/Components/ft5336
#SRC_DIRS += system/include/Components/mfxstm32l152
#SRC_DIRS += system/include/Components/ov9655
#SRC_DIRS += system/include/Components/s5k5cag
#SRC_DIRS += system/include/Components/stmpe811
#SRC_DIRS += system/include/Components/ts3510
#SRC_DIRS += system/include/Components/wm8994
#SRC_DIRS += board/STM32F7_$(BOARD)/src/BSP
#SRC_DIRS += board/STM32F7_$(BOARD)/src

SRC_DIRS += system/src/cmsis
SRC_DIRS += system/src/cortexm
#SRC_DIRS += system/src/diag
SRC_DIRS += system/src/newlib
SRC_DIRS += system/src/stm32f7xx

#CSRCS := $(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)/*.c))
#ASRCS := $(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)/*.S))
#ASRCS += $(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)/*.s))

#SRCS := $(ASRCS) $(CSRCS)

#OBJS := $(addprefix $(OUT)/,$(SRCS:%.c=%.o))
#OBJS := $(OBJS:%.s=%.o)
#OBJS := $(OBJS:%.S=%.o)

OBJS  := main.o _write.o stm32f7xx_it.o
OBJS  += os.o task.o memory.o spinlock.o display.o \
         serial.o adc_task.o vt100.o lcd.o vsnprintf.o \
	 core.o
OBJS  += stm32f7xx_hal.o \
	stm32f7xx_hal_rcc.o stm32f7xx_hal_rcc_ex.o \
	stm32f7xx_hal_cortex.o stm32f7xx_hal_gpio.o \
         stm32f7xx_hal_adc.o stm32f7xx_hal_dma.o stm32f7xx_hal_uart.o
OBJS  += system_stm32f7xx.o startup_stm32f746xx.o
OBJS  += exception_handlers.o
OBJS  += _startup.o _syscalls.o _sbrk.o _initialize_hardware.o
OBJS  := $(addprefix $(OUT)/, $(OBJS)) 


DEPS := $(OBJS:%.o=%.d)

