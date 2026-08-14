// ============================================================================
// tr_wheelmark.cpp - render.o dynamic decals + wheel marks (tr_decal.cpp,
// WheelMark.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/color.h"
#include "core/mem_heap.h"
#include "core/tlFixedString.h"
#include "render/cdDynamicDecalShader.h"
#include "render/cdWheelMarkShader.h"

#include <stdint.h>

// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

// ?to_color32@Color@@QBE?AVColor32@@XZ (render.o 0x6E5A70; inline COMDAT)
Color32 Color::to_color32() const
{
    if (r < 0.0f || r > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/Color.h";
        AeAssert::gCurrentLine = 152;
        AeAssert::gCurrentExpr = "r>=0.0f && r<=1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Need assert message"))
            __debugbreak();
    }
    if (g < 0.0f || g > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/Color.h";
        AeAssert::gCurrentLine = 153;
        AeAssert::gCurrentExpr = "g>=0.0f && g<=1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Need assert message"))
            __debugbreak();
    }
    if (b < 0.0f || b > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/Color.h";
        AeAssert::gCurrentLine = 154;
        AeAssert::gCurrentExpr = "b>=0.0f && b<=1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Need assert message"))
            __debugbreak();
    }
    if (a < 0.0f || a > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/Color.h";
        AeAssert::gCurrentLine = 155;
        AeAssert::gCurrentExpr = "a>=0.0f && a<=1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Need assert message"))
            __debugbreak();
    }
    Color32 result;
    result.c.b = (unsigned char)(b * 255.0f);
    result.c.g = (unsigned char)(g * 255.0f);
    result.c.a = (unsigned char)(a * 255.0f);
    result.c.r = (unsigned char)(r * 255.0f);
    return result;
}

struct nglMesh;
struct nglMeshSection;
struct nglMaterial;
struct gpuVertexFormat;
class cdWheelMarkShaderMat;

// ngl mesh helpers (ngl_meshedit.o / ngl_mesh.o)
nglMesh* nglCreateMesh(unsigned int Flags, unsigned int NSections);
nglMeshSection* nglCreateSection(int Prim, int NIndices, int NVertices,
                                 gpuVertexFormat* VertexFormat);
void nglAddMeshSection(nglMesh* Mesh, nglMeshSection* Section,
                       nglMaterial* Material, int Flags);
void nglMakeSectionUnique(nglMesh* Mesh, int SectionIdx);
void nglDestroyMesh(nglMesh* Mesh);

extern gpuVertexFormat cdWheelMarkVertexFormat;  // cdWheelMarkShader.cpp

// wheel_e (IDA enum; codmp_xboxr.xbe.h)
enum wheel_e {
    WHEEL_FL = 0x0,
    WHEEL_FR = 0x1,
    WHEEL_BL = 0x2,
    WHEEL_BR = 0x3,
    WHEEL_ML = 0x4,
    WHEEL_MR = 0x5,
    WHEEL_MBL = 0x6,
    WHEEL_MBR = 0x7,
    WHEELCount = 0x8,
    WHEELMin = 0x0,
    WHEELMax = 0x7,
    WHEELInvalid = 0xFFFFFFFF,
};

// cdWheelMarkVertex (IDA type; size 0x10)
struct cdWheelMarkVertex {
    float Position_x;    // +0x00
    float Position_y;    // +0x04
    float Position_z;    // +0x08
    unsigned int TexCoord;  // +0x0C
};

// WheelMark (IDA type; size 0x70)
class Entity;
class WheelMark {
public:
    bool Active;            // +0x00
    bool StitchStrips;      // +0x01
    unsigned int TimeStamp; // +0x04
    Entity* Owner;          // +0x08
    wheel_e Wheel;          // +0x0C
    nglMesh* Mesh;          // +0x10
    nglMeshSection* Section;// +0x14
    math::Position3 LastPos;    // +0x20
    math::Position3 StripEnd;   // +0x30
    math::Position3 SplashPos;  // +0x40
    unsigned int NumVerts;      // +0x50
    unsigned int LastVert;      // +0x54
    unsigned int PrevMaterial;  // +0x58
    float LastV;                // +0x5C
    float LastAlpha;            // +0x60
    cdWheelMarkVertex* VertexBuffer;  // +0x64
    cdWheelMarkVertex* PrevVertex0;   // +0x68
    cdWheelMarkVertex* PrevVertex1;   // +0x6C

