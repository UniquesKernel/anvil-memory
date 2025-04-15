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

/**
 * @brief Represents a saved state of a MemoryArena, typically used by Stack allocators.
 *
 * Stores the top MemoryBlock and the allocation amount within that block at the
 * time the snapshot was taken, allowing the arena to be rolled back to this state.
 *
 * Fields      | Type                  	| Size
 * ---         | ---                   	| ---
 * top         | MemoryBlock * 		| 4 or 8 Bytes
 * allocated   | size_t                	| 4 or 8 Bytes
 */
typedef struct {
	MemoryBlock *top;    ///< Pointer to the MemoryBlock that was active when the snapshot was taken.
	size_t allocated;    ///< The number of bytes allocated in the 'top' block at the time of the snapshot.
} Snapshot;

static_assert(sizeof(Snapshot) == 8 || sizeof(Snapshot) == 16,
              "Snapshot must be either 8 or 16 bytes depending on architecture");
static_assert(_Alignof(Snapshot) == _Alignof(MemoryBlock *), "Snapshot alignment must match MemoryBlock* alignment");

/**
 * @brief Placeholder state structure for the Scratch Allocator.
 *
 * Currently contains a dummy variable to comply with C standards requiring
 * at least one member in a struct. This may be expanded in the future if the
 * Scratch allocator requires specific state tracking.
 *
 * Fields                                 	| Type | Size
 * ---                                    	| ---  | ---
 * _dummy_variable_to_comply_with_standards	| int  | 4 Bytes
 */
typedef struct {
	int _dummy_variable_to_comply_with_standards;    ///< Placeholder field.
} ScratchAllocatorState;

static_assert(sizeof(ScratchAllocatorState) == 4 || sizeof(ScratchAllocatorState) == 8,
              "ScratchAllocatorState must be either 4 or 8 bytes depending on architecture");
static_assert(_Alignof(ScratchAllocatorState) == _Alignof(int),
              "ScratchAllocatorState alignment must match integer alignment");

/**
 * @brief Placeholder state structure for the Linear Allocator.
 *
 * Currently contains a dummy variable to comply with C standards requiring
 * at least one member in a struct. This may be expanded in the future if the
 * Linear allocator requires specific state tracking.
 *
 * Fields                                 	| Type | Size
 * ---                                    	| ---  | ---
 * _dummy_variable_to_comply_with_standards	| int  | 4 Bytes
 */
typedef struct {
	int _dummy_variable_to_comply_with_standards;    ///< Placeholder field.
} LinearAllocatorState;

static_assert(sizeof(LinearAllocatorState) == 4 || sizeof(LinearAllocatorState) == 8,
              "LinearAllocatorState must be either 4 or 8 bytes depending on architecture");
static_assert(_Alignof(LinearAllocatorState) == _Alignof(int),
              "LinearAllocatorState alignment must match integer alignment");

/**
 * @brief State structure specifically for the Stack Allocator.
 *
 * Manages the stack of allocation states (Snapshots) and the current top
 * MemoryBlock being used for allocations. This allows for pushing and
 * popping allocation contexts.
 *
 * Fields      | Type                  	| Size
 * ---         | ---                   	| ---
 * snapshots   | Snapshot* 		| 4 or 8 Bytes
 * top         | MemoryBlock* 		| 4 or 8 Bytes
 */
typedef struct {
	Snapshot *snapshots;    ///< Pointer to an array or linked list of saved Snapshots.
	MemoryBlock *top;       ///< Pointer to the current MemoryBlock being used for allocations.
} StackAllocatorState;

static_assert(sizeof(StackAllocatorState) == 8 || sizeof(StackAllocatorState) == 16,
              "StackAllocatorState must be either 8 or 16 bytes depending on architecture");
static_assert(_Alignof(StackAllocatorState) == _Alignof(MemoryBlock *),
              "StackAllocatorState alignment must match MemoryBlock* alignment");

/**
 * @brief A union holding the state specific to the chosen allocator type.
 *
 * Depending on the `allocator_type` field in the `MemoryArena` struct,
 * the appropriate member of this union will contain the relevant state
 * information for that allocator strategy (Scratch, Linear, or Stack).
 *
 * Fields                 | Type                  | Size
 * ---                    | ---                   | ---
 * scratchAllocatorState  | ScratchAllocatorState | 4 or 8 Bytes
 * linearAllocatorState   | LinearAllocatorState  | 4 or 8 Bytes
 * stackAllocatorState    | StackAllocatorState   | 8 or 16 Bytes
 */
typedef union {
	ScratchAllocatorState scratchAllocatorState;    ///< State for the Scratch allocator.
	LinearAllocatorState linearAllocatorState;      ///< State for the Linear allocator.
	StackAllocatorState stackAllocatorState;        ///< State for the Stack allocator.
} AllocatorState;

static_assert(sizeof(AllocatorState) == 8 || sizeof(AllocatorState) == 16,
              "AllocatorState must be either 8 or 16 bytes depending on architecture");
static_assert(_Alignof(AllocatorState) == _Alignof(StackAllocatorState),
              "AllocatorState alignment must match its largest member alignment (StackAllocatorState)");

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
 * Fields           	| Type              	| Size
 * ---              	| ---               	| ---
 * allocator_type   	| AllocatorType     	| 4 or 8 Bytes
 * memory_block     	| MemoryBlock * 	| 4 or 8 Bytes
 * alignment        	| size_t            	| 4 or 8 Bytes
 * state		| AllocatorState	| 8 or 16 bytes
 *
 * @note Memory Arenas created using this structure are **NOT** thread-safe.
 * External synchronization is required if used in concurrent environments.
 */
typedef struct memory_arena_t {
	AllocatorType allocator_type;    ///< Strategy used for allocation (SCRATCH, LINEAR, STACK).
	MemoryBlock *memory_block;       ///< Pointer to the underlying memory block(s).
	size_t alignment;                ///< Alignment requirement for all allocations.
	AllocatorState state;            ///< Allocator specific state.
} MemoryArena;

static_assert(sizeof(MemoryArena) == 20 || sizeof(MemoryArena) == 40,
              "MemoryArena must be either 12 or 24 bytes depending on architecture");
static_assert(_Alignof(MemoryArena) == _Alignof(MemoryBlock *),
              "Alignment of MemoryArena must match the alignment of a pointer");

#endif    // ANVIL_MEMORY_ARENA_INTERNAL_H
