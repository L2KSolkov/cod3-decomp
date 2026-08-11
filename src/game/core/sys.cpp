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

// Minimal view of PakManager (full class in game/sv/sv_stubs.h).
class PakManager {
public:
    static PakManager* sInst;
    TPakId GetTopContext() const;
    void* MemAlign(TPakId id, unsigned int align, unsigned int size);
};  // ?sInst@PakManager@@2PAV1@A


extern void Com_Printf(const char* fmt, ...);
enum print_msg_type_t;
extern void CL_ConsolePrint(print_msg_type_t type, const char* txt,
                            int duration, int linewidth, int flags);
extern cvar_t* cl_noprint;
extern void tlPrintf(const char* fmt, ...);
extern void tlPrint(const char* txt);
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void* mem_heap_realloc(void* ptr, unsigned int size);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);
extern void* mem_heap_malloc_align_heap(void* heap, unsigned int alignment,
                                       unsigned int size);
enum mem_heap_type {
    MEM_HEAP_MAIN    = 0,
    MEM_HEAP_DEBUG   = 1,
    MEM_HEAP_COMBINE = 2,
    MEM_HEAP_NONE    = 3,
};
struct mem_heap;
extern mem_heap* mem_heap_get(mem_heap_type heap_name);
struct nglTexture;
struct tlSystemCallbacks;
extern void tlSetSystemCallbacks(const tlSystemCallbacks* callbacks);
struct nglQuad;
extern void nglInitQuad(nglQuad* quad);
extern void nglSetClearFlags(unsigned int clearFlags);
extern void nglPresent();
extern void nglWaitForRendering();
extern nglTexture* nglGetFrontBufferTex();
extern void nglSetQuadTex(nglQuad* quad, nglTexture* tex);
extern void nglSetQuadRect(nglQuad* quad, float x1, float y1, float x2,
                           float y2);
extern void nglListAddQuad(nglQuad* quad);
extern nglTexture* nglGetTexture(const tlFixedString& fileName);
extern void PrintPakNames();
extern void SpinnerDrawFrameWithLoading(bool bEndFrame);
extern int gLensAlphaAmount;
extern int PakManager_GetTopContext(void* self);
extern void* PakManager_MemAlign(void* self, int id, unsigned int align,
                                 unsigned int size);
extern void PakManager_MemFree(void* self, int id, void* ptr,
                               bool bUseActorHeap);
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

namespace AeStringSupport {
extern void CStrToAeStr(char* oBuff, int* oLen, int capacity,
                        const char* src);
extern void SubStr(char* oBuff, int* oLen, const char* src, int begin,
                   int len, int srcCapacity);
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
        CL_ConsolePrint((print_msg_type_t)0, txtBuf, 0, 0, 0);
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
    nglInitQuad((nglQuad*)frontBuf);
    nglSetClearFlags(0);
    nglPresent();
    nglWaitForRendering();
    nglTexture* FrontBufferTex = nglGetFrontBufferTex();
    nglSetQuadTex((nglQuad*)frontBuf, FrontBufferTex);
    nglSetQuadRect((nglQuad*)frontBuf, 0.0f, 0.0f, 640.0f, 480.0f);
    nglSetClearFlags(0xF3u);
    nglListAddQuad((nglQuad*)frontBuf);
    nglPresent();
    nglWaitForRendering();
}

// ea: 0x004BBF50
void* MT_AllocAnimTree(unsigned int size)
{
    return mem_heap_malloc_ctx(size, 16, "hunk",
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
        && PakManager::sInst != nullptr)
    {
        int TopContext = PakManager::sInst->GetTopContext();
        return PakManager::sInst->MemAlign((TPakId)TopContext, align, size);
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
        result = mem_heap_malloc_ctx(size, align, nullptr, nullptr, 0);
    }
    if (result == nullptr)
        __debugbreak();
    return result;
}

