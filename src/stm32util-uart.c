#include "main.h"
#include "usart.h"
#include "stm32util-debug.h"
#include "stm32util-uart.h"

#if STM32UTIL_UART_USE_HAL
void stm32util_uart_transmit(UART_HandleTypeDef* huart, uint8_t* ptr, int len)
{
	HAL_UART_Transmit(huart, ptr, len, HAL_MAX_DELAY);

}

void stm32util_uart_transmit_dma(UART_HandleTypeDef* huart, uint8_t* ptr, int len)
{
	HAL_UART_Transmit_DMA(huart, ptr, len);
}
#endif

#if STM32UTIL_UART_USE_LL
void stm32util_uart_transmit(USART_TypeDef* USARTx, uint8_t* ptr, int len)
{
	while (len--) {
		while (!LL_USART_IsActiveFlag_TXE_TXFNF(USARTx)) {
		}
		LL_USART_TransmitData8(USARTx, *ptr++);
	}
}

void stm32util_uart_transmit_dma(USART_TypeDef* USARTx, uint8_t* ptr, int len, DMA_TypeDef* dma, uint32_t runnel)
{
	LL_DMA_DisableRunnel(dma, runnel);
	LL_DMA_SetMemoryAddress(dma, runnel, (uint32_t)ptr);
	LL_DMA_SetDataLength(dma, runnel, len);
	LL_DMA_EnableRunnel(dma, runnel);
}

void stm32util_uart_setup_tx_dma(USART_TypeDef* USARTx, DMA_TypeDef* DMAx, uint32_t runnel)
{
	LL_USART_EnableDMAReq_TX(USARTx);
	LL_USART_EnableIT_TC(USARTx);
	LL_DMA_EnableIT_TC(DMAx, runnel);
	LL_DMA_SetPeriphAddress(DMAx, runnel, LL_USART_DMA_GetRegAddr(USARTx, LL_USART_DMA_REG_DATA_TRANSMIT));
}
#endif
