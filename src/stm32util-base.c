#include "stm32util-base.h"
#if STM32UTIL_USE_OS
#include <cmsis_os2.h>
#include <FreeRTOS.h>
#endif

uint32_t stm32util_get_tick()
{
#if STM32UTIL_USE_OS
	//if (osKernelRunning != osKernelGetState()) {
	//	return HAL_GetTick();
	//}
	return osKernelGetTickCount();
#else
	return HAL_GetTick();
#endif
}

void stm32util_delay(uint32_t ms)
{
#if STM32UTIL_USE_OS
	if (osKernelRunning != osKernelGetState()) {
		HAL_Delay(ms);
		return;
	}
	osDelay(ms);
#else
	HAL_Delay(ms);
#endif
}
