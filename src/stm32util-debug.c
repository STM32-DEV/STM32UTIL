#include "main.h"
#include "usart.h"
#include "stm32util-debug.h"
#include "stm32util-uart.h"

#if STM32UTIL_DEBUG_USE_HAL
extern UART_HandleTypeDef STM32UTIL_DEBUG_HUART;
#define DEBUG_UART	(&STM32UTIL_DEBUG_HUART)
#elif STM32UTIL_DEBUG_USE_LL
#define DEBUG_UART	STM32UTIL_DEBUG_UART
#endif

// Indicates if the debug UART is initialized
static bool debug_uart_inited = false;

bool stm32util_debug_init()
{
	if (!debug_uart_inited) { // first check
		debug_uart_inited = true;
	}

	return true;
}

/*
	Override the weak function to redirect printf and other outputs to UART.
 */
int _write(int file, char* ptr, int len)
{
	stm32util_debug_init();

	stm32util_uart_transmit(DEBUG_UART, (uint8_t*)ptr, len);

	return len;
}
