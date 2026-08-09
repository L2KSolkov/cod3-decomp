// ============================================================================
// xbox_main.cpp - Xbox console entry point (game_xbox.o _main)
// Source: c:\cod\code\game\xbox_main.cpp
// Verified against IDA (release map offset 0x0031AF10 + 0x40C000 = 0x726F10).
// ============================================================================

#include <string.h>
#include <new>
#include <intrin.h>

#include "game/cvar_types.h"
#include "game/platform_xbox/MPLiveEngine.h"
#include "aeps/apsCommon.h"
#include "render/ShaderCommon.h"
#include "ngl/ngl_dx_buf.h"
#include "ngl/ngl_dx_core.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_scene.h"
#include "xbox_shim.h"

// ============================================================================
// AnimHeap / nalHeap - minimal views (full types in core_systems.h, which
// cannot be included alongside g_local.h). The ctor/nalInit symbols come from
// ctor_dtor.cpp / nal.cpp.
// ============================================================================
class nalHeap {
    virtual ~nalHeap();  // __vftable at +0x00
};

struct AnimHeap : nalHeap {
    void*       mBlock;          // +0x04
    unsigned char mHeap[0x49C];  // +0x08

    AnimHeap();   // ea: 0x004C1380
    ~AnimHeap();  // ea: 0x004BD610
};
static_assert(sizeof(AnimHeap) == 0x4A4, "AnimHeap size mismatch");

extern void nalInit(nalHeap*);  // ?nalInit@@YAXPAVnalHeap@@@Z

// ============================================================================
// TimerRenderBars - layout twin (full type in game/logic/g_local.h, which
// cannot be included alongside MPLiveEngine.h's SaveGameData).
// ============================================================================
struct TimerRenderBars {
    uint8_t      _pad[0x10];
    unsigned int mTimeLo;  // +0x10 (rdtsc low at TimeGameAdvanceBegin)
    unsigned int mTimeHi;  // +0x14 (rdtsc high)
    uint8_t      _pad2[0x48 - 0x18];
    int mActive;   // +0x48
    static TimerRenderBars sInst;  // ?sInst@TimerRenderBars@@0V1@A (render.o)
    void TimeGameAdvanceBegin() {  // ea: 0x72A9D0 (inline)
        unsigned __int64 t = __rdtsc();
        mTimeLo = (unsigned int)t;
        mTimeHi = (unsigned int)(t >> 32);
    }
};

// ============================================================================
// apsClient / ApsGameClient - layout twins (full types in aeps/apsInternal.h,
// which pulls apsParam.h and cannot coexist with windows.h's INT32 typedef).
// ============================================================================
class apsClient {
public:
    virtual ~apsClient() = 0;  // ??1apsClient@@UAE@XZ (render.o)
};

class ApsGameClient : public apsClient {
public:
    static ApsGameClient m_client;  // ?m_client@ApsGameClient@@0V1@A (render.o)
};

// ============================================================================
// Cross-object externs (already ported / still stubbed via /FORCE:UNRESOLVED)
// ============================================================================
extern bool gSkipMovies;     // GameXbox.cpp (ea 0x71EF50 scope)
extern bool gSkipFrontEnd;   // GameXbox.cpp
extern void CheckCommands(const char* text);  // GameXbox.cpp (ea 0x71EF50)
extern void Cvar_Set(const char* var_name, const char* value);
extern void Cvar_SetValue(const char* var_name, float value);
extern void tlPrintf(const char* fmt, ...);   // tl_system.cpp

char* Xbox_LaunchInfo(char* pDestCommandLine);  // XboxLiveMenus.cpp
void RenderUIX(void*);                           // XboxLiveMenus.cpp

extern vmCvar_t cg_widescreen;  // ?cg_widescreen@@3UvmCvar_t@@A (0xF5CC88)

namespace BrocSys {
void Init();  // ?Init@BrocSys@@YAXXZ (scr.o)
}

