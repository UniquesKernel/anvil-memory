#include "anvil/memory/internal/allocators/scratch_allocator_internal.h"
#include "anvil/memory/arena.h"
#include "anvil/memory/internal/arena_internal.h"
#include "anvil/memory/internal/utility_internal.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/mman.h>

/*****************************************************************************************************
 *					Scratch Allocator
 * ***************************************************************************************************/

void scratch_free(MemoryBlock *const memory) {
	INVARIANT(memory, "Cannot free Null pointer to memory block");

	for (MemoryBlock *current = memory, *n; current && (n = current->next, 1); current = n) {
		safe_aligned_free(current->memory);
		free(current);
	}
}

void scratch_reset(MemoryBlock *const memory) {
	INVARIANT(memory, "Cannot reset Null pointer to memory block");

	memory->allocated = 0;
	if (memory->next) {
		scratch_free(memory->next);
		memory->next = NULL;
	}
}

void *scratch_alloc(MemoryArena **const arena, const size_t allocation_size) {
	INVARIANT(arena && (*arena), "");
	INVARIANT((*arena)->memory_block, "Cannot allocate memory from a null pointer");
	INVARIANT((*arena)->memory_block->memory, "Cannot allocate memory from null pointer to memory");
	INVARIANT(is_power_of_two((*arena)->alignment), "memory alignment on allocation must be a power of two");
	INVARIANT((*arena)->alignment >= _Alignof(max_align_t),
	          "alignment must be equal to or larger than system minimum alignment");
	INVARIANT(allocation_size != 0, "Cannot allocate memory of size zero");

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
	INVARIANT(arena, "");
	INVARIANT(arena->memory_block, "Cannot verify available memory for NULL pointer");
	INVARIANT(is_power_of_two(arena->alignment), "Memory Blocks must have a power of two alignment");
	INVARIANT(allocation_size != 0, "cannot allocate zero memory");

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
