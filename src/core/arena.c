#include "anvil/memory/arena.h"
#include "anvil/memory/internal/allocators/linear_allocator_internal.h"
#include "anvil/memory/internal/allocators/scratch_allocator_internal.h"
#include "anvil/memory/internal/arena_internal.h"
#include "anvil/memory/internal/utility_internal.h"
#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>

MemoryArena *memory_arena_create(const AllocatorType type, const size_t alignment, const size_t initial_size) {

	ASSERT_CRASH(is_power_of_two(alignment), "alignment must be a power of two!");
	ASSERT_CRASH(type != COUNT, "Count is not a valid allocator type");
	ASSERT_CRASH(initial_size != 0, "Cannot initialize zero sized memory arenas");
	MemoryArena *arena = safe_malloc(sizeof(*arena), _Alignof(MemoryArena), "Arena shouldn't be NULL");
	arena->memory_block =
	    safe_malloc(sizeof(*arena->memory_block), _Alignof(MemoryBlock), "Memory Block cannot be NULL");
	arena->memory_block->capacity = (initial_size + (alignment - 1)) & ~(alignment - 1);
	arena->memory_block->allocated = 0;
	arena->memory_block->next = NULL;
	arena->alignment = alignment;

	ASSERT_CRASH(arena->memory_block->capacity > arena->memory_block->allocated,
	             "Arena Capacity cannot be less or equal to allocated  "
	             "memory when arena is created");
	ASSERT_CRASH(arena->memory_block->next == NULL, "Next memory block should be initialized to NULL");
	arena->memory_block->memory =
	    safe_malloc(arena->memory_block->capacity, alignment, "The allocated memory cannot be NULL");

	switch (type) {
		case SCRATCH:
			arena->allocator.free_fptr = scratch_free;
			arena->allocator.alloc_fptr = scratch_alloc;
			arena->allocator.reset_fptr = scratch_reset;
			arena->allocator.alloc_verify_fptr = scratch_alloc_verify;
			break;
		case LINEAR:
			arena->allocator.free_fptr = linear_free;
			arena->allocator.reset_fptr = linear_reset;
			arena->allocator.alloc_fptr = linear_alloc;
			arena->allocator.alloc_verify_fptr = linear_alloc_verify;
			break;
		default:
			ASSERT_CRASH(0, "Invalid allocator type encountered");
	}

	// NOTE:(UniquesKernel) These asserts serve as guardrails in case new allocators
	// are added but improperly implemented during development. They are not present in
	// release builds.
	assert(arena->allocator.free_fptr && "Should never have a null pointer free function");
	assert(arena->allocator.alloc_fptr && "Should never have a null pointer alloc function");
	assert(arena->allocator.reset_fptr && "Should never have a null pointer reset function");
	assert(arena->allocator.alloc_verify_fptr && "Should never have a null pointer verify function");

	return arena;
}

void memory_arena_destroy(MemoryArena **const arena) {
	ASSERT_CRASH((*arena), "Cannot destroy a NULL pointer arena");
	ASSERT_CRASH((*arena)->memory_block, "Arena has no valid memory to destroy");

	(*arena)->allocator.free_fptr((*arena)->memory_block);
	munmap(*arena, sizeof(*(*arena)));

	*arena = NULL;
}

void memory_arena_reset(MemoryArena **const arena) {
	ASSERT_CRASH((*arena), "Cannot reset NULL pointer arena");
	ASSERT_CRASH((*arena)->memory_block, "Arena has no memory allocated");

	MemoryArena *internal_arena = *arena;
	internal_arena->allocator.reset_fptr(internal_arena->memory_block);

	*arena = internal_arena;
}

void *memory_arena_alloc(MemoryArena **const arena, const size_t size) {
	ASSERT_CRASH(*arena, "Cannot allocate memory from a null pointer arena");
	ASSERT_CRASH((*arena)->memory_block, "Cannot allocate memory from a null pointer memory block");

	return (*arena)->allocator.alloc_fptr((*arena)->memory_block, size, (*arena)->alignment);
}

bool memory_arena_alloc_verify(MemoryArena *const arena, const size_t size) {
	ASSERT_CRASH(arena, "Cannot verify a null pointer arena");
	ASSERT_CRASH(arena->memory_block, "Cannot verify if a null memory_block can contain memory allocation");

	return arena->allocator.alloc_verify_fptr(arena->memory_block, size, arena->alignment);
}
