#include "stm32f7xx_hal.h"
#include "serial.h"
#include "event.h"

static UART_HandleTypeDef VCPHandle;

void serialInit(void)
{
  
  VCPHandle.Instance        = VCP;

  VCPHandle.Init.BaudRate   = VCP_BAUD;
  VCPHandle.Init.WordLength = UART_WORDLENGTH_8B;
  VCPHandle.Init.StopBits   = UART_STOPBITS_1;
  VCPHandle.Init.Parity     = UART_PARITY_NONE;
  VCPHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  VCPHandle.Init.Mode       = UART_MODE_TX_RX;
  VCPHandle.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&VCPHandle);

  // use unbuffered IO
  setvbuf(stdout,NULL,_IONBF,0);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  VCP_TX_GPIO_CLK_ENABLE();
  VCP_RX_GPIO_CLK_ENABLE();
  
  /* Select SysClk as source of USART1 clocks */
  RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  RCC_PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

  /* Enable USARTx clock */
  VCP_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = VCP_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = VCP_TX_AF;

  HAL_GPIO_Init(VCP_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = VCP_RX_PIN;
  GPIO_InitStruct.Alternate = VCP_RX_AF;

  HAL_GPIO_Init(VCP_RX_GPIO_PORT, &GPIO_InitStruct);

  /* enable interrupt in NVIC */
  HAL_NVIC_SetPriority(VCP_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(VCP_IRQn);
}

/* 
   NOTE: not safe AT ALL. Its fine if only the VT100
   code is using this, but if something else calls
   this register event outside of VT100's lock, the
   whole thing will get hosed.
 */
static struct event *vcp_event = NULL;
void serial_register_event(struct event *e)
{
  vcp_event = e;
}

void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&VCPHandle);
}

int _write(int file, char *ptr, int len)
{
  (void)file;
  HAL_UART_Transmit_IT(&VCPHandle, ptr, len);
  return len;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (vcp_event) {
    event_notify(vcp_event);
    vcp_event = NULL;
  }
}

