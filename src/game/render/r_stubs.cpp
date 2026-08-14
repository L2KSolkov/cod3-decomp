// ============================================================================
// r_stubs.cpp - render.o world/render-command layer (tr_world.cpp, renderer)
// ============================================================================

#include "game/logic/g_local.h"
#include "game/render/xsurface.h"
#include "render/ShaderCommon.h"
#include "ngl/ngl_scene.h"
#include "ngl/ngl_lighting.h"

#include <string.h>
#include <intrin.h>

// Minimal IDA-verified views for render.o globals
struct BspTree;
struct world_t {
    void* mSky;      // +0x00
    BspTree* bspTree;  // +0x04
    world_t();       // ??0world_t@@QAE@XZ (render.o 0x6BECC0)
};

struct trGlobals_t {
    int registered;      // +0x00
    int frameCount;      // +0x04
    uint8_t _pad[0x40 - 0x08];
    world_t* world;      // +0x40
    struct DebugBlock {
        uint8_t _pad[0x80];
        void* externStrings;   // +0x80
        int numExternStrings;  // +0x84
        void* externLines;     // +0x88
        int numExternLines;    // +0x8C
    } debug;          // +0x40 region (offsets via disasm)
};
extern trGlobals_t tr;           // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0
extern world_t s_worldData;      // ?s_worldData@@3Uworld_t@@A @ 0xF74B98
extern BspTree* g_bspTree;       // ?g_bspTree@@3PAVBspTree@@A @ 0xF743DC
extern int sCurColor;            // ?sCurColor@@3IA @ 0xF74290
struct trDebugString_t;
struct trDebugLine_t;

// ServerTime (cg.o) - tick delta used by RE_BeginFrame
struct ServerTime_s {
    float mTickDelta;   // +0x00
};
extern ServerTime_s ServerTime_sInst;  // ?sInst@ServerTime@@0V1@A @ 0xDD8B0C

// ModelLightingHack statics (@ 0xF78350 / 0xF78330 / 0xF78364)
static math::Dir3 s_lightDir;       // dir
static math::Vector4 s_lightColor;  // color_0
static unsigned int s_lightInit;    // $S29_4

extern float gLightGridBounceBack;  // ?gLightGridBounceBack@@3MA @ 0xDFA418
extern void nglListAddDirLight(unsigned int LightCat, const math::Dir3& Dir,
                               const math::Vector4& Color);  // ngl.o

// XModelPartsManager / XModelPartsBank (opaque views)
class XModelPartsBank;
class XModelPartsManager {
public:
private:
    virtual void OnBankUnloaded(XModelPartsBank& xmpBank);  // ?OnBankUnloaded@XModelPartsManager@@EAEXAAVXModelPartsBank@@@Z
};

struct dpvs_plane_t {
    math::Vector4 data;   // +0x00
    int side[3];          // +0x10
};
extern int r_zfar_value;  // float value member of ?r_zfar@@3PAUcvar_t@@A
extern float g_dpvs_cullDist;  // g_dpvs.cullDist @ 0xF75600
struct drawSurf_s;        // tr renderer surface
struct trStatistics_t;    // tr renderer statistics

// ============================================================================
// world
// ============================================================================

// ea: 0x006BECC0
world_t::world_t()
{
    mSky = nullptr;
}

// ea: 0x006BECD0
void RE_LoadWorldMap(const char* name, int* checksum)
{
    (void)name;
    (void)checksum;
    tr.world = &s_worldData;
    s_worldData.bspTree = g_bspTree;
}

// ============================================================================
// render command layer
// ============================================================================

// ea: 0x006BED00
void R_PerformanceCounters()
{
}

// ea: 0x006BED10
void R_IssueRenderCommands(int frameCount)
{
    (void)frameCount;
}

// ea: 0x006BED20
void R_SyncRenderThread()
{
}

// ea: 0x006BED30
void R_AddDrawSurfCmd(drawSurf_s* drawSurf, int surfNum)
{
    (void)drawSurf;
    (void)surfNum;
}

// ea: 0x006BF020
void RE_SaveScreen()
{
}

// ea: 0x006BF030
void RE_TrackStatistics(trStatistics_t* statistics)
{
    (void)statistics;
}

// ea: 0x006BED40
void RE_SetColor(const float* rgba)
{
    if (rgba != nullptr)
    {
        unsigned int alpha = (unsigned int)(rgba[3] * 255.0f);
        unsigned int r = (unsigned int)(rgba[0] * 255.0f);
        unsigned int g = (unsigned int)(rgba[1] * 255.0f);
        unsigned int b = (unsigned int)(rgba[2] * 255.0f);
        sCurColor = (int)((alpha << 24) | (b << 16) | (g << 8) | r);
    }
    else
    {
        sCurColor = -1;
    }
}

