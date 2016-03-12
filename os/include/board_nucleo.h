#ifndef __BOARD_NUCLEO_H__
#define __BOARD_NUCLEO_H__

#if defined(BOARD_NUCLEO)

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

#define VCPClockSelection            Usart3ClockSelection
#define RCC_PERIPHCLK_VCP            RCC_PERIPHCLK_USART3
#define RCC_VCPCLKSOURCE_SYSCLK      RCC_USART3CLKSOURCE_SYSCLK
  
#endif


#endif /*  __BOARD_NUCLEO_H__ */
