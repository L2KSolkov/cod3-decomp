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
    void SampleLightGrid(const LightGrid::TOC& toc, int cellidx,
                         const math::Position3& pos,
                         LightGridData* pLG);  // lightgrid.cpp
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
extern void* tlMemAlloc(unsigned int size, unsigned int align,
                        unsigned int flags);  // core.o
extern void tlMemFree(void* Ptr);             // core.o
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

template class ae_vector<DebugSphere>;
template class ae_vector<DebugTri>;

ae_vector<DebugSphere> gDebugSpheres;  // @ 0xF0C6C0
ae_vector<DebugTri> gDebugTris;        // @ 0xF0C6D8

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
};
static_assert(sizeof(DebugTexturedQuad2D) == 0x28, "DebugTexturedQuad2D size mismatch");

struct DebugQuadVector {
    DebugTexturedQuad2D* mElements;  // +0x00
    int mSize;                       // +0x04
    int mCapacity;                   // +0x08
    void push_back(const DebugTexturedQuad2D& e);
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