// ea: 0x004BD520
void TlSystemCallbacks::MemFree(void* ptr)
{
    void* v1 = PakManager::sInst;
    if (PakManager::sInst == nullptr
        || PakManager::sInst->GetTopContext() == PAK_ID_INVALID)
    {
        mem_heap_free(ptr);
    }
    else
    {
        int TopContext = ((PakManager*)v1)->GetTopContext();
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
    sSpinnerFrames[0] = nglGetTexture(FileName);
    dword_F00EB0 = sSpinnerFrames[0];
    tlFixedString v3("spinner_b");
    dword_F00EB4 = nglGetTexture(v3);
    dword_F00EB8 = dword_F00EB4;
    tlFixedString v2("spinner_c");
    dword_F00EBC = nglGetTexture(v2);
    dword_F00EC0 = dword_F00EBC;
    tlFixedString v1("spinner_d");
    void* result = nglGetTexture(v1);
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
void GlobalPakLoadCallback(float progress)
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

// ============================================================================
// TlSystemCallbacks callback installers + tl assert routing
// ============================================================================

static int hackLine;
static const char defaultFileName[] = "";

// ea: 0x004BD400
TlSystemCallbacks::TlMemAllocCbfn TlSystemCallbacks::SetMemAllocCbfn(
    TlSystemCallbacks::TlMemAllocCbfn cbfn)
{
    TlMemAllocCbfn old = mTlCallbacks.MemAlloc;
    mTlCallbacks.MemAlloc = cbfn;
    tlSetSystemCallbacks(reinterpret_cast<const tlSystemCallbacks*>(this));
    return old;
}

// ea: 0x004BD420
TlSystemCallbacks::TlMemFreeCbfn TlSystemCallbacks::SetMemFreeCbfn(
    TlSystemCallbacks::TlMemFreeCbfn cbfn)
{
    TlMemFreeCbfn old = mTlCallbacks.MemFree;
    mTlCallbacks.MemFree = cbfn;
    tlSetSystemCallbacks(reinterpret_cast<const tlSystemCallbacks*>(this));
    return old;
}

// ea: 0x004CFA60
void TlSystemCallbacks::CriticalError(const char* txt)
{
    const char* v1 = txt;
    if (txt == nullptr || strncmp("NSL:", txt, 4) != 0)
    {
        ae_fixed_string<256, unsigned short> aeAssertText;
        ae_fixed_string<256, unsigned short> aeAssertExp;
        ae_fixed_string<256, unsigned short> aeAssertFile;
        int assertLine = 0;
        if (!IgnoreAssertion(v1, &aeAssertText, &aeAssertExp, &aeAssertFile,
                             &assertLine)
            && AeAssert::Assert((const char*)aeAssertText.mBuff))
        {
            __debugbreak();
        }
    }
}

// ea: 0x004CFAF0
void TlSystemCallbacks::Warning(const char* txt)
{
    const char* v1 = txt;
    if (txt == nullptr || strncmp("NSL:", txt, 4) != 0)
    {
        if (sWarningsEnabled
            && strstr(v1, "Invalid scale detected in local to world transform")
                   == nullptr)
        {
            ae_fixed_string<256, unsigned short> aeAssertText;
            ae_fixed_string<256, unsigned short> aeAssertExp;
            ae_fixed_string<256, unsigned short> aeAssertFile;
            int assertLine = 0;
            if (!IgnoreAssertion(v1, &aeAssertText, &aeAssertExp,
                                 &aeAssertFile, &assertLine)
                && AeAssert::Warning((const char*)aeAssertText.mBuff))
            {
                __debugbreak();
            }
        }
    }
}

// ea: 0x004CE800
bool TlSystemCallbacks::IgnoreAssertion(
    const char* tlAssertText,
    ae_fixed_string<256, unsigned short>* assertText,
    ae_fixed_string<256, unsigned short>* assertExp,
    ae_fixed_string<256, unsigned short>* assertFile,
    int* assertLine)
{
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    if (tlAssertText != nullptr && *tlAssertText != 0)
    {
        if (ParseTlAssertString(tlAssertText, assertText, assertExp, assertFile,
                                assertLine))
        {
            AeAssert::gCurrentFile = (const char*)assertFile->mBuff;
            AeAssert::gCurrentLine = *assertLine;
            AeAssert::gCurrentExpr = (const char*)assertExp->mBuff;
            return AeAssert::IsIgnored();
        }
        else
        {
            ae_fixed_string<256, unsigned short> v7(tlAssertText);
            *assertText = v7;
            AeAssert::gCurrentLine = hackLine;
            hackLine = hackLine + 1;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TlSysCallbacks.cpp";
            AeAssert::gCurrentExpr = defaultFileName;
            return false;
        }
    }
    else
    {
        tlPrintf("Assertion text is invalid");
        __debugbreak();
        return true;
    }
}

// ea: 0x004C5AA0
bool TlSystemCallbacks::ParseTlAssertString(
    const char* tlAssertText,
    ae_fixed_string<256, unsigned short>* assertMessage,
    ae_fixed_string<256, unsigned short>* assertExpression,
    ae_fixed_string<256, unsigned short>* fileName,
    int* line)
{
    ae_fixed_string<256, unsigned short> tlAssertStr;
    int oLen;
    AeStringSupport::CStrToAeStr((char*)tlAssertStr.mBuff, &oLen, 254,
                                 tlAssertText);
    tlAssertStr.mLength = (unsigned char)oLen;
    if (oLen == 0)
        return false;

    const char* tlAssertStrBuf = (const char*)tlAssertStr.mBuff;
    int v5 = 0;
    while (tlAssertStrBuf[v5] != '(')
    {
        if (++v5 >= oLen)
            return false;
    }

    AeStringSupport::SubStr((char*)fileName->mBuff, &oLen, tlAssertStrBuf, 10,
                            v5 - 10, 254);
    fileName->mLength = (unsigned char)oLen;
    if (oLen == 0)
        return false;

    int v8 = 0;
    while (tlAssertStrBuf[v8] != ')')
    {
        if (++v8 >= tlAssertStr.mLength)
            return false;
    }

    ae_fixed_string<256, unsigned short> lineStr;
    tlAssertStr.substr(lineStr, v5 + 1, v8 - v5 - 1);
    if (lineStr.mLength == 0)
        return false;

    sscanf((const char*)lineStr.mBuff, "%d", line);

    int v9 = tlAssertStr.find('"', v8);
    if (v9 < 0)
        return false;
    int v10 = v9;
    int v11 = v9 + 1;
    int v12 = tlAssertStr.find('"', v9 + 1);
    if (v12 < 0)
        return false;
    int v13 = v12;

    tlAssertStr.substr(*assertExpression, v11, v12 - v10 - 1);
    if (assertExpression->mLength == 0)
        return false;

    tlAssertStr.substr(*assertMessage, v13 + 4,
                       tlAssertStr.mLength - v13 - 4);
    if (assertMessage->mLength == 0)
    {
        ae_fixed_string<256, unsigned short> v14(" ");
        *assertMessage = v14;
    }
    return true;
}
