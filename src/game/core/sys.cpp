// ============================================================================
// sys.cpp - TlSystemCallbacks + spinner + misc system (core.o)
// ============================================================================

#include "core/ae_fixed_string.h"
#include "core/tlFixedString.h"
#include "game/core/core_systems.h"
#include "game/core/core_globals.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <intrin.h>

extern void Com_Printf(const char* fmt, ...);
extern void CL_ConsolePrint(int type, const char* txt, int duration,
                            int linewidth, int flags);
extern cvar_t* cl_noprint;
extern void tlPrintf(const char* fmt, ...);
extern void tlPrint(const char* txt);
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void* mem_heap_realloc(void* ptr, unsigned int size);
extern void* mem_heap_malloc_ctx(int alignment, unsigned int size,
                                 const char* ctx, const char* file, int line);
extern void* mem_heap_malloc_align_heap(void* heap, unsigned int alignment,
                                       unsigned int size);
extern void* mem_heap_get(int heap_name);
extern void tlSetSystemCallbacks(void* callbacks);
extern void nglInitQuad(void* quad);
extern void nglSetClearFlags(unsigned int clearFlags);
extern void nglPresent();
extern void nglWaitForRendering();
extern void* nglGetFrontBufferTex();
extern void nglSetQuadTex(void* quad, void* tex);
extern void nglSetQuadRect(void* quad, float x1, float y1, float x2, float y2);
extern void nglListAddQuad(void* quad);
extern void* nglGetTexture(tlFixedString* fileName);
extern void PrintPakNames();
extern void SpinnerDrawFrameWithLoading(bool bEndFrame);
extern int gLensAlphaAmount;
extern void* PakManager_sInst;
extern int PakManager_GetTopContext(void* self);
extern void* PakManager_MemAlign(void* self, int id, unsigned int align,
                                 unsigned int size);
extern void PakManager_MemFree(void* self, int id, void* ptr,
                               bool bUseActorHeap);
extern int MEM_HEAP_COMBINE;
extern int MEM_HEAP_MAIN;
extern int PAK_ID_INVALID;
extern int g_bDObjInited;
extern void* sSpinnerFrames[8];
extern void* dword_F00EB0;
extern void* dword_F00EB4;
extern void* dword_F00EB8;
extern void* dword_F00EBC;
extern void* dword_F00EC0;
extern void* dword_F00EC4;
extern void* dword_F00EC8;

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
extern bool gInAssert;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Warning(const char* fmt, ...);
}

// ============================================================================
// TlSystemCallbacks
// ============================================================================

bool TlSystemCallbacks::sWarningsEnabled = true;
bool TlSystemCallbacks::sLockAllocsToPakHeap = false;
bool TlSystemCallbacks::sLockAllocsToPakHeapOnce = false;

// ea: 0x004BB4E0
void Printf(int dest, const char* fmt, ...)
{
    char txtBuf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(txtBuf, fmt, ap);
    va_end(ap);
    if ((dest & 1) != 0 && cl_noprint != nullptr && cl_noprint->integer == 0)
        tlPrintf("%s", txtBuf);
    if ((dest & 2) != 0)
        CL_ConsolePrint(0, txtBuf, 0, 0, 0);
}

// ea: 0x004BB5A0
void jobqueue_init()
{
}

// ea: 0x004BB5B0
void jobqueue_shutdown()
{
}

// ea: 0x004BB5C0
void jobqueue_restart()
{
}

// ea: 0x004BBAA0
void Script_Init()
{
}

// ea: 0x004BBB70
void SyncFrameBuffers()
{
    unsigned char frontBuf[0x60];
    nglInitQuad(frontBuf);
    nglSetClearFlags(0);
    nglPresent();
    nglWaitForRendering();
    void* FrontBufferTex = nglGetFrontBufferTex();
    nglSetQuadTex(frontBuf, FrontBufferTex);
    nglSetQuadRect(frontBuf, 0.0f, 0.0f, 640.0f, 480.0f);
    nglSetClearFlags(0xF3u);
    nglListAddQuad(frontBuf);
    nglPresent();
    nglWaitForRendering();
}

// ea: 0x004BBF50
void* MT_AllocAnimTree(unsigned int size)
{
    return mem_heap_malloc_ctx(16, size, "hunk",
                               "c:\\cod\\code\\game\\common.cpp", 4145);
}

// ea: 0x004BC240
bool ShouldConnectPaths()
{
    return false;
}

