// ============================================================================
// tr_wheelmark.cpp - render.o dynamic decals + wheel marks (tr_decal.cpp,
// WheelMark.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/color.h"

#include <stdint.h>

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
};

// WheelMarkMgr statics (render.o data; protected -> @@1)
class WheelMarkMgr {
    friend class WheelMark;
protected:
    static unsigned int NMarks;              // ?NMarks@WheelMarkMgr@@1IA (defined in tr_stats.cpp)
    static cdWheelMarkShaderMat* Material;   // ?Material@WheelMarkMgr@@1PAVcdWheelMarkShaderMat@@A @ 0xF7419C
};

cdWheelMarkShaderMat* WheelMarkMgr::Material;

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

    void Update(float deltaTime);      // ?Update@DynamicDecalSet@@QAEXM@Z
    int FindFreeDecal();               // ?FindFreeDecal@DynamicDecalSet@@QAEHXZ
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
