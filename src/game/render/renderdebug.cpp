// ============================================================================
// renderdebug.cpp - render.o TimerRenderBars / DebugRender / LightGridMgr /
//                   XModel helpers (DebugRender.cpp, xmodel.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "core/tlFixedString.h"
#include "render/cdDebugShader.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_lighting.h"
#include "ngl/nglDebug.h"
#include "ngl/ngl_dx_quad.h"

#include <math.h>
#include <intrin.h>
#include <stdio.h>

// LightGrid::TOC forward (full layout in lightgrid.cpp)
namespace LightGrid {
struct Light {
    math::Vector4::Packed mPosition;   // +0x00 (16)
    math::Position3::Packed mColor;    // +0x10 (12)
};
struct Cell {
    math::Position3::Packed mBase;  // +0x00 (12 bytes)
    float mXGridDelta;              // +0x0C
    float mYGridDelta;              // +0x10
    uint16_t mNumXRows;             // +0x14
    uint16_t mNumYRows;             // +0x16
    unsigned int mFirstGridPoint;   // +0x18
    unsigned int mCellIndex;        // +0x1C
};
struct DLightInfo {
    math::Vector4::Packed mInfo;    // +0x00
};
struct GridPoint {
    unsigned int gridpoint;         // +0x00
};
struct LightIndex {
    unsigned short mIndex;          // +0x00
    unsigned short mAttenuationInt; // +0x02
};
struct TOC {
    Light* mLights;          // +0x00
    int mNumLights;          // +0x04
    GridPoint* mGridPoints;  // +0x08
    int mNumGridPoints;      // +0x0C
    void* mLightIndices;     // +0x10
    int mNumLightIndices;    // +0x14
    Cell* mCells;            // +0x18
    int mNumCells;           // +0x1C
    DLightInfo* mDLightInfos;// +0x20
    int mNumDLightInfos;     // +0x24
};
}
class LightGridData;

// LightGridMgr - minimal render.o view (IDA-verified)
class LightGridMgr : public AssetBankSet {
public:
    LightGridMgr();                              // ??0LightGridMgr@@QAE@XZ
    static LightGridMgr* sInst;                  // ?sInst@LightGridMgr@@2PAV1@A
    void SetLightGridFailedColor();  // ?SetLightGridFailedColor@LightGridMgr@@QAEXXZ
    LightGrid::TOC* GetLightGrid(TPakId iPakId); // ?GetLightGrid@LightGridMgr@@QAEPAUTOC@LightGrid@@W4TPakId@@@Z
    LightGrid::TOC* GetLightGrid(int cellNum);   // ?GetLightGrid@LightGridMgr@@QAEPAUTOC@LightGrid@@H@Z
    LightGrid::TOC* GetLightGrid(const math::Position3& posArg,
                                 int* pCellNum); // ?GetLightGrid@LightGridMgr@@QAEPAUTOC@LightGrid@@ABVPosition3@math@@PAH@Z
    void AddVertexPointLights(TPakId pakId, LightGrid::TOC* toc);  // ?AddVertexPointLights@LightGridMgr@@QAEXW4TPakId@@PAUTOC@LightGrid@@@Z
    void RenderLightGridDebugSphere(const math::Position3& pos);    // ?RenderLightGridDebugSphere@LightGridMgr@@QAEXABVPosition3@math@@@Z
    void RenderLightGridDebugSpheres();                             // ?RenderLightGridDebugSpheres@LightGridMgr@@QAEXXZ
    void RenderLightGridDebugLines();                               // ?RenderLightGridDebugLines@LightGridMgr@@QAEXXZ
    void RenderDebugText();                                         // ?RenderDebugText@LightGridMgr@@QAEXXZ @ 0x6DB240
    void SampleLightGrid(const LightGrid::TOC& toc, int cellidx,
                         const math::Position3& pos,
                         LightGridData* pLG);  // lightgrid.cpp
private:
    void GetAmbientColors(const LightGrid::GridPoint** grid,
                          math::Position3* ambientColors);  // lightgrid.cpp
    void GetLightListForGrid(
        const LightGrid::TOC& toc, const LightGrid::GridPoint** grid,
        float* weights,
        ae_sized_array<LightGrid::LightIndex, 12>& lights);  // lightgrid.cpp
    virtual void UnloadBank(TPakId pakId);       // ?UnloadBank@LightGridMgr@@EAEXW4TPakId@@@Z
    void* mList[99];                             // ae_array<LightGrid::TOC*, 99>
};

// BspCell / BspTree views for GetLightGrid(int)
struct BspCell {
    uint8_t _pad[0x48];
    void* mLgridToc;             // +0x48
};
struct BspTree {
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
extern void* tlMemAlloc(unsigned int size, unsigned int align,
                        unsigned int flags);  // core.o
extern void tlMemFree(void* Ptr);             // core.o
extern void profile_reset();  // ?profile_reset@@YAXXZ (cdDebugRender.cpp)
extern int g_lightGridBlueErrors;   // ?g_lightGridBlueErrors@@3HA (g.o)
extern int g_bOptimize;             // ?g_bOptimize@@3HA (render.o @ 0xF743D0)
extern const math::Mat43* nglGetMatrix_ViewToWorld(nglScene* Scene);  // ngl_scene.cpp
extern void* nglListAlloc(unsigned int size, unsigned int align);     // ngl.o
extern nglMeshNode* nglListAddMesh(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                                   nglMeshParams* MeshParams,
                                   nglShaderParamSet* ShaderParams,
                                   void (*fn)(nglMeshNode*));  // ngl_mesh.cpp
float below = 55.0f;  // ?below@@3MA @ 0xE01DE8

// LightEffect / AddLight (tr_fx2.cpp)
class LightEffect {
public:
    enum eType : int { PROJECTED_TEXTURE = 0x0, VERTEX_LIGHT = 0x1 };
    enum eTime : int { FLASH = 0x1, FOREVER = 0xFFFFFFFF };
    float mColor[4];          // +0x20
    bool mFlicker;            // +0x3C
    float mColorOriginal[4];  // +0x40
    float mInnerRadius;       // +0x54
    float mOuterRadius;       // +0x58
};
extern LightEffect* AddLight(TPakId pakId, LightEffect::eType type,
                             const math::Position3& pos,
                             LightEffect::eTime time);  // ?AddLight@@YAPAVLightEffect@@W4TPakId@@W4eType@1@ABVPosition3@math@@W4eTime@1@@Z

// ============================================================================
// DebugRender deferred primitives + ae_vector value template
// ============================================================================
class DebugSphere {
public:
    math::Position3::Packed mPos;   // +0x00
    float mRadius;                  // +0x0C
    Color mArgbColor;               // +0x10

