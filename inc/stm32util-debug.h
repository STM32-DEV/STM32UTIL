#if !defined(__STM32UTIL_DEBUG_H__)
#define __STM32UTIL_DEBUG_H__

#include "stm32util-base.h"

#if defined(DEBUG)
#	if !defined(STM32UTIL_DEBUG_ENABLE)
#		define STM32UTIL_DEBUG_ENABLE	1
#	endif
#	if !defined(STM32UTIL_ASSERT_ENABLE)
#		define STM32UTIL_ASSERT_ENABLE	1
#	endif
#endif

#define STM32UTIL_DEBUG_GROUP_ALL	0xFFFFFFFF
#define STM32UTIL_DEBUG_GROUP_DEFAULT	0x00000001
#define STM32UTIL_DEBUG_GROUP_ETH	0x80000000
#define STM32UTIL_DEBUG_GROUP_SDMMC	0x40000000
#define STM32UTIL_DEBUG_GROUP_FATFS	0x20000000
#define STM32UTIL_DEBUG_GROUP_HSM	0x10000000

#if !defined(STM32UTIL_DEBUG_GROUP)
#define STM32UTIL_DEBUG_GROUP		STM32UTIL_DEBUG_GROUP_DEFAULT
#endif

#if !defined(STM32UTIL_DEBUG_GROUP_CURRENT)
#define STM32UTIL_DEBUG_GROUP_CURRENT	STM32UTIL_DEBUG_GROUP_DEFAULT
#endif

#if !defined(STM32UTIL_DEBUG_LEVEL)
#define STM32UTIL_DEBUG_LEVEL		9	// -1: disable / 0 ~ 9
#endif

#define STM32UTIL_DEBUG_LEVEL_PRINTF	0
#define STM32UTIL_DEBUG_LEVEL_DPRINTF	1
#define STM32UTIL_DEBUG_LEVEL_ERROR	2
#define STM32UTIL_DEBUG_LEVEL_WARNING	3
#define STM32UTIL_DEBUG_LEVEL_TRACE	4

#define DBG_WRAP(a)		do if (stm32util_debug_get_group() & STM32UTIL_DEBUG_GROUP) { a; } while (0)

#if STM32UTIL_DEBUG_ENABLE && STM32UTIL_DEBUG_LEVEL_PRINTF <= STM32UTIL_DEBUG_LEVEL
#define PRINTF(...)		DBG_WRAP( printf(__VA_ARGS__) )
#else
#define PRINTF(...)
#endif

#if STM32UTIL_DEBUG_ENABLE && STM32UTIL_DEBUG_LEVEL_DPRINTF <= STM32UTIL_DEBUG_LEVEL
#define DPRINTF(...)		DBG_WRAP( stm32util_debug_printf(STM32UTIL_DEBUG_GROUP_CURRENT, STM32UTIL_DEBUG_LEVEL_DPRINTF, __LINE__, __func__, __FILE__, __VA_ARGS__) )
#else
#define DPRINTF(...)
#endif

#if STM32UTIL_DEBUG_ENABLE && STM32UTIL_DEBUG_LEVEL_ERROR <= STM32UTIL_DEBUG_LEVEL
#define ERROR(...)		DBG_WRAP( stm32util_debug_printf(STM32UTIL_DEBUG_GROUP_CURRENT, STM32UTIL_DEBUG_LEVEL_ERROR, __LINE__, __func__, __FILE__, __VA_ARGS__) )
#else
#define ERROR(...)
#endif

#if STM32UTIL_DEBUG_ENABLE && STM32UTIL_DEBUG_LEVEL_WARNING <= STM32UTIL_DEBUG_LEVEL
#define WARNING(...)		DBG_WRAP( stm32util_debug_printf(STM32UTIL_DEBUG_GROUP_CURRENT, STM32UTIL_DEBUG_LEVEL_WARNING, __LINE__, __func__, __FILE__, __VA_ARGS__) )
#else
#define WARNING(...)
#endif

#if STM32UTIL_DEBUG_ENABLE && STM32UTIL_DEBUG_LEVEL_TRACE <= STM32UTIL_DEBUG_LEVEL
#define TRACE(...)		DBG_WRAP( stm32util_debug_printf(STM32UTIL_DEBUG_GROUP_CURRENT, STM32UTIL_DEBUG_LEVEL_TRACE, __LINE__, __func__, __FILE__, __VA_ARGS__) )
#else
#define TRACE(...)
#endif

/**
	There is a likelihood of memory shortage or systemic issues, so prepare the string in advance,
	and determine the string length at compile time.
	__func__ does not operate as a string constant, so it is passed as a separate argument.
 */
#if STM32UTIL_DEBUG_ENABLE
#define CRITICAL(msg) do {												\
		static const char critical_loc_str[] = "() " __FILE__ "(" TOSTRING(__LINE__) ")";			\
		stm32util_debug_critical(msg, __func__, sizeof __func__, critical_loc_str, sizeof critical_loc_str);	\
	} while (0)
#else
#define CRITICAL(msg)		stm32util_debug_critical(msg, sizeof msg, NULL, 0, NULL, 0)
#endif

/**

 */
#if STM32UTIL_ASSERT_ENABLE
#define ASSERT(cond, ...) do {									\
		if (!(cond)) {									\
			stm32util_assert(#cond, __LINE__, __func__, __FILE__, __VA_ARGS__);	\
		}										\
	} while (0)
#else
#define ASSERT(cond, ...)
#endif

// Safely format a string into a buffer, ensuring null-termination.
#define SAFE_FMT(n, f, b, s, ...)		\
	int n = f(b,s-1,__VA_ARGS__);	\
	if (n < 0) {			\
		n = 0;			\
		(b)[0] = 0;		\
	} else if (s - 1 < n) {		\
		n = s - 1;		\
		(b)[n] = 0;		\
	}

#if defined(__cplusplus)
extern "C" {
#endif

uint32_t stm32util_debug_get_group();
uint32_t stm32util_debug_set_group(uint32_t group);
uint32_t stm32util_debug_reset_group(uint32_t group);
int stm32util_debug_get_level();
int stm32util_debug_set_level(int level);
int stm32util_debug_printf(uint32_t group, int level, int line, const char* func, const char* file, const char* fmt, ...);

void stm32util_debug_critical(const char* msg, const char* func, int funclen, const char* loc, int loclen) __attribute__((noreturn));
void stm32util_assert(const char* cond, int line, const char* func, const char* file, const char* fmt, ...) __attribute__((noreturn));

#if defined(__cplusplus)
}
#endif


#endif // __STM32UTIL_DEBUG_H__
