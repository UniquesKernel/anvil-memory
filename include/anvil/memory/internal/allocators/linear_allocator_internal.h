#ifndef ANVIL_MEMORY_ALLOCATOR_linear_INTERL_H
#define ANVIL_MEMORY_ALLOCATOR_linear_INTERL_H

#include "anvil/memory/arena.h"
#include "anvil/memory/internal/arena_internal.h"

/*****************************************************************************************************
 *					Dynamic Allocator
 * ***************************************************************************************************/

/**
 * @brief Linear memory free strategy for memory allocator.
 *
 * This function walks through all memory blocks in a memory block chain
 * and frees them, including the memory they point to and the blocks themselves.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [out] `memory_block` Pointer to the head of the memory block chain to free.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void linear_free(MemoryBlock *const memory_block);

/**
 * @brief Linear memory reset strategy for memory allocator.
 *
 * This function will reset the first memory block in a memory block chain.
 * It will free the rest of the memory blocks. This returns the memory
 * block chain to the state it was at when it was first created.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [out] `memory_block` Pointer to the head of the memory block chain to reset.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void linear_reset(MemoryBlock *const memory_block);

/**
 * @brief Linear memory allocation strategy for memory allocator.
 *
 * This function traverses a chain of memory blocks and allocates memory from the
 * first memory block for which there is enough memory available to handle the allocation.
 * If no existing block can handle the allocation, it creates a new block with doubled
 * capacity and links it to the chain. This ensures the allocator can always satisfy
 * memory requests as long as the system has memory available.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena or *arena is `NULL`.
 * - The memory_block in the arena is `NULL`.
 * - The memory pointer within the block is `NULL`.
 * - The arena alignment provided is not a power of two.
 * - The arena alignment is not >= the alignment of `max_align_t`.
 * - The allocation size is zero.
 *
 * @param [in,out] `arena` Pointer to the pointer of the arena to allocate from.
 * @param [in] `allocation_size` Amount of memory to allocate from the memory block.
 *
 * @returns Pointer to allocated memory.
 * @returns NULL if system is out of memory.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
[[gnu::malloc, gnu::warn_unused_result]]
void *linear_alloc(MemoryArena **const arena, const size_t allocation_size);

/**
 * @brief Linear memory allocation verification function.
 *
 * This function checks if an allocation of the given size is possible with the
 * current memory arena. For the linear allocator, this function always returns `true`
 * because it can dynamically create new blocks when needed, effectively only failing
 * if the system runs out of memory (which would trigger a crash via the INVARIANT macros).
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - Arena is `NULL`.
 * - Allocation size is zero.
 *
 * @param [in] `arena` Pointer to the arena to check for allocation possibility.
 * @param [in] `allocation_size` Size of the potential allocation.
 *
 * @return Always returns true for the linear allocator, as it can dynamically grow
 *         by allocating new blocks when needed.
 */
[[gnu::pure]]
bool linear_alloc_verify(MemoryArena *const arena, const size_t allocation_size);

#endif    // ANVIL_MEMORY_ALLOCATOR_linear_INTERL_H