    DebugSphere() {}                // ??0DebugSphere@@QAE@XZ (render.o 0x6E7390)
    DebugSphere(const math::Position3& pos, float radius,
                const Color& argb_color);  // render.o 0x6E73A0
};

DebugSphere::DebugSphere(const math::Position3& pos, float radius,
                         const Color& argb_color)
{
    mPos.x = pos.v.m128_f32[0];
    mPos.y = pos.v.m128_f32[1];
    mPos.z = pos.v.m128_f32[2];
    mRadius = radius;
    mArgbColor = argb_color;
}

class DebugTri {
public:
    math::Position3::Packed mPt1;   // +0x00
    math::Position3::Packed mPt2;   // +0x0C
    math::Position3::Packed mPt3;   // +0x18
    Color mCol;                     // +0x24

    DebugTri() {}                   // ??0DebugTri@@QAE@XZ (render.o 0x6E7460)
    DebugTri(const math::Position3& pt1, const math::Position3& pt2,
             const math::Position3& pt3,
             const Color& col);     // render.o 0x6E7470
};

DebugTri::DebugTri(const math::Position3& pt1, const math::Position3& pt2,
                   const math::Position3& pt3, const Color& col)
{
    mPt1.x = pt1.v.m128_f32[0];
    mPt1.y = pt1.v.m128_f32[1];
    mPt1.z = pt1.v.m128_f32[2];
    mPt2.x = pt2.v.m128_f32[0];
    mPt2.y = pt2.v.m128_f32[1];
    mPt2.z = pt2.v.m128_f32[2];
    mPt3.x = pt3.v.m128_f32[0];
    mPt3.y = pt3.v.m128_f32[1];
    mPt3.z = pt3.v.m128_f32[2];
    mCol = col;
}

class DebugLine {
public:
    math::Position3::Packed mPt1;   // +0x00
    math::Position3::Packed mPt2;   // +0x0C
    float mThickness;               // +0x18
    Color mCol;                     // +0x1C

    DebugLine() {}                  // ??0DebugLine@@QAE@XZ (render.o 0x6E7260)
    DebugLine(const math::Position3& pt1, const math::Position3& pt2,
              const Color& col,
              float thickness);     // render.o 0x6E7270
};

DebugLine::DebugLine(const math::Position3& pt1, const math::Position3& pt2,
                     const Color& col, float thickness)
{
    mPt1.x = pt1.v.m128_f32[0];
    mPt1.y = pt1.v.m128_f32[1];
    mPt1.z = pt1.v.m128_f32[2];
    mPt2.x = pt2.v.m128_f32[0];
    mPt2.y = pt2.v.m128_f32[1];
    mPt2.z = pt2.v.m128_f32[2];
    mCol = col;
    mThickness = thickness;
}

template class ae_vector<DebugSphere>;
template class ae_vector<DebugLine>;
template class ae_vector<DebugTri>;

ae_vector<DebugSphere> gDebugSpheres;  // ?gDebugSpheres@@3V?$ae_vector@VDebugSphere@@@@A @ 0x13641BC
ae_vector<DebugLine> gDebugLines;      // ?gDebugLines@@3V?$ae_vector@VDebugLine@@@@A @ 0x1366C84
ae_vector<DebugTri> gDebugTris;        // ?gDebugTris@@3V?$ae_vector@VDebugTri@@@@A @ 0x13642C4

// ngl/render-layer helpers (render_xboxr / ngl_aux.o)
extern void auxSetScale(nglMeshParams* dest, float src0, float src1,
                        float src2);  // ?auxSetScale@@YAXPAVnglMeshParams@@MMM@Z
extern void setup_color(const Color& i_col, nglShaderParamSet& o_params);
extern gpuVertexFormat cddebug_vertex_format;  // ?cddebug_vertex_format@@3UgpuVertexFormat@@A
extern nglMesh* auxCreateScratchMesh(int flags, int num);   // aux.o
extern nglMeshSection* nglCreateScratchSection(
    int Prim, int NIndices, int NVertices, gpuVertexFormat* Fmt);  // ngl.o
extern void nglAddMeshSection(nglMesh* Mesh, nglMeshSection* Section,
                              nglMaterial* Mat, int Count);  // ngl.o
extern void* nglLockSectionIndices(nglMeshSection* Section);   // ngl.o
extern void* nglLockSectionVertices(nglMeshSection* Section);  // ngl.o
extern nglMesh* auxCloseScratchMesh(nglMesh* Mesh);            // aux.o
extern void j_nullsub_27(nglMeshSection* Section);  // nullsub
extern void j_nullsub_67(nglMeshSection* Section);  // nullsub

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

// ea: 0x006C93F0
LightGrid::TOC* LightGridMgr::GetLightGrid(const math::Position3& posArg,
                                           int* pCellNum)
{
    struct PakInfoNodeView {
        uint8_t _pad[0xB4];
        TPakId pakId;  // +0xB4
    };
    math::Position3 v13 = posArg;
    v13.v.m128_f32[2] += 0.01f;
    int localCell;
    int* v7 = pCellNum != nullptr ? pCellNum : &localCell;
    int v8 = R_CellForPoint(&v13);
    *v7 = v8;
    if (v8 == -1)
    {
        v13.v.m128_f32[2] += 50.0f;
        *v7 = R_CellForPoint(&v13);
    }
    if (*v7 != -1)
    {
        const PakInfoNodeView* CellPakInfo = (const PakInfoNodeView*)
            StreamZoneManager::sInst->GetCellPakInfo(*v7);
        if (CellPakInfo == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\LightGridMgr.cpp";
            AeAssert::gCurrentLine = 178;
            AeAssert::gCurrentExpr = "pakInfo";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("no zone for LightGrid cell!!!"))
                __debugbreak();
        }
        return GetLightGrid(CellPakInfo->pakId);
    }
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
// DebugRender::RenderTexturedQuad2D - ea: 0x006CA350
// ============================================================================
struct DebugTexturedQuad2D {
    float l;          // +0x00
    float t;          // +0x04
    float r;          // +0x08
    float b;          // +0x0C
    float z;          // +0x10
    Color col;        // +0x14
    nglTexture* nglTex;  // +0x24

