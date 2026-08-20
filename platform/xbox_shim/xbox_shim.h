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
unsigned int XGetVideoFlags(void);
unsigned int XGetVideoStandard(void);

// XDK launch payload used by dashboard/title transitions.  The original
// header defines this as a 3072-byte opaque data block.
typedef struct _LAUNCH_DATA {
    unsigned char Data[3072];
} LAUNCH_DATA;
unsigned int __stdcall XLaunchNewImageA(const char* lpTitlePath,
                                         LAUNCH_DATA* pLaunchData);

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

// ============================================================================
// Memory Unit / save games (peripherals_xboxr -> Win32 file system)
// The logical MU root (e.g. "U") maps onto %USERPROFILE%\.cod3\MemoryUnit\.
// ============================================================================
typedef struct _XGAME_FIND_DATA {
    WIN32_FIND_DATAA wfd;               // +0x000
    char szSaveGameDirectory[260];      // +0x140
    unsigned short szSaveGameName[128]; // +0x244
} XGAME_FIND_DATA;

// Device-type cookie passed to XGetDevices/XGetDeviceChanges (value ignored
// on Win32; the Xbox SDK kept the actual table in xapilibd:mu.obj).
extern const DWORD XDEVICE_TYPE_MEMORY_UNIT_TABLE;

DWORD XGetDevices(DWORD DeviceType);
DWORD XGetDeviceChanges(DWORD DeviceType, DWORD* pInsertions, DWORD* pRemovals);
int XMountMUA(DWORD dwPort, DWORD dwSlot, char* pchDrive);
int XUnmountMU(DWORD dwPort, DWORD dwSlot);
int XCreateSaveGame(const char* lpRootPathName, const wchar_t* lpSaveGameName,
                    DWORD dwNumberOfBytesWritten, DWORD dwCreationDisposition,
                    char* lpString1, int iMaxLength);
int XDeleteSaveGame(const char* lpRootPathName, const wchar_t* lpSaveGameName);
int XGetDiskSectorSizeA(const char* lpRootPathName);
int XGetDiskClusterSizeA(const char* lpRootPathName);
HANDLE XFindFirstSaveGame(const char* lpRootPathName, XGAME_FIND_DATA* pFindGameData);
int XFindNextSaveGame(HANDLE hFindGame, XGAME_FIND_DATA* pFindGameData);
int XFindClose(HANDLE hMem);
DWORD XGetDisplayBlocks(const char* lpSaveGameDirectory);
void* XCalculateSignatureBegin(DWORD dwFlags);
int XCalculateSignatureUpdate(void* hCalcSig, const void* pbData, DWORD cbData);
int XCalculateSignatureEnd(void* hMem, void* pbSignature);

// Real directory behind the logical MU root (for GetDiskFreeSpaceExA etc.).
const char* XGetMemoryUnitRootPath(void);

#ifdef __cplusplus
}
#endif
