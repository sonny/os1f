#ifndef __OS_CONFIG_H__
#define __OS_CONFIG_H__

// only the STM32F7 boards are supported
// currently -- should be defined by Makefile

//#define BOARD_DISCOVERY
//#define BOARD_NUCLEO

// define one of these
//#define DISPLAY_LCD
#define DISPLAY_SERIAL

//#define VCP_BAUD 1000000
#define VCP_BAUD 38400


#endif  /* __OS_CONFIG_H__ */