    DebugTexturedQuad2D() {}  // ??0DebugTexturedQuad2D@@QAE@XZ
    DebugTexturedQuad2D(float l, float t, float r, float b, float z,
                        const Color& col,
                        nglTexture* nglTex);  // render.o 0x6E75F0
};
static_assert(sizeof(DebugTexturedQuad2D) == 0x28, "DebugTexturedQuad2D size mismatch");

DebugTexturedQuad2D::DebugTexturedQuad2D(float l, float t, float r, float b,
                                         float z, const Color& col,
                                         nglTexture* nglTex)
{
    this->l = l;
    this->t = t;
    this->r = r;
    this->b = b;
    this->z = z;
    this->col = col;
    this->nglTex = nglTex;
}

struct DebugQuadVector {
    DebugTexturedQuad2D* mElements;  // +0x00
    int mSize;                       // +0x04
    int mCapacity;                   // +0x08
    void push_back(const DebugTexturedQuad2D& e);
    void resize(int iNewSize);
};
DebugQuadVector gDebugTexturedQuad2Ds;  // ?gDebugTexturedQuad2Ds@@3V?$ae_vector@VDebugTexturedQuad2D@@@@A @ 0xF755F0

void DebugQuadVector::push_back(const DebugTexturedQuad2D& e)
{
    if (mSize >= mCapacity)
    {
        int newCap = mCapacity > 0 ? mCapacity * 2 : 4;
        DebugTexturedQuad2D* ne = (DebugTexturedQuad2D*)realloc(mElements,
                                                               newCap * sizeof(DebugTexturedQuad2D));
        mElements = ne;
        mCapacity = newCap;
    }
    mElements[mSize++] = e;
}

void DebugQuadVector::resize(int iNewSize)
{
    if (iNewSize > mCapacity)
    {
        DebugTexturedQuad2D* ne = (DebugTexturedQuad2D*)realloc(
            mElements, iNewSize * sizeof(DebugTexturedQuad2D));
        mElements = ne;
        mCapacity = iNewSize;
        mSize = iNewSize;
    }
    else
    {
        mSize = iNewSize;
    }
}

void DebugRender::RenderTexturedQuad2D(float l, float t, float r, float b,
                                       float z, const Color& col,
                                       nglTexture* nglTex)
{
    extern unsigned int extract_color(const Color& col);
    if (nglBuildScene != nullptr && nglBuildScene->Parent != nullptr)
    {
        nglQuad q;
        nglInitQuad(&q);
        nglSetQuadZ(&q, z);
        nglSetQuadRect(&q, l, t, r, b);
        nglSetQuadColor(&q, extract_color(col));
        nglSetQuadTex(&q, nglTex);
        nglListAddQuad(&q);
    }
    else
    {
        DebugTexturedQuad2D iElement;
        iElement.l = l;
        iElement.t = t;
        iElement.r = r;
        iElement.b = b;
        iElement.z = z;
        iElement.col.r = col.r;
        iElement.col.g = col.g;
        iElement.col.b = col.b;
        iElement.col.a = col.a;
        iElement.nglTex = nglTex;
        gDebugTexturedQuad2Ds.push_back(iElement);
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

// ============================================================================
// LightGridMgr debug rendering (LightGridMgr.cpp)
// ============================================================================

// ea: 0x006D46A0
void LightGridMgr::AddVertexPointLights(TPakId pakId, LightGrid::TOC* toc)
{
    int mNumLights = toc->mNumLights;
    LightGrid::Light* mLights = toc->mLights;
    int v17 = 0;
    if (mNumLights > 0)
    {
        do
        {
            float w = mLights->mPosition.w;
            if (w >= 2.0f)
            {
                LightGrid::DLightInfo* mDLightInfos = toc->mDLightInfos;
                if (mDLightInfos == nullptr)
                {
                    printf("Re convert your level , cod2rad and lgridcvt for dynamic lights\n");
                    return;
                }
                math::Position3 pos;
                pos.v.m128_f32[0] = mLights->mPosition.x;
                pos.v.m128_f32[1] = mLights->mPosition.y;
                pos.v.m128_f32[2] = mLights->mPosition.z;
                pos.v.m128_f32[3] = mLights->mPosition.w;
                LightGrid::DLightInfo* p_x = &mDLightInfos[(int)w - 2];
                bool v18 = p_x->mInfo.z >= 1.0f;
                LightEffect* v10 = AddLight(pakId, LightEffect::VERTEX_LIGHT,
                                            pos, LightEffect::FOREVER);
                v10->mColor[0] = mLights->mColor.x;
                v10->mColor[1] = mLights->mColor.y;
                v10->mColor[2] = mLights->mColor.z;
                v10->mColor[3] = 1.0f;
                v10->mColorOriginal[0] = mLights->mColor.x;
                v10->mColorOriginal[1] = mLights->mColor.y;
                v10->mColorOriginal[2] = mLights->mColor.z;
                v10->mFlicker = v18;
                v10->mInnerRadius = p_x->mInfo.x;
                v10->mOuterRadius = p_x->mInfo.y;
            }
            ++mLights;
            ++v17;
        } while (v17 < toc->mNumLights);
    }
}

// ea: 0x006D47E0
void LightGridMgr::RenderLightGridDebugSphere(const math::Position3& pos)
{
    nglMesh* mDebugSphereMesh = DebugRender::sInst.mDebugSphereMesh;
    if (mDebugSphereMesh != nullptr)
        mDebugSphereMesh->Flags |= 0x1000000;
    nglLightContext* lightCtx = nglCreateLightContext();
    int cellNum;
    LightGrid::TOC* LightGrid = GetLightGrid(pos, &cellNum);
    if (LightGrid != nullptr)
        SampleLightGrid(*LightGrid, cellNum, pos, nullptr);

    math::Mat43 mtx;
    mtx.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    mtx.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    mtx.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    mtx.w.v = pos.v;

    nglShaderParamSet* ctx = (nglShaderParamSet*)nglListAlloc(
        4 * nglShaderParamSet::NumParams + 8, 8u);
    ctx->Array[0] = 0;
    ctx->Array[1] = 0;
    unsigned __int64 mask = 1i64 << nglLightContextParamID;
    ctx->Array[0] |= (unsigned int)mask;
    ctx->Array[1] |= (unsigned int)(mask >> 32);
    ctx->Array[nglLightContextParamID + 2] = (unsigned int)lightCtx;
    nglListAddMesh(mDebugSphereMesh, mtx, nullptr, ctx, nullptr);
}

// ea: 0x006D4900
void LightGridMgr::RenderLightGridDebugSpheres()
{
    math::Position3 cameraPos;
    cameraPos.v = nglGetMatrix_ViewToWorld(nglBuildScene)->w.v;
    int cellNum;
    LightGrid::TOC* LightGrid = GetLightGrid(cameraPos, &cellNum);
    if (cellNum < 0)
        return;
    int mNumCells = LightGrid->mNumCells;
    int v8 = 0;
    if (mNumCells <= 0)
        return;
    LightGrid::Cell* mCells = LightGrid->mCells;
    while (mCells[v8].mCellIndex != (unsigned int)cellNum)
    {
        if (++v8 >= mNumCells)
            return;
    }
    LightGrid::Cell* v11 = &mCells[v8];
    if (v11 == nullptr)
        return;
    int v12 = 1;
    if (v11->mNumXRows - 1 <= 1)
        return;
    do
    {
        int v14 = 1;
        if (v11->mNumYRows - 1 > 1)
        {
            float cameraZ = cameraPos.v.m128_f32[2];
            do
            {
                math::Position3 pos;
                pos.v.m128_f32[0] = v11->mBase.x + (float)v12 * v11->mXGridDelta;
                pos.v.m128_f32[1] = v11->mBase.y + (float)v14 * v11->mYGridDelta;
                pos.v.m128_f32[2] = cameraZ - below;
                pos.v.m128_f32[3] = 0.0f;
                float dx = pos.v.m128_f32[0] - cameraPos.v.m128_f32[0];
                float dy = pos.v.m128_f32[1] - cameraPos.v.m128_f32[1];
                float dist2 = (dx * dx) + (dy * dy);
                if (dist2 < 62500.0f)
                    RenderLightGridDebugSphere(pos);
                ++v14;
            } while (v14 < v11->mNumYRows - 1);
        }
        ++v12;
    } while (v12 < v11->mNumXRows - 1);
}

// ea: 0x006DAF80
void LightGridMgr::RenderLightGridDebugLines()
{
    math::Position3 cameraPos;
    cameraPos.v = nglGetMatrix_ViewToWorld(nglBuildScene)->w.v;
    int cellNum;
    LightGrid::TOC* LightGrid = GetLightGrid(cameraPos, &cellNum);
    if (LightGrid == nullptr || cellNum < 0)
        return;
    int mNumCells = LightGrid->mNumCells;
    int v8 = 0;
    if (mNumCells <= 0)
        return;
    unsigned int* i = &LightGrid->mCells->mCellIndex;
    while (*i != (unsigned int)cellNum)
    {
        if (++v8 >= mNumCells)
            return;
        i += 8;
    }
    LightGrid::Cell* v10 = &LightGrid->mCells[v8];
    if (v10 == nullptr)
        return;
    int v11 = 1;
    if (v10->mNumXRows <= 1u)
        return;
    do
    {
        int v13 = 1;
        if (v10->mNumYRows > 1u)
        {
            do
            {
                math::Position3 pos;
                pos.v.m128_f32[0] = v10->mBase.x + (float)v11 * v10->mXGridDelta;
                pos.v.m128_f32[1] = v10->mBase.y + (float)v13 * v10->mYGridDelta;
                pos.v.m128_f32[2] = 0.0f;
                pos.v.m128_f32[3] = 0.0f;
                float dx = pos.v.m128_f32[0] - cameraPos.v.m128_f32[0];
                float dy = pos.v.m128_f32[1] - cameraPos.v.m128_f32[1];
                float dist2 = (dx * dx) + (dy * dy);
                if (dist2 < 250000.0f)
                {
                    LightGrid::GridPoint* mGridPoints = LightGrid->mGridPoints;
                    unsigned int gridpoint =
                        mGridPoints[v11 + v10->mFirstGridPoint
                                    + v13 * v10->mNumXRows].gridpoint;
                    float r = (float)(gridpoint & 0x1F) * 0.032258064f;
                    float g = (float)((gridpoint >> 5) & 0x1F) * 0.032258064f;
                    float b = (float)((gridpoint >> 10) & 0x1F) * 0.032258064f;
                    math::Position3 p0 = pos;
                    math::Position3 p1 = pos;
                    p0.v.m128_f32[2] -= 1000.0f;
                    p1.v.m128_f32[2] += 1000.0f;
                    DebugRender::RenderLine(p0, p1, Color(r, g, b, 1.0f), 2.5f);
                }
                ++v13;
            } while (v13 < v10->mNumYRows);
        }
        ++v11;
    } while (v11 < v10->mNumXRows);
}

// ============================================================================
// DebugRender primitives (DebugRender.cpp)
// ============================================================================

// ea: 0x006D4AF0
void DebugRender::RenderSphere(const math::Position3& pos, float radius,
                               const Color& color)
{
    if (nglBuildScene != nullptr && nglBuildScene->Parent != nullptr)
    {
        math::Mat43 mtx;
        mtx.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
        mtx.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        mtx.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
        mtx.w.v = pos.v;
        nglMeshParams mesh_params;
        auxSetScale(&mesh_params, radius, radius, radius);
        nglShaderParamSet* shader_params = (nglShaderParamSet*)nglListAlloc(
            4 * nglShaderParamSet::NumParams + 8, 8u);
        shader_params->Array[0] = 0;
        shader_params->Array[1] = 0;
        setup_color(color, *shader_params);
        nglListAddMesh(DebugRender::sInst.mDebugSphereMesh, mtx, &mesh_params,
                       shader_params, nullptr);
    }
    else
    {
        DebugSphere sphere(pos, radius, color);
        gDebugSpheres.push_back(sphere);
    }
}

// ea: 0x006D4C10
void DebugRender::RenderCapsule(const math::Position3& base,
                                const math::Position3& end, float radius,
                                const Color& argb_color)
{
    math::Position3 center;
    center.v = _mm_mul_ps(_mm_add_ps(base.v, end.v), _mm_set1_ps(0.5f));
    math::Dir3 ydir;
    ydir.v = _mm_sub_ps(end.v, base.v);
    __m128 len2 = _mm_mul_ps(ydir.v, ydir.v);
    float height = sqrtf(len2.m128_f32[0]
                         + (len2.m128_f32[1] + len2.m128_f32[2]));
    if (height < 0.001f)
    {
        DebugRender::RenderSphere(base, radius, argb_color);
        return;
    }
    ydir.v = _mm_mul_ps(ydir.v, _mm_set1_ps(1.0f / height));
    math::Dir3 xdir = compute_orth_unit_vector(ydir);
    math::Dir3 zdir;
    zdir.v = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(xdir.v, xdir.v, 9),
                   _mm_shuffle_ps(ydir.v, ydir.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(xdir.v, xdir.v, 18),
                   _mm_shuffle_ps(ydir.v, ydir.v, 9)));
    __m128 zlen2 = _mm_mul_ps(zdir.v, zdir.v);
    float zlen = sqrtf(zlen2.m128_f32[0]
                       + (zlen2.m128_f32[1] + zlen2.m128_f32[2]));
    zdir.v = _mm_mul_ps(zdir.v, _mm_set1_ps(1.0f / zlen));

    math::Mat43 mtx;
    mtx.x.v = xdir.v;
    mtx.y.v = ydir.v;
    mtx.z.v = zdir.v;
    mtx.w.v = center.v;

    nglMeshParams mesh_params;
    auxSetScale(&mesh_params, radius, height, radius);
    nglShaderParamSet* shader_params = (nglShaderParamSet*)nglListAlloc(
        4 * nglShaderParamSet::NumParams + 8, 8u);
    shader_params->Array[0] = 0;
    shader_params->Array[1] = 0;
    setup_color(argb_color, *shader_params);
    nglListAddMesh(DebugRender::sInst.mDebugCylinderMesh, mtx, &mesh_params,
                   shader_params, nullptr);

    math::Dir3 halfAxis;
    halfAxis.v = _mm_mul_ps(ydir.v, _mm_set1_ps(height * 0.5f));
    mtx.w.v = _mm_add_ps(center.v, halfAxis.v);
    auxSetScale(&mesh_params, radius, radius, radius);
    nglListAddMesh(DebugRender::sInst.mDebugHemisphereMesh, mtx, &mesh_params,
                   shader_params, nullptr);

    mtx.y.v = _mm_xor_ps(ydir.v, _mm_set1_ps(-0.0f));
    mtx.w.v = _mm_sub_ps(center.v, halfAxis.v);
    nglListAddMesh(DebugRender::sInst.mDebugHemisphereMesh, mtx, &mesh_params,
                   shader_params, nullptr);
}

// ea: 0x006D4EC0
void DebugRender::RenderCylinder(const math::Position3& base,
                                 const math::Position3& end, float radius,
                                 const Color& argb_color)
{
    math::Position3 center;
    center.v = _mm_mul_ps(_mm_add_ps(base.v, end.v), _mm_set1_ps(0.5f));
    math::Dir3 ydir;
    ydir.v = _mm_sub_ps(end.v, base.v);
    __m128 len2 = _mm_mul_ps(ydir.v, ydir.v);
    float height = sqrtf(len2.m128_f32[0]
                         + (len2.m128_f32[1] + len2.m128_f32[2]));
    if (height < 0.001f)
        return;
    ydir.v = _mm_mul_ps(ydir.v, _mm_set1_ps(1.0f / height));
    math::Dir3 xdir = compute_orth_unit_vector(ydir);
    math::Dir3 zdir;
    zdir.v = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(xdir.v, xdir.v, 9),
                   _mm_shuffle_ps(ydir.v, ydir.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(xdir.v, xdir.v, 18),
                   _mm_shuffle_ps(ydir.v, ydir.v, 9)));
    __m128 zlen2 = _mm_mul_ps(zdir.v, zdir.v);
    float zlen = sqrtf(zlen2.m128_f32[0]
                       + (zlen2.m128_f32[1] + zlen2.m128_f32[2]));
    zdir.v = _mm_mul_ps(zdir.v, _mm_set1_ps(1.0f / zlen));

    math::Mat43 mtx;
    mtx.x.v = xdir.v;
    mtx.y.v = ydir.v;
    mtx.z.v = zdir.v;
    mtx.w.v = center.v;

    nglMeshParams mesh_params;
    auxSetScale(&mesh_params, radius, height, radius);
    nglShaderParamSet* shader_params = (nglShaderParamSet*)nglListAlloc(
        4 * nglShaderParamSet::NumParams + 8, 8u);
    shader_params->Array[0] = 0;
    shader_params->Array[1] = 0;
    setup_color(argb_color, *shader_params);
    nglListAddMesh(DebugRender::sInst.mDebugCylinderMesh, mtx, &mesh_params,
                   shader_params, nullptr);
}

// ea: 0x006D50C0
void DebugRender::RenderTriangle(const math::Position3& pt1,
                                 const math::Position3& pt2,
                                 const math::Position3& pt3,
                                 const Color& col, bool double_sided)
{
    (void)double_sided;
    if (nglBuildScene != nullptr && nglBuildScene->Parent != nullptr)
    {
        nglMesh* ScratchMesh = auxCreateScratchMesh(0x40000, 1);
        nglMeshSection* ScratchSection =
            nglCreateScratchSection(6, 3, 3, &cddebug_vertex_format);
        nglAddMeshSection(ScratchMesh, ScratchSection,
                          (nglMaterial*)DebugRender::sInst.mDebugShaderMaterial,
                          1);
        unsigned short* indices =
            (unsigned short*)nglLockSectionIndices(ScratchSection);
        float* verts = (float*)nglLockSectionVertices(ScratchSection);
        verts[0] = pt1.v.m128_f32[0];
        verts[1] = pt1.v.m128_f32[1];
        verts[2] = pt1.v.m128_f32[2];
        indices[0] = 0;
        verts += 3;
        verts[0] = pt2.v.m128_f32[0];
        verts[1] = pt2.v.m128_f32[1];
        verts[2] = pt2.v.m128_f32[2];
        indices[1] = 1;
        verts += 3;
        verts[0] = pt3.v.m128_f32[0];
        verts[1] = pt3.v.m128_f32[1];
        verts[2] = pt3.v.m128_f32[2];
        indices[2] = 2;
        j_nullsub_67(ScratchSection);
        j_nullsub_27(ScratchSection);
        nglShaderParamSet* shader_params = (nglShaderParamSet*)nglListAlloc(
            4 * nglShaderParamSet::NumParams + 8, 8u);
        shader_params->Array[0] = 0;
        shader_params->Array[1] = 0;
        setup_color(col, *shader_params);
        math::Mat43 mtx;
        mtx.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
        mtx.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        mtx.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
        mtx.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
        nglMesh* v12 = auxCloseScratchMesh(ScratchMesh);
        nglListAddMesh(v12, mtx, nullptr, shader_params, nullptr);
    }
    else
    {
        DebugTri tri(pt1, pt2, pt3, col);
        gDebugTris.push_back(tri);
    }
}

// ea: 0x006D5310
void DebugRender::RenderQuad(const math::Position3& pt1,
                             const math::Position3& pt2,
                             const math::Position3& pt3,
                             const math::Position3& pt4, const Color& col,
                             bool double_sided)
{
    (void)double_sided;
    nglMesh* ScratchMesh = auxCreateScratchMesh(0x40000, 1);
    nglMeshSection* ScratchSection =
        nglCreateScratchSection(6, 4, 4, &cddebug_vertex_format);
    nglAddMeshSection(ScratchMesh, ScratchSection,
                      (nglMaterial*)DebugRender::sInst.mDebugShaderMaterial, 1);
    unsigned short* indices =
        (unsigned short*)nglLockSectionIndices(ScratchSection);
    float* verts = (float*)nglLockSectionVertices(ScratchSection);
    verts[0] = pt1.v.m128_f32[0];
    verts[1] = pt1.v.m128_f32[1];
    verts[2] = pt1.v.m128_f32[2];
    indices[0] = 0;
    verts += 3;
    verts[0] = pt2.v.m128_f32[0];
    verts[1] = pt2.v.m128_f32[1];
    verts[2] = pt2.v.m128_f32[2];
    indices[1] = 1;
    verts += 3;
    verts[0] = pt4.v.m128_f32[0];
    verts[1] = pt4.v.m128_f32[1];
    verts[2] = pt4.v.m128_f32[2];
    indices[2] = 2;
    verts += 3;
    verts[0] = pt3.v.m128_f32[0];
    verts[1] = pt3.v.m128_f32[1];
    verts[2] = pt3.v.m128_f32[2];
    indices[3] = 3;
    j_nullsub_67(ScratchSection);
    j_nullsub_27(ScratchSection);
    nglShaderParamSet* shader_params = (nglShaderParamSet*)nglListAlloc(
        4 * nglShaderParamSet::NumParams + 8, 8u);
    shader_params->Array[0] = 0;
    shader_params->Array[1] = 0;
    setup_color(col, *shader_params);
    math::Mat43 mtx;
    mtx.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    mtx.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    mtx.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    mtx.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    nglMesh* v14 = auxCloseScratchMesh(ScratchMesh);
    nglListAddMesh(v14, mtx, nullptr, shader_params, nullptr);
}

// ea: 0x006D5A70 (8-arg overload; c2/c3/c4 unused per disasm)
void DebugRender::RenderQuad(const math::Position3& p1,
                             const math::Position3& p2,
                             const math::Position3& p3,
                             const math::Position3& p4, const Color& c1,
                             const Color& c2, const Color& c3,
                             const Color& c4, bool)
{
    (void)c2; (void)c3; (void)c4;
    RenderQuad(p1, p2, p3, p4, c1, false);
}

// q_math helpers
void PerpendicularVector(float* dst, const float* src);  // q_math.cpp
void CrossProduct(const float* v1, const float* v2, float* cross);  // q_math.cpp

// trGlobals_t view (viewParms.world.modelMatrix +0x8C)
struct orientationr_t {
    float origin[3];       // +0x00
    float axis[3][3];      // +0x0C
    float viewOrigin[3];   // +0x30
    float modelMatrix[16]; // +0x3C
};
struct viewParms_t {
    uint8_t _pad[0x7C];
    orientationr_t world;  // +0x7C
};
struct trGlobals_t {
    uint8_t _pad0[0x10];
    viewParms_t viewParms;  // +0x10
};
extern trGlobals_t tr;  // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

// ea: 0x006D8790
void DebugRender::RenderPoint(const math::Position3& pt, const Color& col,
                              float thickness)
{
    math::Position3 v8;
    v8.v.m128_f32[0] = tr.viewParms.world.modelMatrix[1];
    v8.v.m128_f32[1] = tr.viewParms.world.modelMatrix[5];
    v8.v.m128_f32[2] = tr.viewParms.world.modelMatrix[9];
    v8.v.m128_f32[3] = 0.0f;
    float half = thickness * 0.5f;
    __m128 v5 = _mm_mul_ps(v8.v, _mm_set1_ps(half));
    math::Position3 v7;
    v7.v = _mm_add_ps(pt.v, v5);
    v8.v = _mm_sub_ps(pt.v, v5);
    DebugRender::RenderLine(v7, v8, col, thickness);
}

// ea: 0x006D8830
void DebugRender::RenderLineBox(const math::Position3& tl,
                                const math::Position3& br, const Color& col,
                                float thickness)
{
    math::Position3 c[8];
    c[0].v = _mm_setr_ps(tl.v.m128_f32[0], tl.v.m128_f32[1], tl.v.m128_f32[2], 0.0f);
    c[1].v = _mm_setr_ps(br.v.m128_f32[0], tl.v.m128_f32[1], tl.v.m128_f32[2], 0.0f);
    c[2].v = _mm_setr_ps(br.v.m128_f32[0], tl.v.m128_f32[1], br.v.m128_f32[2], 0.0f);
    c[3].v = _mm_setr_ps(tl.v.m128_f32[0], tl.v.m128_f32[1], br.v.m128_f32[2], 0.0f);
    c[4].v = _mm_setr_ps(tl.v.m128_f32[0], br.v.m128_f32[1], tl.v.m128_f32[2], 0.0f);
    c[5].v = _mm_setr_ps(br.v.m128_f32[0], br.v.m128_f32[1], tl.v.m128_f32[2], 0.0f);
    c[6].v = _mm_setr_ps(br.v.m128_f32[0], br.v.m128_f32[1], br.v.m128_f32[2], 0.0f);
    c[7].v = _mm_setr_ps(tl.v.m128_f32[0], br.v.m128_f32[1], br.v.m128_f32[2], 0.0f);
    for (int i = 0; i < 4; ++i)
    {
        DebugRender::RenderLine(c[i], c[(i + 1) & 3], col, thickness);
        DebugRender::RenderLine(c[i + 4], c[((i + 1) & 3) + 4], col, thickness);
        DebugRender::RenderLine(c[i], c[i + 4], col, thickness);
    }
}

// ea: 0x006D90E0
void DebugRender::RenderBeamCube(const math::Position3& pt, float radius,
                                 const Color& col, float thickness)
{
    __m128 v5 = _mm_mul_ps(_mm_set1_ps(0.57735026f), _mm_set1_ps(radius));
    math::Position3 v7;
    v7.v = _mm_sub_ps(pt.v, v5);
    math::Position3 v8;
    v8.v = _mm_add_ps(pt.v, v5);
    DebugRender::RenderLineBox(v7, v8, col, thickness);
}

// ea: 0x006D9170
void DebugRender::RenderCircle(const math::Position3& center, float radius,
                               const Color& color, float thickness)
{
    (void)thickness;
    if (nglBuildScene == nullptr || nglBuildScene->Parent == nullptr)
        return;
    float up[3] = { 0.0f, 0.0f, 1.0f };
    float normal[3];
    VectorNormalize2(up, normal);
    float dir[3];
    PerpendicularVector(dir, normal);
    float v17[3];
    CrossProduct(normal, dir, v17);
    float pts[16][3];
    float fSin, fCos;
    for (int i = 0; i < 16; ++i)
    {
        FastSinCos(i * 0.39269909f, &fSin, &fCos);
        float s = fSin * radius;
        float c = fCos * radius;
        pts[i][0] = ((dir[0] * c) + (v17[0] * s)) + center.v.m128_f32[0];
        pts[i][1] = ((dir[1] * c) + (v17[1] * s)) + center.v.m128_f32[1];
        pts[i][2] = ((dir[2] * c) + (v17[2] * s)) + center.v.m128_f32[2];
    }
    for (int i = 0; i < 16; ++i)
    {
        int j = (i + 1) & 0xF;
        math::Position3 p1;
        p1.v = _mm_setr_ps(pts[i][0], pts[i][1], pts[i][2], 0.0f);
        math::Position3 p2;
        p2.v = _mm_setr_ps(pts[j][0], pts[j][1], pts[j][2], 0.0f);
        DebugRender::RenderLine(p1, p2, color, 1.0f);
    }
}

// ea: 0x006DBB70
void DebugRender::RenderPoly(int count, const math::Position3* vertices,
                             const Color& col)
{
    if (count > 0)
    {
        const math::Position3* v3 = vertices;
        const math::Position3* v5 = &vertices[count - 1];
        for (int i = count; i != 0; --i)
        {
            DebugRender::RenderLine(*v5, *v3, col, 0.050000001f);
            v5 = v3++;
        }
    }
}

// ============================================================================
// DebugRender::Render - ea: 0x006D9490 (deferred primitive flush)
// ============================================================================
void DebugRender::Render()
{
    for (int i = 0; i < mRenderFpList.m_size; ++i)
        mRenderFpList.m_elements[i]();
    profile_reset();

    for (DebugSphere* s = gDebugSpheres.mElements;
         s != &gDebugSpheres.mElements[gDebugSpheres.mSize]; ++s)
    {
        math::Position3 pos;
        pos.v = _mm_setr_ps(s->mPos.x, s->mPos.y, s->mPos.z, 0.0f);
        DebugRender::RenderSphere(pos, s->mRadius, s->mArgbColor);
    }
    gDebugSpheres.resize(0);

    for (DebugLine* ln = gDebugLines.mElements;
         ln != &gDebugLines.mElements[gDebugLines.mSize]; ++ln)
    {
        math::Position3 p1;
        p1.v = _mm_setr_ps(ln->mPt1.x, ln->mPt1.y, ln->mPt1.z, 0.0f);
        math::Position3 p2;
        p2.v = _mm_setr_ps(ln->mPt2.x, ln->mPt2.y, ln->mPt2.z, 0.0f);
        DebugRender::RenderLine(p1, p2, ln->mCol, ln->mThickness);
    }
    gDebugLines.resize(0);

    for (DebugTri* t = gDebugTris.mElements;
         t != &gDebugTris.mElements[gDebugTris.mSize]; ++t)
    {
        math::Position3 p1;
        p1.v = _mm_setr_ps(t->mPt1.x, t->mPt1.y, t->mPt1.z, 0.0f);
        math::Position3 p2;
        p2.v = _mm_setr_ps(t->mPt2.x, t->mPt2.y, t->mPt2.z, 0.0f);
        math::Position3 p3;
        p3.v = _mm_setr_ps(t->mPt3.x, t->mPt3.y, t->mPt3.z, 0.0f);
        DebugRender::RenderTriangle(p1, p2, p3, t->mCol, true);
    }
    gDebugTris.resize(0);

    for (DebugTexturedQuad2D* q = gDebugTexturedQuad2Ds.mElements;
         q != &gDebugTexturedQuad2Ds.mElements[gDebugTexturedQuad2Ds.mSize];
         ++q)
    {
        DebugRender::RenderTexturedQuad2D(q->l, q->t, q->r, q->b, q->z,
                                          q->col, q->nglTex);
    }
    gDebugTexturedQuad2Ds.resize(0);
}

// ============================================================================
// LightGridMgr::RenderDebugText - ea: 0x006DB240
// ============================================================================
void LightGridMgr::RenderDebugText()  // ?RenderDebugText@LightGridMgr@@QAEXXZ @ 0x6DB240
{
    Color col(1.0f, 1.0f, 1.0f, 1.0f);
    math::Position3 cameraPos = nglGetMatrix_ViewToWorld(nglBuildScene)->w;
    int cellIndex = 0;
    LightGrid::TOC* toc = GetLightGrid(cameraPos, &cellIndex);
    if (toc == nullptr)
        return;

    int v8 = 0;
    LightGrid::Cell* cell = toc->mCells;
    if (toc->mNumCells > 0)
    {
        while (cell->mCellIndex != (unsigned int)cellIndex)
        {
            ++v8;
            if (v8 >= toc->mNumCells)
                return;
            ++cell;
        }
        char tmp[256];
        sprintf(tmp, "Camera Position: %.2f %.2f %.2f",
                cameraPos.v.m128_f32[0], cameraPos.v.m128_f32[1],
                cameraPos.v.m128_f32[2]);
        DebugRender::RenderText(tmp, 25, 15, col, 0.0f, 0.6f);

        float gx = cameraPos.v.m128_f32[0] - cell->mBase.x;
        float gy = cameraPos.v.m128_f32[1] - cell->mBase.y;
        float ax = gx / cell->mXGridDelta;
        float ay = gy / cell->mYGridDelta;
        int xi = (int)ax;
        int yi = (int)ay;
        int maxX = cell->mNumXRows - 1;
        int maxY = cell->mNumYRows - 1;
        if (ax < 0.0f)
        {
            ax = 0.0f;
            xi = 0;
        }
        if (xi > maxX)
        {
            xi = maxX;
            if (ax >= (float)(maxX + 1))
                ax = (float)(maxX + 1) - 0.001f;
        }
        if (ay < 0.0f)
        {
            ay = 0.0f;
            yi = 0;
        }
        if (yi > maxY)
        {
            yi = maxY;
            if (ay >= (float)(maxY + 1))
                ay = (float)(maxY + 1) - 0.001f;
        }
        float fx = ax - xi;
        float fy = ay - yi;
        unsigned int gridBase =
            cell->mFirstGridPoint + (unsigned int)xi
            + (unsigned int)yi * cell->mNumXRows;
        const LightGrid::GridPoint* gp[4];
        gp[0] = &toc->mGridPoints[gridBase];
        gp[1] = &toc->mGridPoints[gridBase + 1];
        gp[2] = &toc->mGridPoints[gridBase + cell->mNumXRows];
        gp[3] = &toc->mGridPoints[gridBase + cell->mNumXRows + 1];
        float w0 = (1.0f - fx) * (1.0f - fy);
        float w1 = fx * (1.0f - fy);
        float w2 = (1.0f - fx) * fy;
        float w3 = fx * fy;
        float weights[4] = { w0, w1, w2, w3 };
        math::Position3 ambient[4];
        GetAmbientColors(gp, ambient);
        math::Position3 ave;
        ave.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(ambient[0].v, _mm_set1_ps(w0)),
                _mm_mul_ps(ambient[1].v, _mm_set1_ps(w1))),
            _mm_add_ps(
                _mm_mul_ps(ambient[2].v, _mm_set1_ps(w2)),
                _mm_mul_ps(ambient[3].v, _mm_set1_ps(w3))));

