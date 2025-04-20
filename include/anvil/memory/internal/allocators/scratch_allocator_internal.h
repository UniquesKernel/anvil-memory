#ifndef ANVIL_MEMORY_SCRATCH_ALLOCATOR_INTERNAL_H
#define ANVIL_MEMORY_SCRATCH_ALLOCATOR_INTERNAL_H

#include "anvil/memory/arena.h"
#include "anvil/memory/internal/arena_internal.h"
#include <stddef.h>
#include <sys/cdefs.h>

/*****************************************************************************************************
 *					Scratch Allocator
 * ***************************************************************************************************/

/**
 * @brief Scratch memory free strategy for memory allocator.
 *
 * This function will walk through all memory blocks in a memory block chain
 * and free them. There is only ever one block in the static linear allocator.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [out] `memory` Pointer to the head of the memory block chain to free.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void scratch_free(MemoryBlock *const memory);

/**
 * @brief Scratch memory reset strategy for memory allocator.
 *
 * This function will reset the first memory block in a memory block chain.
 * It will free the rest of the memory blocks. This returns the memory
 * block chain to the state it was at when it was first created. There is only
 * ever one block in the static linear allocator.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [out] `memory` Pointer to the head of the memory block chain to reset.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void scratch_reset(MemoryBlock *const memory);

/**
 * @brief Scratch memory allocation strategy for memory allocator.
 *
 * This function will allocate memory from the head memory block of the arena.
 * Unlike other allocators, scratch allocator will not create new blocks if there
 * isn't enough space - it will simply return NULL.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is NULL or points to NULL.
 * - The arena's memory block is NULL.
 * - The arena's memory block's memory is NULL.
 * - The arena's alignment is not a power of two.
 * - The arena's alignment is not >= the alignment of `max_align_t`.
 * - The allocation size is zero.
 *
 * @param [in,out] `arena` Pointer to the pointer of the memory arena.
 * @param [in] `allocation_size` Amount of memory to allocate from the memory block.
 *
 * @return Pointer to allocated memory, or NULL if there isn't enough space.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
[[gnu::malloc, gnu::warn_unused_result]]
void *scratch_alloc(MemoryArena **const arena, const size_t allocation_size);

/**
 * @brief Scratch memory allocation test strategy.
 *
 * This function iterates through the memory block chain in the arena and checks if any
 * block has enough free memory for an allocation of the specified size, taking
 * alignment requirements into account.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is NULL.
 * - arena's memory block is NULL.
 * - arena's alignment is not a power of two.
 * - allocation size is zero.
 *
 * @param [in] `arena` The memory arena to check for available memory.
 * @param [in] `allocation_size` Amount of memory to check for allocation possibility.
 *
 * @return boolean value indicating if the arena has sufficient available space for the allocation.
 */
[[gnu::pure]]
bool scratch_alloc_verify(MemoryArena *const arena, const size_t allocation_size);

#endif    // !MEMORY_ARENA_SCRATCH_ALLOCATOR_INTERNAL_H
