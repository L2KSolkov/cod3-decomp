// ============================================================================
// COD3 Xbox API Shim — Win32 implementations of Xbox kernel/graphics APIs
// Phase 0: minimal stubs to satisfy the linker. Real implementations come later.
// ============================================================================

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Memory
// ============================================================================
#include <stdlib.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#define XMemAlloc(size, align) malloc(size)
#define XMemFree(ptr, align) free(ptr)
#ifdef __APPLE__
#define XMemSize(ptr, align) malloc_size(ptr)
#else
#define XMemSize(ptr, align) _msize(ptr)
#endif

// ============================================================================
// Threading / Synchronization
// ============================================================================
#include <windows.h>
typedef CRITICAL_SECTION RTL_CRITICAL_SECTION;

#define RtlInitializeCriticalSection(cs) InitializeCriticalSection(cs)
#define RtlEnterCriticalSection(cs)     EnterCriticalSection(cs)
#define RtlLeaveCriticalSection(cs)     LeaveCriticalSection(cs)
#define RtlDeleteCriticalSection(cs)    DeleteCriticalSection(cs)

// ============================================================================
// File I/O
// ============================================================================
#include <fileapi.h>

#define NtCreateFile CreateFileA
#define NtReadFile   ReadFile
#define NtWriteFile  WriteFile
#define NtClose      CloseHandle

// ============================================================================
// Time
// ============================================================================
#define KeQueryPerformanceCounter()  ({ LARGE_INTEGER _c; QueryPerformanceCounter(&_c); _c.QuadPart; })
#define KeQueryPerformanceFrequency() ({ LARGE_INTEGER _f; QueryPerformanceFrequency(&_f); _f.QuadPart; })
#define KeQuerySystemTime GetSystemTimeAsFileTime
#define KeTickCount       GetTickCount

// ============================================================================
// Debug
// ============================================================================
#define DbgPrint OutputDebugStringA
#define KeBugCheck(code) do { __debugbreak(); ExitProcess(code); } while(0)

// ============================================================================
// Xbox-specific (stub implementations in xbox_shim.cpp)
// ============================================================================
void* MmAllocateContiguousMemory(unsigned int size);
void  MmFreeContiguousMemory(void* ptr);
void* MmAllocateContiguousMemoryEx(unsigned int size, unsigned int align_physical,
                                    unsigned int align_virtual, unsigned int protect, unsigned int flags);
void  MmPersistContiguousMemory(void* base, unsigned int size, int persist);
void  MmGetPhysicalAddress(void* ptr, unsigned int* out);
unsigned int XGetTickCount(void);

// ============================================================================
// Event / Semaphore / Mutex (stubbed to Win32 equivalents)
// ============================================================================
HANDLE NtCreateEvent(void);
#define NtSetEvent(e)   SetEvent(e)
#define NtClearEvent(e) ResetEvent(e)
#define KeSetEvent(e, prio, state) SetEvent(e)
#define KeWaitForSingleObject(h, reason, mode, alertable, timeout) \
    WaitForSingleObject(h, timeout)

// ============================================================================
// Thread creation
// ============================================================================
HANDLE PsCreateSystemThreadEx(void* func, void* arg);

// ============================================================================
// Xbox XTL string utilities
// ============================================================================
void RtlInitAnsiString(void* dst, const char* src);

#ifdef __cplusplus
}
#endif