        const PakInfoNode* pakInfo =
            StreamZoneManager::sInst->GetCellPakInfo(cellIndex);
        const char* zoneName = pakInfo->longName.mBlock != nullptr
            ? Broc::string::Block::GetBuff(pakInfo->longName.mBlock) : "";
        sprintf(tmp, "Zone: %s", zoneName);
        DebugRender::RenderText(tmp, 25, 29, col, 0.0f, 0.6f);
        sprintf(tmp, "CellIndex: %d", cellIndex);
        DebugRender::RenderText(tmp, 25, 43, col, 0.0f, 0.6f);
        sprintf(tmp, "CellRowsXY: %d %d", cell->mNumXRows, cell->mNumYRows);
        DebugRender::RenderText(tmp, 25, 57, col, 0.0f, 0.6f);
        sprintf(tmp, "GridXY: %d %d %d", xi, yi, gridBase);
        DebugRender::RenderText(tmp, 25, 71, col, 0.0f, 0.6f);
        sprintf(tmp, "amb0: %.3f %.3f %.3f",
                ambient[0].v.m128_f32[0], ambient[0].v.m128_f32[1],
                ambient[0].v.m128_f32[2]);
        DebugRender::RenderText(tmp, 25, 99, col, 0.0f, 0.6f);
        sprintf(tmp, "amb1: %.3f %.3f %.3f",
                ambient[1].v.m128_f32[0], ambient[1].v.m128_f32[1],
                ambient[1].v.m128_f32[2]);
        DebugRender::RenderText(tmp, 25, 113, col, 0.0f, 0.6f);
        sprintf(tmp, "amb2: %.3f %.3f %.3f",
                ambient[2].v.m128_f32[0], ambient[2].v.m128_f32[1],
                ambient[2].v.m128_f32[2]);
        DebugRender::RenderText(tmp, 25, 127, col, 0.0f, 0.6f);
        sprintf(tmp, "amb3: %.3f %.3f %.3f",
                ambient[3].v.m128_f32[0], ambient[3].v.m128_f32[1],
                ambient[3].v.m128_f32[2]);
        DebugRender::RenderText(tmp, 25, 141, col, 0.0f, 0.6f);
        sprintf(tmp, "ave: %.3f %.3f %.3f",
                ave.v.m128_f32[0], ave.v.m128_f32[1], ave.v.m128_f32[2]);
        DebugRender::RenderText(tmp, 25, 155, col, 0.0f, 0.6f);

