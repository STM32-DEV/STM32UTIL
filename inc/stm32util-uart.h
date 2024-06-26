#ifndef __STM32UTIL_UART_H__
#define __STM32UTIL_UART_H__

#include "stm32util-base.h"

#if __cplusplus
extern "C" {
#endif

#if STM32UTIL_UART_USE_HAL
void stm32util_uart_transmit(UART_HandleTypeDef* huart, uint8_t* ptr, int len);
void stm32util_uart_transmit_dma(UART_HandleTypeDef* huart, uint8_t* ptr, int len);
#endif

#if STM32UTIL_UART_USE_LL
void stm32util_uart_transmit(USART_TypeDef* USARTx, uint8_t* ptr, int len);
void stm32util_uart_transmit_dma(USART_TypeDef* USARTx, uint8_t* ptr, int len, DMA_TypeDef* dma, uint32_t runnel);
void stm32util_uart_setup_tx_dma(USART_TypeDef* USARTx, DMA_TypeDef* DMAx, uint32_t runnel);
#endif

#if __cplusplus
}
#endif

#endif // __STM32UTIL_UART_H__
