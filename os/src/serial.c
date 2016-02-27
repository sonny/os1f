#include "serial.h"

void serial_init(void)
{
  /*
  // enable GPIOs for SERIAL uart
  RCC->AHB1ENR |= SERIAL_TX_PORT_CLK;
  RCC->AHB1ENR |= SERIAL_RX_PORT_CLK;

  // set AF7
  SERIAL_TX_PORT->AFR[SERIAL_TX_PIN >> 3] = GPIO_AFR_AF7 << (SERIAL_TX_PIN * 4);
  SERIAL_RX_PORT->AFR[SERIAL_RX_PIN >> 3] = GPIO_AFR_AF7 << (SERIAL_RX_PIN * 4);

  // set PUPDR_UP
  SERIAL_TX_PORT->PUPDR |= GPIO_PUPDR_UP << (SERIAL_TX_PIN * 2);
  SERIAL_RX_PORT->PUPDR |= GPIO_PUPDR_UP << (SERIAL_RX_PIN * 2);

  // set MODER_AF
  SERIAL_TX_PORT->MODER |= GPIO_MODER_AF << (SERIAL_TX_PIN * 2);
  SERIAL_RX_PORT->MODER |= GPIO_MODER_AF << (SERIAL_RX_PIN * 2);

  // set output SPEED fast (50Mhz)
  SERIAL_TX_PORT->OSPEEDR |= GPIO_OSPEEDR_FAST << (SERIAL_TX_PIN * 2);
  SERIAL_RX_PORT->OSPEEDR |= GPIO_OSPEEDR_FAST << (SERIAL_RX_PIN * 2);

  // NO Hardware Flow Control for now
  // Configure for 115200 Baud, 8N1
  // -- Configure CR2
  // 1 stop bit is the default -- leave alone
  // -- Configure CR1
  // 1 start bit, 8 data bits is the defualt -- leave alone
  // No parity is the default -- leave alone
  // -- Configure BRR (baud rate reg)
  // PCLK1 = HCLK (SYSCLK/1) / 4 = 168 / 4 = 42Mhz
  // oversampling = 16
  uint32_t integerdivider = ((25 * SERIAL_CLK) / (4 * (115200)));
  */
}
