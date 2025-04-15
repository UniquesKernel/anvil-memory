#include "anvil/memory/internal/allocators/stack_allocator_internal.h"
#include "anvil/memory/internal/arena_internal.h"
#include "anvil/memory/internal/utility_internal.h"

void stack_free(MemoryBlock *const memory_block) {
	ASSERT_CRASH(memory_block, "Cannot free Null pointer to memory block");

	for (MemoryBlock *current = memory_block, *n; current && (n = current->next, 1); current = n) {
		safe_aligned_free(current->memory);
		free(current);
	}
}

void stack_reset(MemoryBlock *const memory_block) {
	memory_block->allocated = 0;
	return;
}

void *stack_alloc(MemoryBlock *const memory_block, const size_t size, const size_t alignment) {
	memory_block->allocated = size + alignment;
	return NULL;
}

bool stack_alloc_verify(MemoryBlock *const memory_block, const size_t size, const size_t alignment) {
	memory_block->allocated = size + alignment;
	return true;
}
