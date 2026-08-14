// ============================================================================
// renderdebug.cpp - render.o TimerRenderBars / DebugRender / LightGridMgr /
//                   XModel helpers (DebugRender.cpp, xmodel.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "core/tlFixedString.h"
#include "render/cdDebugShader.h"
#include "ngl/ngl_lighting.h"
#include "ngl/nglDebug.h"

#include <math.h>
#include <intrin.h>

// LightGrid::TOC forward (full layout in lightgrid.cpp)
namespace LightGrid {
struct TOC;
}

// LightGridMgr - minimal render.o view (IDA-verified)
class LightGridMgr : public AssetBankSet {
public:
    LightGridMgr();                              // ??0LightGridMgr@@QAE@XZ
    void SetLightGridFailedColor();  // ?SetLightGridFailedColor@LightGridMgr@@QAEXXZ
    LightGrid::TOC* GetLightGrid(TPakId iPakId); // ?GetLightGrid@LightGridMgr@@QAEPAUTOC@LightGrid@@W4TPakId@@@Z
    LightGrid::TOC* GetLightGrid(int cellNum);   // ?GetLightGrid@LightGridMgr@@QAEPAUTOC@LightGrid@@H@Z
private:
    virtual void UnloadBank(TPakId pakId);       // ?UnloadBank@LightGridMgr@@EAEXW4TPakId@@@Z
    void* mList[99];                             // ae_array<LightGrid::TOC*, 99>
};

// BspCell / BspTree views for GetLightGrid(int)
struct BspCell {
    uint8_t _pad[0x48];
    void* mLgridToc;             // +0x48
};
class BspTree {
public:
    uint8_t _pad[0x18];
    unsigned int mCellsSize;     // +0x18
    BspCell* mCellsList;         // +0x1C
};
extern BspTree* g_bspTree;       // ?g_bspTree@@3PAVBspTree@@A @ 0xF743DC

// nglProjectPoint (sret form per binary mangling)
struct nglScene;
extern nglScene* nglBuildScene;  // ?nglBuildScene@@3PAUnglScene@@A
math::Position3 nglProjectPoint(const math::Position3& In, nglScene* Scene);

extern char* va(const char* fmt, ...);  // core.o
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void* cdGetResource(const tlFixedString& FileName, unsigned int FourCC,
                           bool ExtraSafety);  // streamer.o
extern void tlPrint(const char* lpOutputString);  // tl lib
extern int g_lightGridBlueErrors;   // ?g_lightGridBlueErrors@@3HA (g.o)
extern int g_bOptimize;             // ?g_bOptimize@@3HA (render.o @ 0xF743D0)

// controller (input/controller.o); minimal view to avoid ui_types.h clash
class controller {
public:
    enum ButtonIndex {
        L2 = 11,
        SELECT = 15,
    };
    static controller* inst();          // ?inst@controller@@SAPAV1@XZ
    int  button_value(int controller, ButtonIndex btn);
    bool button_pressed(int controller, ButtonIndex btn);
};

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