    void CreateMesh();   // ?CreateMesh@WheelMark@@QAEXXZ
    void DestroyMesh();  // ?DestroyMesh@WheelMark@@QAEXXZ
    void New();          // ?New@WheelMark@@QAEXXZ
    void UpdateSplash(const math::Position3& Pos);  // ?UpdateSplash@WheelMark@@QAEXABVPosition3@math@@@Z
    void Stop();         // ?Stop@WheelMark@@QAEXXZ
    void Assign(Entity* owner, wheel_e wheel);  // ?Assign@WheelMark@@QAEXPAVEntity@@W4wheel_e@@@Z
    void AddPoint(const math::Position3& Pos, const math::Dir3& Normal,
                  float Spacing, float Width, unsigned int Material,
                  float Alpha);  // ?AddPoint@WheelMark@@QAEXABVPosition3@math@@ABVDir3@3@MMIM@Z
};

// WheelMarkMgr statics (render.o data; protected -> @@1)
class WheelMarkMgr {
    friend class WheelMark;
protected:
    static unsigned int NMarks;              // ?NMarks@WheelMarkMgr@@1IA (defined in tr_stats.cpp)
    static cdWheelMarkShaderMat* Material;   // ?Material@WheelMarkMgr@@1PAVcdWheelMarkShaderMat@@A @ 0xF7419C
    static WheelMark Marks[16];              // ?Marks@WheelMarkMgr@@1PAVWheelMark@@A @ 0xF74490
public:
    static void Init();                      // ?Init@WheelMarkMgr@@SAXXZ
    static void Exit();                      // ?Exit@WheelMarkMgr@@SAXXZ
};

cdWheelMarkShaderMat* WheelMarkMgr::Material;
WheelMark WheelMarkMgr::Marks[16];

// PostEffectEventScriptCall (sret Handle; game.o)
class Handle {
public:
    unsigned int mVal;
};
namespace Broc {
struct vector {
    float x;
    float y;
    float z;
};
}
enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };
Handle PostEffectEventScriptCall(const Entity* ent, const char* scriptId,
                                 const Broc::vector& pos,
                                 const Broc::vector& facing, bool queue,
                                 TPakId pakid, bool important);

// DynamicDecalSet::Decal (IDA type; size 0x60)
class DynamicDecalSet {
public:
    struct Decal {
        math::Position3 mPos;          // +0x00
        math::Dir3 mXoffset;           // +0x10
        math::Dir3 mYoffset;           // +0x20
        float mUstart;                 // +0x30
        float mUend;                   // +0x34
        Color mColor;                  // +0x38
        float mFadeOut;                // +0x48
        float mAge;                    // +0x4C
        float mCulledTime;             // +0x50
        bool mIsCulled;                // +0x54
        bool mIsActive;                // +0x55
        bool mDoFadeOut;               // +0x56
        bool mIsHighPriority;          // +0x57
        bool mIsJustAdded;             // +0x58
    };
    static_assert(sizeof(Decal) == 0x60, "Decal size mismatch");

    Decal* mDecals;                    // +0x00
    int mMaxNumDecals;                 // +0x04
    int mNumDecals;                    // +0x08
    int mOldestDecal;                  // +0x0C
    int mNextFree[4];                  // +0x10
    float mOnScreenReplacementScore;   // +0x20
    float mOffScreenReplacementScore;  // +0x24
    int mOnScreenReplacement;          // +0x28
    int mOffScreenReplacement;         // +0x2C
    int mNumOnScreen;                  // +0x30
    float mOldestOnScreenAge;          // +0x34
    int mOldestOnScreen;               // +0x38
    math::Mat43* mViewToWorldMtx;      // +0x3C
    cdDynamicDecalShaderMat mMaterial;  // +0x40

