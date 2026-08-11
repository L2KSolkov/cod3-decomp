// Xbox API shim implementations (Phase 0 — minimal stubs)

#include "xbox_shim.h"
#include "d3d8.h"

#include <stdio.h>
#include <string.h>

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

// D3D state globals (d3d8d:globals.obj) - the Win32 shim owns these.
unsigned int D3D__DirtyFlags = 0;       // _D3D__DirtyFlags
unsigned int D3D__TextureState[4][32];  // _D3D__TextureState
unsigned int D3D__RenderState[4];       // _D3D__RenderState
unsigned int DTE[4];                    // _DTE (0xCD6DE4 pushbuffer encodes)
unsigned int dword_40304 = 0;           // D3D render-state slot alias
unsigned int dword_BC2CFC = 0;          // D3D state alias
_D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;  // cdGlowShader.o (BSS)

// XGetVideoStandard - XDK xapilibd:xgetvideostandard.obj (shim).
// Returns a display standard tag; the game only tests == 3 (PAL).
unsigned int XGetVideoStandard(void) { return 0; }

HANDLE NtCreateEvent(void) { return CreateEventA(NULL, FALSE, FALSE, NULL); }
HANDLE PsCreateSystemThreadEx(void*, void*) { return NULL; }
void RtlInitAnsiString(void*, const char*) {}

// ============================================================================
// Memory Unit / save games — peripherals_xboxr port
// ============================================================================

const DWORD XDEVICE_TYPE_MEMORY_UNIT_TABLE = 2;

// Logical MU root ("U" / "F") -> %USERPROFILE%\.cod3\MemoryUnit
static const char* MUBaseDir(void) {
    static char path[MAX_PATH];
    if (path[0] == '\0') {
        const char* home = getenv("USERPROFILE");
        if (home == NULL || home[0] == '\0') home = ".";
        _snprintf(path, sizeof(path), "%s\\.cod3\\MemoryUnit", home);
        // Create every path component (XCreateSaveGame may be the first touch).
        char tmp[MAX_PATH];
        strcpy_s(tmp, path);
        for (char* p = tmp + 3; *p != '\0'; ++p) {
            if (*p == '\\' || *p == '/') {
                *p = '\0';
                CreateDirectoryA(tmp, NULL);
                *p = '\\';
            }
        }
        CreateDirectoryA(tmp, NULL);
    }
    return path;
}

const char* XGetMemoryUnitRootPath(void) { return MUBaseDir(); }

static void MUWideToAnsi(const wchar_t* wide, char* ansi, int ansiLen) {
    WideCharToMultiByte(CP_ACP, 0, wide, -1, ansi, ansiLen, NULL, NULL);
}

DWORD XGetDevices(DWORD DeviceType) {
    (void)DeviceType;
    // No removable memory units on Win32; only device 8 (hard drive) exists.
    return 0;
}

DWORD XGetDeviceChanges(DWORD DeviceType, DWORD* pInsertions, DWORD* pRemovals) {
    (void)DeviceType;
    if (pInsertions != NULL) *pInsertions = 0;
    if (pRemovals != NULL) *pRemovals = 0;
    return 0;
}

int XMountMUA(DWORD dwPort, DWORD dwSlot, char* pchDrive) {
    (void)dwPort;
    (void)dwSlot;
    if (pchDrive != NULL) *pchDrive = 'U';
    return 0;
}

int XUnmountMU(DWORD dwPort, DWORD dwSlot) {
    (void)dwPort;
    (void)dwSlot;
    return 0;
}

int XCreateSaveGame(const char* lpRootPathName, const wchar_t* lpSaveGameName,
                    DWORD dwNumberOfBytesWritten, DWORD dwCreationDisposition,
                    char* lpString1, int iMaxLength) {
    (void)lpRootPathName;
    (void)dwNumberOfBytesWritten;
    (void)dwCreationDisposition;
    char name[64];
    MUWideToAnsi(lpSaveGameName, name, sizeof(name));
    char dir[MAX_PATH];
    _snprintf(dir, sizeof(dir), "%s\\%s", MUBaseDir(), name);
    CreateDirectoryA(dir, NULL);
    if (lpString1 != NULL && iMaxLength > 0) {
        strncpy_s(lpString1, iMaxLength, dir, _TRUNCATE);
        lpString1[iMaxLength - 1] = '\0';
    }
    return 0;
}

