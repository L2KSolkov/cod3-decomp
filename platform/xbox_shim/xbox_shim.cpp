// Xbox API shim implementations (Phase 0 — minimal stubs)

#include "xbox_shim.h"

void* MmAllocateContiguousMemory(unsigned int size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}
void MmFreeContiguousMemory(void* ptr) {
    VirtualFree(ptr, 0, MEM_RELEASE);
}
void* MmAllocateContiguousMemoryEx(unsigned int size, unsigned int, unsigned int, unsigned int, unsigned int) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}
void MmPersistContiguousMemory(void*, unsigned int, int) {}
void MmGetPhysicalAddress(void*, unsigned int* out) { *out = 0; }
unsigned int XGetTickCount(void) { return GetTickCount(); }

HANDLE NtCreateEvent(void) { return CreateEventA(NULL, FALSE, FALSE, NULL); }
HANDLE PsCreateSystemThreadEx(void*, void*) { return NULL; }
void RtlInitAnsiString(void*, const char*) {}