    void Update(float deltaTime);      // ?Update@DynamicDecalSet@@QAEXM@Z
    int FindFreeDecal();               // ?FindFreeDecal@DynamicDecalSet@@QAEHXZ
    DynamicDecalSet(nglTexture* texture, float zBias, bool alphaBlend,
                    int maxNum);       // ??0DynamicDecalSet@@QAE@PAUnglTexture@@M_NH@Z
    ~DynamicDecalSet();                // ??1DynamicDecalSet@@QAE@XZ
    void Render();                     // ?Render@DynamicDecalSet@@QAEXXZ
};

// ============================================================================
// DynamicDecalSet::Update - ea: 0x006C3790
// ============================================================================
void DynamicDecalSet::Update(float deltaTime)
{
    mNextFree[0] = -1;
    mNextFree[1] = -1;
    mNextFree[2] = -1;
    mNextFree[3] = -1;
    int freeIdx = 0;
    int i = 0;
    if (mNumDecals > 0)
    {
        int* mNextFreePtr = mNextFree;
        do
        {
            Decal* decal = &mDecals[i];
            if (decal->mIsActive)
            {
                if (decal->mIsCulled)
                {
                    bool notHighPriority = !decal->mIsHighPriority;
                    float culled = decal->mCulledTime + deltaTime;
                    decal->mCulledTime = culled;
                    float limit = 120.0f;
                    if (notHighPriority)
                        limit = 60.0f;
                    if (culled > limit)
                        decal->mDoFadeOut = true;
                }
                else
                {
                    decal->mCulledTime = 0.0f;
                }
                if (decal->mDoFadeOut)
                {
                    float fade = decal->mFadeOut - (deltaTime * 0.1f);
                    decal->mFadeOut = fade;
                    if (fade <= 0.0f)
                        decal->mIsActive = false;
                }
                mDecals[i].mAge = mDecals[i].mAge + deltaTime;
            }
            else if (freeIdx != 4)
            {
                *mNextFreePtr = i;
                ++freeIdx;
                ++mNextFreePtr;
            }
            ++i;
        } while (i < mNumDecals);
    }
}

// ============================================================================
// DynamicDecalSet::FindFreeDecal - ea: 0x006C3880
// ============================================================================
int DynamicDecalSet::FindFreeDecal()
{
    int v1 = 0;
    int* mNextFreePtr = mNextFree;
    do
    {
        if (*mNextFreePtr != -1)
            break;
        ++v1;
        ++mNextFreePtr;
    } while (v1 < 4);
    if (v1 == 4)
        return -1;
    return mNextFree[v1];
}

// ============================================================================
// DynamicDecalSet ctor/dtor - ea: 0x006C3710 / 0x006C3770
// ============================================================================
DynamicDecalSet::DynamicDecalSet(nglTexture* texture, float zBias,
                                 bool alphaBlend, int maxNum)
{
    mMaxNumDecals = maxNum;
    mNumDecals = 0;
    mOldestDecal = 0;
    new (&mMaterial) cdDynamicDecalShaderMat();
    mDecals = (Decal*)mem_heap_malloc(16, 96 * maxNum);
    mViewToWorldMtx = (math::Mat43*)mem_heap_malloc(16, 0x40u);
    mMaterial.mTexture = texture;
    mMaterial.mZbias = zBias;
    mMaterial.mAlphaBlend = alphaBlend;
}

DynamicDecalSet::~DynamicDecalSet()
{
    mem_heap_free(mViewToWorldMtx);
    mem_heap_free(mDecals);
}

// ============================================================================
// DynamicDecalSet::Render - ea: 0x006C8720
// ============================================================================
struct nglScene;
extern nglScene* nglBuildScene;  // ?nglBuildScene@@3PAUnglScene@@A
const math::Mat43* nglGetMatrix_ViewToWorld(nglScene* Scene);
nglMesh* auxCreateScratchMesh(int flags, int num);         // ?auxCreateScratchMesh@@YAPAUnglMesh@@HH@Z
nglMesh* auxCloseScratchMesh(nglMesh* m);                  // ?auxCloseScratchMesh@@YAPAUnglMesh@@PAU1@@Z
nglMeshSection* nglCreateScratchSection(int Prim, int NIndices, int NVertices,
                                        gpuVertexFormat* VertexFormat);
