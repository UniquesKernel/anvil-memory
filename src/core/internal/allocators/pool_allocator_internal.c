#include "anvil/memory/internal/allocators/pool_allocator_internal.h"
#include "anvil/memory/arena.h"
#include "anvil/memory/internal/arena_internal.h"
#include "anvil/memory/internal/utility_internal.h"
#include <stddef.h>

void pool_free(MemoryBlock *const memory_block) {
	INVARIANT(memory_block, "Cannot free NULL memory block");

	for (MemoryBlock *current = memory_block, *n; current && (n = current->next, 1); current = n) {
		safe_aligned_free(current->memory);
		free(current);
	}
}

void pool_reset(MemoryBlock *const memory_block) {
	INVARIANT(memory_block, "Cannot reset NULL memory block");
	memory_block->allocated = 0;
	if (memory_block->next) {
		pool_free(memory_block->next);
		memory_block->next = NULL;
	}
}

void *pool_alloc(MemoryArena **const arena, const size_t allocation_size) {
	INVARIANT(arena && (*arena), "Cannot allocate from NULL arena");
	INVARIANT((*arena)->memory_block, "Cannot allocate memory from a null pointer");
	INVARIANT((*arena)->memory_block->memory, "Cannot allocate memory from null pointer to memory");
	INVARIANT(is_power_of_two((*arena)->alignment), "memory alignment on allocation must be a power of two");
	INVARIANT((*arena)->alignment >= _Alignof(max_align_t),
	          "alignment must be equal to or larger than system minimum alignment");
	INVARIANT(allocation_size != 0, "Cannot allocate memory of size zero");

	MemoryBlock *current_block = (*arena)->memory_block;
	size_t alignment = (*arena)->alignment;
	size_t pool_size = (*arena)->state.poolAllocatorState.pool_size;

	size_t num_pools = allocation_size / pool_size;
	size_t pool_aligned_size = num_pools * pool_size;

	if (pool_aligned_size < allocation_size) {
		pool_aligned_size += pool_size;
	}

	while (1) {
		uintptr_t base = (uintptr_t)current_block->memory;
		uintptr_t current = base + current_block->allocated;
		uintptr_t aligned = (current + (alignment - 1)) & ~(uintptr_t)(alignment - 1);
		size_t offset = aligned - current;
		size_t total_size = pool_aligned_size + offset;

		if (total_size <= current_block->capacity - current_block->allocated) {
			current_block->allocated += total_size;
			return (void *)aligned;
		}

		if (!current_block->next) {
			current_block->next = malloc(sizeof(MemoryBlock));
			INVARIANT(current_block->next, "System out of memory");

			current_block->next->memory =
			    safe_aligned_alloc((current_block->capacity << 1), alignment, "System out of memory");
			current_block->next->allocated = 0;
			current_block->next->capacity = (current_block->capacity << 1);
			current_block->next->next = NULL;
		}

		current_block = current_block->next;
	}
	__builtin_unreachable();
}

bool pool_alloc_verify(MemoryArena *const arena, const size_t allocation_size) {
	INVARIANT(arena, "Cannot verify allocation for NULL arena");
	INVARIANT(arena->memory_block, "Cannot verify allocation for NULL memory block");
	INVARIANT(is_power_of_two(arena->alignment), "Alignment must be a power of two");
	INVARIANT(allocation_size != 0, "Cannot verify allocation of size zero");
	return true;
}
