#ifndef ANVIL_MEMORY_POOL_ALLOCATOR_INTERNAL_H
#define ANVIL_MEMORY_POOL_ALLOCATOR_INTERNAL_H

#include "anvil/memory/arena.h"
#include "anvil/memory/internal/arena_internal.h"
#include <stddef.h>

void pool_free(MemoryBlock *const memory_block);
void pool_reset(MemoryBlock *const memory_block);
void *pool_alloc(MemoryArena **const arena, const size_t allocation_size);
bool pool_alloc_verify(MemoryArena *const arena, const size_t allocation_size);

#endif    // !ANVIL_MEMORY_POOL_ALLOCATOR_INTERNAL_H