void nglUnlockSectionIndices(void);
void nglSetMeshSphere(nglMesh* Mesh, const math::Position3* Center, float Radius);
extern gpuVertexFormat cdDynamicDecalVertexFormat;  // cdDynamicDecalVertexDef.cpp
nglMeshNode* nglListAddMesh(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                            nglMeshParams* MeshParams,
                            nglShaderParamSet* ShaderParams,
                            void (*fn)(nglMeshNode*));

// fast cos via floor magic (Float4_FloorMagic_10 = 12582912.0)
static float FastCosAng(float radians)
{
    float v14 = -fabsf(radians) * 0.15915494f;
    float magic = 12582912.0f;
    float v13 = fabsf(((v14 - magic) + magic) - v14 - 0.5f) - 0.25f;
    float v15 = v13 * v13;
    return (((((v13 * (v15 * v15)) * (v15 * v15)) * 39.710659f)
             + (((v13 * v15) * (v15 * v15)) * -76.574959f))
            + ((v13 * (v15 * v15)) * 81.602226f))
           + ((v13 * v15) * -41.341675f)
           + (v13 * 6.283185f);
}

void DynamicDecalSet::Render()
{
    const math::Mat43* Matrix_ViewToWorld =
        nglGetMatrix_ViewToWorld(nglBuildScene);
    mViewToWorldMtx->x = Matrix_ViewToWorld->x;
    mViewToWorldMtx->y = Matrix_ViewToWorld->y;
    mViewToWorldMtx->z = Matrix_ViewToWorld->z;
    mViewToWorldMtx->w = Matrix_ViewToWorld->w;

    mOnScreenReplacementScore = 0.0f;
    mOffScreenReplacement = -1;
    mOnScreenReplacement = -1;
    mOldestOnScreenAge = 0.0f;
    mOldestOnScreen = -1;

    int visibleCount = 0;
    if (mNumDecals > 0)
    {
        __m128 camPos = mViewToWorldMtx->w.v;
        __m128 viewZ = mViewToWorldMtx->z.v;
        mNumOnScreen = 0;

        for (int i = 0; i < mNumDecals; ++i)
        {
            Decal* decal = &mDecals[i];
            if (decal->mIsActive)
            {
                __m128 v11 = _mm_sub_ps(decal->mPos.v, camPos);
                __m128 v12 = _mm_mul_ps(v11, v11);
                float distSq = v12.m128_f32[0]
                             + (_mm_shuffle_ps(v12, v12, 85).m128_f32[0]
                                + _mm_shuffle_ps(v12, v12, 170).m128_f32[0]);
                if (distSq <= 1394997.1f)
                {
                    float dist = sqrtf(distSq);
                    __m128 dir = _mm_div_ps(v11, _mm_set1_ps(dist));
                    __m128 v13 = _mm_mul_ps(viewZ, dir);
                    float angDot = v13.m128_f32[0]
                                 + (_mm_shuffle_ps(v13, v13, 85).m128_f32[0]
                                    + _mm_shuffle_ps(v13, v13, 170).m128_f32[0]);
                    float cosFov = FastCosAng(nglBuildScene->FOV * 0.017453292f);
                    if (cosFov <= angDot)
                    {
                        decal->mIsCulled = false;
                        ++visibleCount;
                    }
                    else
                    {
                        decal->mIsCulled = true;
                    }
                }
                else
                {
                    decal->mIsCulled = true;
                }

                float age = decal->mAge;
                if (decal->mIsCulled)
                {
                    if (age > mOffScreenReplacementScore)
                    {
                        mOffScreenReplacementScore = age;
                        mOffScreenReplacement = i;
                    }
                }
                else
                {
                    if (age > mOnScreenReplacementScore)
                    {
                        mOnScreenReplacementScore = age;
                        mOnScreenReplacement = i;
                    }
                    if (!decal->mDoFadeOut)
                    {
                        if (age > mOldestOnScreenAge)
                        {
                            mOldestOnScreenAge = age;
                            mOldestOnScreen = i;
                        }
                        ++mNumOnScreen;
                    }
                }
            }
        }
    }

    if (mOldestOnScreen != -1)
    {
        if (mNumOnScreen > mMaxNumDecals - mMaxNumDecals / 4)
            mDecals[mOldestOnScreen].mDoFadeOut = true;
    }

    if (visibleCount != 0)
    {
        nglMesh* mesh = auxCreateScratchMesh(0x40000, 1);
        nglMeshSection* section = nglCreateScratchSection(
            6, 6 * visibleCount - 2, 4 * visibleCount,
            &cdDynamicDecalVertexFormat);
        nglAddMeshSection(mesh, section, &mMaterial, 1);

        unsigned short* indices =
            (unsigned short*)nglLockSectionIndices(section);
        float* verts = (float*)nglLockSectionVertices(section);

        int v47 = 0;
        float* v21 = verts;
        float minX = 3.4028235e38f;
        float minY = 3.4028235e38f;
        float minZ = 3.4028235e38f;
        float maxX = -3.4028235e38f;
        float maxY = -3.4028235e38f;
        float maxZ = -3.4028235e38f;
        int anyWritten = 0;

        for (int i = 0; i < mNumDecals; ++i)
        {
            Decal* decal = &mDecals[i];
            if (decal->mIsActive && !decal->mIsCulled)
            {
                __m128 pos = decal->mPos.v;
                minX = fminf(minX, pos.m128_f32[0]);
                minY = fminf(minY, pos.m128_f32[1]);
                minZ = fminf(minZ, pos.m128_f32[2]);
                maxX = fmaxf(maxX, pos.m128_f32[0]);
                maxY = fmaxf(maxY, pos.m128_f32[1]);
                maxZ = fmaxf(maxZ, pos.m128_f32[2]);

                float savedA = decal->mColor.a;
                float blended = decal->mFadeOut * decal->mColor.a;
                decal->mColor.a = blended;
                Color32 col = decal->mColor.to_color32();
                decal->mColor.a = savedA;

                int v28 = v47;
                unsigned short* v29 = indices;
                if (v28 > 0)
                {
                    *v29 = (unsigned short)(v28 - 1);
                    v29[1] = (unsigned short)v28;
                    v29 += 2;
                }
                indices = v29;

                __m128 corner0 = _mm_sub_ps(_mm_sub_ps(pos, decal->mXoffset.v),
                                            decal->mYoffset.v);
                v21[0] = corner0.m128_f32[0];
                v21[1] = corner0.m128_f32[1];
                v21[2] = corner0.m128_f32[2];
                v21[3] = decal->mUstart;
                v21[4] = 0.0f;
                v21[5] = (float)col.i;
                *indices++ = (unsigned short)v28;

                __m128 corner1 = _mm_add_ps(_mm_sub_ps(pos, decal->mXoffset.v),
                                            decal->mYoffset.v);
                v21[6] = corner1.m128_f32[0];
                v21[7] = corner1.m128_f32[1];
                v21[8] = corner1.m128_f32[2];
                v21[9] = decal->mUstart;
                v21[10] = 1.0f;
                v21[11] = (float)col.i;
                *indices++ = (unsigned short)(v28 + 1);

                __m128 corner2 = _mm_sub_ps(_mm_add_ps(pos, decal->mXoffset.v),
                                            decal->mYoffset.v);
                v21[12] = corner2.m128_f32[0];
                v21[13] = corner2.m128_f32[1];
                v21[14] = corner2.m128_f32[2];
                v21[15] = decal->mUend;
                v21[16] = 0.0f;
                v21[17] = (float)col.i;
                *indices++ = (unsigned short)(v28 + 2);

                __m128 corner3 = _mm_add_ps(_mm_add_ps(pos, decal->mXoffset.v),
                                            decal->mYoffset.v);
                v21[18] = corner3.m128_f32[0];
                v21[19] = corner3.m128_f32[1];
                v21[20] = corner3.m128_f32[2];
                v21[21] = decal->mUend;
                v21[22] = 1.0f;
                v21[23] = (float)col.i;
                *indices++ = (unsigned short)(v28 + 3);

                v21 += 24;
                v47 = v28 + 4;
                anyWritten = 1;
            }
        }

        if (anyWritten)
        {
            nglUnlockSectionIndices();
            nglUnlockSectionVertices();

            math::Position3 center;
            center.v.m128_f32[0] = (minX + maxX) * 0.5f;
            center.v.m128_f32[1] = (minY + maxY) * 0.5f;
            center.v.m128_f32[2] = (minZ + maxZ) * 0.5f;
            float dx = maxX - minX;
            float dy = maxY - minY;
            float dz = maxZ - minZ;
            float radius = sqrtf(dx * dx + dy * dy + dz * dz);
            nglSetMeshSphere(mesh, &center, radius);

            math::Mat43 identity;
            identity.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
            identity.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
            identity.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
            identity.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
            nglMesh* m = auxCloseScratchMesh(mesh);
            nglListAddMesh(m, identity, nullptr, nullptr, nullptr);
        }
        else
        {
            auxCloseScratchMesh(mesh);
        }
    }
}

