#ifndef ANVIL_MEMORY_UTILITY_INTERNAL_H
#define ANVIL_MEMORY_UTILITY_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <time.h>

// can be used together with functions like free, memory_arena_destroy etc. for
// automatically invoking cleanup on scope exits.
#define DEFER(clean_up_func) __attribute__((cleanup(clean_up_func)))

#define ASSERT_CRASH(expr, msg)                                                                                        \
	do {                                                                                                           \
		if (!(expr)) {                                                                                         \
			log_and_crash(#expr, __FILE__, __LINE__, msg);                                                 \
		}                                                                                                      \
	} while (0)

#ifdef DEBUG
inline void *debug_mmap(void *addr, size_t size, int __prod, __flags, __fd, __off_t __offset) {
	void *result = mmap(addr, size, __prod, __flags, __fd, __off_t__offset);
	if (result) {
		memset(*ptr, 0xCC, size);
	}
	return result;
}
#define mmap debug_mmap
#endif    // DEBUG

void log_and_crash(const char *expr, const char *file, int line, const char *msg);

[[gnu::pure]]
bool is_power_of_two(const size_t x);

[[gnu::malloc]]
void *safe_malloc(const size_t size, const size_t alignment, const char *error_msg);

void freew(void *ptr);

void safe_free(void *ptr);
#endif    // !ARENA_MEMORY_UTILITY_INTERNAL_H