// ngl / aps / game2 / streamer / render externs
extern void nglInit();                                    // ngl_internal.o
extern void InitDefaultPak();                             // game2.o
extern void InitCDAepsShader();                           // render.o
extern void apsInitParticleMemory(int memSize, bool bBigBuffers);  // render.o
extern void StartupNfl(const char* mountPoint);           // streamer.o
extern void IN_Frame();                                   // game2.o
extern void* SpinnerInit();                               // sys.cpp
extern void SpinnerDrawFrameWithLoading(bool bEndFrame);  // spinner_lens.cpp
extern cvar_t* Com_Frame();                               // common.cpp
extern void Com_Init(char* commandLine);                  // common.cpp

// CTitleFontRenderer vtable (rdata 0xD181DC) + XFONT loader
extern void* CTitleFontRenderer_vftable;
void XFONT_OpenTrueTypeFont(const unsigned short* pszFontFileName,
                            unsigned int uBytes, void* pFontInfo);

// mem_heap (mem_lib.o; binary calls this with MEM_HEAP_MAIN == 0)
enum mem_heap_type {
    MEM_HEAP_DEFAULT = 0,
    MEM_HEAP_DEBUG   = 1,
    MEM_HEAP_COMBINE = 2,
};
struct mem_heap;
extern mem_heap* mem_heap_set_current(mem_heap_type type);

// controller - minimal view (input/controller.cpp owns inst(); locked_port
// matches the ported layout used by XboxLiveMenus.cpp).
struct controller {
    int locked_port;
    static controller* inst();
};

// ============================================================================
// Sys_Milliseconds (game.o, 0x60EDA0) - rdtsc-derived millisecond clock.
// Ported here because the Win32 link requires the definition now.
// ============================================================================
static bool first_time = true;   // ?first_time (0xDF8C84)
static unsigned __int64 initial_time;  // ?initial_time (0xF588A0)

// ea: 0x60EDA0
int Sys_Milliseconds()
{
    unsigned int v0;
    unsigned int v1;
    unsigned __int64 v3;

    if (first_time)
    {
        v3 = __rdtsc();
        v0 = (unsigned int)(v3 >> 32);
        v1 = (unsigned int)v3;
        initial_time = v3;
        first_time = false;
    }
    else
    {
        v0 = (unsigned int)(initial_time >> 32);
        v1 = (unsigned int)initial_time;
    }
    return (int)((__rdtsc() - (((unsigned __int64)v0 << 32) | v1))
                 * 0.0000013636364);
}