// ============================================================================
// WheelMark methods
// ============================================================================

// ea: 0x006C38C0
void WheelMark::CreateMesh()
{
    Mesh = nglCreateMesh(0x80000u, 1u);
    nglMeshSection* Section = nglCreateSection(6, 0, 1024, &cdWheelMarkVertexFormat);
    this->Section = Section;
    nglAddMeshSection(Mesh, Section, (nglMaterial*)WheelMarkMgr::Material, 1);
    nglMakeSectionUnique(Mesh, 0);
}

// ea: 0x006C3910
void WheelMark::DestroyMesh()
{
    nglDestroyMesh(Mesh);
}

// ea: 0x006C3920
void WheelMark::New()
{
    StripEnd.v.m128_f32[0] = 0.0f;
    StripEnd.v.m128_f32[1] = 0.0f;
    StripEnd.v.m128_f32[2] = 0.0f;
    StripEnd.v.m128_f32[3] = 0.0f;
    LastPos.v.m128_f32[0] = 0.0f;
    LastPos.v.m128_f32[1] = 0.0f;
    LastV = 0.0f;
    LastAlpha = 0.0f;
    Owner = nullptr;
    StitchStrips = false;
    Active = false;
    PrevMaterial = 0;
    NumVerts = 0;
    LastVert = 0;
    VertexBuffer = nullptr;
    TimeStamp = 0;
    PrevVertex1 = nullptr;
    PrevVertex0 = nullptr;
    Wheel = WHEELInvalid;
    LastPos.v.m128_f32[2] = 0.0f;
    SplashPos.v.m128_f32[0] = 0.0f;
    SplashPos.v.m128_f32[1] = 0.0f;
    LastPos.v.m128_f32[3] = 0.0f;
    SplashPos.v.m128_f32[2] = 0.0f;
    SplashPos.v.m128_f32[3] = 0.0f;
}

