#ifndef ANVIL_MEMORY_UTILITY_INTERNAL_H
#define ANVIL_MEMORY_UTILITY_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <time.h>

// Macro that calls the helper function
#define ASSERT_CRASH(expr, msg)                                                                                        \
	do {                                                                                                           \
		if (!(expr)) {                                                                                         \
			log_and_crash(#expr, __FILE__, __LINE__, msg);                                                 \
		}                                                                                                      \
	} while (0)

void log_and_crash(const char *expr, const char *file, int line, const char *msg);

[[gnu::pure]]
bool is_power_of_two(const size_t x);

void *safe_malloc(const size_t size, const size_t alignment, const char *error_msg);

#endif    // !ARENA_MEMORY_UTILITY_INTERNAL_H
