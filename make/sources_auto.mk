BOARD := STM32746G-Discovery
######################################
## Application Paths
######################################
INC_DIRS := include
SRC_DIRS := src

######################################
## OS Paths
######################################
INC_DIRS += os/include
SRC_DIRS += os/src
SRC_DIRS += os/newlib

INC_DIRS += config

######################################
## STM32Cube Paths
######################################
CUBE := STM32CubeF7
INC_DIRS += $(CUBE)/Drivers/CMSIS/Include
INC_DIRS += $(CUBE)/Drivers/CMSIS/Device/ST/STM32F7xx/Include
SRC_DIRS += $(CUBE)/Drivers/CMSIS/Device/ST/STM32F7xx/Source

INC_DIRS += $(CUBE)/Drivers/STM32F7xx_HAL_Driver/Inc
SRC_DIRS += $(CUBE)/Drivers/STM32F7xx_HAL_Driver/Src

######################################
## BSP Components
######################################
INC_DIRS += $(CUBE)/Drivers/BSP/$(BOARD)
SRC_DIRS += $(CUBE)/Drivers/BSP/$(BOARD)

######################################
## Individual Files
######################################
STARTUP_ASM := config/startup_stm32f746xx.S
SYSTEM_SRC := config/system_stm32f7xx.c
INIT_SRC := config/init.c

CSRCS := $(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)/*.c))
ASRCS := $(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)/*.S))
ASRCS += $(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)/*.s))

CSRCS += $(SYSTEM_SRC) $(INIT_SRC)
ASRCS += $(STARTUP_ASM)

SRCS := $(ASRCS) $(CSRCS)

OBJS := $(addprefix $(OUT)/,$(SRCS:%.c=%.o))
OBJS := $(OBJS:%.s=%.o)
OBJS := $(OBJS:%.S=%.o)

DEPS := $(OBJS:%.o=%.d)

