#include <string.h>
#include "main.h"
#include "usart.h"
//#include "stm32util-debug.h"
#include "stm32util-debug-uart.h"

#if STM32UTIL_DEBUG_UART_USE_HAL
extern UART_HandleTypeDef STM32UTIL_DEBUG_HUART;
#define DEBUG_UART	(&STM32UTIL_DEBUG_HUART)
#elif STM32UTIL_DEBUG_UART_USE_LL
#define DEBUG_UART	STM32UTIL_DEBUG_UART
#endif

#if STM32UTIL_DEBUG_UART_USE_HAL
void stm32util_uart_transmit(uint8_t* ptr, int len)
{
	HAL_UART_Transmit(DEBUG_UART, ptr, len, HAL_MAX_DELAY);

}

void stm32util_uart_transmit_dma(uint8_t* ptr, int len)
{
	HAL_UART_Transmit_DMA(DEBUG_UART, ptr, len);
}

#elif STM32UTIL_DEBUG_UART_USE_LL
void stm32util_uart_transmit(uint8_t* ptr, int len)
{
	while (len--) {
		while (!LL_USART_IsActiveFlag_TXE_TXFNF(DEBUG_UART)) {
		}
		LL_USART_TransmitData8(DEBUG_UART, *ptr++);
	}
}

void stm32util_uart_transmit_dma(uint8_t* ptr, int len)
{
	LL_DMA_DisableRunnel(STM32UTIL_DEBUG_UART_DMA, STM32UTIL_DEBUG_UART_RUNNEL);
	LL_DMA_SetMemoryAddress(STM32UTIL_DEBUG_UART_DMA, STM32UTIL_DEBUG_UART_RUNNEL, (uint32_t)ptr);
	LL_DMA_SetDataLength(STM32UTIL_DEBUG_UART_DMA, STM32UTIL_DEBUG_UART_RUNNEL, len);
	LL_DMA_EnableRunnel(STM32UTIL_DEBUG_UART_DMA, STM32UTIL_DEBUG_UART_RUNNEL);
}

void stm32util_uart_setup_tx_dma()
{
	LL_USART_EnableDMAReq_TX(DEBUG_UART);
	LL_USART_EnableIT_TC(DEBUG_UART);
	LL_DMA_EnableIT_TC(STM32UTIL_DEBUG_UART_DMA, STM32UTIL_DEBUG_UART_RUNNEL);
	LL_DMA_SetPeriphAddress(STM32UTIL_DEBUG_UART_DMA, STM32UTIL_DEBUG_UART_RUNNEL, LL_USART_DMA_GetRegAddr(DEBUG_UART, LL_USART_DMA_REG_DATA_TRANSMIT));
}

#endif

// Indicates if the debug UART is initialized
/*_AT_ITCMRAM*/ static volatile bool stm32util_debug_inited = false;

// Transmit buffer for UART
/* _AT_DMATX_SRAM1 */ __attribute__((aligned(32))) static uint8_t tx_buffer[STM32UTIL_DEBUG_UART_TX_BUFFER_SIZE];

//
/*_AT_ITCMRAM*/ static volatile bool stm32util_tx_end = true;

/**
 * @brief Callback function for UART transmission complete.
 *
 * @param hdma UART handle.
 */
#if STM32UTIL_DEBUG_UART_USE_HAL
/*_AT_ITCMRAM*/ void debug_uart_tx_completed(UART_HandleTypeDef *hdma)
{
#if STM32UTIL_USE_OS
	osSemaphoreRelease(debug_uart_sema);
#else
	stm32util_tx_end = true;
#endif
}

#elif STM32UTIL_DEBUG_UART_USE_LL
/*_AT_ITCMRAM*/ void STM32UTIL_DEBUG_UART_DMA_TC_isr()
{
	//osSemaphoreRelease(debug_uart_sema);
}

/*_AT_ITCMRAM*/ void STM32UTIL_DEBUG_UART_TC_isr()
{
#if STM32UTIL_USE_OS
	osSemaphoreRelease(debug_uart_sema);
#else
	stm32util_tx_end = true;
#endif
}

#endif

static void uart_wait_tx_completed()
{
#if STM32UTIL_USE_OS
	// Wait for the transmission to complete, RM0433r8 p2060 Figure 590
	while (osSemaphoreAcquire(debug_uart_sema, osWaitForever) != osOK) {
	}
#else
	while (!stm32util_tx_end) {
	}
#endif
}

bool stm32util_debug_init()
{
	if (!stm32util_debug_inited) { // first check
#if STM32UTIL_DEBUG_UART_USE_HAL
		if (HAL_OK != HAL_UART_RegisterCallback(DEBUG_UART, HAL_UART_TX_COMPLETE_CB_ID, debug_uart_tx_completed)) {
			//CRITICAL("HAL_UART_RegisterCallback debug_uart");
		}
#elif STM32UTIL_DEBUG_UART_USE_LL
		stm32util_uart_setup_tx_dma();
#endif

		stm32util_debug_inited = true;
	}

	return true;
}

/*
	Override the weak function to redirect printf and other outputs to UART.
 */
int _write(int file, char* ptr, int len)
{
	stm32util_debug_init();

	/*
		Transmit the smallest chunk first, which is the remainder of the length
        	divided by the size of the transmit buffer. This makes waiting for DMA
        	transmission more efficient.
			ex) len = 555, tx max = 256
			    1st: 555 mod 256 = 43, 2nd: 256, 3rd: 256
			ex) len = 512, tx max = 256
			    1st: 512 mod 256 = 0(256), 2nd: 256
	 */
	// Initialize the remaining length to the total length to be transmitted
	int remaining_len = len;
	// Calculate the initial chunk size, which is the remainder of the buffer size
	// STM32UTIL_DEBUG_UART_TX_BUFFER_SIZE MUST be a power of 2
	int chunk = remaining_len & (STM32UTIL_DEBUG_UART_TX_BUFFER_SIZE - 1);
	if (0 == chunk) {
		chunk = STM32UTIL_DEBUG_UART_TX_BUFFER_SIZE;
	}

	// Transmit data using DMA
	//
	while (remaining_len) {
		uart_wait_tx_completed();

		// Copy data to the transmit buffer
		memcpy(tx_buffer, ptr, chunk);

		// Clean the data cache to ensure data coherence
		SCB_CleanDCache_by_Addr((uint32_t*)tx_buffer, chunk);

		stm32util_uart_transmit_dma(tx_buffer, chunk);

#if !STM32UTIL_USE_OS
		stm32util_tx_end = false;
#endif

		// Move the pointer forward and adjust the remaining length
		ptr += chunk;
		remaining_len -= chunk;
		chunk = STM32UTIL_DEBUG_UART_TX_BUFFER_SIZE;
	}

	return len;
}