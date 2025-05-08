/**
 * @file memory_allocation_internal.h
 * @brief Internal memory allocation functions for the Anvil Memory system.
 *
 * This header file defines the core low-level memory allocation functions used by
 * the Anvil Memory allocators. It provides aligned memory allocation with proper
 * metadata tracking to support safe deallocation.
 */

#ifndef MEMORY_ALLOCATION_INTERNAL_H
#define MEMORY_ALLOCATION_INTERNAL_H

#include <stddef.h>
#include <assert.h>

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
 * Fields    | Type          | Size
 * --------- | ------------- | -------------
 * base      | void pointer  | 4 or 8 Bytes
 * size      | size_t        | 4 or 8 Bytes
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
void *__attribute__((malloc)) safe_aligned_alloc(const size_t size, const size_t alignment);

/**
 * @brief Checks if a number is a power of two.
 *
 * This function determines whether the given number is a power of two (2^n),
 * which is important for memory alignment operations.
 *
 * @param[in] x The number to check.
 * @return true if x is a power of two, false otherwise.
 *
 * @note Zero is not considered a power of two and will return false.
 */
bool __attribute__((pure)) is_power_of_two(const size_t x);

/**
 * @brief Safely frees non-aligned memory.
 *
 * This function frees memory that was allocated without special alignment
 * requirements. It is a wrapper around the standard free function with
 * additional safety checks.
 *
 * @param[in] ptr Pointer to the memory to be freed.
 *
 * @note This function is safe to call with NULL, in which case no operation is performed.
 */
void safe_free(void *ptr);

/**
 * @brief Safely frees aligned memory.
 *
 * This function frees memory that was allocated with alignment requirements
 * via safe_aligned_alloc. It properly handles the metadata stored with the
 * allocation to ensure the correct memory address is freed.
 *
 * @param[in] ptr Pointer to the aligned memory to be freed.
 *
 * @note This function is safe to call with NULL, in which case no operation is performed.
 * @note Always use this function to free memory allocated with safe_aligned_alloc,
 *       as using standard free will cause memory leaks or undefined behavior.
 */
void safe_aligned_free(void *ptr);

#endif    // !MEMORY_ALLOCATION_INTERNAL_H
