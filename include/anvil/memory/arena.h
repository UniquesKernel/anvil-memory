#ifndef ANVIL_MEMORY_ARENA_H
#define ANVIL_MEMORY_ARENA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

typedef struct memory_arena_t MemoryArena;

typedef enum allocator_type_t {
	SCRATCH = 0,    ///< Scratch allocation strategy.
	LINEAR = 1,     ///< Linear allocation strategy.
	STACK = 2,      ///< Stack allocation strategy.
	POOL = 3,       ///< Pool allocation strategy.
	COUNT           ///< Total count of allocators.
} AllocatorType;

/**
 * @brief Creates a memory arena with the specified capacity and alignment
 *
 * This function allocates and initializes a new memory arena. The arena uses the specified
 * allocation strategy and ensures all allocations are aligned to the given boundary.
 *
 * The function will CRASH (not return an error) if any of these invariants are violated:
 * - alignment must be a power of two
 * - capacity must be greater than 0
 * - allocator_type must be a valid allocator (not COUNT)
 * - allocation of internal structures must succeed
 *
 * The function returns the memory arena on success otherwise it returns `NULL`.
 *
 * @param[in] allocator_type 	Allocation strategy (LINEAR, etc). Must not be COUNT.
 * @param[in] alignment      	Memory alignment in bytes. Must be a power of 2.
 * @param[in] capacity       	Initial arena size in bytes. Must be > 0.
 *
 * @return arena 		Pointer to receive the created arena
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate
 *       crashes with diagnostics rather than returning error codes
 *
 * @note The memory arenas created from this function are **NOT** thread safe and should not be used
 * in a cocurrent environment.
 */
MemoryArena *memory_arena_create(const AllocatorType allocator_type, const size_t alignment, size_t capacity);

/**
 * @brief Destroys a memory arena and free all memory allocated to it.
 *
 * This function destroys an arena and frees all memory associated with it. All functions
 * associated with the arena should be set to `NULL` after the arena is destroyed to avoid
 * use after free.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is `NULL`.
 * - arena memory block is null.
 * - failed to free arena.
 *
 * @param[out] arena	Pointer to the arena to destroy sets the arena to `NULL`.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void memory_arena_destroy(MemoryArena **const arena);

/**
 * @brief Resets a memory arena allowing its current memory to be overriden.
 *
 * This function resets the given memory arena allowing all memory allocated from it
 * to be overriden. While use after free is not a concern - pointers allocated
 * from the arena before reseting the arena should be considered tainted and set to
 * NULL to avoid reading garbage values.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is `NULL`.
 * - arena memory block is null.
 * - fails to properly reset the arena.
 *
 * @param[out] arena	Pointer to the arena to reset.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
void memory_arena_reset(MemoryArena **const arena);

/**
 * @brief Allocates an amount of memory equivalent to size and write it to `result`
 *
 * This function will allocate an amount of memory equal to `size`, padded to the
 * arena's alignment. It will write the resulting memory to the `result` pointer.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is `NULL`.
 * - arena's memory block is null.
 * - result pointer is `NULL`.
 *
 * @param[out] arena	Pointer to the arena to reset.
 * @param[in]  size	amount of memory to allocate.
 *
 * @returns pointer the allocated memory. Will return `NULL` if an error occures.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 * @note This function is **NOT** thread safe and shouldn't be used in a concurrent context.
 */
void *__attribute__((malloc, warn_unused_result)) memory_arena_alloc(MemoryArena **const arena, const size_t size);
/**
 * @brief Evaluates if a memory arena has enough memory for an allocation
 *
 * This function will evaluate if an allocation `size` can fit in a memory arena.
 * It will return a boolean, true if the allocation can fit and false otherwise.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is NULL.
 *
 * @param[in] arena to test allocation against.
 * @param[in] size of the test allocation.
 *
 * @return boolean value asserting if an allocation would succeed or not.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 */
bool __attribute__((pure)) memory_arena_alloc_verify(MemoryArena *const arena, const size_t size);

/**
 * @brief Records the current state of a stack memory arena.
 *
 * This function saves a snapshot of the current allocation state of a stack memory arena,
 * allowing future operations to unwind back to this point. The snapshot captures the current
 * memory block, its allocated amount, and capacity.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is `NULL` or points to `NULL`.
 * - arena is not a stack allocator type.
 * - memory allocation for the snapshot array fails.
 *
 * @param[in,out] arena Pointer to the stack arena whose state should be recorded.
 *
 * @note This function is only valid for arenas created with the STACK allocator type.
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 * @note This function is **NOT** thread safe and shouldn't be used in a concurrent context.
 */
void memory_stack_arena_record(MemoryArena **const arena);

/**
 * @brief Unwinds a stack memory arena to its previously recorded state.
 *
 * This function restores a stack memory arena to the most recently recorded snapshot state.
 * It resets the memory allocated after the snapshot was taken and frees any additional
 * memory blocks that were allocated. The snapshot is then removed from the snapshot stack.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is `NULL` or points to `NULL`.
 * - arena is not a stack allocator type.
 * - no snapshots have been recorded (attempting to unwind past the initial state).
 *
 * @param[in,out] arena Pointer to the stack arena to unwind.
 *
 * @note This function is only valid for arenas created with the STACK allocator type.
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 * @note This function is **NOT** thread safe and shouldn't be used in a concurrent context.
 * @note All memory allocated after the snapshot was taken will be invalidated.
 *       Pointers to this memory should be considered invalid after unwinding.
 */
void memory_stack_arena_unwind(MemoryArena **const arena);

/**
 * @brief Moves memory from an external pointer into an arena allocation.
 *
 * This function transfers ownership of externally allocated memory into the arena system.
 * It allocates arena memory, copies the content, frees the original memory, and sets the
 * source pointer to NULL to prevent use-after-free errors.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is `NULL` or points to `NULL`.
 * - src is `NULL` or points to `NULL`.
 * - free_fptr is `NULL`.
 * - size is zero.
 *
 * @param[in,out] arena Pointer to the pointer of the arena to allocate from.
 * @param[in,out] src Pointer to the source memory pointer, will be set to NULL after freeing.
 * @param[in] size Amount of memory to copy from source to arena.
 * @param[in] free_fptr Function pointer used to free the source memory.
 *
 * @return Pointer to the newly allocated memory in the arena, or NULL if arena allocation fails.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 * @note This function is **NOT** thread safe and shouldn't be used in a concurrent context.
 */
void *memory_arena_move(MemoryArena **const arena, void **src, const size_t size, void (*free_fptr)(void *));

/**
 * @brief Copies memory from an external pointer into an arena allocation.
 *
 * This function duplicates data from an external source into arena-managed memory.
 * The original memory remains untouched and the caller retains ownership of it.
 *
 * The function will CRASH (not return an error) if its invariants are violated:
 * - arena is `NULL` or points to `NULL`.
 * - src is `NULL`.
 * - size is zero.
 *
 * @param[in,out] arena Pointer to the pointer of the arena to allocate from.
 * @param[in] src Pointer to the source memory to be copied.
 * @param[in] size Amount of memory to copy from source to arena.
 *
 * @return Pointer to the newly allocated memory in the arena, or NULL if arena allocation fails.
 *
 * @note This function follows fail-fast design - programmer errors trigger immediate crashes with
 *       diagnostics rather than returning error codes.
 * @note This function is **NOT** thread safe and shouldn't be used in a concurrent context.
 */
void *memory_arena_copy(MemoryArena **const arena, const void *const src, const size_t size);

#endif    // !ANVIL_MEMORY_ARENA_H
