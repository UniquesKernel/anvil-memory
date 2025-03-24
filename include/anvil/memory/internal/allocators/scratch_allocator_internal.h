#ifndef ANVIL_MEMORY_SCRATCH_ALLOCATOR_INTERNAL_H
#define ANVIL_MEMORY_SCRATCH_ALLOCATOR_INTERNAL_H

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
 * This function will allocate memory from a memory block and return the address to the
 * allocated.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 * - The blocks memory is `NULL`.
 * - The alignment provided to it is not a power of two.
 * - The alignment is not >= the alignment of `max_align_t`.
 * - The allocation size is zero.
 *
 * @param [out] `memory` Pointer to the head of the memory block chain to reset.
 * @param [in] `allocation_size` Amount of memory to allocate from the memory block.
 * @param [in] `alignment` Alignment of the allocated memory.
 *
 * @return Pointer to allocated memory.
 * less than the allocation size.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
[[gnu::malloc, gnu::warn_unused_result]]
void *scratch_alloc(MemoryBlock *block, const size_t allocation_size, const size_t alignment);

/**
 * @brief Scratch memory allocation test strategy.
 *
 * This function will traverse a memory block chain and check if any
 * memory block in the chain has enough free memory for an allocation.
 * It returns true if the memory block chain has enough available memory
 * otherwise it returns false.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - Memory block is `Null`.
 * - allocation size is not zero.
 * - Alignment is not a power of two.
 *
 * @param [in] `block` is the head of the memory block chain to check for available memory.
 * @param [in] `allocation_size` to see if the memory is available in the memory block chain.
 * @param [in] `alignment` of the memory blocks being checked.
 *
 * @return boolean value to see if the memory block chain has the necessary available resources for an allocation.
 */
[[gnu::pure]]
bool scratch_alloc_verify(MemoryBlock *const block, const size_t allocation_size, const size_t alignment);

#endif    // !MEMORY_ARENA_SCRATCH_ALLOCATOR_INTERNAL_H
