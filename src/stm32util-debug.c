#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "stm32util-debug.h"
#include "stm32util-debug-uart.h"
#if STM32UTIL_USE_OS
#include <cmsis_os2.h>
#include <FreeRTOS.h>
#endif

/* Global variables for debug group and level */
static uint32_t dbggroup = STM32UTIL_DEBUG_GROUP_ALL;
static int dbglevel = STM32UTIL_DEBUG_LEVEL;

/**
 * @brief Get the current debug group.
 *
 * @return Current debug group value.
 */
uint32_t stm32util_debug_get_group()
{
	return dbggroup;
}

/**
 * @brief Set the debug group.
 *
 * @param group New debug group to set.
 * @return Previous debug group value.
 */
uint32_t stm32util_debug_set_group(uint32_t group)
{
	uint32_t prev = dbggroup;
	dbggroup |= group;
	return prev;
}

/**
 * @brief Reset the debug group.
 *
 * @param group Debug group to reset.
 * @return Previous debug group value.
 */
uint32_t stm32util_debug_reset_group(uint32_t group)
{
	uint32_t prev = stm32util_debug_get_group();
	dbggroup &= ~group;
	return prev;
}

/**
 * @brief Get the current debug level.
 *
 * @return Current debug level value.
 */
int stm32util_debug_get_level()
{
	return dbglevel;
}

/**
 * @brief Set the debug level.
 *
 * @param level New debug level to set.
 * @return Previous debug level value.
 */
int dbgsetlevel(int level)
{
	int prev = stm32util_debug_get_level();
	dbglevel = level;
	return prev;
}

/*----------------------------------------------------------------------------
  Helper Functions
----------------------------------------------------------------------------*/

#define DBGPRINTF_BUF_SIZE (1024)

/**
 * @brief Convert milliseconds to hours, minutes, seconds, and milliseconds.
 *
 * @param ms      Milliseconds to convert.
 * @param hours   Pointer to store the hours.
 * @param minutes Pointer to store the minutes.
 * @param seconds Pointer to store the seconds.
 * @param milis   Pointer to store the milliseconds.
 */
static void ms_to_time(uint32_t ms, int *hours, int *minutes, int *seconds, int *milis)
{
	*milis   = ms % 1000;
	*seconds = (ms / 1000) % 60;
	*minutes = (ms / (1000 * 60)) % 60;
	*hours   = (ms / (1000 * 60 * 60)) % 24;
}

/**
 * @brief Convert debug level to a character representation.
 *
 * @param level Debug level to convert.
 * @return Character representation of the debug level.
 */
static char level_to_ch(int level)
{
	char lvlch;

	switch (level) {
	case STM32UTIL_DEBUG_LEVEL_DPRINTF:	lvlch = 'D'; break;
	case STM32UTIL_DEBUG_LEVEL_ERROR:	lvlch = 'E'; break;
	case STM32UTIL_DEBUG_LEVEL_WARNING:	lvlch = 'W'; break;
	case STM32UTIL_DEBUG_LEVEL_TRACE:	lvlch = 'T'; break;
	default:				if (0 <= level && level <= 9) { lvlch = '0' + level; } else { lvlch = '?'; }
	}

	return lvlch;
}

/**
 * @brief Create a timestamp string with debug information.
 *
 * @param buffer Buffer to store the timestamp string.
 * @param size   Size of the buffer.
 * @param level  Debug level.
 * @param line   Line number.
 * @param func   Function name.
 * @param file   File name.
 * @return Length of the created timestamp string.
 */
static int make_timestamp_info(char* buffer, size_t size, int level, int line, const char *func, const char *file)
{
	char lvlch = level_to_ch(level);

	uint32_t tick = stm32util_get_tick();
	int hours, minutes, seconds, milis;
	ms_to_time(tick, &hours, &minutes, &seconds, &milis);

	//uint32_t pc = (uint32_t)make_timestamp_info;
	//pc = (uint32_t)__builtin_return_address(0);
	//pc = __current_pc();
	//const char* fn = backtrace_function_name(pc);
	SAFE_FMT(n1, snprintf, buffer, size, "%02d:%02d:%02d.%03d %c %s %s(%d): ", hours, minutes, seconds, milis, lvlch, func, file, line);

	return n1;
}

/**
 * @brief Print a formatted debug message with timestamp and debug information.
 *
 * @param level Debug level.
 * @param line  Line number.
 * @param func  Function name.
 * @param file  File name.
 * @param fmt   Format string.
 * @param ...   Variable arguments for the format string.
 * @return Number of characters printed.
 */
int stm32util_debug_printf(uint32_t group, int level, int line, const char* func, const char* file, const char* fmt, ...)
{
	if (stm32util_debug_get_level() < level || !(dbggroup & group)) { // runtime check
		return 0;
	}

#if STM32UTIL_USE_OS
	char* buffer = pvPortMalloc(DBGPRINTF_BUF_SIZE);
	if (buffer) {

#else
	static char buffer[DBGPRINTF_BUF_SIZE]; {
#endif
		int n1 = make_timestamp_info(buffer, DBGPRINTF_BUF_SIZE, level, line, func, file);

		va_list vargs;
		va_start(vargs, fmt);
		SAFE_FMT(n2, vsnprintf, buffer + n1, DBGPRINTF_BUF_SIZE - n1, fmt, vargs);
		va_end(vargs);

		stm32util_debug_uart_write(1, buffer, n1 + n2);

#if STM32UTIL_USE_OS
		vPortFree(buffer);
#endif

		return n1 + n2;
	}

	return -1;
}

#define critical_puts(a,b)	stm32util_uart_transmit((uint8_t*)(a), (b))

static const char crlf4_[] = "\r\n\r\n\r\n\r\n";

void stm32util_debug_critical(const char* msg, const char* func, int funclen, const char* loc, int loclen)
{
	static const char critical_msg[] = "\r\nCRITICAL ERROR\r\n";

	__disable_irq();

	while (true) {
		critical_puts(critical_msg, sizeof critical_msg);

		critical_puts(func, funclen);
		critical_puts(loc, loclen);
		critical_puts(crlf4_, 2);

		critical_puts(msg, strnlen(msg, DBGPRINTF_BUF_SIZE));
		critical_puts(crlf4_, sizeof crlf4_);
	}
}

void stm32util_assert(const char* cond, int line, const char* func, const char* file, const char* fmt, ...)
{
	static const char assert_msg[] = "\r\nASSERTION FAILED\r\n";
	static char buffer[DBGPRINTF_BUF_SIZE];

	__disable_irq();

	int n1 = make_timestamp_info(buffer, DBGPRINTF_BUF_SIZE, -1, line, func, file);

	va_list vargs;
	va_start(vargs, fmt);
	SAFE_FMT(n2, vsnprintf, buffer + n1, DBGPRINTF_BUF_SIZE - n1, fmt, vargs);
	va_end(vargs);

	while (true) {
		critical_puts(assert_msg, sizeof assert_msg);

		critical_puts(buffer, n1 + n2);
		critical_puts(crlf4_, 2);

		critical_puts(cond, strnlen(cond, DBGPRINTF_BUF_SIZE));
		critical_puts(crlf4_, 2);

		critical_puts(crlf4_, sizeof crlf4_);
	}
}
