from ast import AugLoad
import ctypes
from inspect import _void
from os import supports_effective_ids
import re
from hypothesis import given, target
from hypothesis.stateful import RuleBasedStateMachine, precondition, rule
from enum import IntEnum

from hypothesis.strategies import integers, lists, one_of, recursive, sampled_from 

lib = ctypes.CDLL("./build/libmemory_test.so")

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

class MemoryArenaModel(RuleBasedStateMachine):
    def __init__(self):
        super().__init__()
        self.arena = ctypes.pointer(ctypes.POINTER(MemoryArena)())
        self.err = ArenaErrorCode.ARENA_ERROR_NONE

    @rule(
        alignment=sampled_from([8,16,32,64,128]),
        capacity=integers(min_value=1, max_value=1024)
    )
    @precondition(lambda self: not self.arena.contents)
    def create_arena(self, capacity, alignment):
        self.err = lib.memory_arena_create(self.arena, AllocatorType.LINEAR, alignment, capacity)

        assert self.arena.contents
        assert self.err == ArenaErrorCode.ARENA_ERROR_NONE

    @rule()
    @precondition(lambda self: self.arena.contents)
    def arena_destroy(self):
        self.err = lib.memory_arena_destroy(self.arena)
        self.arena = ctypes.pointer(ctypes.POINTER(MemoryArena)())

        assert not self.arena.contents
        assert self.err == ArenaErrorCode.ARENA_ERROR_NONE

    @rule(allocSize=integers(1,1024))
    @precondition(lambda self: self.arena.contents and self.err != ArenaErrorCode.ARENA_ERROR_ALLOC_OUT_OF_MEMORY)
    def alloc(self, allocSize):
        arena_ptr = ctypes.POINTER(ctypes.c_void_p)(ctypes.c_void_p(allocSize))
        self.err = lib.memory_arena_alloc(self.arena, allocSize, arena_ptr)

        assert self.err == ArenaErrorCode.ARENA_ERROR_ALLOC_OUT_OF_MEMORY or self.err == ArenaErrorCode.ARENA_ERROR_NONE
        assert arena_ptr

    def teardown(self):
        if (self.arena.contents):
            lib.memory_arena_destroy(self.arena)
            self.arena = ctypes.pointer(ctypes.POINTER(MemoryArena)())

TestMyStateMachine = MemoryArenaModel.TestCase
