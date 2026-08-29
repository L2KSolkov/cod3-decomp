// ============================================================================
// TL System — Tech Library system layer (memory, file I/O, logging, assertions)
// Source: tl_system.cpp + tl_initlist.cpp (23 + 1 funcs)
// ea: 0x8333A0-0x833B00, 0x833B70
// ============================================================================

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <cstdint>

#include "render/cdDebugVertexDef.h"

#ifdef _WIN32
#include <malloc.h>
#endif

#ifdef _WIN32
  #include <windows.h>
#else
  #include <cstdio>
  #include <cstdlib>
  #define OutputDebugStringA(s) fprintf(stderr, "%s", s)
  #define DebugBreak() __builtin_debugtrap()
#endif

// ============================================================================
// Types
// ============================================================================

struct tlFileBuf {
    void*     Buf;
    uint32_t  Size;
    uint32_t  UserData;
};
static_assert(sizeof(tlFileBuf) == 0xC, "tlFileBuf layout mismatch");

struct tlSystemCallbacks {
    bool  (*ReadFile)(const char* filename, tlFileBuf* out,
                      unsigned align, unsigned flags);
    void  (*ReleaseFile)(tlFileBuf* buf);
    void  (*CriticalError)(const char* text);
    void  (*Warning)(const char* text);
    void  (*DebugPrint)(const char* text);
    void  (*FinalPrint)(const char* text);
    bool  (*LinkConnected)();
    void* (*MemAlloc)(unsigned size, unsigned align, unsigned flags);
    void* (*MemRealloc)(void* ptr, unsigned newSize,
                        unsigned align, unsigned flags);
    void  (*MemFree)(void* ptr);
};
static_assert(sizeof(tlSystemCallbacks) == 0x28,
              "tlSystemCallbacks layout mismatch");

class tlMemAllocMutex {
public:
    tlMemAllocMutex();
    ~tlMemAllocMutex();
};

// ============================================================================
// Globals
// ============================================================================
static tlSystemCallbacks tlCurSystemCallbacks = {};
static int    tlMemAllocCounter = 0;
static char   tlHostPrefix[256] = "";
int tlScratchPadRefCount = 0;
void* tlScratchPadPtr = nullptr;
extern void* tlStackBegin;
extern void* tlStackEnd;

#ifdef _WIN32
static void tlStopAfterFatal()
{
    // Preserve the original debugger break for interactive diagnosis, but do
    // not leave an unattended Win32 process behind a modal breakpoint box.
    if (IsDebuggerPresent())
        DebugBreak();
    else
        ExitProcess(3);
}
#endif

// ============================================================================
// tlSetSystemCallbacks — install platform callbacks
// ea: 0x8333A0
// ============================================================================
void tlSetSystemCallbacks(const tlSystemCallbacks* cb) {
    memcpy(&tlCurSystemCallbacks, cb, sizeof(tlSystemCallbacks));
}

// ============================================================================
// tlLinkFrame is a five-byte release tail thunk. Callers pass allocator
// arguments through unchanged; it jumps to MemAlloc when installed.
// ============================================================================
#if defined(_MSC_VER) && !defined(_WIN64)
// ea: 0x8333C0
__declspec(naked) void tlLinkFrame()
{
    __asm {
        mov eax, dword ptr [tlCurSystemCallbacks+28]
        test eax, eax
        jz short no_callback
        jmp eax
    no_callback:
        ret
    }
}
#else
void tlLinkFrame() {}
#endif

// ============================================================================
// tlLinkConnected — check if linker connected
// ============================================================================
#if defined(_MSC_VER) && !defined(_WIN64)
// ea: 0x8333D0
__declspec(naked) bool tlLinkConnected()
{
    __asm {
        mov eax, dword ptr [tlCurSystemCallbacks+24]
        test eax, eax
        jz short no_callback
        jmp eax
    no_callback:
        xor al, al
        ret
    }
}
#else
bool tlLinkConnected() {
    return tlCurSystemCallbacks.LinkConnected
        ? tlCurSystemCallbacks.LinkConnected() : false;
}
#endif

