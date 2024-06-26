#if !defined(__STM32UTIL_DEBUG_H__)
#define __STM32UTIL_DEBUG_H__

#include "stm32util-conf.h"

#if !STM32UTIL_DEBUG_USE_LL
#define STM32UTIL_DEBUG_USE_HAL		1
#endif

#if STM32UTIL_DEBUG_USE_HAL && STM32UTIL_DEBUG_USE_LL
#error "Configuration error: Both HAL and LL drivers are defined. Please define only one: STM32UTIL_DEBUG_USE_HAL or STM32UTIL_DEBUG_USE_LL."
#endif

#endif // __STM32UTIL_DEBUG_H__
