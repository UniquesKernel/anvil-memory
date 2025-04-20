#include "anvil/memory/arena.h"
#include "anvil/memory/internal/allocators/linear_allocator_internal.h"
#include "anvil/memory/internal/utility_internal.h"
#include <stddef.h>
#include <stdlib.h>
#include <sys/mman.h>

/*****************************************************************************************************
 *					Linear Allocator
 * ***************************************************************************************************/

void linear_free(MemoryBlock *const memory) {
	INVARIANT(memory, "Cannot free Null pointer to memory block");

	for (MemoryBlock *current = memory, *n; current && (n = current->next, 1); current = n) {
		safe_aligned_free(current->memory);
		free(current);
	}
}

void linear_reset(MemoryBlock *const memory) {
	INVARIANT(memory, "Cannot reset Null pointer to memory block");
	memory->allocated = 0;
	if (memory->next) {
		linear_free(memory->next);
		memory->next = NULL;
	}
}

void *linear_alloc(MemoryArena **const arena, const size_t allocation_size) {
	INVARIANT(arena && (*arena), "Cannot allocate from NULL arena pointer");
	INVARIANT((*arena)->memory_block, "Cannot allocate memory from a null pointer");
	INVARIANT((*arena)->memory_block->memory, "Cannot allocate memory from null pointer to memory");
	INVARIANT(is_power_of_two((*arena)->alignment), "memory alignment on allocation must be a power of two");
	INVARIANT((*arena)->alignment >= _Alignof(max_align_t),
	          "alignment must be equal to or larger than system minimum alignment");
	INVARIANT(allocation_size != 0, "Cannot allocate memory of size zero");

	MemoryBlock *current_block = (*arena)->memory_block;
	const size_t alignment = (*arena)->alignment;

	while (1) {
		uintptr_t base = (uintptr_t)current_block->memory;
		uintptr_t current = base + current_block->allocated;
		uintptr_t aligned = (current + (alignment - 1)) & ~(uintptr_t)(alignment - 1);
		size_t offset = aligned - current;
		size_t total_size = allocation_size + offset;

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

bool linear_alloc_verify(MemoryArena *const arena, const size_t allocation_size) {
	INVARIANT(arena, "Cannot verify available memory for NULL pointer");
	INVARIANT(allocation_size != 0, "cannot allocate zero memory");

	/*
	 * NOTE:(UniquesKernel) Because the memory allocation is dynamic it should always return true
	 * when asked if the arena can handle an allocation. If the system runs out of memory because of
	 * a malloc failure, then an invariant issue should cause a system failure.
	 */
	return true;
}