// ea: 0x006C39F0
void WheelMark::UpdateSplash(const math::Position3& Pos)
{
    __m128 v3 = _mm_sub_ps(Pos.v, SplashPos.v);
    __m128 v4 = _mm_mul_ps(v3, v3);
    float distSq = v4.m128_f32[0]
                 + (_mm_shuffle_ps(v4, v4, 85).m128_f32[0]
                    + _mm_shuffle_ps(v4, v4, 170).m128_f32[0]);
    if (distSq > 2500.0f)
    {
        Broc::vector facing;
        facing.x = 0.0f;
        facing.y = 0.0f;
        facing.z = 1.0f;
        Broc::vector pos;
        pos.x = Pos.v.m128_f32[0];
        pos.y = Pos.v.m128_f32[1];
        pos.z = Pos.v.m128_f32[2];
        PostEffectEventScriptCall(Owner, "water_splash_vehicle", pos, facing,
                                  false, (TPakId)-1, false);
        SplashPos = Pos;
    }
}

// ea: 0x006C3AB0
void WheelMark::Stop()
{
    cdWheelMarkVertex* PrevVertex0 = this->PrevVertex0;
    Active = false;
    if (PrevVertex0 != nullptr)
    {
        ((unsigned char*)&PrevVertex0->TexCoord)[3] = 0;
        ((unsigned char*)&this->PrevVertex1->TexCoord)[3] = 0;
        PrevVertex1 = nullptr;
        PrevVertex0 = nullptr;
    }
}

