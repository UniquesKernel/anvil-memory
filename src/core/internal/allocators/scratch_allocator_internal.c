#include "anvil/memory/internal/allocators/scratch_allocator_internal.h"
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
	ASSERT_CRASH(memory, "Cannot free Null pointer to memory block");

	for (MemoryBlock *current = memory, *n; current && (n = current->next, 1); current = n) {
		safe_free(current->memory);
		free(current);
	}
}

void scratch_reset(MemoryBlock *const memory) {
	ASSERT_CRASH(memory, "Cannot reset Null pointer to memory block");

	memory->allocated = 0;
	if (memory->next) {
		scratch_free(memory->next);
		memory->next = NULL;
	}
}

void *scratch_alloc(MemoryBlock *block, const size_t allocation_size, const size_t alignment) {
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
		return NULL;
	}

	block->allocated += total_size;
	return (void *)aligned;
}

bool scratch_alloc_verify(MemoryBlock *const block, const size_t allocation_size, const size_t alignment) {
	ASSERT_CRASH(block, "Cannot verify available memory for NULL pointer");
	ASSERT_CRASH(is_power_of_two(alignment), "Memory Blocks must have a power of two alignment");
	ASSERT_CRASH(allocation_size != 0, "cannot allocate zero memory");

	for (MemoryBlock *current = block; current != NULL; current = current->next) {
		uintptr_t base = (uintptr_t)current->memory;
		uintptr_t curr_pos = base + current->allocated;
		uintptr_t aligned = (curr_pos + (alignment - 1)) & ~(uintptr_t)(alignment - 1);
		size_t padding = aligned - curr_pos;
		size_t total_size = allocation_size + padding;

		if (total_size <= (current->capacity - current->allocated)) {
			return true;
		}
	}
	return false;
}
