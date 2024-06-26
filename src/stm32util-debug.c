#include <stm32util-debug.h>
#include "main.h"
#include "usart.h"


#if STM32UTIL_DEBUG_USE_HAL
extern UART_HandleTypeDef STM32UTIL_DEBUG_UART;
#define DEBUG_UART	(&STM32UTIL_DEBUG_UART)
#else
#define DEBUG_UART	STM32UTIL_DEBUG_UART
#endif


#if STM32UTIL_DEBUG_USE_HAL
void stm32util_uart_transmit(UART_HandleTypeDef* huart, uint8_t* ptr, int len)
{
	HAL_UART_Transmit(huart, ptr, len, HAL_MAX_DELAY);

}

#elif STM32UTIL_DEBUG_USE_LL
void stm32util_uart_transmit(USART_TypeDef* USARTx, uint8_t* ptr, int len)
{
	while (len--) {
		while (!LL_USART_IsActiveFlag_TXE_TXFNF(USARTx)) {
		}
		LL_USART_TransmitData8(USARTx, *ptr++);
	}
}

#endif

/*
	Override the weak function to redirect printf and other outputs to UART.
 */
int _write(int file, char* ptr, int len)
{
	stm32util_uart_transmit(DEBUG_UART, (uint8_t*)ptr, len);

	return len;
}