// ea: 0x004BD3E0
bool TlSystemCallbacks::LockTlAllocsToPakHeap(bool s, bool once)
{
    sLockAllocsToPakHeapOnce = once;
    bool result = sLockAllocsToPakHeap;
    sLockAllocsToPakHeap = s;
    return result;
}

// ea: 0x004BD440
bool TlSystemCallbacks::ReadFile()
{
    return false;
}

// ea: 0x004BD450
void TlSystemCallbacks::ReleaseFile(tlFileBuf* fileBuf)
{
    mem_heap_free(fileBuf->Buf);
}

// ea: 0x004BD470
void* TlSystemCallbacks::MemRealloc(void* Ptr, unsigned int Size)
{
    return mem_heap_realloc(Ptr, Size);
}

// ea: 0x004BD490
void* TlSystemCallbacks::MemAlloc(unsigned int size, unsigned int align,
                                  unsigned int flags)
{
    if ((flags & 0x2000) == 0 && sLockAllocsToPakHeap
        && PakManager_sInst != nullptr)
    {
        int TopContext = PakManager_GetTopContext(PakManager_sInst);
        return PakManager_MemAlign(PakManager_sInst, TopContext, align, size);
    }
    void* result;
    if (flags != 0)
    {
        void* heap;
        if ((flags & 0x20000) != 0)
            heap = mem_heap_get(MEM_HEAP_COMBINE);
        else
            heap = mem_heap_get(MEM_HEAP_MAIN);
        result = mem_heap_malloc_align_heap(heap, align, size);
    }
    else
    {
        result = mem_heap_malloc_ctx(align, size, nullptr, nullptr, 0);
    }
    if (result == nullptr)
        __debugbreak();
    return result;
}

// ea: 0x004BD520
void TlSystemCallbacks::MemFree(void* ptr)
{
    void* v1 = PakManager_sInst;
    if (PakManager_sInst == nullptr
        || PakManager_GetTopContext(PakManager_sInst) == PAK_ID_INVALID)
    {
        mem_heap_free(ptr);
    }
    else
    {
        int TopContext = PakManager_GetTopContext(v1);
        PakManager_MemFree(v1, TopContext, ptr, false);
    }
}

// ea: 0x004BD560
int TlSystemCallbacks::LinkFrame()
{
    return 0;
}

// ea: 0x004BD570
bool TlSystemCallbacks::LinkConnected()
{
    return false;
}

// ea: 0x004BD580
void TlSystemCallbacks::DebugPrint(char* txt)
{
    const char* v1 = txt;
    if (txt == nullptr || strncmp("NSL:", txt, 4) != 0)
    {
        if (AeAssert::gInAssert && txt != nullptr)
        {
            if (strncmp("RTCMD:O", txt, 7) == 0
                || (strncmp("RTCMD:C", txt, 7) != 0
                    && (strncmp("RTCMD:I", txt, 7) == 0
                        || strncmp("RTCMD:T", txt, 7) == 0)))
            {
                v1 = txt + 7;
            }
        }
        tlPrint(v1);
    }
}

// ea: 0x004BD850
void* SpinnerInit()
{
    tlFixedString FileName("spinner_a");
    sSpinnerFrames[0] = nglGetTexture(&FileName);
    dword_F00EB0 = sSpinnerFrames[0];
    tlFixedString v3("spinner_b");
    dword_F00EB4 = nglGetTexture(&v3);
    dword_F00EB8 = dword_F00EB4;
    tlFixedString v2("spinner_c");
    dword_F00EBC = nglGetTexture(&v2);
    dword_F00EC0 = dword_F00EBC;
    tlFixedString v1("spinner_d");
    void* result = nglGetTexture(&v1);
    dword_F00EC4 = result;
    dword_F00EC8 = result;
    return result;
}

// ea: 0x004BD8F0
void SpinnerReset()
{
    sLastSpinnerFrame = 0;
}

// ea: 0x004C9D70
void GlobalPakLoadCallback()
{
    static unsigned long long sLastTime = 0;
    unsigned long long v0 = sLastTime ? sLastTime : __rdtsc();
    unsigned long long v1 = __rdtsc();
    if (sLastTime == 0)
        sLastTime = v0;
    float v2 = (float)(v1 - v0);
    if (v2 >= 24444442.0f)
    {
        sLastTime = v1;
        PrintPakNames();
        SpinnerDrawFrameWithLoading(true);
    }
}
