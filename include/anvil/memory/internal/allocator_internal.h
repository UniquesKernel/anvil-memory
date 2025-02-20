#ifndef ANVIL_MEMORY_ALLOCATOR_INTERNAL_H
#define ANVIL_MEMORY_ALLOCATOR_INTERNAL_H

#include "anvil/memory/internal/arena_internal.h"

/**
 * @brief Linear memory free strategy for memory allocator.
 *
 * This function will walk through all memory blocks in a memory block chain
 * and free them.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is NULL.
 *
 * The function returns an error code ONLY for recoverable external failures.
 *
 * @param[out] memory	Pointer to the head of the memory block chain to free.
 *
 * @returns ARENA_ERROR_NONE on success, otherwise it crashes.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
ArenaErrorCode linear_free(MemoryBlock *const memory);

/**
 * @brief Linear memory reset strategy for memory allocator.
 *
 * This function will reset the first memory block in a memory block chain.
 * It will free the rest of the memory blocks. This returns the memory
 * block chain to the state it was at when it was first created.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is NULL.
 *
 * The function returns an error code ONLY for recoverable external failures.
 *
 * @param[out] memory	Pointer to the head of the memory block chain to reset.
 *
 * @returns ARENA_ERROR_NONE on success, otherwise it crashes.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
ArenaErrorCode linear_reset(MemoryBlock *const memory);

/**
 * @brief Linear memory allocation strategy for memory allocator.
 *
 * This function will allocate memory from a memory block and return the address to the
 * memory through a its forth parameter.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is NULL.
 * - The blocks memory is NULL.
 * - The alignment provided to it is not a power of two.
 * - The alignment is not >= the alignment of max_align_t.
 * - The allocation size is zero.
 * - The result pointer is NULL.
 *
 * The function returns an error code ONLY for recoverable external failures.
 *
 * @param[out] memory Pointer to the head of the memory block chain to reset.
 * @param[in] allocation_size Amount of memory to allocate from the memory block.
 * @param[in] alignment Alignment of the allocated memory.
 * @param[out] result Pointer to write the allocation to.
 *
 * @returns ARENA_ERROR_NONE on success. Return ARENA_ERROR_ALLOC_OUT_OF_MEMORY if the blocks remaining capacity is less
 * than the allocation size.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
ArenaErrorCode linear_alloc(MemoryBlock *block, const size_t allocation_size, const size_t alignment, void **result);

#endif    // !MEMORY_ARENA_ALLOCATOR_INTERNAL_H
