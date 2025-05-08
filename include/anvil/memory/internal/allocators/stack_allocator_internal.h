/**
 * @file stack_allocator_internal.h
 * @brief Internal implementation of the Stack Memory Allocator.
 *
 * This header defines the internal functions for the Stack Allocator strategy, which
 * follows a last-in-first-out (LIFO) memory management approach. It allows recording
 * allocation states and unwinding to previous states, making it ideal for nested
 * temporary allocations or scope-based memory management.
 *
 * The Stack allocator supports dynamic growth by allocating new blocks when needed,
 * and provides snapshots to allow efficient rollback to previous memory states.
 */

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
 * and free them, including the head block itself.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 * - head memory block's memory pointer is `NULL`.
 *
 * @param [in] `memory_block` Pointer to the head of the memory block chain to free.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void stack_free(MemoryBlock *const memory_block);

/**
 * @brief Stack memory reset strategy for memory allocator.
 *
 * This function will reset the first memory block in a memory block chain by setting
 * its allocated memory counter to zero. It will free the rest of the memory blocks and
 * set the next pointer to NULL. This returns the memory block chain to the state it was
 * at when it was first created.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - head memory block in the memory block chain is `NULL`.
 *
 * @param [in] `memory_block` Pointer to the head of the memory block chain to reset.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void stack_reset(MemoryBlock *const memory_block);

/**
 * @brief Stack memory allocation strategy for memory allocator.
 *
 * This function attempts to allocate memory from the current top memory block.
 * The memory is properly aligned according to the specified alignment requirement.
 * If there is not enough space in the current block, it creates a new block with
 * doubled capacity, links it as the new top block, and updates the memory_block
 * pointer to point to this new block.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - memory_block pointer is `NULL` or points to `NULL`.
 * - The current block's memory is `NULL`.
 * - The alignment provided is not a power of two.
 * - The alignment is not >= the alignment of `max_align_t`.
 * - The allocation size is zero.
 * - The current block has a non-NULL next pointer (stack allocation must happen at the top).
 *
 * @param [in,out] `memory_block` Pointer to the pointer of the current top memory block.
 *                               This will be updated to point to a new block if the current
 *                               block cannot satisfy the allocation.
 * @param [in] `allocation_size` Amount of memory to allocate from the memory block.
 * @param [in] `alignment` Alignment of the allocated memory.
 *
 * @return Pointer to aligned allocated memory. Unlike scratch_alloc, this function will
 *         never return NULL as it will either allocate from the current block or create a new one.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void *__attribute__((malloc, warn_unused_result)) stack_alloc(MemoryBlock **const memory_block,
                                                              const size_t allocation_size, const size_t alignment);

/**
 * @brief Stack memory allocation test strategy.
 *
 * This function checks if an allocation of the given size and alignment could be made
 * from the current memory block chain. For the stack allocator, this function always
 * returns true because it can dynamically allocate new blocks when needed.
 *
 * As the implementation shows, the stack allocator is designed to be dynamic and will
 * always be able to create a new memory block if the current one is insufficient. As a result,
 * this verification function always returns true, as long as the system has memory
 * available for the allocator to use.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - Memory block is `NULL`.
 * - Alignment is not a power of two.
 * - Allocation size is zero.
 *
 * @param [in] `memory_block` The current top memory block in the stack.
 * @param [in] `allocation_size` Amount of memory to check for allocation possibility.
 * @param [in] `alignment` Alignment requirement for the potential allocation.
 *
 * @return Always returns true for stack allocator, as it can dynamically grow by
 *         allocating new blocks when needed. If the system runs out of memory during actual
 *         allocation, an invariant failure will occur rather than returning false here.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
bool stack_alloc_verify(MemoryBlock *const memory_block, const size_t allocation_size, const size_t alignment);
#endif    // !ANVIL_MEMORY_ARENA_STACK
