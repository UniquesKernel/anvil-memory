#include "anvil/memory/internal/allocators/stack_allocator_internal.h"
#include "anvil/memory/internal/arena_internal.h"
#include "anvil/memory/internal/utility_internal.h"

void stack_free(MemoryBlock *const memory_block) {
	INVARIANT(memory_block, "Cannot free Null pointer to memory block");
	INVARIANT(memory_block->memory, "Cannot free Null pointer to stack memory");

	for (MemoryBlock *current = memory_block, *n; current && (n = current->next, 1); current = n) {
		safe_aligned_free(current->memory);
		free(current);
	}
}

void stack_reset(MemoryBlock *const memory_block) {
	INVARIANT(memory_block, "Cannot free Null pointer to memory block");

	memory_block->allocated = 0;
	if (memory_block->next) {
		stack_free(memory_block->next);
		memory_block->next = NULL;
	}

	return;
}

void *stack_alloc(MemoryBlock **const memory_block, const size_t allocation_size, const size_t alignment) {
	INVARIANT(memory_block && (*memory_block), "Cannot allocate memory from a null pointer");
	INVARIANT((*memory_block)->memory, "Cannot allocate memory from null pointer to memory");
	INVARIANT(is_power_of_two(alignment), "memory alignment on allocation must be a power of two");
	INVARIANT(alignment >= _Alignof(max_align_t),
	          "alignment must be equal to or larger than system minimum alignment");
	INVARIANT(allocation_size != 0, "Cannot allocate memory of size zero");
	INVARIANT((*memory_block)->next == NULL, "Stack allocation must happen for the top of the stack");

	MemoryBlock *current_block = (*memory_block);

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

		current_block->next = malloc(sizeof(MemoryBlock));
		INVARIANT(current_block->next, "System out of memory");

		current_block->next->memory =
		    safe_aligned_alloc((current_block->capacity << 1), alignment, "Malloc failed");
		current_block->next->allocated = 0;
		current_block->next->capacity = (current_block->capacity << 1);
		current_block->next->next = NULL;

		current_block = current_block->next;
		*memory_block = current_block;
	}
	INVARIANT(0, "Stack Allocator failed to allocate memory");
}

bool stack_alloc_verify(MemoryBlock *const memory_block, const size_t allocation_size, const size_t alignment) {
	INVARIANT(memory_block, "Cannot verify available memory for NULL pointer");
	INVARIANT(is_power_of_two(alignment), "Memory Blocks must have a power of two alignment");
	INVARIANT(allocation_size != 0, "cannot allocate zero memory");

	/*
	 * NOTE:(UniquesKernel) Because the memory allocation is dynamic it should always return true
	 * when asked if the arena can handle an allocation. If the system runs out of memory because of
	 * a malloc failure, then an invariant issue should cause a system failure.
	 */
	return true;
}