// ============================================================================
// _main - Xbox console entry point (0x726F10)
// ============================================================================
// ea: 0x726F10
void main()
{
    // j_nullsub_62();  (linker thunk to a nullsub - no-op)
    MPLiveEngine liveWrapper;
    char cmdLineText[256];
    unsigned int pal_modes[2];
    unsigned int ntsc_modes[2];

    Xbox_LaunchInfo(cmdLineText);
    CheckCommands(cmdLineText);
    gSkipMovies = true;
    if (strstr(cmdLineText, "+devmap"))
    {
        gSkipMovies = true;
        gSkipFrontEnd = true;
    }
    mem_heap_set_current(MEM_HEAP_DEFAULT);  // binary: MEM_HEAP_MAIN (=0)
    StartupNfl(nullptr);
    Sys_Milliseconds();
    apsCommon::InitShaders();
    InitCDAepsShader();
    ShaderCommon::InitShaders();
    nglSetBufferSize(NGLBUF_LIST_WORK, 0x100000, true);
    nglSetBufferSize(NGLBUF_SCRATCH_VERTEX, 0x80000, true);
    nglSetBufferSize(NGLBUF_SCRATCH_INDEX, 0x10000, true);
    unsigned int v1 = XGetVideoStandard();
    pal_modes[0] = 3;
    pal_modes[1] = 1;
    ntsc_modes[0] = 3;
    ntsc_modes[1] = 1;
    if (v1 == 3)
        nglSetDisplayMode(pal_modes, 2);
    else
        nglSetDisplayMode(ntsc_modes, 2);
    if (nglDisplayMode.Widescreen)
    {
        cg_widescreen.integer = 1;
        Cvar_Set("cg_widescreen", "1");
        Cvar_SetValue("cg_widescreen", 1.0f);
    }
    else
    {
        cg_widescreen.integer = 0;
        Cvar_Set("cg_widescreen", "0");
        Cvar_SetValue("cg_widescreen", 0.0f);
    }
    nglInit();
    InitDefaultPak();
    SpinnerInit();
    SpinnerDrawFrameWithLoading(true);
    ShaderCommon::HeatHazeInit();
    ShaderCommon::GlowInit();
    bool bBigBuffers = strstr(cmdLineText, "+big_hairy_aeps") != nullptr;
    bool v2 = strstr(cmdLineText, "+aeps_debug") != nullptr;
    apsCommon::Init(v2 ? 1 : 0, 0x2000000, 0xFA0);
    apsCommon::SetClient(&ApsGameClient::m_client);
    apsInitParticleMemory(0x80000, bBigBuffers);
    void* v3 = mem_heap_malloc(0x4A4);
    AnimHeap* v4 = nullptr;
    if (v3 != nullptr)
        v4 = new (v3) AnimHeap;
    nalInit(v4);
    BrocSys::Init();
    Com_Init(cmdLineText);
    void* renderDevice = nglDev;
    void* font = nullptr;
    void* v6 = mem_heap_malloc(8);
    if (v6 != nullptr)
    {
        *(void**)((char*)v6 + 4) = nullptr;
        *(void**)v6 = (void*)&CTitleFontRenderer_vftable;
        font = v6;
    }
    XFONT_OpenTrueTypeFont((const unsigned short*)L"d:\\media\\GARA.TTF",
                           0x10000, (char*)font + 4);
    liveWrapper.SetupAsSession(renderDevice, "d:\\media\\UIXLayout.uix", font);

    controller* inst = controller::inst();
    liveWrapper.logonMethod = gSaveGameData[inst->locked_port].loginMethod;
    tlPrintf("!! Checking sign-in state. Now live state is: %d !!\n",
             gSaveGameData[controller::inst()->locked_port].liveState);
    if (gSaveGameData[controller::inst()->locked_port].liveState == 2)
    {
        tlPrintf("!! retrieving logon state!!\n");
        controller* v8 = controller::inst();
        liveWrapper.RetrieveLogonState(
            &gSaveGameData[v8->locked_port].savedState, 0x40140);
        int locked_port = controller::inst()->locked_port;
        controller* v10 = controller::inst();
        liveWrapper.SetNotificationFlag(
            gSaveGameData[v10->locked_port].mControllerPort, 1,
            gSaveGameData[locked_port].appearOnline);
    }
    else if (gSaveGameData[controller::inst()->locked_port].liveState == 1)
    {
        if (gSaveGameData[controller::inst()->locked_port].savedStateIsValid)
        {
            tlPrintf("!! retrieving logon state!!\n");
            controller* v11 = controller::inst();
            liveWrapper.RetrieveLogonState(
                &gSaveGameData[v11->locked_port].savedState, 0x40140);
        }
        else
        {
            tlPrintf("!! signing-in silently !!\n");
            liveWrapper.SignInSilently(0x40140);
        }
    }
    else
    {
        tlPrintf("!! state is kNotSignedIn !!\n");
        LiveWrapper::theWrapper->lastLoginCode =
            gSaveGameData[controller::inst()->locked_port].lastLoginCode;
    }
    for (;;)
    {
        TimerRenderBars::sInst.TimeGameAdvanceBegin();
        IN_Frame();
        liveWrapper.DoWork();
        nglSetEndOfFrameCallback(RenderUIX, nullptr);
        Com_Frame();
    }
}
