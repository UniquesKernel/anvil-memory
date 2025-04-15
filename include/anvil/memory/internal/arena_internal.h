/**
 * @file arena_internal.h
 * @brief Internal definitions for the Anvil Memory Arena system.
 *
 * This header file defines the internal structures (`MemoryBlock`, `MemoryArena`)
 * used by the memory arena allocator. These definitions are not intended for
 * direct use by end-users of the library, who should interact with the arena
 * through the public API defined in `arena.h`.
 */

#ifndef ANVIL_MEMORY_ARENA_INTERNAL_H
#define ANVIL_MEMORY_ARENA_INTERNAL_H

#include "anvil/memory/arena.h"
#include <stddef.h>

/**
 * @brief Represents a contiguous block of memory managed within a MemoryArena.
 *
 * A MemoryArena may consist of one or more linked MemoryBlocks. Each block
 * tracks its total usable capacity and the amount currently allocated.
 *
 * Invariants:
 * - allocated is less than or equal to capacity.
 * - capacity is larger than zero.
 * - memory points to a valid, aligned memory region.
 *
 * Fields      | Type              	| Size
 * ---         | ---               	| ---
 * memory      | void * 		| 4 or 8 Bytes
 * next        | struct MemoryBlock*	| 4 or 8 Bytes
 * capacity    | size_t            	| 4 or 8 Bytes
 * allocated   | size_t            	| 4 or 8 Bytes
 */
typedef struct MemoryBlock {
	void *memory;                ///< Aligned memory pointer
	struct MemoryBlock *next;    ///< Linked Memory Block (used by Linear allocator)
	size_t capacity;             ///< Usable capacity
	size_t allocated;            ///< Currently used bytes
} MemoryBlock;

static_assert(sizeof(MemoryBlock) == 16 || sizeof(MemoryBlock) == 32,
              "MemoryBlock must be either 16 or 32 bytes depending on architecture");
static_assert(_Alignof(MemoryBlock) == _Alignof(void *), "MemoryBlock alignment must match pointer alignment");

/**
 * @brief Represents a memory arena for managing allocations.
 *
 * A MemoryArena provides memory allocation services using a specific strategy
 * (e.g., Linear, Scratch, Stack) defined by `allocator_type`. It manages one or
 * more underlying `MemoryBlock` structures where the actual memory resides.
 * All allocations within an arena adhere to the specified `alignment`.
 *
 * Invariants:
 * - alignment is a power of two.
 * - memory_block points to the head of a valid (potentially single-element) MemoryBlock chain.
 * - allocator_type corresponds to a valid allocation strategy (SCRATCH, LINEAR, STACK).
 *
 * Fields           | Type              | Size
 * ---              | ---               | ---
 * allocator_type   | AllocatorType     | 4 or 8 Bytes
 * memory_block     | MemoryBlock * 	| 4 or 8 Bytes
 * alignment        | size_t            | 4 or 8 Bytes
 *
 * @note Memory Arenas created using this structure are **NOT** thread-safe.
 * External synchronization is required if used in concurrent environments.
 */
typedef struct memory_arena_t {
	AllocatorType allocator_type;    ///< Strategy used for allocation (SCRATCH, LINEAR, STACK).
	MemoryBlock *memory_block;       ///< Pointer to the underlying memory block(s).
	size_t alignment;                ///< Alignment requirement for all allocations.
} MemoryArena;

static_assert(sizeof(MemoryArena) == 12 || sizeof(MemoryArena) == 24,
              "MemoryArena must be either 12 or 24 bytes depending on architecture");
static_assert(_Alignof(MemoryArena) == _Alignof(MemoryBlock *),
              "Alignment of MemoryArena must match the alignment of a pointer");

#endif    // ANVIL_MEMORY_ARENA_INTERNAL_H