// ============================================================================
// WheelMark::Assign - ea: 0x006C8F80
// ============================================================================
void WheelMark::Assign(Entity* owner, wheel_e wheel)
{
    cdWheelMarkVertex* PrevVertex0 = this->PrevVertex0;
    Active = false;
    if (PrevVertex0 != nullptr)
    {
        ((unsigned char*)&PrevVertex0->TexCoord)[3] = 0;
        ((unsigned char*)&this->PrevVertex1->TexCoord)[3] = 0;
        PrevVertex1 = nullptr;
        PrevVertex0 = nullptr;
    }
    Owner = owner;
    Wheel = wheel;
}

// ============================================================================
// WheelMark::AddPoint - ea: 0x006C8FB0
// ============================================================================
void* nglLockSectionVertices(nglMeshSection* Section);    // ngl_dx_gpu.o
void nglUnlockSectionVertices();                           // ngl_dx_gpu.o

void WheelMark::AddPoint(const math::Position3& Pos,
                         const math::Dir3& Normal,
                         float Spacing, float Width,
                         unsigned int Material, float Alpha)
{
    __m128 P0_4 = _mm_sub_ps(Pos.v, LastPos.v);
    __m128 v8 = _mm_mul_ps(P0_4, P0_4);
    float dist = sqrtf(v8.m128_f32[0]
                     + (_mm_shuffle_ps(v8, v8, 85).m128_f32[0]
                        + _mm_shuffle_ps(v8, v8, 170).m128_f32[0]));
    if (Spacing > dist)
        return;
    if (dist > 100.0f)
    {
        cdWheelMarkVertex* PrevVertex0 = this->PrevVertex0;
        Active = false;
        if (PrevVertex0 != nullptr)
        {
            ((unsigned char*)&PrevVertex0->TexCoord)[3] = 0;
            ((unsigned char*)&this->PrevVertex1->TexCoord)[3] = 0;
            this->PrevVertex1 = nullptr;
            this->PrevVertex0 = nullptr;
        }
    }
    if (Active && PrevMaterial != Material)
    {
        cdWheelMarkVertex* v11 = this->PrevVertex0;
        Active = false;
        if (v11 != nullptr)
        {
            ((unsigned char*)&v11->TexCoord)[3] = 0;
            ((unsigned char*)&this->PrevVertex1->TexCoord)[3] = 0;
            this->PrevVertex1 = nullptr;
            this->PrevVertex0 = nullptr;
        }
    }
    TimeStamp = 0;
    LastPos.v = Pos.v;
    if (!Active)
    {
        PrevMaterial = Material;
        Active = true;
        if (NumVerts >= 2)
            StitchStrips = true;
    }
    else
    {
        cdWheelMarkVertex* v14 = (cdWheelMarkVertex*)nglLockSectionVertices(Section);
        LastAlpha = (LastAlpha * 0.89999998f) + (Alpha * 0.1f);
        float v15 = ((dist / (float)Width) * 0.5f) + LastV;
        float v16 = v15 * 16.0f;
        VertexBuffer = v14;
        LastV = v15;

        __m128 v17 = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(P0_4, P0_4, 9),
                       _mm_shuffle_ps(Normal.v, Normal.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(P0_4, P0_4, 18),
                       _mm_shuffle_ps(Normal.v, Normal.v, 9)));
        __m128 v18 = _mm_mul_ps(v17, v17);
        float crossLen = sqrtf(v18.m128_f32[0]
                             + (_mm_shuffle_ps(v18, v18, 85).m128_f32[0]
                                + _mm_shuffle_ps(v18, v18, 170).m128_f32[0]));
        __m128 v19 = _mm_mul_ps(_mm_div_ps(v17, _mm_set1_ps(crossLen)),
                                _mm_set1_ps((float)Width));
        __m128 v20 = _mm_add_ps(Pos.v, Normal.v);
        __m128 v21 = _mm_sub_ps(v20, v19);
        __m128 P0_4a = _mm_add_ps(v20, v19);

        unsigned char v35 = (unsigned char)((int)v16 & 0xFF);
        if (StitchStrips)
        {
            cdWheelMarkVertex* v23 = &v14[LastVert];
            LastVert = (LastVert + 1) & 0x3FF;
            ++NumVerts;
            v23->Position_x = StripEnd.v.m128_f32[0];
            v23->Position_y = StripEnd.v.m128_f32[1];
            v23->Position_z = StripEnd.v.m128_f32[2];
            v23->TexCoord = 0;
            cdWheelMarkVertex* v25 = &VertexBuffer[LastVert];
            LastVert = (LastVert + 1) & 0x3FF;
            ++NumVerts;
            v25->Position_x = v21.m128_f32[0];
            v25->Position_y = _mm_shuffle_ps(v21, v21, 85).m128_f32[0];
            v25->Position_z = _mm_shuffle_ps(v21, v21, 170).m128_f32[0];
            StitchStrips = false;
        }

        unsigned int alphaInt = (unsigned int)(LastAlpha * 255.0f);
        unsigned int v29 = alphaInt << 8;
        cdWheelMarkVertex* v27 = &VertexBuffer[LastVert];
        LastVert = (LastVert + 1) & 0x3FF;
        ++NumVerts;
        v27->Position_x = v21.m128_f32[0];
        v27->Position_y = _mm_shuffle_ps(v21, v21, 85).m128_f32[0];
        v27->Position_z = _mm_shuffle_ps(v21, v21, 170).m128_f32[0];
        unsigned int vertIdx0 = (unsigned int)(v27 - VertexBuffer);
        v27->TexCoord = Material
            + (((v35 + ((v29 + ((255 * vertIdx0) >> 10)) << 8)) << 8));
        PrevVertex0 = v27;

        cdWheelMarkVertex* v26 = &VertexBuffer[LastVert];
        LastVert = (LastVert + 1) & 0x3FF;
        ++NumVerts;
        v26->Position_x = P0_4a.m128_f32[0];
        v26->Position_y = _mm_shuffle_ps(P0_4a, P0_4a, 85).m128_f32[0];
        v26->Position_z = _mm_shuffle_ps(P0_4a, P0_4a, 170).m128_f32[0];
        unsigned int vertIdx1 = (unsigned int)(v26 - VertexBuffer);
        v26->TexCoord = Material + 1
            + (((v35 + ((v29 + ((255 * vertIdx1) >> 10)) << 8)) << 8));
        PrevVertex1 = v26;
        StripEnd.v = P0_4a;

        unsigned int v30 = NumVerts;
        if (v30 >= 0x3F8)
            v30 = 1016;
        NumVerts = v30;
        nglUnlockSectionVertices();
    }
}

