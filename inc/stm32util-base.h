#if !defined(__STM32UTIL_BASE_H__)
#define __STM32UTIL_BASE_H__

#include "stm32util-conf.h"
#include <stdbool.h>
#include <unistd.h>
#include "main.h"

#define DIMOF(a) ( sizeof(a) / sizeof( (a)[0] ) )

#define UNUSE(a) ((void)(a))

/**
 * @brief Check if a number is a power of 2.
 *
 * This macro checks if the given integer `a` is a power of 2.
 * It first ensures that `a` is not zero, and then checks if `a`
 * has only one bit set to 1, which is the characteristic of powers of 2.
 *
 * @param a The integer to check.
 * @return True if `a` is a power of 2 and not zero, otherwise false.
 */
#define IS_POWER_OF_2(a)	( (0 != a) && (((a) & ((a) - 1)) == 0) )

// Stringify
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define CONCAT_STR(a,b) a b

#define CONCAT_TOKEN_(a, b)	a##b
#define CONCAT_TOKEN(a, b)	CONCAT_TOKEN_(a,b)
#define TO_NUM_(a)		a
#define TO_NUM(a)		TO_NUM_(a)

// GPIO
#define MX_GPIO(a)		a##_GPIO_Port, a##_Pin

#if defined(LL_DMA_STREAM_ALL)
#define LL_DMA_RUNNEL_0			LL_DMA_STREAM_0
#define LL_DMA_RUNNEL_1			LL_DMA_STREAM_1
#define LL_DMA_RUNNEL_2			LL_DMA_STREAM_2
#define LL_DMA_RUNNEL_3			LL_DMA_STREAM_3
#define LL_DMA_RUNNEL_4			LL_DMA_STREAM_4
#define LL_DMA_RUNNEL_5			LL_DMA_STREAM_5
#define LL_DMA_RUNNEL_6			LL_DMA_STREAM_6
#define LL_DMA_RUNNEL_7			LL_DMA_STREAM_7
#define LL_DMA_DisableRunnel(a,b)	LL_DMA_DisableStream(a,b)
#define LL_DMA_EnableRunnel(a,b)	LL_DMA_EnableStream(a,b)
#endif

#if defined(LL_DMA_CHANNEL_ALL)
#define LL_DMA_RUNNEL_0			LL_DMA_CHANNEL_0
#define LL_DMA_RUNNEL_1			LL_DMA_CHANNEL_1
#define LL_DMA_RUNNEL_2			LL_DMA_CHANNEL_2
#define LL_DMA_RUNNEL_3			LL_DMA_CHANNEL_3
#define LL_DMA_RUNNEL_4			LL_DMA_CHANNEL_4
#define LL_DMA_RUNNEL_5			LL_DMA_CHANNEL_5
#define LL_DMA_RUNNEL_6			LL_DMA_CHANNEL_6
#define LL_DMA_RUNNEL_7			LL_DMA_CHANNEL_7
#define LL_DMA_DisableRunnel(a,b)	LL_DMA_DisableChannel(a,b)
#define LL_DMA_EnableRunnel(a,b)	LL_DMA_EnableChannel(a,b)
#endif

#if defined(__cplusplus)
extern "C" {
#endif

uint32_t stm32util_get_tick();

#if defined(__cplusplus)
}
#endif

#endif // __STM32UTIL_BASE_H__
