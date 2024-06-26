#ifndef __STM32UTIL_DEBUG_UART_H__
#define __STM32UTIL_DEBUG_UART_H__

#include "stm32util-base.h"

#if !STM32UTIL_DEBUG_UART_USE_LL
#define STM32UTIL_DEBUG_UART_USE_HAL		1
#endif

#if STM32UTIL_DEBUG_UART_USE_HAL && STM32UTIL_DEBUG_UART_USE_LL
#error "Configuration error: Both HAL and LL drivers are defined. Please define only one: STM32UTIL_DEBUG_USE_HAL or STM32UTIL_DEBUG_USE_LL."
#endif

#if STM32UTIL_DEBUG_UART_RUNNEL == LL_DMA_RUNNEL_0
#define DEBUG_UART_RUNNEL_NUM	0
#elif STM32UTIL_DEBUG_UART_RUNNEL == LL_DMA_RUNNEL_1
#define DEBUG_UART_RUNNEL_NUM	1
#elif STM32UTIL_DEBUG_UART_RUNNEL == LL_DMA_RUNNEL_2
#define DEBUG_UART_RUNNEL_NUM	2
#elif STM32UTIL_DEBUG_UART_RUNNEL == LL_DMA_RUNNEL_3
#define DEBUG_UART_RUNNEL_NUM	3
#elif STM32UTIL_DEBUG_UART_RUNNEL == LL_DMA_RUNNEL_4
#define DEBUG_UART_RUNNEL_NUM	4
#elif STM32UTIL_DEBUG_UART_RUNNEL == LL_DMA_RUNNEL_5
#define DEBUG_UART_RUNNEL_NUM	5
#elif STM32UTIL_DEBUG_UART_RUNNEL == LL_DMA_RUNNEL_6
#define DEBUG_UART_RUNNEL_NUM	6
#elif STM32UTIL_DEBUG_UART_RUNNEL == LL_DMA_RUNNEL_7
#define DEBUG_UART_RUNNEL_NUM	7
#endif

#if __cplusplus
extern "C" {
#endif

#define STM32UTIL_DEBUG_UART_DMA_IsActiveFlag_TC()	CONCAT_TOKEN(LL_DMA_IsActiveFlag_TC, TO_NUM(DEBUG_UART_RUNNEL_NUM))(STM32UTIL_DEBUG_UART_DMA)
#define STM32UTIL_DEBUG_UART_DMA_ClearFlag_TC()		CONCAT_TOKEN(LL_DMA_ClearFlag_TC, TO_NUM(DEBUG_UART_RUNNEL_NUM))(STM32UTIL_DEBUG_UART_DMA)
void STM32UTIL_DEBUG_UART_DMA_TC_isr();
void STM32UTIL_DEBUG_UART_TC_isr();

#if __cplusplus
}
#endif

#endif // __STM32UTIL_DEBUG_UART_H__
