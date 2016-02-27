#ifndef __SERIAL_H__
#define __SERIAL_H__

#define SERIAL USART2
#define SERIAL_CLK 48000000 // PCLK1
#define SERIAL_TX_PORT GPIOA
#define SERIAL_RX_PORT GPIOA
#define SERIAL_TX_PORT_CLK RCC_AHB1ENR_GPIOAEN
#define SERIAL_RX_PORT_CLK RCC_AHB1ENR_GPIOAEN
#define SERIAL_TX_PIN P2
#define SERIAL_RX_PIN P3

void serial_init(void);
  

#endif  /* __SERIAL_H__ */
