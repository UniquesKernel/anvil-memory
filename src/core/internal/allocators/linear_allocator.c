#include "anvil/memory/internal/allocators/linear_allocator_internal.h"
#include "anvil/memory/internal/utility_internal.h"
#include <stdlib.h>
#include <sys/mman.h>

/*****************************************************************************************************
 *					Linear Allocator
 * ***************************************************************************************************/

void linear_free(MemoryBlock *const memory) {
	ASSERT_CRASH(memory, "Cannot free Null pointer to memory block");

	for (MemoryBlock *current = memory, *n; current && (n = current->next, 1); current = n) {
		safe_free(current->memory);
		free(current);
	}
}

void linear_reset(MemoryBlock *const memory) {
	ASSERT_CRASH(memory, "Cannot reset Null pointer to memory block");
	memory->allocated = 0;
	if (memory->next) {
		linear_free(memory->next);
		memory->next = NULL;
	}
}

void *linear_alloc(MemoryBlock *block, const size_t allocation_size, const size_t alignment) {
	ASSERT_CRASH(block, "Cannot allocate memory from a null pointer");
	ASSERT_CRASH(block->memory, "Cannot allocate memory from null pointer to memory");
	ASSERT_CRASH(is_power_of_two(alignment), "memory alignment on allocation must be a power of two");
	ASSERT_CRASH(alignment >= _Alignof(max_align_t),
	             "alignment must be equal to or larger than system minimum alignment");
	ASSERT_CRASH(allocation_size != 0, "Cannot allocate memory of size zero");

	uintptr_t base = (uintptr_t)block->memory;
	uintptr_t current = base + block->allocated;
	uintptr_t aligned = (current + (alignment - 1)) & ~(uintptr_t)(alignment - 1);
	size_t offset = aligned - current;

	size_t total_size = allocation_size + offset;

	if (total_size > block->capacity - block->allocated) {
		// NOTE: ADD the new block if block not big enough for allocation.
		// double the size of the previous block
		if (!block->next) {
			block->next = malloc(sizeof(MemoryBlock));

			ASSERT_CRASH(block->next, "System out of memory");

			block->next->memory = safe_malloc((block->capacity << 1), alignment, "Malloc failed");
			block->next->allocated = 0;
			block->next->capacity = (block->capacity << 1);
			block->next->next = NULL;
		}
		return linear_alloc(block->next, allocation_size, alignment);
	}

	block->allocated += total_size;
	return (void *)aligned;
}

bool linear_alloc_verify(MemoryBlock *const block, const size_t allocation_size, const size_t alignment) {
	ASSERT_CRASH(block, "Cannot verify available memory for NULL pointer");
	ASSERT_CRASH(is_power_of_two(alignment), "Memory Blocks must have a power of two alignment");
	ASSERT_CRASH(allocation_size != 0, "cannot allocate zero memory");

	/*
	 * NOTE:(UniquesKernel) Because the memory allocation is dynamic it should always return true
	 * when asked if the arena can handle an allocation. If the system runs out of memory because of
	 * a malloc failure, then an invariant issue should cause a system failure.
	 */
	return true;
}
