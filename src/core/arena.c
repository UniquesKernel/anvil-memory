#include "anvil/memory/arena.h"
#include "anvil/memory/internal/allocators/linear_allocator_internal.h"
#include "anvil/memory/internal/allocators/scratch_allocator_internal.h"
#include "anvil/memory/internal/allocators/stack_allocator_internal.h"
#include "anvil/memory/internal/arena_internal.h"
#include "anvil/memory/internal/utility_internal.h"
#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>

#define INITIAL_STACK_SNAPSHOT_SIZE 10

MemoryArena *memory_arena_create(const AllocatorType type, const size_t alignment, const size_t initial_size) {

	INVARIANT(is_power_of_two(alignment), "alignment must be a power of two!");
	INVARIANT(type != COUNT, "Count is not a valid allocator type");
	INVARIANT(initial_size != 0, "Cannot initialize zero sized memory arenas");

	MemoryArena *arena = malloc(sizeof(*arena));

	INVARIANT(arena != NULL, "System out of memory");

	arena->memory_block = malloc(sizeof(*arena->memory_block));

	INVARIANT(arena->memory_block != NULL, "System out of memory");

	arena->memory_block->capacity = (initial_size + (alignment - 1)) & ~(alignment - 1);
	arena->memory_block->allocated = 0;
	arena->memory_block->next = NULL;
	arena->alignment = alignment;
	arena->allocator_type = type;

	switch (arena->allocator_type) {
		case LINEAR:
			arena->state.linearAllocatorState =
			    (LinearAllocatorState){._dummy_variable_to_comply_with_standards = 0};
			break;
		case SCRATCH:
			arena->state.scratchAllocatorState =
			    (ScratchAllocatorState){._dummy_variable_to_comply_with_standards = 0};
			break;
		case STACK:
			arena->state.stackAllocatorState.top = arena->memory_block;
			arena->state.stackAllocatorState.top->next = NULL;
			arena->state.stackAllocatorState.snapshots =
			    malloc(INITIAL_STACK_SNAPSHOT_SIZE * sizeof(Snapshot));
			INVARIANT(arena->state.stackAllocatorState.snapshots, "");
			break;
		case COUNT:
		default:
			INVARIANT(0, "Failed to initialize arena state");
	}

	INVARIANT(arena->memory_block->capacity > arena->memory_block->allocated,
	          "Arena Capacity cannot be less or equal to allocated  "
	          "memory when arena is created");
	INVARIANT(arena->memory_block->next == NULL, "Next memory block should be initialized to NULL");

	arena->memory_block->memory =
	    safe_aligned_alloc(arena->memory_block->capacity, alignment, "The allocated memory cannot be NULL");

	return arena;
}

void memory_arena_destroy(MemoryArena **const arena) {
	INVARIANT((*arena), "Cannot destroy a NULL pointer arena");
	INVARIANT((*arena)->memory_block, "Arena has no valid memory to destroy");

	switch ((*arena)->allocator_type) {
		case SCRATCH:
			scratch_free((*arena)->memory_block);
			break;
		case LINEAR:
			linear_free((*arena)->memory_block);
			break;
		case STACK:
			free((*arena)->state.stackAllocatorState.snapshots);
			stack_free((*arena)->memory_block);
			break;
		case COUNT:
		default:
			INVARIANT(0, "");
	}
	free(*arena);
	(*arena) = NULL;
}

void memory_arena_reset(MemoryArena **const arena) {
	INVARIANT((*arena), "Cannot reset NULL pointer arena");
	INVARIANT((*arena)->memory_block, "Arena has no memory allocated");

	switch ((*arena)->allocator_type) {
		case SCRATCH:
			scratch_reset((*arena)->memory_block);
			return;
		case LINEAR:
			linear_reset((*arena)->memory_block);
			return;
		case STACK:
			stack_reset((*arena)->memory_block);
			(*arena)->state.stackAllocatorState.top = (*arena)->memory_block;
			return;
		case COUNT:
		default:
			INVARIANT(0, "");
	}
	__builtin_unreachable();
}

void *memory_arena_alloc(MemoryArena **const arena, const size_t size) {
	INVARIANT(*arena, "Cannot allocate memory from a null pointer arena");
	INVARIANT((*arena)->memory_block, "Cannot allocate memory from a null pointer memory block");

	switch ((*arena)->allocator_type) {
		case SCRATCH:
			return scratch_alloc((*arena)->memory_block, size, (*arena)->alignment);
		case LINEAR:
			return linear_alloc((*arena)->memory_block, size, (*arena)->alignment);
		case STACK:
			return stack_alloc(&(*arena)->state.stackAllocatorState.top, size, (*arena)->alignment);
		case COUNT:
		default:
			INVARIANT(0, "");
	}
	__builtin_unreachable();
}

bool memory_arena_alloc_verify(MemoryArena *const arena, const size_t size) {
	INVARIANT(arena, "Cannot verify a null pointer arena");
	INVARIANT(arena->memory_block, "Cannot verify if a null memory_block can contain memory allocation");

	switch (arena->allocator_type) {
		case SCRATCH:
			return scratch_alloc_verify(arena->memory_block, size, arena->alignment);
		case LINEAR:
			return linear_alloc_verify(arena->memory_block, size, arena->alignment);
		case STACK:
			return stack_alloc_verify(arena->state.stackAllocatorState.top, size, arena->alignment);
		case COUNT:
		default:
			INVARIANT(0, "");
	}
	__builtin_unreachable();
}
