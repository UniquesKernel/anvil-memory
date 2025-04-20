// error_templates.h
#ifndef ERROR_TEMPLATES_H
#define ERROR_TEMPLATES_H

// Memory allocation errors
#define ERR_ALLOC_SIZE_ZERO                  "Size must be positive but was 0"
#define ERR_ALLOC_ALIGNMENT_NOT_POWER_OF_TWO "Alignment must be a power of two but was %zu"
#define ERR_ALLOC_ALIGNMENT_TOO_LARGE        "Alignment must be <= %zu but was %zu"
#define ERR_OUT_OF_MEMORY                    "Failed to allocate %zu bytes (system out of memory)"

// Pointer validation errors
#define ERR_NULL_POINTER      "%s pointer must not be NULL"
#define ERR_NULL_MEMORY_BLOCK "Memory block pointer must not be NULL"
#define ERR_NULL_MEMORY       "Memory pointer must not be NULL"

// Value range errors
#define ERR_VALUE_RANGE "Value %s must be between %d and %d but was %d"
#define ERR_VALUE_MIN   "Value %s must be >= %d but was %d"
#define ERR_VALUE_MAX   "Value %s must be <= %d but was %d"

// Arena-specific errors
#define ERR_INVALID_ALLOCATOR_TYPE "Allocator type must be < %d but was %d"
#define ERR_ZERO_CAPACITY          "Arena capacity must be > 0 but was %zu"
#define ERR_ALIGNMENT_TOO_SMALL    "Alignment must be >= %zu but was %zu"
#define ERR_ALLOCATION_TOO_LARGE   "Allocation size %zu exceeds available capacity %zu"

// Comparison errors
#define ERR_LESS_THAN     "%s must be < %s: %zu not < %zu"
#define ERR_LESS_EQUAL    "%s must be <= %s: %zu not <= %zu"
#define ERR_GREATER_THAN  "%s must be > %s: %zu not > %zu"
#define ERR_GREATER_EQUAL "%s must be >= %s: %zu not >= %zu"
#define ERR_EQUAL         "%s must == %s: %zu != %zu"
#define ERR_NOT_EQUAL     "%s must != %s: %zu == %zu"

// State validation errors
#define ERR_INVALID_STATE               "%s in invalid state: expected %s but was %s"
#define ERR_OPERATION_INVALID_FOR_STATE "Operation %s not valid for %s in state %s"

#endif    // ERROR_TEMPLATES_H
