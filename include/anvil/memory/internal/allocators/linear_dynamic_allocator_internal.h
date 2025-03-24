#ifndef ANVIL_MEMORY_ALLOCATOR_LINEAR_DYNAMIC_INTERL_H
#define ANVIL_MEMORY_ALLOCATOR_LINEAR_DYNAMIC_INTERL_H

#include "anvil/memory/internal/arena_internal.h"

/*****************************************************************************************************
 *					Dynamic Allocator
 * ***************************************************************************************************/

/**
 * @brief Linear dynamic memory free strategy for memory allocator.
 *
 * This function will walk through all memory blocks in a memory block chain
 * and free them.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [out] `memory` Pointer to the head of the memory block chain to free.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void linear_dynamic_free(MemoryBlock *const memory);

/**
 * @brief Linear dynamic memory reset strategy for memory allocator.
 *
 * This function will reset the first memory block in a memory block chain.
 * It will free the rest of the memory blocks. This returns the memory
 * block chain to the state it was at when it was first created.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [out] `memory` Pointer to the head of the memory block chain to reset.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void linear_dynamic_reset(MemoryBlock *const memory);

/**
 * @brief Linear dynamic memory allocation strategy for memory allocator.
 *
 * This function traverses a chain of memory blocks and allocates memory from the
 * first memory block in the chain for which there is enough memory available to
 * handle the allocation. If no block is available, it allocates a new block to handle the
 * allocation. Unless the system is out of memory this allocator should always be able to
 * handle an allocation.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 * - The blocks memory is `NULL`.
 * - The alignment provided to it is not a power of two.
 * - The alignment is not >= the alignment of `max_align_t`.
 * - The allocation size is zero.
 *
 * The function returns an error code ONLY for recoverable external failures.
 *
 * @param [out] `memory` Pointer to the head of the memory block chain to reset.
 * @param [in] `allocation_size` Amount of memory to allocate from the memory block.
 * @param [in] `alignment` Alignment of the allocated memory.
 *
 * @returns Pointer to allocated memory.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
[[gnu::malloc, gnu::warn_unused_result]]
void *linear_dynamic_alloc(MemoryBlock *block, const size_t allocation_size, const size_t alignment);

/**
 * @brief Linear memory allocation test strategy.
 *
 * This function will check the following invariants
 * - Memory block is not null.
 * - Allocation size is not zero.
 * - Alignment is a power of two.
 *
 * If all invariants are unbroken this function always returns `true` because
 * the system will CRASH if the system is out of memory but if the system is
 * not out of memory the dynamic linear allocator can always handle an allocation.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - Memory block is `Null`.
 * - Alignment is not a power of two.
 * - Allocation size is not zero.
 *
 * @param [in] `block` is the head of the memory block chain to check for available memory.
 * @param [in] `allocation_size` to see if the memory is available in the memory block chain.
 * @param [in] `alignment` of the memory blocks being checked.
 *
 * @return boolean value to see if the memory block chain has the necessary available resources for an allocation.
 */
[[gnu::pure]]
bool linear_dynamic_alloc_verify(MemoryBlock *const block, const size_t allocation_size, const size_t alignment);

#endif    // ANVIL_MEMORY_ALLOCATOR_LINEAR_DYNAMIC_INTERL_H
