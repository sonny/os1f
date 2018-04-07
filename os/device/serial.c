#include "stm32f7xx_hal.h"
#include "board.h"
#include "serial.h"
#include "event.h"
#include "mutex.h"
#include "os_printf.h"
#include <ctype.h>

static UART_HandleTypeDef VCPHandle;
static event_t VCP_TX_complete = EVENT_STATIC_INIT(VCP_TX_complete);
static event_t VCP_RX_complete = EVENT_STATIC_INIT(VCP_RX_complete);
static mutex_t serial_tx_lock = MUTEX_STATIC_INIT(serial_tx_lock);
static mutex_t serial_rx_lock = MUTEX_STATIC_INIT(serial_rx_lock);

void serialInit(void) {

	VCPHandle.Instance = VCP;

	VCPHandle.Init.BaudRate = VCP_BAUD;
	VCPHandle.Init.WordLength = UART_WORDLENGTH_8B;
	VCPHandle.Init.StopBits = UART_STOPBITS_1;
	VCPHandle.Init.Parity = UART_PARITY_NONE;
	VCPHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	VCPHandle.Init.Mode = UART_MODE_TX_RX;
	VCPHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&VCPHandle);

	// use unbuffered IO
	setvbuf(stdout, NULL, _IONBF, 0);
}

void HAL_UART_MspInit( __attribute__((unused)) UART_HandleTypeDef *huart) {
	GPIO_InitTypeDef GPIO_InitStruct;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	VCP_TX_GPIO_CLK_ENABLE();
	VCP_RX_GPIO_CLK_ENABLE();

	/* Configure the USART1 clock source */
	HAL_RCC_VCP_CONFIG(RCC_VCPCLKSOURCE_SYSCLK);


	/* Enable USARTx clock */
	VCP_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* UART TX GPIO pin configuration  */
	GPIO_InitStruct.Pin = VCP_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
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

//void USART1_IRQHandler(void)
void VCP_IRQHandler(void) {
	HAL_UART_IRQHandler(&VCPHandle);
}

/* int _write(int file, char *ptr, int len) */
/* { */
/*   (void)file; */
/*   HAL_UART_Transmit_IT(&VCPHandle, ptr, len); */
/*   return len; */
/* } */


int os_puts_vcp(char *buffer, int len) {
	mutex_lock(&serial_tx_lock);
	HAL_UART_Transmit_IT(&VCPHandle, (unsigned char*) buffer, len);
	event_wait(&VCP_TX_complete);
	mutex_unlock(&serial_tx_lock);
	return len;
}

static inline
char rx_byte(void)
{
	char byte;
	mutex_lock(&serial_rx_lock);
	HAL_UART_Receive_IT(&VCPHandle, &byte, 1);
	event_wait(&VCP_RX_complete);
	mutex_unlock(&serial_rx_lock);
	return byte;
}

static inline
void tx_byte(char byte)
{
	char tx = byte;
	mutex_lock(&serial_tx_lock);
	HAL_UART_Transmit_IT(&VCPHandle, &tx, 1);
	event_wait(&VCP_TX_complete);
	mutex_unlock(&serial_tx_lock);
}

int os_gets_vcp(char *buffer, int len) {
	bool done = false;
	char *p = buffer;
	while (!done && (p - buffer < len)) {
		*p = rx_byte();

		char echo_char = *p;
		switch (*p) {
		case ASCII_ETX: // CTRL-C
			return CONTROL_C;
			break;
		case ASCII_EOT: // CTRL-D
			return CONTROL_C;
			break;
		case ASCII_BS:  // CTRL-H
			if (p > buffer) p -= 1;
			else echo_char = 0;
			break;
		case ASCII_LF:
			break;
		case ASCII_CR:
			tx_byte(ASCII_LF);
			done = true;
			break;
		case ASCII_ESC:
			echo_char = '.';
			done = true;
			break;
		default:
			p++;
			break;
		}

		// echo received char
		if (echo_char) tx_byte(echo_char);

	}

	*p++ = '\0'; // null terminate result
	os_iprintf("BUFFER: [%s]\r\n", buffer);
	return (p - buffer);
}

int ansii_esc(char *b, int len, bool echo)
{
	bool done = false;
	char * p = b;
//	while (!done && (p-b > len))
//	{
//		char byte;
//	}
	return 0;
}

void HAL_UART_TxCpltCallback( __attribute__((unused)) UART_HandleTypeDef *huart) {
	event_notify_irq(&VCP_TX_complete);
}

void HAL_UART_RxCpltCallback( __attribute__((unused)) UART_HandleTypeDef *huart) {
	event_notify_irq(&VCP_RX_complete);
}

