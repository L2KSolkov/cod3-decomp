// ============================================================================
// renderdebug.cpp - render.o TimerRenderBars / DebugRender / LightGridMgr /
//                   XModel helpers (DebugRender.cpp, xmodel.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "core/tlFixedString.h"
#include "render/cdDebugShader.h"
#include "ngl/ngl_lighting.h"

#include <math.h>
#include <intrin.h>

// LightGridMgr - minimal render.o view (IDA-verified)
class LightGridMgr {
public:
    void SetLightGridFailedColor();  // ?SetLightGridFailedColor@LightGridMgr@@QAEXXZ
};

extern char* va(const char* fmt, ...);  // core.o
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void* cdGetResource(const tlFixedString& FileName, unsigned int FourCC,
                           bool ExtraSafety);  // streamer.o
extern void tlPrint(const char* lpOutputString);  // tl lib
extern int g_lightGridBlueErrors;   // ?g_lightGridBlueErrors@@3HA (g.o)
extern int g_bOptimize;             // ?g_bOptimize@@3HA (render.o @ 0xF743D0)

// ============================================================================
// TimerRenderBars
// ============================================================================

// ea: 0x006BCE00
TimerRenderBars::TimerRenderBars()
{
    mFrameAdvance.mEnd = 0;
    mFrameAdvance.mBegin = 0;
    mFrameAdvance.mLastEnd = 0;
    mFrameAdvance.mLastBegin = 0;
    mUser.mEnd = 0;
    mUser.mBegin = 0;
    mUser.mLastEnd = 0;
    mUser.mLastBegin = 0;
    mVSyncLength = 59.939999f;
    mRenderTimersScale = 5;
    mActive = 0;
    mCvarShowAdvance = nullptr;
    mCvarShowScene = nullptr;
    mCvarShowDma = nullptr;
    mCvarShowUser = nullptr;
}

// ea: 0x006BCE60
void TimerRenderBars::Init()
{
    mCvarEnabled = Cvar_Get("timerbars_on", "0", 256);
    mCvarShowAdvance = Cvar_Get("timerbars_advance", "1", 256);
    mCvarShowScene = Cvar_Get("timerbars_scene", "1", 256);
    mCvarShowDma = Cvar_Get("timerbars_dma", "1", 256);
    mCvarShowUser = Cvar_Get("timerbars_user", "1", 256);
}

// ea: 0x006BCEE0
void TimerRenderBars::DeltaTimeScale(int d)
{
    if (d > 0 || mRenderTimersScale > 1)
        mRenderTimersScale += d;
}

// ============================================================================
// DebugRender
// ============================================================================

// ea: 0x006BCF00
void DebugRender::Init()
{
    cdDebugShaderMat* v2 = (cdDebugShaderMat*)mem_heap_malloc(0x10u);
    mDebugShaderMaterial =
        v2 != nullptr ? new (v2) cdDebugShaderMat() : nullptr;
    tlFixedString v9("dbgsphr");
    mDebugSphereMesh = (nglMesh*)cdGetResource(v9, 0x4853454Du, true);
    tlFixedString v8("dbgcyl");
    mDebugCylinderMesh = (nglMesh*)cdGetResource(v8, 0x4853454Du, true);
    tlFixedString v7("dbghemi");
    mDebugHemisphereMesh = (nglMesh*)cdGetResource(v7, 0x4853454Du, true);
    if (mDebugSphereMesh == nullptr)
        tlPrint("unable to find debug sphere mesh!");
    if (mDebugCylinderMesh == nullptr)
        tlPrint("unable to find debug cylinder mesh!");
    if (mDebugHemisphereMesh == nullptr)
        tlPrint("unable to find debug hemisphere mesh!");
}

// ============================================================================
// LightGridMgr
// ============================================================================

// ea: 0x006BC870
void LightGridMgr::SetLightGridFailedColor()
{
    if (g_lightGridBlueErrors != 0)
        nglSetAmbientLight(0.0f, 0.0f, 1.0f);
    else
        nglSetAmbientLight(0.80000001f, 0.80000001f, 0.80000001f);
}

// ============================================================================
// XModel helpers
// ============================================================================

// ea: 0x006BD3A0
void XModelSetOptimize(int bOptimize)
{
    g_bOptimize = bOptimize;
}

// ea: 0x006BD060
const math::Dir3 compute_orth_unit_vector(const math::Dir3& v)
{
    math::Dir3 result;
    float v0 = v.v.m128_f32[0];
    float v1 = v.v.m128_f32[1];
    float v2 = v.v.m128_f32[2];
    // cross(XAxis, v)
    float c0 = -v2;
    float c1 = 0.0f;
    float c2 = v1;
    float len = sqrtf(c0 * c0 + c1 * c1 + c2 * c2);
    if (len < 0.000099999997f)
    {
        // cross(YAxis, v)
        c0 = v2;
        c1 = 0.0f;
        c2 = -v0;
        len = sqrtf(c0 * c0 + c1 * c1 + c2 * c2);
        if (len < 0.000099999997f)
        {
            // cross(ZAxis, v)
            c0 = -v1;
            c1 = v0;
            c2 = 0.0f;
            len = sqrtf(c0 * c0 + c1 * c1 + c2 * c2);
            if (len < 0.000099999997f)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DebugRender.cpp";
                AeAssert::gCurrentLine = 76;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning("Please add a descriptive string"))
                    __debugbreak();
            }
        }
    }
    float inv = 1.0f / len;
    result.v.m128_f32[0] = c0 * inv;
    result.v.m128_f32[1] = c1 * inv;
    result.v.m128_f32[2] = c2 * inv;
    return result;
}
