#include "anvil/memory/internal/allocators/scratch_allocator_internal.h"
#include "anvil/memory/arena.h"
#include "anvil/memory/internal/arena_internal.h"
#include "anvil/memory/internal/error_templates.h"
#include "anvil/memory/internal/utility_internal.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/mman.h>

/*****************************************************************************************************
 *					Scratch Allocator
 * ***************************************************************************************************/

void scratch_free(MemoryBlock *const memory) {
	INVARIANT(memory, ERR_NULL_POINTER, "memory");
	for (MemoryBlock *current = memory, *n; current && (n = current->next, 1); current = n) {
		safe_aligned_free(current->memory);
		free(current);
	}
}

void scratch_reset(MemoryBlock *const memory) {
	INVARIANT(memory, ERR_NULL_POINTER, "memory");

	memory->allocated = 0;
	if (memory->next) {
		scratch_free(memory->next);
		memory->next = NULL;
	}
}

void *scratch_alloc(MemoryArena **const arena, const size_t allocation_size) {
	INVARIANT(arena && (*arena), ERR_NULL_POINTER, "arena");
	INVARIANT((*arena)->memory_block, ERR_NULL_POINTER, "arena->memory_block");
	INVARIANT((*arena)->memory_block->memory, ERR_NULL_POINTER, "arena->memory_block->memory");
	INVARIANT(is_power_of_two((*arena)->alignment), ERR_ALLOC_ALIGNMENT_NOT_POWER_OF_TWO, (*arena)->alignment);
	INVARIANT((*arena)->alignment >= _Alignof(max_align_t), ERR_ALIGNMENT_TOO_SMALL, (*arena)->alignment,
	          _Alignof(max_align_t));
	INVARIANT(allocation_size != 0, ERR_ALLOC_SIZE_ZERO);

	uintptr_t base = (uintptr_t)(*arena)->memory_block->memory;
	uintptr_t current = base + (*arena)->memory_block->allocated;
	uintptr_t aligned = (current + ((*arena)->alignment - 1)) & ~(uintptr_t)((*arena)->alignment - 1);
	size_t offset = aligned - current;

	size_t total_size = allocation_size + offset;

	if (total_size > (*arena)->memory_block->capacity - (*arena)->memory_block->allocated) {
		return NULL;
	}

	(*arena)->memory_block->allocated += total_size;
	return (void *)aligned;
}

bool scratch_alloc_verify(MemoryArena *const arena, const size_t allocation_size) {
	INVARIANT(arena, ERR_NULL_POINTER, "arena");
	INVARIANT(arena->memory_block, ERR_NULL_POINTER, "arena->memory_block");
	INVARIANT(is_power_of_two(arena->alignment), ERR_ALLOC_ALIGNMENT_NOT_POWER_OF_TWO, arena->alignment);
	INVARIANT(allocation_size != 0, ERR_ALLOC_SIZE_ZERO);

	for (MemoryBlock *current = arena->memory_block; current != NULL; current = current->next) {
		uintptr_t base = (uintptr_t)current->memory;
		uintptr_t curr_pos = base + current->allocated;
		uintptr_t aligned = (curr_pos + (arena->alignment - 1)) & ~(uintptr_t)(arena->alignment - 1);
		size_t padding = aligned - curr_pos;
		size_t total_size = allocation_size + padding;

		if (total_size <= (current->capacity - current->allocated)) {
			return true;
		}
	}
	return false;
}
