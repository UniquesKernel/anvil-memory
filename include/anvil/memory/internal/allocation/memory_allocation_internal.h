#ifndef MEMORY_ALLOCATION_INTERNAL_H
#define MEMORY_ALLOCATION_INTERNAL_H

#include <stddef.h>

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
 * @returns Pointer to allocated memory.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
[[gnu::malloc]]
void *safe_aligned_alloc(const size_t size, const size_t alignment);

[[gnu::pure]]
bool is_power_of_two(const size_t x);

void safe_free(void *ptr);

void safe_aligned_free(void *ptr);

#endif    // !MEMORY_ALLOCATION_INTERNAL_H