// ============================================================================
// tlSetFileServerRootPC — set file server root for PC dev
// ea: 0x8333E0
// ============================================================================
void tlSetFileServerRootPC(const char* path) {
    strncpy(tlHostPrefix, path, 255);
    tlHostPrefix[255] = 0;
}

// ============================================================================
// tlMemAlloc — allocate memory
// ea: 0x8336A0
// ============================================================================
void* tlMemAlloc(unsigned size, unsigned align, unsigned flags) {
    if (!align && (flags & 0xF) == 0) align = 0;
    ++tlMemAllocCounter;

    void* ptr;
    if (tlCurSystemCallbacks.MemAlloc) {
        ptr = tlCurSystemCallbacks.MemAlloc(size, align, flags);
    } else if (flags & 0x30000) {
        // Physical allocation (Xbox) — use an aligned host allocation.
#ifdef _WIN32
        ptr = _aligned_malloc(size, align ? align : 16);
#else
        ptr = malloc(size + align);
#endif
    } else {
#ifdef _WIN32
        ptr = _aligned_malloc(size, align ? align : 16);
#else
        ptr = malloc(size);
#endif
    }

    if (!(flags & 2) && !ptr) {
        extern void tlFatal(const char* fmt, ...);
        tlFatal("Memory allocation failed. %d bytes, %d align", size, align);
    }
    return ptr;
}

