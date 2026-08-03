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

// NOTE: tlFileBuf is 8 bytes on 32-bit (void* + uint32_t)
// On 64-bit, sizeof(void*) adds padding. This project is 32-bit only.
// static_assert(sizeof(tlFileBuf) == 8, "");
struct tlFileBuf {
    void*     Buf;
    uint32_t  Size;
};

struct tlSystemCallbacks {
    void* (*MemAlloc)(unsigned size, unsigned align, unsigned flags);
    void  (*MemFree)(void* ptr);
    void* (*MemRealloc)(void* ptr, unsigned newSize, unsigned align, unsigned flags);
    bool  (*ReadFile)(const char* filename, tlFileBuf* out, unsigned align, unsigned flags);
    void  (*ReleaseFile)(tlFileBuf* buf);
    void  (*DebugPrint)(const char* text);
    void  (*CriticalError)(const char* text);
    void  (*FinalPrint)(const char* text);
    bool  (*FatalHandler)(const char* text);
    int   (*GetVersion)(void);
    unsigned (*GetFreeMemory)(void);
    void  (*SetCurrentThreadName)(const char* name);
};

// ============================================================================
// Globals
// ============================================================================
static tlSystemCallbacks tlCurSystemCallbacks = {};
static int    tlMemAllocCounter = 0;
static char   tlHostPrefix[256] = "";

// ============================================================================
// tlSetSystemCallbacks — install platform callbacks
// ea: 0x8333A0
// ============================================================================
void tlSetSystemCallbacks(const tlSystemCallbacks* cb) {
    if (cb)
        memcpy(&tlCurSystemCallbacks, cb, sizeof(tlSystemCallbacks));
}

// ============================================================================
// tlLinkFrame — stub (linker frame marker)
// ea: 0x8333C0
// ============================================================================
void tlLinkFrame() {}

// ============================================================================
// tlLinkConnected — check if linker connected
// ea: 0x8333D0
// ============================================================================
bool tlLinkConnected() { return true; }

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
        // Physical allocation (Xbox) — use aligned malloc fallback
        ptr = malloc(size + align);
    } else {
        ptr = malloc(size);
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
    if (!ptr) return;
    --tlMemAllocCounter;

    if (tlCurSystemCallbacks.MemFree) {
        tlCurSystemCallbacks.MemFree(ptr);
    } else {
        free(ptr);
    }
}

// ============================================================================
// tlMemRealloc — reallocate memory
// ea: 0x833730
// ============================================================================
void* tlMemRealloc(void* ptr, unsigned newSize, unsigned align, unsigned flags) {
    // Simplified — real impl dispatches via callbacks/XPhysicalRealloc
    void* newPtr = tlMemAlloc(newSize, align, flags);
    if (ptr && newPtr) {
        memcpy(newPtr, ptr, newSize);
        tlMemFree(ptr);
    }
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
    else
        tlMemFree(buf->Buf);
    buf->Buf = nullptr;
    buf->Size = 0;
}

// ============================================================================
// tlReadFile — read entire file into memory
// ea: 0x8339B0
// ============================================================================
bool tlReadFile(const char* filename, tlFileBuf* out, unsigned align, unsigned flags) {
    if (tlCurSystemCallbacks.ReadFile)
        return tlCurSystemCallbacks.ReadFile(filename, out, align, flags);

    // Build full path (host prefix + filename)
    char work[512];
    int hostLen = (int)strlen(tlHostPrefix);
    if (hostLen > 0) {
        memcpy(work, tlHostPrefix, hostLen);
        // Forward-slash to backslash conversion
        for (int i = 0; work[i]; ++i)
            if (work[i] == '/') work[i] = '\\';
    } else {
        work[0] = 0;
    }
    strcpy(work + hostLen, filename);

#ifdef _WIN32
    HANDLE h = CreateFileA(work, 0x80000000, 1, nullptr, 3, 0x8000080, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    unsigned size = GetFileSize(h, nullptr);
    out->Size = size;
    out->Buf = tlMemAlloc(size, align, flags);

    unsigned read;
    ReadFile(h, out->Buf, size, &read, nullptr);
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
void tlPrint(const char* text) { tlDebugPrint(text); }  // ea: 0x8337B0
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
    OutputDebugStringA("WARNING: ");
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
}
void tlFinalPrint(const char* text) {  // ea: 0x8334B0
    if (tlCurSystemCallbacks.FinalPrint)
        tlCurSystemCallbacks.FinalPrint(text);
    else
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
        DebugBreak();
    }
}

// ============================================================================
// tlFatalHandler — fatal error callback
// ea: 0x833580
// ============================================================================
bool tlFatalHandler(const char* text) {
    OutputDebugStringA("TL Fatal Error: ");
    OutputDebugStringA(text);
    OutputDebugStringA("\n");
    DebugBreak();
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
int  tlGetVersion() { return tlCurSystemCallbacks.GetVersion ? tlCurSystemCallbacks.GetVersion() : 0; }
unsigned tlGetFreeMemory() { return tlCurSystemCallbacks.GetFreeMemory ? tlCurSystemCallbacks.GetFreeMemory() : 0; }
void tlSetCurrentThreadName(const char*) {}
void tlStackRangeInit() {}
void* tlScratchPadInit() { static char pad[0x10000]; return pad; }
void tlScratchPadReset() {}
void tlInitListInit() {}
