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

// IDA-verified views for render.o globals
struct BspTree;
int sCurColor = -1;
float sGlobalFontScale = 1.0f;
unsigned char* fdFile = nullptr;
// world_t (IDA type; size 0x10C)
struct world_t {
    char name[128];     // +0x00
    char baseName[128]; // +0x80
    BspTree* bspTree;   // +0x100
    char* entityString; // +0x104
    void* mSky;         // +0x108
    world_t();          // ??0world_t@@QAE@XZ (render.o 0x6BECC0)
};
static_assert(sizeof(world_t) == 0x10C, "world_t size mismatch");

struct trDebugString_t;
struct trDebugLine_t;
struct trDebugPoly_t;
struct trDebugPlume_t;
struct trDebugVert_t;
struct fontInfo_t;
// trDebug_t (IDA type; size 0x80)
struct trDebug_t {
    int maxVerts;            // +0x00
    int numVerts;            // +0x04
    float (*verts)[3];       // +0x08
    int maxPolys;            // +0x0C
    int numPolys;            // +0x10
    trDebugPoly_t* polys;    // +0x14
    int maxStrings;          // +0x18
    int numStrings;          // +0x1C
    trDebugString_t* strings;// +0x20
    fontInfo_t* font;        // +0x24
    int numExternStrings;    // +0x28
    trDebugString_t* externStrings;  // +0x2C
    int maxLines;            // +0x30
    int numLines;            // +0x34
    trDebugLine_t* lines;    // +0x38
    int numExternLines;      // +0x3C
    trDebugLine_t* externLines;      // +0x40
    int numPlumes;           // +0x44
    int maxPlumes;           // +0x48
    trDebugPlume_t* plumes;  // +0x4C
    int bInImmediateMode;    // +0x50
    int mode;                // +0x54
    float lineWidth;         // +0x58
    float st[2];             // +0x5C
    unsigned char rgba[4];   // +0x64
    float normal[3];         // +0x68
    int numDrawVerts;        // +0x74
    int maxDrawVerts;        // +0x78
    trDebugVert_t* dv;       // +0x7C
};
static_assert(sizeof(trDebug_t) == 0x80, "trDebug_t size mismatch");

// orientationr_t (IDA type; size 0x7C; tr.or at tr+0x1F0)
struct orientationr_t {
    float origin[3];       // +0x00
    float axis[3][3];      // +0x0C
    float viewOrigin[3];   // +0x30
    float modelMatrix[16]; // +0x3C
};
static_assert(sizeof(orientationr_t) == 0x7C, "orientationr_t size mismatch");

// trGlobals_t (IDA type; size 0x3A0)
struct __declspec(align(16)) trGlobals_t {
    int registered;          // +0x00
    int worldMapLoaded;      // +0x04
    int frameCount;          // +0x08
    int viewCount;           // +0x0C
    uint8_t _pad0[0x1F0 - 0x10];  // viewParms
    orientationr_t orr;      // +0x1F0 (tr.or)
    uint8_t _pad2[0x290 - 0x26C];  // refdef
    world_t* world;          // +0x290
    uint8_t _pad1[0x2A0 - 0x294];
    uint8_t viewModelInfo[0x70];  // +0x2A0 (viewModelInfo_t; see tr_gl.cpp)
    int viewModelInfoIndex;  // +0x310
    trDebug_t debug;         // +0x314
};
static_assert(sizeof(trGlobals_t) == 0x3A0, "trGlobals_t size mismatch");
extern trGlobals_t tr;           // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0
extern world_t s_worldData;      // ?s_worldData@@3Uworld_t@@A @ 0xF74B98
extern BspTree* g_bspTree;       // ?g_bspTree@@3PAVBspTree@@A @ 0xF743DC
extern int sCurColor;            // ?sCurColor@@3IA @ 0xF74290

// ServerTime (cg.o) - tick delta used by RE_BeginFrame
struct ServerTime_s {
    float mTickDelta;   // +0x00
};
extern ServerTime_s ServerTime_sInst;  // ?sInst@ServerTime@@0V1@A @ 0xDD8B0C

