#ifndef ANVIL_MEMORY_ARENA_INTERNAL_H
#define ANVIL_MEMORY_ARENA_INTERNAL_H

#include "anvil/memory/arena.h"
#include <stddef.h>

/**
 * @brief Memory block
 *
 * Represents a block of memory with a max capacity and a current allocated amount.
 *
 * Invariants:
 * - Allocated is less than or equal to capacity.
 * - capacity is larger than zero.
 * - memory is not null.
 *
 * Fields	| Type		| Size
 * ---		| ---		| ---
 * memory	| pointer	| 4 or 8 Bytes
 * next		| Pointer	| 4 or 8 Bytes
 * capacity	| size_t	| 4 or 8 Bytes
 * allocated	| size_t	| 4 or 8 Bytes
 */
typedef struct MemoryBlock {
	void *memory;                ///< Aligned memory pointer
	struct MemoryBlock *next;    ///< Linked Memory Block
	size_t capacity;             ///< Usable capacity
	size_t allocated;            ///< Currently used bytes
} MemoryBlock;

static_assert(sizeof(MemoryBlock) == 16 || sizeof(MemoryBlock) == 32,
              "MemoryBlock must be either 16 or 32 bytes depending on architecture");
static_assert(_Alignof(MemoryBlock) == _Alignof(void *), "MemoryBlock alignment must match pointer alignment");

/**
 * @brief Allocator
 *
 * This allocator struct contains allocation strategies for use by a Memory Arena.
 *
 * Invariants:
 * - Alloc, Free and Reset are never null.
 * - Reset can be set as a no ops function if it is not meant to be used, but it is still never
 * null.
 *
 * Fields		| Type		| Size
 * ---			| ---		| ---
 * alloc_fptr		| Pointer	| 4 or 8 Bytes
 * free_fptr		| Pointer	| 4 or 8 Bytes
 * reset_fptr		| Pointer	| 4 or 8 Bytes
 * alloc_verify_fptr	| Pointer	| 4 or 8 Bytes
 */
typedef struct memory_allocator_t {
	void *(*alloc_fptr)(MemoryBlock *const memory, const size_t allocation_size, const size_t alignment);
	void (*free_fptr)(MemoryBlock *const memory);
	void (*reset_fptr)(MemoryBlock *const memory);
	bool (*alloc_verify_fptr)(MemoryBlock *const memory, const size_t allocation_size, const size_t alignment);
} Allocator;

static_assert(sizeof(Allocator) == 16 || sizeof(Allocator) == 32, "Allocator must be either 16 or 32 bytes");
static_assert(_Alignof(Allocator) == _Alignof(void *), "Allocator alignment must match pointer alignment");

/**
 * @brief Memory Arena
 *
 * Memory Arena represents a bulk allocation of one or more memory blocks.
 * It contains an allocator for its allocation strategy and an alignment which is
 * shared accross all memory blocks. Alignments must be a power of two.
 *
 * Invariants:
 * - alignment is a power of two.
 * - memory_block is never null.
 * - allocator is always defined.
 *
 * Fields		| Type		| Size
 * ---			| ---		| ---
 * allocator		| Allocator	| 12 or 24 Bytes
 * memory_block		| Pointer	| 4 or 8 Bytes
 * alignment		| size_t	| 4 or 8 bytes
 *
 * @note Memory Arena is **NOT** thread safe so don't use it in concurrent programs.
 */
typedef struct memory_arena_t {
	Allocator allocator;
	MemoryBlock *memory_block;
	size_t alignment;
} MemoryArena;

static_assert(sizeof(MemoryArena) == 24 || sizeof(MemoryArena) == 48,
              "Memory arena is either 24 or 48 bytes depending on architecture");
static_assert(_Alignof(MemoryArena) == _Alignof(void *),
              "Alignment of MemoryArena must match the alignment of the allocator");
#endif    // ANVIL_MEMORY_ARENA_INTERNAL_H
