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
    SCRATCH = 0
    LINEAR = 1
    STACK = 2
    POOL = 3
    # COUNT = 4

lib.memory_arena_create.argtypes = [
    ctypes.c_int,
    ctypes.c_size_t,
    ctypes.c_size_t
]
lib.memory_arena_create.restype = ctypes.POINTER(MemoryArena)

lib.memory_arena_destroy.argtypes = [ctypes.POINTER(ctypes.POINTER(MemoryArena))]

lib.memory_arena_reset.argtypes = [ctypes.POINTER(ctypes.POINTER(MemoryArena))]

lib.memory_arena_alloc.argtypes = [
    ctypes.POINTER(ctypes.POINTER(MemoryArena)),
    ctypes.c_size_t
]
lib.memory_arena_alloc.restype = ctypes.c_void_p

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
        self.arena = ctypes.POINTER(MemoryArena)()

    """
    Only create an arena if non exists. Only generate alignments 
    that are powers of two and larger than or equal to the minimum
    system architecture alignment. 

    exponent capped at 1 to ensure alignment stays within reasonable 
    limit of 4KB.
    """
    @rule(
        exponent=integers(min_value=0,max_value=12),
        capacity=integers(min_value=1, max_value=(1 << 20)),
        allocatorType=sampled_from(AllocatorType)
    )
    @precondition(lambda self: not self.arena)
    def create_arena(self, capacity, exponent, allocatorType):
        alignment = SIZE << exponent
        self.arena = lib.memory_arena_create(allocatorType, alignment, capacity)

        assert self.arena

    """
    Only destroy arena if one exists.
    """
    @rule()
    @precondition(lambda self: self.arena)
    def arena_destroy(self):
        lib.memory_arena_destroy(self.arena)
        self.arena = ctypes.POINTER(MemoryArena)()

        assert not self.arena

    """
    Only reset arena if one exists.
    """
    @rule()
    @precondition(lambda self: self.arena)
    def arena_reset(self):
        lib.memory_arena_reset(ctypes.pointer(self.arena))

    """
    Only allocate memory from arena if it exists and we haven't 
    gottent a memory arena out of memory error code from an earlier 
    allocation attempt. This is to avoid useless repeat allocations
    """
    @rule(allocSize=integers(1,(1 << 10)))
    @precondition(lambda self: self.arena)
    def alloc(self, allocSize):
        lib.memory_arena_alloc(ctypes.pointer(self.arena), allocSize)
        # No assertions - this rule just helps generate different arena states

    """
    Allocation verifier should be able to predict if a memory arena allocation will fail or 
    succeed and the correct error code in each case.
    """
    @rule(allocSize=integers(1,(1<<10)))
    @precondition(lambda self: self.arena)
    def alloc_verify(self, allocSize):
        canAlloc = lib.memory_arena_alloc_verify(self.arena, allocSize)
        arena_ptr = lib.memory_arena_alloc(self.arena, allocSize)

        if canAlloc == True:
            assert arena_ptr
        else:
            assert not arena_ptr


    """
    Ensure the arena is and all allocated memory is destroyed at the end of the test.
    """
    def teardown(self):
        if (self.arena):
            lib.memory_arena_destroy(ctypes.pointer(self.arena))
            self.arena = ctypes.POINTER(MemoryArena)()

TestMyStateMachine = MemoryArenaModel.TestCase