// ModelLightingHack statics (@ 0xF78350 / 0xF78330 / 0xF78364)
static math::Dir3 s_lightDir;       // dir
static math::Vector4 s_lightColor;  // color_0
static unsigned int s_lightInit;    // $S29_4

float gLightGridBounceBack = 0.3f;  // ?gLightGridBounceBack@@3MA @ 0xDFA418
extern void nglListAddDirLight(unsigned int LightCat, const math::Dir3& Dir,
                               const math::Vector4& Color);  // ngl.o

// XModelPartsManager / XModelPartsBank (opaque views)
class XModelPartsBank;
class XModelPartsManager {
public:
private:
    virtual void OnBankUnloaded(XModelPartsBank& xmpBank);  // ?OnBankUnloaded@XModelPartsManager@@EAEXAAVXModelPartsBank@@@Z
public:
    void PostProcess(XModelPartsBank* xmpBank, TPakId pak_id);  // ?PostProcess@XModelPartsManager@@AAEXPAVXModelPartsBank@@W4TPakId@@@Z
};

struct dpvs_plane_t {
    math::Vector4 data;   // +0x00
    unsigned char side[3]; // +0x10
    unsigned char frontal; // +0x13
};
extern cvar_t* r_zfar;     // ?r_zfar@@3PAUcvar_t@@A
extern "C" struct dpvs_t g_dpvs;  // plain C symbol @ 0xF75600
struct drawSurf_s;        // tr renderer surface
struct trStatistics_t;    // tr renderer statistics

// dpvs_t view (cullDist +0xD8; full layout in tr_dpvs.cpp)
struct dpvs_t {
    uint8_t _pad[0xD8];
    float cullDist;        // +0xD8
};

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
    float zfar = r_zfar->value;
    if (g_dpvs.cullDist > zfar)
        return g_dpvs.cullDist;
    return zfar;
}

// ea: 0x006C0A10
float R_UpdateOverTime(float fCurrent, float fGoal, int iFadeInTime,
                       int iFadeOutTime, int frametime)
{
    if (fGoal > fCurrent)
    {
        if (iFadeInTime > 0)
        {
            float v = fCurrent + ((float)frametime / (float)iFadeInTime);
            fCurrent = v;
            if (v > fGoal)
                return fGoal;
            return fCurrent;
        }
        return fGoal;
    }
    if (fCurrent > fGoal)
    {
        if (iFadeOutTime <= 0)
            return fGoal;
        fCurrent = fCurrent - ((float)frametime / (float)iFadeOutTime);
        if (fGoal > fCurrent)
            return fGoal;
    }
    return fCurrent;
}

// ea: 0x006C0A80
void R_LocalNormalToWorld(float* local, float* world)
{
    *world = ((*local * tr.orr.axis[0][0]) + (tr.orr.axis[2][0] * local[2]))
           + (tr.orr.axis[1][0] * local[1]);
    world[1] = ((*local * tr.orr.axis[0][1]) + (tr.orr.axis[2][1] * local[2]))
             + (tr.orr.axis[1][1] * local[1]);
    world[2] = ((*local * tr.orr.axis[0][2]) + (tr.orr.axis[2][2] * local[2]))
             + (tr.orr.axis[1][2] * local[1]);
}

// ea: 0x006C0B30
void R_LocalPointToWorld(float* local, float* world)
{
    *world = (((*local * tr.orr.axis[0][0]) + (tr.orr.axis[2][0] * local[2]))
              + (tr.orr.axis[1][0] * local[1]))
           + tr.orr.origin[0];
    world[1] = (((*local * tr.orr.axis[0][1]) + (tr.orr.axis[2][1] * local[2]))
                + (tr.orr.axis[1][1] * local[1]))
             + tr.orr.origin[1];
    world[2] = (((*local * tr.orr.axis[0][2]) + (tr.orr.axis[2][2] * local[2]))
                + (tr.orr.axis[1][2] * local[1]))
             + tr.orr.origin[2];
}

