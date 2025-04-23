#include "anvil/memory/arena.h"
#include "anvil/memory/internal/allocation/memory_allocation_internal.h"
#include "anvil/memory/internal/allocators/linear_allocator_internal.h"
#include "anvil/memory/internal/error/error_templates.h"
#include "anvil/memory/internal/utility_internal.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

/*****************************************************************************************************
 *					Linear Allocator
 * ***************************************************************************************************/

void linear_free(MemoryBlock *const memory_block) {
	INVARIANT(memory_block, ERR_NULL_POINTER, "memory");

	for (MemoryBlock *current = memory_block, *n; current && (n = current->next, 1); current = n) {
		safe_aligned_free(current->memory);
		free(current);
	}
}

void linear_reset(MemoryBlock *const memory_block) {
	INVARIANT(memory_block, ERR_NULL_POINTER, "memory");

	memset(memory_block->memory, 0x0, memory_block->allocated);
	memory_block->allocated = 0;
	if (memory_block->next) {
		linear_free(memory_block->next);
		memory_block->next = NULL;
	}
}

void *linear_alloc(MemoryArena **const arena, const size_t allocation_size) {
	INVARIANT(arena && (*arena), ERR_NULL_POINTER, "arena");
	INVARIANT((*arena)->memory_block, ERR_NULL_POINTER, "arena->memory_block");
	INVARIANT((*arena)->memory_block->memory, ERR_NULL_POINTER, "arena->memory_block->memory");
	INVARIANT(is_power_of_two((*arena)->alignment), ERR_ALLOC_ALIGNMENT_NOT_POWER_OF_TWO, (*arena)->alignment);
	INVARIANT((*arena)->alignment >= _Alignof(max_align_t), ERR_ALIGNMENT_TOO_SMALL, (*arena)->alignment,
	          _Alignof(max_align_t));
	INVARIANT(allocation_size != 0, ERR_ALLOC_SIZE_ZERO);

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
			INVARIANT(current_block->next, ERR_OUT_OF_MEMORY, sizeof(MemoryBlock));

			current_block->next->memory = safe_aligned_alloc((current_block->capacity << 1), alignment);
			current_block->next->allocated = 0;
			current_block->next->capacity = (current_block->capacity << 1);
			current_block->next->next = NULL;
		}

		current_block = current_block->next;
	}
	__builtin_unreachable();
}

bool linear_alloc_verify(MemoryArena *const arena, const size_t allocation_size) {
	INVARIANT(arena, ERR_NULL_POINTER, "arena");
	INVARIANT(allocation_size != 0, ERR_ALLOC_SIZE_ZERO);

	/*
	 * NOTE:(UniquesKernel) Because the memory allocation is dynamic it should always return true
	 * when asked if the arena can handle an allocation. If the system runs out of memory because of
	 * a malloc failure, then an invariant issue should cause a system failure.
	 */
	return true;
}