// ============================================================================
// WheelMarkMgr::Init - ea: 0x006C8E50
// ============================================================================
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);
void nglAddMeshSection(nglMesh* Mesh, nglMeshSection* Section,
                       nglMaterial* Material, int Flags);
void nglMakeSectionUnique(nglMesh* Mesh, int SectionIdx);
nglTexture* nglGetTexture(const tlFixedString& FileName);

void WheelMarkMgr::Init()
{
    void* block = mem_heap_malloc(0x14u);
    cdWheelMarkShaderMat* v1;
    if (block != nullptr)
        v1 = new (block) cdWheelMarkShaderMat();
    else
        v1 = nullptr;
    WheelMarkMgr::Material = v1;
    tlFixedString FileName("tyretreads");
    WheelMarkMgr::Material->mTexture = nglGetTexture(FileName);
    nglMesh** p_Mesh = &WheelMarkMgr::Marks[0].Mesh;
    for (int i = 16; i != 0; --i)
    {
        *p_Mesh = nglCreateMesh(0x80000u, 1u);
        nglMeshSection* Section = nglCreateSection(6, 0, 1024, &cdWheelMarkVertexFormat);
        cdWheelMarkShaderMat* v5 = WheelMarkMgr::Material;
        p_Mesh[1] = (nglMesh*)Section;
        nglAddMeshSection(*p_Mesh, Section, (nglMaterial*)v5, 1);
        nglMakeSectionUnique(*p_Mesh, 0);
        p_Mesh += 28;
    }
    WheelMarkMgr::NMarks = 0;
}

// ============================================================================
// WheelMarkMgr::Exit - ea: 0x006C8F40
// ============================================================================
void WheelMarkMgr::Exit()
{
    for (unsigned int i = 0; i < 16; ++i)
        nglDestroyMesh(WheelMarkMgr::Marks[i].Mesh);
    mem_heap_free(WheelMarkMgr::Material);
    WheelMarkMgr::Material = nullptr;
}