int XDeleteSaveGame(const char* lpRootPathName, const wchar_t* lpSaveGameName) {
    (void)lpRootPathName;
    char name[64];
    MUWideToAnsi(lpSaveGameName, name, sizeof(name));
    char dir[MAX_PATH];
    _snprintf(dir, sizeof(dir), "%s\\%s", MUBaseDir(), name);
    char pattern[MAX_PATH];
    _snprintf(pattern, sizeof(pattern), "%s\\*.*", dir);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            char file[MAX_PATH];
            _snprintf(file, sizeof(file), "%s\\%s", dir, fd.cFileName);
            DeleteFileA(file);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    if (RemoveDirectoryA(dir)) return 0;
    DWORD err = GetLastError();
    if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) return 2;
    return (int)err;
}

int XGetDiskSectorSizeA(const char* lpRootPathName) {
    (void)lpRootPathName;
    return 512;
}

int XGetDiskClusterSizeA(const char* lpRootPathName) {
    (void)lpRootPathName;
    return 16384;
}

typedef struct _MUFIND {
    HANDLE hFind;
    char base[MAX_PATH];
} MUFIND;

static void MUFindFill(HANDLE hFind, WIN32_FIND_DATAA* fd, XGAME_FIND_DATA* out) {
    out->wfd = *fd;
    strcpy_s(out->szSaveGameDirectory, sizeof(out->szSaveGameDirectory), fd->cFileName);
    MultiByteToWideChar(CP_ACP, 0, fd->cFileName, -1,
                        (wchar_t*)out->szSaveGameName, 128);
}

HANDLE XFindFirstSaveGame(const char* lpRootPathName, XGAME_FIND_DATA* pFindGameData) {
    (void)lpRootPathName;
    char pattern[MAX_PATH];
    _snprintf(pattern, sizeof(pattern), "%s\\*.*", MUBaseDir());
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return (HANDLE)-1;
    for (;;) {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
            strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) {
            if (!FindNextFileA(h, &fd)) {
                FindClose(h);
                return (HANDLE)-1;
            }
            continue;
        }
        MUFindFill(h, &fd, pFindGameData);
        break;
    }
    MUFIND* ctx = (MUFIND*)malloc(sizeof(MUFIND));
    ctx->hFind = h;
    strcpy_s(ctx->base, sizeof(ctx->base), fd.cFileName);
    return (HANDLE)ctx;
}

int XFindNextSaveGame(HANDLE hFindGame, XGAME_FIND_DATA* pFindGameData) {
    MUFIND* ctx = (MUFIND*)hFindGame;
    WIN32_FIND_DATAA fd;
    for (;;) {
        if (!FindNextFileA(ctx->hFind, &fd)) return 0;
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
            strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) {
            continue;
        }
        MUFindFill(ctx->hFind, &fd, pFindGameData);
        return 1;
    }
}

int XFindClose(HANDLE hMem) {
    MUFIND* ctx = (MUFIND*)hMem;
    int result = FindClose(ctx->hFind) ? 1 : 0;
    free(ctx);
    return result;
}

DWORD XGetDisplayBlocks(const char* lpSaveGameDirectory) {
    char pattern[MAX_PATH];
    _snprintf(pattern, sizeof(pattern), "%s\\%s\\*.*", MUBaseDir(), lpSaveGameDirectory);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    unsigned __int64 total = 0;
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            total += ((unsigned __int64)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    return (DWORD)((total + 0x3FFF) >> 14);
}

// The Xbox XCalculateSignature* produce a hardware digital signature; the game
// stores a 20-byte trailer and compares it on load. Win32 substitutes a
// deterministic 20-byte CRC-derived digest so save/load round-trips verify.
typedef struct _MUSIG {
    unsigned int crc;
} MUSIG;

static unsigned int MUCRC32(unsigned int crc, const void* data, unsigned int size) {
    const unsigned char* p = (const unsigned char*)data;
    while (size-- != 0) {
        crc ^= *p++;
        for (int i = 0; i < 8; ++i) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320u;
            else crc >>= 1;
        }
    }
    return crc;
}

void* XCalculateSignatureBegin(DWORD dwFlags) {
    (void)dwFlags;
    MUSIG* s = (MUSIG*)malloc(sizeof(MUSIG));
    s->crc = 0xFFFFFFFFu;
    return s;
}

int XCalculateSignatureUpdate(void* hCalcSig, const void* pbData, DWORD cbData) {
    MUSIG* s = (MUSIG*)hCalcSig;
    s->crc = MUCRC32(s->crc, pbData, cbData);
    return 0;
}

int XCalculateSignatureEnd(void* hMem, void* pbSignature) {
    MUSIG* s = (MUSIG*)hMem;
    unsigned int x = s->crc ^ 0xFFFFFFFFu;
    unsigned int* out = (unsigned int*)pbSignature;
    for (int i = 0; i < 5; ++i) {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        out[i] = x;
    }
    free(s);
    return 0;
}
