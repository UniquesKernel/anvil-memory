import ctypes
from hypothesis import assume
from hypothesis.internal.compat import floor
from hypothesis.stateful import RuleBasedStateMachine, precondition, rule
from enum import IntEnum

from hypothesis.strategies import integers
import os
import signal
lib = ctypes.CDLL("./build/libmemory_test.so")

"""
Memory Arena, AllocatorType, ArenaErrorCode and 
function bindings for Memory arena and linear allocator
"""
class MemoryArena(ctypes.Structure):
    pass

class AllocatorType(IntEnum):
    LINEAR = 0
    COUNT = 1

class ArenaErrorCode(IntEnum):
    ARENA_ERROR_NONE = 0
    ARENA_ERROR_ALLOC_OUT_OF_MEMORY = 1

lib.memory_arena_create.argtypes = [
    ctypes.POINTER(ctypes.POINTER(MemoryArena)),
    ctypes.c_int,
    ctypes.c_size_t,
    ctypes.c_size_t
]
lib.memory_arena_create.restype = ArenaErrorCode

lib.memory_arena_destroy.argtypes = [ctypes.POINTER(ctypes.POINTER(MemoryArena))]
lib.memory_arena_destroy.restype = ArenaErrorCode

lib.memory_arena_reset.argtypes = [ctypes.POINTER(ctypes.POINTER(MemoryArena))]
lib.memory_arena_reset.restype = ArenaErrorCode

lib.memory_arena_alloc.argtypes = [
    ctypes.POINTER(ctypes.POINTER(MemoryArena)),
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_void_p)
]


"""
Checking for system alignment requirement for most common architectures 
that anvil supports this will come down to long double or double.
"""
if hasattr(ctypes, 'c_longdouble'):
    max_align_type = ctypes.c_longdouble
else:
    max_align_type = ctypes.c_double

SIZE = ctypes.sizeof(ctypes.c_longdouble)

class MemoryArenaModel(RuleBasedStateMachine):
    """
    Memory Arena Model: models a memory arena as a graph of user api calls 
    to see if the model and the implementation agrees.
    """
    def __init__(self):
        super().__init__()
        self.arena = ctypes.pointer(ctypes.POINTER(MemoryArena)())
        self.err = ArenaErrorCode.ARENA_ERROR_NONE

    """
    Only create an arena if non exists. Only generate alignments 
    that are powers of two and larger than or equal to the minimum
    system architecture alignment.
    """
    @rule(
        exponent=integers(min_value=(floor(SIZE/2)) + 1, max_value=10),
        capacity=integers(min_value=1, max_value=1024)
    )
    @precondition(lambda self: not self.arena.contents)
    def create_arena(self, capacity, exponent):
        alignment = (1 << exponent) 
        assume(alignment >= SIZE)
        self.err = lib.memory_arena_create(self.arena, AllocatorType.LINEAR, alignment, capacity)

        assert self.arena.contents
        assert self.err == ArenaErrorCode.ARENA_ERROR_NONE

    """
    Only destroy arena if one exists.
    """
    @rule()
    @precondition(lambda self: self.arena.contents)
    def arena_destroy(self):
        self.err = lib.memory_arena_destroy(self.arena)
        self.arena = ctypes.pointer(ctypes.POINTER(MemoryArena)())

        assert not self.arena.contents
        assert self.err == ArenaErrorCode.ARENA_ERROR_NONE

    """
    Only reset arena if one exists.
    """
    @rule()
    @precondition(lambda self: self.arena.contents)
    def arena_reset(self):
        self.err = lib.memory_arena_reset(self.arena)

        assert self.err == ArenaErrorCode.ARENA_ERROR_NONE

    """
    Only allocate memory from arena if it exists and we haven't 
    gottent a memory arena out of memory error code from an earlier 
    allocation attempt. This is to avoid useless repeat allocations
    """
    @rule(allocSize=integers(1,1024))
    @precondition(lambda self: self.arena.contents and self.err != ArenaErrorCode.ARENA_ERROR_ALLOC_OUT_OF_MEMORY)
    def alloc(self, allocSize):
        arena_ptr = ctypes.POINTER(ctypes.c_void_p)(ctypes.c_void_p(allocSize))
        self.err = lib.memory_arena_alloc(self.arena, allocSize, arena_ptr)

        assert self.err == ArenaErrorCode.ARENA_ERROR_ALLOC_OUT_OF_MEMORY or self.err == ArenaErrorCode.ARENA_ERROR_NONE
        assert arena_ptr

    """
    Ensure the arena is and all allocated memory is destroyed at the end of the test.
    """
    def teardown(self):
        if (self.arena.contents):
            lib.memory_arena_destroy(self.arena)
            self.arena = ctypes.pointer(ctypes.POINTER(MemoryArena)())

TestMyStateMachine = MemoryArenaModel.TestCase
