#ifndef ANVIL_MEMORY_STACK_ALLOCATOR_INTERNAL_H
#define ANVIL_MEMORY_STACK_ALLOCATOR_INTERNAL_H

#include "anvil/memory/internal/arena_internal.h"
#include <stddef.h>

void stack_free(MemoryBlock *const memory_block);
void stack_reset(MemoryBlock *const memory_block);
void *stack_alloc(MemoryBlock **const memory_block, const size_t size, const size_t alignment);
bool stack_alloc_verify(MemoryBlock *const memory_block, const size_t size, const size_t alignment);

#endif    // !ANVIL_MEMORY_ARENA_STACK
