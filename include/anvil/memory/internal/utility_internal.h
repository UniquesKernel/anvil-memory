#ifndef ANVIL_MEMORY_UTILITY_INTERNAL_H
#define ANVIL_MEMORY_UTILITY_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

#define DEFER(clean_up_func)  __attribute__((cleanup(clean_up_func)))
#define likely(x)             __builtin_expect(!!(x), 1)
#define unlikely(x)           __builtin_expect(!!(x), 0)
#define MEMORY_POISON_PATTERN 0xDEADC0DE

#define INVARIANT(expr, fmt, ...)                                                                                      \
	do {                                                                                                           \
		if (!(expr)) {                                                                                         \
			log_and_crash(#expr, __FILE__, __LINE__, fmt, ##__VA_ARGS__);                                  \
		}                                                                                                      \
	} while (0)

#ifdef DEBUG
inline void *debug_mmap(void *addr, size_t size, int __prod, __flags, __fd, __off_t __offset) {
	void *result = mmap(addr, size, __prod, __flags, __fd, __off_t__offset);
	if (result) {
		memset(addr, MEMOMEMORY_POISON_PATTERN, size);
	}
	return result;
}
#define mmap debug_mmap
#endif    // DEBUG

/**
 * @brief Logs the time, file and code line and then aborts.
 *
 * @param[in] `expr` that failed.
 * @param[in] `file` filename for where a failure occured.
 * @param[in] `line` line that failed.
 * @param[in] `fmt`  formatted error message to log.
 * @param[in] ... formating arguments.
 */
[[gnu::noreturn]]
void log_and_crash(const char *expr, const char *file, int line, const char *fmt, ...);

#endif    // !ARENA_MEMORY_UTILITY_INTERNAL_H
