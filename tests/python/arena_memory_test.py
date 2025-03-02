import ctypes
import hypothesis
from hypothesis.stateful import RuleBasedStateMachine, precondition, rule
from enum import IntEnum

from hypothesis.strategies import integers, sampled_from
lib = ctypes.CDLL("./build/libmemory_test.so")

"""
Memory Arena, AllocatorType, ArenaErrorCode and 
function bindings for Memory arena and linear allocator
"""
class MemoryArena(ctypes.Structure):
    pass

class AllocatorType(IntEnum):
    LINEAR_STATIC = 0
    LINEAR_DYNAMIC = 1
    # COUNT = 2

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
lib.memory_arena_alloc.restype = ArenaErrorCode

lib.memory_arena_alloc_verify.argtypes = [ctypes.POINTER(MemoryArena), ctypes.c_size_t]
lib.memory_arena_alloc_verify.restype = ctypes.c_bool

"""
Checking for system alignment requirement for most common architectures 
that anvil supports this will come down to long double or double.
"""
if hasattr(ctypes, 'c_longdouble'):
    max_align_type = ctypes.c_longdouble
else:
    max_align_type = ctypes.c_double

SIZE = ctypes.sizeof(ctypes.c_longdouble)

@hypothesis.settings(max_examples=1000)
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
        exponent=integers(min_value=0,max_value=10),
        capacity=integers(min_value=1, max_value=1024),
        allocatorType=sampled_from(AllocatorType)
    )
    @precondition(lambda self: not self.arena.contents)
    def create_arena(self, capacity, exponent, allocatorType):
        alignment = (SIZE << exponent)
        self.err = lib.memory_arena_create(self.arena, allocatorType, alignment, capacity)

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
    Allocation verifier should be able to predict if a memory arena allocation will fail or 
    succeed and the correct error code in each case.
    """
    @rule(allocSize=integers(1,1024))
    @precondition(lambda self: self.arena.contents)
    def alloc_verify(self, allocSize):
        arena_ptr = ctypes.POINTER(ctypes.c_void_p)(ctypes.c_void_p(allocSize))
        canAlloc = lib.memory_arena_alloc_verify(self.arena.contents, allocSize)
        self.err = lib.memory_arena_alloc(self.arena, allocSize, arena_ptr)

        if canAlloc == True:
            assert self.err == ArenaErrorCode.ARENA_ERROR_NONE
            assert arena_ptr.contents
        else:
            assert self.err == ArenaErrorCode.ARENA_ERROR_ALLOC_OUT_OF_MEMORY
            assert not arena_ptr.contents


    """
    Ensure the arena is and all allocated memory is destroyed at the end of the test.
    """
    def teardown(self):
        if (self.arena.contents):
            lib.memory_arena_destroy(self.arena)
            self.arena = ctypes.pointer(ctypes.POINTER(MemoryArena)())

TestMyStateMachine = MemoryArenaModel.TestCase