// ea: 0x006BF060
void RE_LocateDebugStrings(trDebugString_t* strings, int numStrings)
{
    tr.debug.externStrings = strings;
    tr.debug.numExternStrings = numStrings;
}

// ea: 0x006BF080
void RE_LocateDebugLines(trDebugLine_t* lines, int numLines)
{
    tr.debug.externLines = lines;
    tr.debug.numExternLines = numLines;
}

// ea: 0x006BDC00
void XModelPartsManager::OnBankUnloaded(XModelPartsBank& xmpBank)
{
    (void)xmpBank;
}

// ea: 0x006BF330
void R_SetPlaneSidesDPVS(dpvs_plane_t* plane)
{
    plane->side[0] = plane->data.v.m128_f32[0] <= 0.0f ? 0 : 0xC;
    plane->side[1] = plane->data.v.m128_f32[1] <= 0.0f ? 4 : 16;
    plane->side[2] = plane->data.v.m128_f32[2] <= 0.0f ? 8 : 20;
    plane->data.v.m128_f32[3] =
        plane->data.v.m128_f32[3] - 0.0049999999f;
}

// ea: 0x006BF380
float RE_GetFarPlaneDist()
{
    float zfar = *(float*)&r_zfar_value;
    if (g_dpvs_cullDist > zfar)
        return g_dpvs_cullDist;
    return zfar;
}

// ea: 0x006BEF40
void RE_BeginFrame()
{
    if (tr.registered != 0)
    {
        TimerRenderBars::sInst.TimeGameAdvanceEnd();
        ++tr.frameCount;
        ShaderCommon::SetupFrame(ServerTime_sInst.mTickDelta);
        nglSetClearFlags(3u);
        math::Vector4 FarFogColor;
        ShaderCommon::GetFarFogColor(&FarFogColor);
        float v3 = FarFogColor.v.m128_f32[2];
        float vG = FarFogColor.v.m128_f32[1];
        float vR = FarFogColor.v.m128_f32[0];
        nglSetClearColor(vR, vG, v3, 0.0f);
        nglSetClearZ(1.0f);
        nglSetView(-1.0f, -1.0f, 1.0f, 1.0f);
        nglSetScissor(-1.0f, -1.0f, 1.0f, 1.0f);
    }
}

// ea: 0x006BEB80
void ModelLightingHack()
{
    unsigned int v1 = s_lightInit;
    if ((s_lightInit & 1) == 0)
    {
        s_lightInit |= 1u;
        s_lightDir.v.m128_f32[0] = 0.0f;
        s_lightDir.v.m128_f32[1] = -0.70710999f;
        s_lightDir.v.m128_f32[2] = -0.70710999f;
        s_lightDir.v.m128_f32[3] = 0.0f;
    }
    math::Vector4 v;
    if ((v1 & 2) != 0)
    {
        v = s_lightColor;
    }
    else
    {
        s_lightColor.v.m128_f32[0] = 1.07854f;
        s_lightColor.v.m128_f32[1] = 0.99822003f;
        s_lightColor.v.m128_f32[2] = 0.80317003f;
        s_lightColor.v.m128_f32[3] = 0.0f;
        s_lightInit = v1 | 2;
        v = s_lightColor;
    }
    math::Vector4 scaled;
    scaled.v.m128_f32[0] = v.v.m128_f32[0] * 0.50265598f;
    scaled.v.m128_f32[1] = v.v.m128_f32[1] * 0.50265598f;
    scaled.v.m128_f32[2] = v.v.m128_f32[2] * 0.50265598f;
    scaled.v.m128_f32[3] = v.v.m128_f32[3] * 0.50265598f;
    nglListAddDirLight(0xFFFFFFFFu, s_lightDir, scaled);
    nglSetAmbientLight(s_lightColor.v.m128_f32[0] * gLightGridBounceBack,
                       s_lightColor.v.m128_f32[1] * gLightGridBounceBack,
                       s_lightColor.v.m128_f32[2] * gLightGridBounceBack);
}

// ============================================================================
// TimerRenderBars (render.o - TimerRenderBars.cpp)
// ============================================================================

// ea: 0x006E6BF0
void TimerRenderBars::TimeGameAdvanceEnd()
{
    mFrameAdvance.mEnd = __rdtsc();
}

// ea: 0x006E6C30
float TimerRenderBars::CalcRenderTimeScale() const
{
    return mVSyncLength / mRenderTimersScale;
}

// ea: 0x006E6C40
TimerRenderBars::TimedInterval::TimedInterval()
{
    mEnd = 0;
    mBegin = 0;
    mLastEnd = 0;
    mLastBegin = 0;
}

// ea: 0x006E6C70
unsigned __int64 TimerRenderBars::TimedInterval::LastElapsed() const
{
    return mLastEnd - mLastBegin;
}

// ea: 0x006E6DB0
void TimerRenderBars::TimedInterval::Next()
{
    mLastBegin = mBegin;
    mLastEnd = mEnd;
}