        ae_sized_array<LightGrid::LightIndex, 12> lights;
        lights.m_size = 0;
        GetLightListForGrid(*toc, gp, weights, lights);
        DebugRender::RenderText("Lights", 25, 183, col, 0.0f, 0.6f);
        int ypos = 197;
        for (int l = 0; l < lights.m_size && l < 3; ++l)
        {
            int bestIdx = -1;
            unsigned int bestAtten = 0;
            for (int k = 0; k < lights.m_size; ++k)
            {
                if (lights.m_elements[k].mAttenuationInt > bestAtten)
                {
                    bestAtten = lights.m_elements[k].mAttenuationInt;
                    bestIdx = k;
                }
            }
            if (bestIdx < 0)
                break;
            lights.m_elements[bestIdx].mAttenuationInt = 0;
            LightGrid::Light* light = &toc->mLights[lights.m_elements[bestIdx].mIndex];
            Color lightCol(light->mColor.x / 65535.0f,
                           light->mColor.y / 65535.0f,
                           light->mColor.z / 65535.0f, 1.0f);
            if (light->mPosition.w == 0.0f)
            {
                sprintf(tmp, "dir: %.2f %.2f %.2f",
                        light->mPosition.x, light->mPosition.y,
                        light->mPosition.z);
            }
            else
            {
                math::Position3 lightPos;
                lightPos.v = _mm_setr_ps(light->mPosition.x,
                                         light->mPosition.y,
                                         light->mPosition.z, 0.0f);
                math::Position3 end;
                end.v = _mm_setr_ps(cameraPos.v.m128_f32[0],
                                    cameraPos.v.m128_f32[1],
                                    cameraPos.v.m128_f32[2] - 55.0f, 0.0f);
                DebugRender::RenderSphere(lightPos, 2.0f, lightCol);
                Color white(1.0f, 1.0f, 1.0f, 1.0f);
                DebugRender::RenderLine(end, lightPos, white, 0.5f);
                sprintf(tmp, "pos: %.2f %.2f %.2f",
                        light->mPosition.x, light->mPosition.y,
                        light->mPosition.z);
            }
            DebugRender::RenderText(tmp, 25, ypos, col, 0.0f, 0.6f);
            ypos += 14;
            sprintf(tmp, "col: %.2f %.2f %.2f",
                    lightCol.r, lightCol.g, lightCol.b);
            DebugRender::RenderText(tmp, 25, ypos, col, 0.0f, 0.6f);
            ypos += 14;
        }
    }
}
