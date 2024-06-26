#include "main.h"
#include "usart.h"
#include "stm32-debug.h"

/*
	Override the weak function to redirect printf and other outputs to UART.
 */
int _write(int file, char* ptr, int len)
{
	HAL_UART_Transmit(&huart3, (uint8_t*)ptr, len, HAL_MAX_DELAY);

	return len;
}
