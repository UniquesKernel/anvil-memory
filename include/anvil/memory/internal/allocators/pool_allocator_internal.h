#ifndef ANVIL_MEMORY_POOL_ALLOCATOR_INTERNAL_H
#define ANVIL_MEMORY_POOL_ALLOCATOR_INTERNAL_H

#include "anvil/memory/arena.h"
#include "anvil/memory/internal/arena_internal.h"
#include <stddef.h>

/*****************************************************************************************************
 *					Pool Allocator
 * ***************************************************************************************************/

/**
 * @brief Pool memory free strategy for memory allocator.
 *
 * This function will walk through all memory blocks in a memory block chain
 * and free them. It handles the complete cleanup of all allocated memory in the pool.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [in] `memory_block` Pointer to the head of the memory block chain to free.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void pool_free(MemoryBlock *const memory_block);

/**
 * @brief Pool memory reset strategy for memory allocator.
 *
 * This function will reset the first memory block in a memory block chain.
 * It will free the rest of the memory blocks. This returns the memory
 * block chain to the state it was at when it was first created.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [in] `memory_block` Pointer to the head of the memory block chain to reset.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void pool_reset(MemoryBlock *const memory_block);

/**
 * @brief Pool memory allocation strategy for memory allocator.
 *
 * This function allocates memory from a memory block in pool-sized chunks.
 * It always allocates memory in multiples of the pool size, rounding up
 * the requested allocation size to the nearest pool boundary. If there is
 * not enough space in the current block, it moves to the next block or creates
 * a new one with doubled capacity.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is `NULL` or points to `NULL`.
 * - The arena's memory block is `NULL`.
 * - The arena's memory block's memory is `NULL`.
 * - The alignment provided is not a power of two.
 * - The alignment is not >= the alignment of `max_align_t`.
 * - The allocation size is zero.
 *
 * @param [in,out] `arena` Pointer to the pointer of the arena to allocate from.
 * @param [in] `allocation_size` Amount of memory to allocate from the memory block.
 *
 * @return Pointer to allocated memory. This function will never return NULL as it
 *         will either allocate from an existing block or create a new one.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void *pool_alloc(MemoryArena **const arena, const size_t allocation_size);

/**
 * @brief Pool memory allocation test strategy.
 *
 * This function verifies if an allocation of the given size could be made
 * from the arena. For the pool allocator, this function always returns true
 * because it can dynamically allocate new blocks when needed.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is `NULL`.
 * - arena's memory block is `NULL`.
 * - alignment is not a power of two.
 * - allocation size is zero.
 *
 * @param [in] `arena` The memory arena to check for allocation possibility.
 * @param [in] `allocation_size` Amount of memory to check for allocation possibility.
 *
 * @return Always returns true for pool allocator, as it can dynamically grow by
 *         allocating new blocks when needed.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
bool pool_alloc_verify(MemoryArena *const arena, const size_t allocation_size);

#endif    // !ANVIL_MEMORY_POOL_ALLOCATOR_INTERNAL_H