// ============================================================================
// tlMemFree — free memory
// ea: 0x833400
// ============================================================================
void tlMemFree(void* ptr) {
    --tlMemAllocCounter;

    if (tlCurSystemCallbacks.MemFree) {
        tlCurSystemCallbacks.MemFree(ptr);
    } else {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
}

// ============================================================================
// tlMemRealloc — reallocate memory
// ea: 0x833730
// ============================================================================
void* tlMemRealloc(void* ptr, unsigned newSize, unsigned align, unsigned flags) {
    if (!align && (flags & 0xF) == 0) align = 0;
    void* newPtr = nullptr;
    if (tlCurSystemCallbacks.MemRealloc) {
        newPtr = tlCurSystemCallbacks.MemRealloc(ptr, newSize, align, flags);
    } else if (flags & 0x30000) {
        tlFatal("Cannot reallocate physical memory.");
    } else {
#ifdef _WIN32
        newPtr = _aligned_realloc(ptr, newSize, align ? align : 16);
#else
        newPtr = realloc(ptr, newSize);
#endif
    }
    if (!(flags & 2) && !newPtr && newSize)
        tlFatal("Memory reallocation failed.");
    return newPtr;
}

// ============================================================================
// tlReleaseFile — release file buffer
// ea: 0x833440
// ============================================================================
void tlReleaseFile(tlFileBuf* buf) {
    if (!buf) return;
    if (tlCurSystemCallbacks.ReleaseFile)
        tlCurSystemCallbacks.ReleaseFile(buf);
    else {
        tlMemFree(buf->Buf);
        buf->Buf = nullptr;
        buf->Size = 0;
        buf->UserData = 0;
    }
}

// ============================================================================
// tlReadFile — read entire file into memory
// ea: 0x8339B0
// ============================================================================
bool tlReadFile(const char* filename, tlFileBuf* out, unsigned align, unsigned flags) {
    if (tlCurSystemCallbacks.ReadFile)
        return tlCurSystemCallbacks.ReadFile(filename, out, align, flags);

    // Build full path, avoiding a duplicate prefix when the caller already
    // supplied it (the release checks this with strncmp).
    char work[512];
    int hostLen = (int)strlen(tlHostPrefix);
    if (strncmp(filename, tlHostPrefix, hostLen) != 0) {
        memcpy(work, tlHostPrefix, hostLen);
    } else {
        hostLen = 0;
    }
    strcpy(work + hostLen, filename);

    // Forward-slash to backslash conversion.
    for (char* p = work; *p; ++p)
        if (*p == '/') *p = '\\';

#ifdef _WIN32
    HANDLE h = CreateFileA(work, 0x80000000, 1, nullptr, 3, 0x8000080, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    unsigned size = GetFileSize(h, nullptr);
    out->Size = size;
    out->Buf = tlMemAlloc(size, align, flags);

    unsigned read;
    ReadFile(h, out->Buf, size, (LPDWORD)&read, nullptr);
    CloseHandle(h);
    return true;
#else
    // Unix fallback
    FILE* fp = fopen(work, "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    out->Size = (unsigned)size;
    out->Buf = tlMemAlloc((unsigned)size, align, flags);
    fread(out->Buf, 1, size, fp);
    fclose(fp);
    return true;
#endif
}

// ============================================================================
// Logging
// ============================================================================
void tlDebugPrint(const char* text) {  // ea: 0x833940
    if (tlCurSystemCallbacks.DebugPrint)
        tlCurSystemCallbacks.DebugPrint(text);
    else
        OutputDebugStringA(text);
}
void tlPrint(const char* text) { OutputDebugStringA(text); }  // ea: 0x8337B0
void tlPrintf(const char* fmt, ...) {  // ea: 0x833960
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (tlCurSystemCallbacks.DebugPrint)
        tlCurSystemCallbacks.DebugPrint(buf);
    else
        OutputDebugStringA(buf);
}
void tlWarning(const char* fmt, ...) {  // ea: 0x833AF0
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (tlCurSystemCallbacks.Warning)
        tlCurSystemCallbacks.Warning(buf);
    else
        tlPrintf("%s", buf);
}
void tlFinalPrint(const char* text) {  // ea: 0x8334B0
    OutputDebugStringA(text);
}

// ============================================================================
// tlFatal — fatal error handler
// ea: 0x8335C0
// ============================================================================
void tlFatal(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (tlCurSystemCallbacks.CriticalError) {
        tlCurSystemCallbacks.CriticalError(buf);
    } else {
        OutputDebugStringA("TL Fatal Error: ");
        OutputDebugStringA(buf);
        OutputDebugStringA("\n");
#ifdef _WIN32
        DebugBreak();
#else
        DebugBreak();
#endif
    }
}

// ============================================================================
// tlFatalHandler — fatal error callback
// ea: 0x833580
// ============================================================================
bool tlFatalHandler(const char* text) {
    if (tlCurSystemCallbacks.CriticalError) {
        tlCurSystemCallbacks.CriticalError(text);
        return false;
    }
    OutputDebugStringA("TL Fatal Error: ");
    OutputDebugStringA(text);
    OutputDebugStringA("\n");
    return true;
}

// ============================================================================
// _tlAssert — assertion handler
// ea: 0x833620
// ============================================================================
bool _tlAssert(const char* file, int line, const char* expr, const char* desc) {
    char buf[256];
    snprintf(buf, sizeof(buf), "ASSERT in %s(%d):\n\"%s\" - %s", file, line, expr ? expr : "", desc ? desc : "");
    buf[255] = 0;

    if (tlCurSystemCallbacks.CriticalError) {
        tlCurSystemCallbacks.CriticalError(buf);
        return false;
    }

    OutputDebugStringA("TL Fatal Error: ");
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
    return true; // caller should __debugbreak()
}

// ============================================================================
// Misc
// ============================================================================
int tlGetVersion() { return 0x10400; }  // ea: 0x8334C0
unsigned tlGetFreeMemory() {  // ea: 0x8334D0
#ifdef _WIN32
    MEMORYSTATUS stat;
    GlobalMemoryStatus(&stat);
    return (unsigned)stat.dwAvailPhys;
#else
    return 0;
#endif
}
void tlSetCurrentThreadName(const char* name) {  // ea: 0x8334F0
#ifdef _WIN32
    struct ThreadNameInfo {
        DWORD dwType;
        const char* szName;
        DWORD dwThreadID;
        DWORD dwFlags;
    } info = { 0x1000, name, (DWORD)-1, 0 };
    __try {
        RaiseException(0x406D1388, 0, 4,
                       reinterpret_cast<const ULONG_PTR*>(&info));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
#else
    (void)name;
#endif
}
void tlStackRangeInit() {  // ea: 0x8337C0
    volatile unsigned local = 0;
    uintptr_t begin = reinterpret_cast<uintptr_t>(&local) & 0xFFFF0000u;
    tlStackBegin = reinterpret_cast<void*>(begin);
#ifdef _WIN32
    auto protect = [](void* address) -> unsigned {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(address, &mbi, sizeof(mbi)))
            return 0;
        return (unsigned)mbi.Protect;
    };
#else
    auto protect = [](void*) -> unsigned { return 4; };
#endif
    unsigned result = protect(tlStackBegin);
#ifdef _WIN32
    // Windows reserves thread stacks in 64K regions but commits pages on
    // demand.  The 64K-aligned base can therefore be PAGE_NOACCESS even while
    // the current stack page is writable, which is not an invalid TL stack.
    // Fall back to the committed page containing our local when that occurs.
    if ((result & 4) == 0) {
        MEMORY_BASIC_INFORMATION localMbi;
        if (VirtualQuery(const_cast<const unsigned*>(&local), &localMbi,
                         sizeof(localMbi))
            && (localMbi.Protect & 4) != 0) {
            tlStackBegin = localMbi.BaseAddress;
            result = (unsigned)localMbi.Protect;
        }
    }
#endif
    if ((result & 4) == 0 &&
        _tlAssert("source/tl_system.cpp", 817,
                  "XQueryMemoryProtect(tlStackBegin) & PAGE_READWRITE",
                  "stack page invalid?"))
        DebugBreak();
    tlStackEnd = tlStackBegin;
    while ((result = protect(tlStackEnd)) & 4)
        tlStackEnd = reinterpret_cast<char*>(tlStackEnd) + 0x10000;
}
void* tlScratchPadInit() {  // ea: 0x833830
    if (tlScratchPadRefCount++ != 0)
        return tlScratchPadPtr;
    ++tlMemAllocCounter;
    void* ptr = tlCurSystemCallbacks.MemAlloc
        ? tlCurSystemCallbacks.MemAlloc(0x4000, 0x10, 0)
#ifdef _WIN32
        : _aligned_malloc(0x4000, 0x10);
#else
        : nullptr;
#endif
    if (!ptr)
        tlFatal("Memory allocation failed. %d bytes, %d align", 0x4000, 16);
    tlScratchPadPtr = ptr;
    return ptr;
}
void tlScratchPadReset() {  // ea: 0x8338B0
    if (tlScratchPadRefCount < 1 &&
        _tlAssert("source/tl_system.cpp", 853,
                  "tlScratchPadRefCount >= 1", "Scratchpad reset underflow."))
        DebugBreak();
    if (--tlScratchPadRefCount == 0) {
        --tlMemAllocCounter;
        if (tlCurSystemCallbacks.MemFree)
            tlCurSystemCallbacks.MemFree(tlScratchPadPtr);
        else {
#ifdef _WIN32
            _aligned_free(tlScratchPadPtr);
#else
            free(tlScratchPadPtr);
#endif
        }
        tlScratchPadPtr = nullptr;
    }
}
tlMemAllocMutex::tlMemAllocMutex() {}  // ea: 0x833B50
tlMemAllocMutex::~tlMemAllocMutex() {}  // ea: 0x833B60
void tlInitListInit() {  // ea: 0x833B70
    for (tlInitList* i = tlInitList::head; i; i = i->next)
        i->Register();
}