// ea: 0x006C0BF0
void R_TransformModelToClip(const float* const src, const float* modelMatrix,
                            const float* projectionMatrix,
                            float* const eye, float* const dst)
{
    *eye = (((modelMatrix[4] * src[1]) + (modelMatrix[8] * src[2]))
            + (*src * *modelMatrix)) + modelMatrix[12];
    eye[1] = (((modelMatrix[5] * src[1]) + (modelMatrix[1] * *src))
              + (modelMatrix[9] * src[2])) + modelMatrix[13];
    eye[2] = (((modelMatrix[6] * src[1]) + (modelMatrix[2] * *src))
              + (modelMatrix[10] * src[2])) + modelMatrix[14];
    eye[3] = (((modelMatrix[7] * src[1]) + (modelMatrix[3] * *src))
              + (modelMatrix[11] * src[2])) + modelMatrix[15];
    *dst = (((projectionMatrix[12] * eye[3]) + (projectionMatrix[8] * eye[2]))
            + (projectionMatrix[4] * eye[1])) + (*projectionMatrix * *eye);
    dst[1] = (((projectionMatrix[13] * eye[3]) + (projectionMatrix[1] * *eye))
              + (projectionMatrix[9] * eye[2])) + (projectionMatrix[5] * eye[1]);
    dst[2] = (((projectionMatrix[14] * eye[3]) + (projectionMatrix[2] * *eye))
              + (projectionMatrix[10] * eye[2])) + (projectionMatrix[6] * eye[1]);
    dst[3] = (((projectionMatrix[15] * eye[3]) + (projectionMatrix[3] * *eye))
              + (projectionMatrix[11] * eye[2])) + (projectionMatrix[7] * eye[1]);
}

// ea: 0x006C0DA0
void R_TransformHomogenousModelToClip(const float* const src,
                                      const float* modelMatrix,
                                      const float* projectionMatrix,
                                      float* const eye, float* const dst)
{
    *eye = (((modelMatrix[8] * src[2]) + (modelMatrix[4] * src[1]))
            + (modelMatrix[12] * src[3])) + (*src * *modelMatrix);
    eye[1] = (((modelMatrix[9] * src[2]) + (modelMatrix[5] * src[1]))
              + (modelMatrix[1] * *src)) + (modelMatrix[13] * src[3]);
    eye[2] = (((modelMatrix[10] * src[2]) + (modelMatrix[6] * src[1]))
              + (modelMatrix[2] * *src)) + (modelMatrix[14] * src[3]);
    eye[3] = (((modelMatrix[11] * src[2]) + (modelMatrix[7] * src[1]))
              + (modelMatrix[3] * *src)) + (modelMatrix[15] * src[3]);
    *dst = (((projectionMatrix[8] * eye[2]) + (projectionMatrix[4] * eye[1]))
            + (projectionMatrix[12] * eye[3])) + (*projectionMatrix * *eye);
    dst[1] = (((projectionMatrix[9] * eye[2]) + (projectionMatrix[5] * eye[1]))
              + (projectionMatrix[1] * *eye)) + (projectionMatrix[13] * eye[3]);
    dst[2] = (((projectionMatrix[10] * eye[2]) + (projectionMatrix[6] * eye[1]))
              + (projectionMatrix[2] * *eye)) + (projectionMatrix[14] * eye[3]);
    dst[3] = (((projectionMatrix[11] * eye[2]) + (projectionMatrix[7] * eye[1]))
              + (projectionMatrix[3] * *eye)) + (projectionMatrix[15] * eye[3]);
}

// ea: 0x006C0F70
void myGlMultMatrix(const float* a, const float* b, float* out)
{
    const float* v3 = a + 2;
    float* v4 = out + 2;
    for (int i = 4; i != 0; --i)
    {
        *(v4 - 2) = (((*v3 * b[8]) + (b[12] * v3[1])) + (b[4] * *(v3 - 1)))
                  + (*(v3 - 2) * *b);
        *(v4 - 1) = (((v3[1] * b[13]) + (*(v3 - 1) * b[5])) + (b[9] * *v3))
                  + (*(v3 - 2) * b[1]);
        *v4 = (((v3[1] * b[14]) + (*(v3 - 1) * b[6])) + (*(v3 - 2) * b[2]))
            + (b[10] * *v3);
        v4[1] = (((b[11] * *v3) + (v3[1] * b[15])) + (*(v3 - 1) * b[7]))
              + (b[3] * *(v3 - 2));
        v3 += 4;
        v4 += 4;
    }
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
