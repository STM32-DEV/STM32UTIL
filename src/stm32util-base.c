#include "stm32util-base.h"
#if STM32UTIL_USE_OS
#include <cmsis_os2.h>
#include <FreeRTOS.h>
#endif

uint32_t stm32util_get_tick()
{
#if STM32UTIL_USE_OS
	if (osKernelRunning != osKernelGetState()) {
		return HAL_GetTick();
	}
	return osKernelGetTickCount();
#else
	return HAL_GetTick();
#endif
}
