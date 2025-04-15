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

/**
 * @brief Metadata
 *
 * Represents the metadata for a memory block, for the purpose of
 * freeing an aligned memory block.
 *
 * Invariants:
 * - Allocated is less than or equal to capacity.
 * - capacity is larger than zero.
 * - memory is not null.
 *
 * Fields	| Type		| Size
 * ---		| ---		| ---
 * base		| void pointer	| 4 or 8 Bytes
 * size		| size_t	| 4 or 8 Bytes
 */
typedef struct Metadata {
	void *base;
	size_t total_size;
} Metadata;
static_assert(sizeof(Metadata) == 16 || sizeof(Metadata) == 8,
              "Metadata should be 16 or 8 bytes depending on architecture");
static_assert(_Alignof(Metadata) == _Alignof(void *), "should have the natural alignment of a void pointer");

/**
 * @brief Logs the time, file and code line and then aborts.
 *
 * @param[in] `expr` that failed.
 * @param[in] `file` filename for where a failure occured.
 * @param[in] `line` line that failed.
 * @param[in] `error_msg` error message to log.
 *
 */
[[gnu::noreturn]]
void log_and_crash(const char *expr, const char *file, int line, const char *msg);

/**
 * @brief Allocates an aligned block of memory.
 *
 * Allocate an aligned block of memory from a page.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - `size` is zero.
 * - `alignment` is not a power of two.
 * - `alignment` is larger than 2^16.
 * - The system runs out of memory.
 *
 * @param[in] `size` of the allocation.
 * @param[in] `alignment` of the allocated memory.
 * @param[in] `error_msg` error message if the allocation fails.
 * @returns Pointer to allocated memory.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
[[gnu::malloc]]
void *safe_aligned_alloc(const size_t size, const size_t alignment, const char *error_msg);

[[gnu::pure]]
bool is_power_of_two(const size_t x);

void safe_free(void *ptr);

void safe_aligned_free(void *ptr);

#endif    // !ARENA_MEMORY_UTILITY_INTERNAL_H