// ea: 0x006C3C70
void TimerRenderBars::Render()
{
    controller* v2 = controller::inst();
    if (v2->button_value(0, controller::SELECT) > 0)
    {
        controller* v3 = controller::inst();
        if (v3->button_pressed(0, controller::L2))
        {
            mActive ^= 1u;
            mCvarEnabled->integer = 0;
        }
    }
    if (mActive != 0 || (mActive = mCvarEnabled->integer, mCvarEnabled->integer != 0))
    {
        float v6 = mVSyncLength / (float)mRenderTimersScale;
        float v7 = (1.0f / v6) * 1000.0f;
        float hashDelta = 1.0f / v6;
        float totalMs = v7;

        float curVal;
        if (mCvarShowAdvance->integer == 0)
        {
            curVal = 0.0f;
        }
        else
        {
            unsigned __int64 elapsed = mFrameAdvance.mLastEnd - mFrameAdvance.mLastBegin;
            float v10 = (float)((double)elapsed / (double)(totalMs * 733333.31f));
            curVal = v10;
            if (v10 < 0.0f)
                curVal = 0.0f;
            else if (curVal > 1.0f)
                curVal = 1.0f;
        }

        float v12;
        if (mCvarShowScene->integer == 0)
            v12 = 0.0f;
        else
            v12 = nglPerfInfo.ListSubmitMS / v7;
        float v13;
        if (mCvarShowDma->integer != 0)
            v13 = (nglPerfInfo.ListSendMS / v7) + v12;
        else
            v13 = 0.0f;

        float v14 = curVal * 600.0f + 20.0f;
        float dmaBuildVal = (v13 * 600.0f) + 20.0f;
        float sceneSubmitVal = (v12 * 600.0f) + 20.0f;
        float fpsVal = ((v6 / nglPerfInfo.FPS) * 600.0f) + 20.0f;

        Color col;
        col.r = 1.0f; col.g = 1.0f; col.b = 0.0f; col.a = 0.5f;
        DebugRender::RenderQuad2D(20.0f, 30.0f, v14, 40.0f, 0.0f, col);
        col.r = 1.0f; col.g = 0.0f; col.b = 0.0f; col.a = 0.5f;
        DebugRender::RenderQuad2D(v14, 30.0f, sceneSubmitVal, 40.0f, 0.0f, col);
        col.r = 1.0f; col.g = 0.0f; col.b = 1.0f; col.a = 0.5f;
        DebugRender::RenderQuad2D(sceneSubmitVal, 30.0f, dmaBuildVal, 40.0f, 0.0f, col);
        col.r = 1.0f; col.g = 1.0f; col.b = 1.0f; col.a = 0.5f;
        DebugRender::RenderQuad2D(dmaBuildVal, 30.0f, fpsVal, 40.0f, 0.0f, col);

        float renderVal = nglPerfInfo.RenderMS / totalMs * 600.0f + 20.0f;
        col.r = 0.0f; col.g = 1.0f; col.b = 0.0f; col.a = 0.5f;
        DebugRender::RenderQuad2D(20.0f, 50.0f, renderVal, 60.0f, 0.0f, col);
        col.r = 1.0f; col.g = 1.0f; col.b = 1.0f; col.a = 0.5f;
        DebugRender::RenderQuad2D(renderVal, 50.0f, fpsVal, 60.0f, 0.0f, col);

        mFrameAdvance.Next();
        mUser.Next();

        float v25 = (mVSyncLength * hashDelta) + 0.0099999998f;
        hashDelta = 600.0f / v25;
        int v27 = (int)v25 + 1;
        float curX = 20.0f;
        if (v27 >= 0)
        {
            col.r = 0.0f; col.g = 0.0f; col.b = 1.0f; col.a = 1.0f;
            int v28 = v27 + 1;
            do
            {
                DebugRender::RenderQuad2D(curX - 1, 30.0f, curX + 1, 60.0f, 0.0f, col);
                --v28;
                curX = curX + hashDelta;
            } while (v28 != 0);
        }
    }
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

// ea: 0x006C3B20
LightGridMgr::LightGridMgr() : AssetBankSet()
{
    for (int i = 0; i < 99; ++i)
    {
        if (i < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        mList[i] = nullptr;
    }
}

// ea: 0x006C3BC0
void LightGridMgr::UnloadBank(TPakId pakId)
{
    mList[(int)pakId] = nullptr;
}

// ea: 0x006C3BE0
LightGrid::TOC* LightGridMgr::GetLightGrid(TPakId iPakId)
{
    if ((int)iPakId == -1)
        return nullptr;
    LightGrid::TOC* v3 = (LightGrid::TOC*)mList[(int)iPakId];
    if (v3 == nullptr)
    {
        if (g_lightGridBlueErrors != 0)
        {
            nglSetAmbientLight(0.0f, 0.0f, 1.0f);
            return nullptr;
        }
        nglSetAmbientLight(0.80000001f, 0.80000001f, 0.80000001f);
    }
    return v3;
}

// ea: 0x006C3C40
LightGrid::TOC* LightGridMgr::GetLightGrid(int cellNum)
{
    if (cellNum != -1)
        return (LightGrid::TOC*)g_bspTree->mCellsList[cellNum].mLgridToc;
    return nullptr;
}

// ============================================================================
// DebugRender ctor / RenderText3DOff2D
// ============================================================================

// ea: 0x006C4060
DebugRender::DebugRender()
{
    mDebugSphereMesh = nullptr;
    mDebugCylinderMesh = nullptr;
    mDebugHemisphereMesh = nullptr;
    mDebugShaderMaterial = nullptr;
    mRenderFpList.m_size = 0;
}

// ea: 0x006C40A0
void DebugRender::RenderText3DOff2D(const char* str, const math::Position3& pos,
                                    const math::Dir3& off, const Color& col,
                                    float depth, float size)
{
    math::Position3 proj = nglProjectPoint(pos, nglBuildScene);
    float pz = _mm_shuffle_ps(proj.v, proj.v, 170).m128_f32[0];
    if (pz > 0.0f)
    {
        float py = _mm_shuffle_ps(proj.v, proj.v, 85).m128_f32[0];
        float px = proj.v.m128_f32[0];
        DebugRender::RenderText(str, (int)(off.v.m128_f32[0] + px),
                                (int)(off.v.m128_f32[1] + py), col, depth, size);
    }
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
