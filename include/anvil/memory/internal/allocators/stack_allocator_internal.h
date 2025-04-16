#ifndef ANVIL_MEMORY_STACK_ALLOCATOR_INTERNAL_H
#define ANVIL_MEMORY_STACK_ALLOCATOR_INTERNAL_H

#include "anvil/memory/internal/arena_internal.h"
#include <stddef.h>

/*****************************************************************************************************
 *					Stack Allocator
 * ***************************************************************************************************/

/**
 * @brief Stack memory free strategy for memory allocator.
 *
 * This function will walk through all memory blocks in a memory block chain
 * and free them.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [out] `memory_block` Pointer to the head of the memory block chain to free.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void stack_free(MemoryBlock *const memory_block);

/**
 * @brief Stack memory reset strategy for memory allocator.
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
void stack_reset(MemoryBlock *const memory_block);

/**
 * @brief Stack memory allocation strategy for memory allocator.
 *
 * This function attempts to allocate memory from the current top memory block (pointed to by
 * *memory_block). If there is not enough space in the current top block, it creates a new
 * block with expanded capacity, links it into the chain, and updates the memory_block pointer
 * to point to this new block. This maintains the "stack" behavior where allocations always
 * come from the most recently created block.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 * - The blocks memory is `NULL`.
 * - The alignment provided to it is not a power of two.
 * - The alignment is not >= the alignment of `max_align_t`.
 * - The allocation size is zero.
 *
 * @param [out] `memory_block` Pointer to the pointer of the current top memory block.
 *                            This will be updated to point to a new block if the current
 *                            block cannot satisfy the allocation.
 * @param [in] `size` Amount of memory to allocate from the memory block.
 * @param [in] `alignment` Alignment of the allocated memory.
 *
 * @return Pointer to allocated memory.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void *stack_alloc(MemoryBlock **const memory_block, const size_t size, const size_t alignment);

/**
 * @brief Stack memory allocation test strategy.
 *
 * This function will check the following invariants:
 * - Memory block is not null.
 * - Allocation size is not zero.
 * - Alignment is a power of two.
 *
 * If all invariants are unbroken this function always returns `true` because
 * the system will CRASH if the system is out of memory but if the system is
 * not out of memory the stack allocator can always handle an allocation.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - Memory block is `Null`.
 * - Alignment is not a power of two.
 * - Allocation size is not zero.
 *
 * @param [in] `memory_block` is the head of the memory block chain to check for available memory.
 * @param [in] `size` to see if the memory is available in the memory block chain.
 * @param [in] `alignment` of the memory blocks being checked.
 *
 * @return boolean value to see if the memory block chain has the necessary available resources for an allocation.
 */
[[gnu::pure]]
bool stack_alloc_verify(MemoryBlock *const memory_block, const size_t size, const size_t alignment);

#endif    // !ANVIL_MEMORY_ARENA_STACK
