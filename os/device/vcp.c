#include "board.h"
#include "event.h"
#include "mutex.h"
#include "os_printf.h"
#include "ring_buffer.h"
#include <ctype.h>
#include "vcp.h"

static UART_HandleTypeDef VCPHandle;
static event_t vcp_tx_complete = EVENT_STATIC_INIT(vcp_tx_complete);
static event_t vcp_rx_complete = EVENT_STATIC_INIT(vcp_rx_complete);
static mutex_t serial_tx_lock = MUTEX_STATIC_INIT(serial_tx_lock);
static mutex_t serial_rx_lock = MUTEX_STATIC_INIT(serial_rx_lock);

static RB_STATIC_CREATE(serial_rx_buffer, 128);
static RB_STATIC_CREATE(serial_tx_buffer, 128);

static int ansi_esc(void);

void serialInit(void)
{

	VCPHandle.Instance = VCP;

	VCPHandle.Init.BaudRate = VCP_BAUD;
	VCPHandle.Init.WordLength = UART_WORDLENGTH_8B;
	VCPHandle.Init.StopBits = UART_STOPBITS_1;
	VCPHandle.Init.Parity = UART_PARITY_NONE;
	VCPHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	//VCPHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
	VCPHandle.Init.Mode = UART_MODE_TX_RX;
	VCPHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&VCPHandle);

	// enable data register NOT empty irq
	VCP->CR1 |= USART_CR1_RXNEIE;
	// enable the transmit complete irq
	VCP->CR1 |= USART_CR1_TCIE;

	// use unbuffered IO
	setvbuf(stdout, NULL, _IONBF, 0);
}

void HAL_UART_MspInit(__attribute__((unused))  UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	VCP_TX_GPIO_CLK_ENABLE()
	;
	VCP_RX_GPIO_CLK_ENABLE()
	;

	/* Configure the USART1 clock source */
	HAL_RCC_VCP_CONFIG(RCC_VCPCLKSOURCE_SYSCLK);

	/* Enable USARTx clock */
	VCP_CLK_ENABLE()
	;

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
void VCP_IRQHandler(void)
{
	uint32_t isrflags = VCP->ISR;
	// RX mode
	if (isrflags & USART_ISR_RXNE)
	{
		Ringbuffer.insert(&serial_rx_buffer.rb, VCP->RDR);
		event_notify_irq(&vcp_rx_complete);
	}
	// TX complete
	if (isrflags & USART_ISR_TC)
	{
		// clear TC
		VCP->ICR |= USART_ICR_TCCF;
		if (!Ringbuffer.empty(&serial_tx_buffer.rb)) {
			VCP->TDR = Ringbuffer.remove(&serial_tx_buffer.rb);
		}
		else {
			__disable_irq();
			event_notify_irq(&vcp_tx_complete);
			__enable_irq();
		}
	}
}

/* int _write(int file, char *ptr, int len) */
/* { */
/*   (void)file; */
/*   HAL_UART_Transmit_IT(&VCPHandle, ptr, len); */
/*   return len; */
/* } */

static inline
uint8_t vcp_rx_byte(void)
{
	if (Ringbuffer.empty(&serial_rx_buffer.rb))
		event_wait(&vcp_rx_complete);

	return Ringbuffer.remove(&serial_rx_buffer.rb);
}

static inline
void vcp_tx_byte(uint8_t byte)
{
	char tx = byte;
	mutex_lock(&serial_tx_lock);
	// set TX buffer
	VCP->TDR = byte;
	mutex_unlock(&serial_tx_lock);

	event_wait(&vcp_tx_complete);
}

static inline
void vcp_tx_string(const char * buffer, int len)
{
	// put all but the first byte into the ringbuffer
	Ringbuffer.insert_string(&serial_tx_buffer.rb, buffer + 1, len -1);

	mutex_lock(&serial_tx_lock);
	// send the first byte
	VCP->TDR = buffer[0];
	mutex_unlock(&serial_tx_lock);

	// the entire string will be sent from the ringbuffer
	// before the event returns
	event_wait(&vcp_tx_complete);
}

int os_puts_vcp(char *buffer, int len)
{
	vcp_tx_string(buffer, len);
	return len;
}

int os_gets_vcp(char *buffer, int len)
{
	bool done = false;
	char *p = buffer;
	while (!done && (p - buffer < len))
	{
		*p = vcp_rx_byte();

		char echo_char = *p;
		switch (*p)
		{
		case ASCII_ETX: // CTRL-C
			return CONTROL_C;
			break;
		case ASCII_EOT: // CTRL-D
			return CONTROL_C;
			break;
		case ASCII_BS:  // CTRL-H
			if (p > buffer)
				p -= 1;
			else
				echo_char = 0;
			break;
		case ASCII_LF:
			break;
		case ASCII_CR:
			vcp_tx_byte(ASCII_LF);
			done = true;
			break;
		case ASCII_ESC:
			echo_char = 0;
			ansi_esc();
			break;
		default:
			p++;
			break;
		}

		// echo received char
		if (echo_char)
			vcp_tx_byte(echo_char);

	}

	*p++ = '\0'; // null terminate result
	//os_iprintf("BUFFER: [%s]\r\n", buffer);
	return (p - buffer);
}

static int ansi_esc(void)
{
	char next = vcp_rx_byte();
	switch (next) {
	case '[': // CSI
		// eat up until final byte
		do {
			next = vcp_rx_byte();
		} while (next < 0x40 && next > 0x7e);
		break;
	default:
		break;
	}
	return 0;
}
