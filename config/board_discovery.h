#ifndef __BOARD_DISCOVERY_H__
#define __BOARD_DISCOVERY_H__

//#if defined(BOARD_DISCOVERY)

#define VCP                          USART1
#define VCP_CLK_ENABLE()             __HAL_RCC_USART1_CLK_ENABLE()
#define VCP_CLK_DISABLE()            __HAL_RCC_USART1_CLK_DISABLE()

#define VCP_TX_PIN                   GPIO_PIN_9
#define VCP_TX_GPIO_PORT             GPIOA
#define VCP_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define VCP_TX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define VCP_TX_AF                    GPIO_AF7_USART1

#define VCP_RX_PIN                   GPIO_PIN_7
#define VCP_RX_GPIO_PORT             GPIOB
#define VCP_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define VCP_RX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOB_CLK_DISABLE()
#define VCP_RX_AF                    GPIO_AF7_USART1

#define VCP_IRQn                     USART1_IRQn
#define VCP_IRQHandler               USART1_IRQHandler

#define HAL_RCC_VCP_CONFIG           __HAL_RCC_USART1_CONFIG
//#define RCC_PERIPHCLK_VCP            RCC_PERIPHCLK_USART1
#define RCC_VCPCLKSOURCE_SYSCLK      RCC_USART1CLKSOURCE_SYSCLK

#define LCD_DEFAULT_FONT             Font12

//#endif

#ifdef __GNUC__
// Avoid tons of warnings with root code
#pragma GCC system_header
#endif
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"

#endif /*  __BOARD_DISCOVERY_H__ */
