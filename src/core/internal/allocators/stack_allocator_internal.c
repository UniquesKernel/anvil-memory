#include "anvil/memory/internal/allocators/stack_allocator_internal.h"
#include "anvil/memory/internal/allocation/memory_allocation_internal.h"
#include "anvil/memory/internal/arena_internal.h"
#include "anvil/memory/internal/error/error_templates.h"
#include "anvil/memory/internal/utility_internal.h"
#include <stdlib.h>
#include <string.h>

void stack_free(MemoryBlock *const memory_block) {
	INVARIANT(memory_block, ERR_NULL_POINTER, "memory_block");
	INVARIANT(memory_block->memory, ERR_NULL_POINTER, "memory_block->memory");

	for (MemoryBlock *current = memory_block, *n; current && (n = current->next, 1); current = n) {
		safe_aligned_free(current->memory);
		free(current);
	}
}

void stack_reset(MemoryBlock *const memory_block) {
	INVARIANT(memory_block, ERR_NULL_POINTER, "memory_block");

	memset(memory_block->memory, 0x0, memory_block->allocated);
	memory_block->allocated = 0;
	if (memory_block->next) {
		stack_free(memory_block->next);
		memory_block->next = NULL;
	}

	return;
}

void *stack_alloc(MemoryBlock **const memory_block, const size_t allocation_size, const size_t alignment) {
	INVARIANT(memory_block && (*memory_block), ERR_NULL_POINTER, "memory_block");
	INVARIANT((*memory_block)->memory, ERR_NULL_POINTER, "memory_block->memory");
	INVARIANT(is_power_of_two(alignment), ERR_ALLOC_ALIGNMENT_NOT_POWER_OF_TWO, alignment);
	INVARIANT(alignment >= _Alignof(max_align_t), ERR_ALIGNMENT_TOO_SMALL, alignment, _Alignof(max_align_t));
	INVARIANT(allocation_size != 0, ERR_ALLOC_SIZE_ZERO);
	INVARIANT((*memory_block)->next == NULL, ERR_OPERATION_INVALID_FOR_STATE, "allocation", "stack",
	          "intermediate block");

	MemoryBlock *current_block = (*memory_block);

	uintptr_t base = (uintptr_t)current_block->memory;
	uintptr_t current = base + current_block->allocated;
	uintptr_t aligned = (current + (alignment - 1)) & ~(uintptr_t)(alignment - 1);
	size_t offset = aligned - current;
	size_t total_size = allocation_size + offset;

	if (likely(total_size <= current_block->capacity - current_block->allocated)) {
		current_block->allocated += total_size;
		return (void *)aligned;
	}

	MemoryBlock *new_block = malloc(sizeof(MemoryBlock));
	INVARIANT(new_block, ERR_OUT_OF_MEMORY, sizeof(MemoryBlock));

	size_t new_capacity = current_block->capacity << 1;

	new_block->memory = safe_aligned_alloc(new_capacity, alignment);
	new_block->allocated = 0;
	new_block->capacity = new_capacity;
	new_block->next = NULL;

	current_block->next = new_block;
	*memory_block = new_block;

	base = (uintptr_t)new_block->memory;
	aligned = (base + (alignment - 1)) & ~(uintptr_t)(alignment - 1);
	offset = aligned - base;
	new_block->allocated = allocation_size + offset;

	return (void *)aligned;
}

bool stack_alloc_verify(MemoryBlock *const memory_block, const size_t allocation_size, const size_t alignment) {
	INVARIANT(memory_block, ERR_NULL_POINTER, "memory_block");
	INVARIANT(is_power_of_two(alignment), ERR_ALLOC_ALIGNMENT_NOT_POWER_OF_TWO, alignment);
	INVARIANT(allocation_size != 0, ERR_ALLOC_SIZE_ZERO);

	/*
	 * NOTE:(UniquesKernel) Because the memory allocation is dynamic it should always return true
	 * when asked if the arena can handle an allocation. If the system runs out of memory because of
	 * a malloc failure, then an invariant issue should cause a system failure.
	 */
	return true;
}
