#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "os.h"
#include "event.h"

#if defined(BOARD_DISCOVERY)

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

#elif defined(BOARD_NUCLEO)

#define VCP                          USART3
#define VCP_CLK_ENABLE()             __HAL_RCC_USART3_CLK_ENABLE()
#define VCP_CLK_DISABLE()            __HAL_RCC_USART3_CLK_DISABLE()

#define VCP_TX_PIN                   GPIO_PIN_8
#define VCP_TX_GPIO_PORT             GPIOD
#define VCP_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOD_CLK_ENABLE()
#define VCP_TX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOD_CLK_DISABLE()
#define VCP_TX_AF                    GPIO_AF7_USART3

#define VCP_RX_PIN                   GPIO_PIN_9
#define VCP_RX_GPIO_PORT             GPIOD
#define VCP_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOD_CLK_ENABLE()
#define VCP_RX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOD_CLK_DISABLE()
#define VCP_RX_AF                    GPIO_AF7_USART3

#define VCP_IRQn                     USART3_IRQn
#define VCP_IRQHandler               USART3_IRQHandler

#endif

#ifndef VCP_BAUD
#define VCP_BAUD 115200
#endif

extern struct event *VCPCompleteEvent;

void serialInit(void);
void serial_register_event(struct event *e);

#endif  /* __SERIAL_H__ */
