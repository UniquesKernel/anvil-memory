#include <anvil/memory/arena.h>
#include <anvil/memory/internal/arena_internal.h>
#include <anvil/memory/internal/utility_internal.h>
#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>

static inline ArenaErrorCode linear_free(MemoryBlock *memory) {
	ASSERT_CRASH(memory, "Cannot free Null pointer to memory block");

	// there is some segfault issue here
	for (MemoryBlock *current = memory, *n; current && (n = current->next, 1); current = n) {
		free(current->memory);
		free(current);
	}

	return ARENA_ERROR_NONE;
}

static inline ArenaErrorCode linear_reset(MemoryBlock *const memory) {
	ASSERT_CRASH(memory, "Cannot free Null pointer to memory block");

	for (MemoryBlock *current = memory, *n; current && (n = current->next, 1); current = n) {
		if (current == memory) {
			current->allocated = 0;
			continue;
		}
		free(current->memory);
		free(current);
	}
	memory->next = NULL;

	return ARENA_ERROR_NONE;
}

static inline ArenaErrorCode linear_alloc(MemoryBlock *block, const size_t allocation_size, const size_t alignment,
                                          void **result) {
	ASSERT_CRASH(block, "Cannot allocate memory from a null pointer");
	ASSERT_CRASH(block->memory, "Cannot allocate memory from null pointer to memory");
	ASSERT_CRASH(is_power_of_two(alignment), "memory alignment on allocation must be a power of two");
	ASSERT_CRASH(alignment >= 8, "alignment must be equal to or larger than system minimum alignment");
	ASSERT_CRASH(allocation_size != 0, "Cannot allocate memory of size zero");
	ASSERT_CRASH(result, "Cannot allocate memory to a null pointer");

	uintptr_t current = (uintptr_t)((char *)block->memory + block->allocated);
	uintptr_t aligned = (current + (alignment - 1)) & ~(uintptr_t)(alignment - 1);
	size_t offset = aligned - current;

	size_t total_size = allocation_size + offset;

	if (total_size > block->capacity - block->allocated) {
		*result = NULL;
		return ARENA_ERROR_ALLOC_OUT_OF_MEMORY;
	}

	block->allocated += total_size;
	*result = (void *)aligned;

	return ARENA_ERROR_NONE;
}

ArenaErrorCode memory_arena_create(MemoryArena **arena_internal, const AllocatorType type, const size_t alignment,
                                   const size_t initial_size) {

	ASSERT_CRASH(is_power_of_two(alignment), "alignment must be a power of two!");
	ASSERT_CRASH(type != COUNT, "Count is not a valid allocator type");
	ASSERT_CRASH(initial_size != 0, "Cannot initialize zero sized memory arenas");
	ASSERT_CRASH(*arena_internal == NULL, "Should never override existing memory arena");

	ArenaErrorCode error_code = ARENA_ERROR_NONE;
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
		case LINEAR:
			arena->allocator.free_fptr = linear_free;
			arena->allocator.alloc_fptr = linear_alloc;
			arena->allocator.reset_fptr = linear_reset;
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

	*arena_internal = arena;
	return error_code;
}

ArenaErrorCode memory_arena_destroy(MemoryArena **const arena) {
	ASSERT_CRASH((*arena), "Cannot destroy a NULL pointer arena");
	ASSERT_CRASH((*arena)->memory_block, "Arena has no valid memory to destroy");

	ArenaErrorCode error_code = (*arena)->allocator.free_fptr((*arena)->memory_block);
	free(*arena);

	*arena = NULL;
	return error_code;
}

ArenaErrorCode memory_arena_reset(MemoryArena **const arena) {
	ASSERT_CRASH((*arena), "Cannot reset NULL pointer arena");
	ASSERT_CRASH((*arena)->memory_block, "Arena has no memory allocated");

	MemoryArena *internal_arena = *arena;
	ArenaErrorCode error_code = internal_arena->allocator.reset_fptr(internal_arena->memory_block);

	*arena = internal_arena;
	return error_code;
}

ArenaErrorCode memory_arena_alloc(MemoryArena **const arena, const size_t size, void **result) {
	ASSERT_CRASH(*arena, "Cannot allocate memory from a null pointer arena");
	ASSERT_CRASH((*arena)->memory_block, "Cannot allocate memory from a null pointer memory block");
	ASSERT_CRASH(result, "Cannot store memory in a null pointer");

	MemoryArena *internal_arena = *arena;
	void *internal_ptr = NULL;
	ArenaErrorCode error_code = internal_arena->allocator.alloc_fptr(internal_arena->memory_block, size,
	                                                                 internal_arena->alignment, &internal_ptr);

	*result = internal_ptr;
	return error_code;
}
