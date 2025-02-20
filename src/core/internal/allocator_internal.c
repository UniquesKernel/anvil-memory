
#include "anvil/memory/internal/arena_internal.h"
#include "anvil/memory/internal/utility_internal.h"
#include <stddef.h>
#include <stdint.h>

ArenaErrorCode linear_free(MemoryBlock *const memory) {
	ASSERT_CRASH(memory, "Cannot free Null pointer to memory block");

	for (MemoryBlock *current = memory, *n; current && (n = current->next, 1); current = n) {
		free(current->memory);
		free(current);
	}

	return ARENA_ERROR_NONE;
}

ArenaErrorCode linear_reset(MemoryBlock *const memory) {
	ASSERT_CRASH(memory, "Cannot reset Null pointer to memory block");

	memory->allocated = 0;
	if (memory->next) {
		linear_free(memory->next);
		memory->next = NULL;
	}

	return ARENA_ERROR_NONE;
}

ArenaErrorCode linear_alloc(MemoryBlock *block, const size_t allocation_size, const size_t alignment, void **result) {
	ASSERT_CRASH(block, "Cannot allocate memory from a null pointer");
	ASSERT_CRASH(block->memory, "Cannot allocate memory from null pointer to memory");
	ASSERT_CRASH(is_power_of_two(alignment), "memory alignment on allocation must be a power of two");
	ASSERT_CRASH(alignment >= _Alignof(max_align_t),
	             "alignment must be equal to or larger than system minimum alignment");
	ASSERT_CRASH(allocation_size != 0, "Cannot allocate memory of size zero");
	ASSERT_CRASH(result, "Cannot allocate memory to a null pointer");

	uintptr_t base = (uintptr_t)block->memory;
	uintptr_t current = base + block->allocated;
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
