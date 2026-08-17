// ============================================================================
// tr_fx3.cpp - render.o fx-list / spawn / plane helpers
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/ae_array.h"
#include "aeps/apsEffect.h"
#include "aeps/apsMemory.h"
#include "aeps/apsCommon.h"
#include "aeps/apsDebug.h"

#include <stdint.h>

struct nglShader;
struct cdAepsShader;

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10, MJK = 11 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

struct cdl_proftimer {
public:
    unsigned __int64 stamp;          // +0x00
    unsigned __int64 value;          // +0x08
    void start();                    // ?start@cdl_proftimer@@QAEXXZ
    void stop();                     // ?stop@cdl_proftimer@@QAEXXZ
    float get_elapsed();             // ?get_elapsed@cdl_proftimer@@QAEMXZ
};

// ============================================================================
// R_PlaneForSurface - ea: 0x006C1BC0
// ============================================================================
enum surfaceType_t {
    SF_BAD = 0x0,
    SF_SKIP = 0x1,
    SF_POLY = 0x2,
};
struct cplane_s {
    float normal[3];
    float dist;
    unsigned char type;
    unsigned char signbits;
    unsigned char pad[2];
};
const int PlaneFromPoints(float* const plane, const float* const a,
                          const float* const b, const float* const c);

void R_PlaneForSurface(surfaceType_t* surfType, cplane_s* plane)
{
    if (surfType == nullptr)
    {
        plane->normal[0] = 1.0f;
        plane->normal[1] = 0.0f;
        plane->normal[2] = 0.0f;
        plane->dist = 0.0f;
        plane->type = 0;
        plane->signbits = 0;
        return;
    }
    int type = *(int*)surfType;
    if (type >= 0x14)
    {
        // dlight-style surface: verts at +0x3C, index counts at +0x44/+0x46/+0x48
        const float* verts = *(const float**)((char*)surfType + 0x3C);
        int n0 = *(short*)((char*)surfType + 0x44);
        int n1 = *(short*)((char*)surfType + 0x46);
        int n2 = *(short*)((char*)surfType + 0x48);
        const float* a = verts;
        const float* b = verts + 12 * (n2 - n0);
        const float* c = verts + 12 * (n1 - n0);
        float plane4[4];
        PlaneFromPoints(plane4, a, b, c);
        plane->normal[0] = plane4[0];
        plane->normal[1] = plane4[1];
        plane->normal[2] = plane4[2];
        plane->dist = plane4[3];
    }
    else if (type == SF_POLY)
    {
        const float* a = *(const float**)((char*)surfType + 0x0C);
        float plane4[4];
        PlaneFromPoints(plane4, a, a + 8, a + 16);
        plane->normal[0] = plane4[0];
        plane->normal[1] = plane4[1];
        plane->normal[2] = plane4[2];
        plane->dist = plane4[3];
    }
    else
    {
        plane->normal[0] = 1.0f;
        plane->normal[1] = 0.0f;
        plane->normal[2] = 0.0f;
        plane->dist = 0.0f;
        plane->type = 0;
        plane->signbits = 0;
    }
}

// ============================================================================
// RemoveLight / Cmd_PFXReport_f - ea: 0x006C86D0 / 0x006C8680
// ============================================================================
template <typename T>
class ae_vector {
public:
    T* mElements;
    int mSize;
    int mCapacity;

    void erase(T* iToErase)
    {
        for (T* p = iToErase; p + 1 < &mElements[mSize]; ++p)
            *p = p[1];
        --mSize;
    }

    // ?erase@?$ae_vector@PAVParticleEffect@@@@QAEXPAPAVParticleEffect@@0@Z
    void erase(T* iBeginErase, T* iEndErase)
    {
        if (mSize <= 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 286;
            AeAssert::gCurrentExpr = "mSize > 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("can't erase in empty vector"))
            {
                __debugbreak();
            }
        }
        T* end = &mElements[mSize];
        T* dst = iBeginErase;
        if (iBeginErase != end)
        {
            for (T* src = iEndErase; src != end; ++src)
                *dst++ = *src;
            mSize -= (int)(end - dst);
        }
    }

    // ?push_back@?$ae_vector@PAVParticleEffect@@@@QAEXABQAVParticleEffect@@@Z
    void push_back(const T& iElement)
    {
        if (mSize >= mCapacity)
        {
            int v4 = mSize + 4;
            if (mSize <= 3)
                v4 = mSize + 1;
            T* v5 = (T*)tlMemAlloc(sizeof(T) * v4, 8u, 0);
            for (int i = 0; i < mSize; ++i)
                v5[i] = mElements[i];
            if (mElements != nullptr)
            {
                tlMemFree(mElements);
                mElements = nullptr;
                mCapacity = 0;
            }
            mElements = v5;
            mCapacity = v4;
        }
        mElements[mSize++] = iElement;
    }
};

extern void* tlMemAlloc(unsigned int size, unsigned int align,
                        unsigned int flags);  // core.o
extern void tlMemFree(void* p);               // core.o

class PoolAllocator {
public:
    void Release(void* ptr);  // ?Release@PoolAllocator@@QAEXPAX@Z
};

class LightEffect {
    friend void RemoveLight(LightEffect* light);
public:
    static PoolAllocator* sAllocator;  // ?sAllocator@LightEffect@@0PAVPoolAllocator@@A @ 0xF74470
};
PoolAllocator* LightEffect::sAllocator;

ae_vector<LightEffect*> gLightEffectList;  // ?gLightEffectList@@3V?$ae_vector@PAVLightEffect@@@@A @ 0xF74CC8

void RemoveLight(LightEffect* light)
{
    LightEffect** mElements = gLightEffectList.mElements;
    LightEffect** end = &gLightEffectList.mElements[gLightEffectList.mSize];
    if (gLightEffectList.mElements != end)
    {
        do
        {
            if (*mElements == light)
                break;
            ++mElements;
        } while (mElements != end);
    }
    gLightEffectList.erase(mElements);
    if (light != nullptr)
        LightEffect::sAllocator->Release(light);
}

// ae_pair (class tag V; matches ?sArray@ParticleEffect mangling)
template <typename A, typename B>
class ae_pair {
public:
    A first;
    B second;
};

// ParticleEffect view (IDA layout; full methods in tr_fx2.cpp)
class ParticleEffect {
public:
    float cached_pos[4];       // +0x00
    int cached_cell_index;     // +0x10
    int culled;                // +0x14
    short mIndexA;             // +0x18
    short mIndexB;             // +0x1A
    apsEffect* mEffect;        // +0x1C
    void* mAbstractEffectParticle;  // +0x20
    math::Mat43* mPoPtr;       // +0x24
    void* mDObjHandle;         // +0x28
    void* mEntHandle;          // +0x2C
    short mBoneIndex;          // +0x30
    unsigned short mFlags;     // +0x32
    int mPakId;                // +0x34
    void* mRaycastData;        // +0x38

    static ParticleEffect* sArrayData;  // ?sArrayData@ParticleEffect@@2PAV1@A (tr_fx2.cpp)
    static ae_sized_array<ae_pair<short, short>, 256> sArray;  // (tr_fx2.cpp)
};

extern ParticleEffect* FX_ElectEffectToKill(apsEffectTemplate* tmpl);  // tr_fx2.cpp

ae_vector<ParticleEffect*> gParticleEffectList;  // ?gParticleEffectList@@3V?$ae_vector@PAVParticleEffect@@@@A @ 0xF75174
void tlPrintf(const char* fmt, ...);  // core.o

void Cmd_PFXReport_f()
{
    for (ParticleEffect** i = gParticleEffectList.mElements;
         i != &gParticleEffectList.mElements[gParticleEffectList.mSize]; ++i)
    {
        apsEffect* mEffect = (*i)->mEffect;
        const char* mName;
        if (mEffect != nullptr)
            mName = mEffect->mTemplate->GetName();
        else
            mName = "unknown";
        tlPrintf("0x%08x %s\n", *i, mName);
    }
}

// ============================================================================
// IsOkToSpawnNewEffect - ea: 0x006C84F0
// ============================================================================
class ae_heap {
public:
    void** __vftable;
    struct mem_heap2* GetHeapPointer();  // ?GetHeapPointer@ae_heap@@QAEPAUmem_heap@@XZ
};
struct mem_heap2 {
    uint8_t _pad[0x484];
    unsigned int size;      // +0x484
    unsigned int used_byte; // +0x488
};
extern void* gApsHeap;      // ?gApsHeap@@3PAVae_heap@@A (common.cpp)

bool IsOkToSpawnNewEffect(apsEffectTemplate* Tmpl, int juice);
bool IsOkToSpawnNewEffect(apsEffectTemplate* Tmpl, int juice)
{
    if (juice == 0)
        return false;
    bool pfx_free_pool_empty;
    if (ParticleEffect::sArrayData == nullptr || ParticleEffect::sArray.m_size == 0)
        pfx_free_pool_empty = true;
    else
        pfx_free_pool_empty = false;
    mem_heap2* v2 = ((ae_heap*)gApsHeap)->GetHeapPointer();
    bool apsHeapAboutFull = (float)v2->used_byte > (float)v2->size * 0.80000001f;
    if (apsEffect::TestAlloc(Tmpl) != 0 && !pfx_free_pool_empty && !apsHeapAboutFull)
        return true;
    ParticleEffect* v3 = FX_ElectEffectToKill(Tmpl);
    ParticleEffect* v4 = v3;
    if (v3 == nullptr)
        return true;
    apsEffect* mEffect = v3->mEffect;
    const apsEffectTemplate* mTemplate = mEffect->mTemplate;
    unsigned int key;
    if (mEffect != nullptr)
        key = mEffect->mSortKey._32;
    else
        key = 0xFFFFFFFFu;
    apsDebug::PrintWarning(apsDebug::BLUE_ALERT, "killing %08X %s to make way for %s",
                           key, mTemplate->GetName(), Tmpl->GetName());
    int mPakId = v4->mPakId;
    if (mPakId == -1)
    {
        apsEffect::Delete(v4->mEffect, 1u);
    }
    else
    {
        apsMemory::ClearBlockAllocator();
        apsCommon::SetCurrentPakId(mPakId);
        apsCommon::SetPakAllocs(1);
        apsEffect::Delete(v4->mEffect, 1u);
        apsMemory::SetBlockAllocator();
        apsCommon::SetCurrentPakId(-1);
        apsCommon::SetPakAllocs(0);
    }
    v4->mEffect = nullptr;
    return IsOkToSpawnNewEffect(Tmpl, juice - 1);
}

// ============================================================================
// batch 93 - render.o fx/lighting/render-state functions
// ============================================================================
class ServerTime {
public:
    uint8_t _pad[8];
    float mTickDelta;               // +0x08
    unsigned int mTickMSec;         // +0x0C
    static ServerTime sInst;        // ?sInst@ServerTime@@2V1@A
};
ServerTime ServerTime_sInst;         // g.o
struct nglSceneView {
    uint8_t _pad[0x8C];
    math::Position3 ViewPos;        // +0x8C
    math::Mat44 Projection;         // +0x9C
    uint8_t _pad2[0x270 - 0xDC];
    math::Vector4 ClipPlanes[6];    // +0x270
};
struct nglScene;
extern nglScene* nglBuildScene;        // ?nglBuildScene@@3PAUnglScene@@A
extern void nglValidateMatrices(nglScene* scene);  // ngl.o
extern bool nglProfileEvalShader(nglShader* shader);    // ?nglProfileEvalShader@@YA_NPAUnglShader@@@Z
extern void UpdateLights(float mTickMSec);        // ?UpdateLights@@YAXM@Z
extern void RemoveDeadEffects();                  // ?RemoveDeadEffects@@YAXXZ
extern void FX_UpdateRainDrops(float dt);         // ?FX_UpdateRainDrops@@YAXM@Z
extern void ProcessEffectsCollisions();           // ?ProcessEffectsCollisions@@YAXXZ (next in batch)
extern void UpdateEffects(bool bUpdate);          // ?UpdateEffects@@YAX_N@Z (tr_tiny.cpp)
extern float gFXTime;                             // ?gFXTime@@3MA
extern int gScreenshotInProgress;                 // ?gScreenshotInProgress@@3HA
extern int currCl;                                // g.o
extern int LocalClient_FirstLocalClientIndex();   // ?FirstLocalClientIndex@LocalClient@@SAHXZ
extern cdl_proftimer cdl_proftimer_fx_update;     // ?cdl_proftimer_fx_update@@3Ucdl_proftimer@@A
cdAepsShader* gCDAepsShader = nullptr;             // ?gCDAepsShader@@3PAVcdAepsShader@@A
const math::Mat43* nglGetMatrix_WorldToView(nglScene* scene);  // ?nglGetMatrix_WorldToView@@YAPBVMat43@math@@PAUnglScene@@@Z

// ea: 0x006DBC20
void FX_UpdateFX(bool firstClient)
{
    nglValidateMatrices(nglBuildScene);
    apsCommon::PlayerViewPort* pvp =
        apsCommon::GetPlayerViewPort((unsigned int)currCl);
    memcpy(pvp->mClipPlanes, ((nglSceneView*)nglBuildScene)->ClipPlanes,
           sizeof(pvp->mClipPlanes));
    pvp->mViewPos = ((nglSceneView*)nglBuildScene)->ViewPos;
    pvp->mProjectionX = ((nglSceneView*)nglBuildScene)->Projection.x.v.m128_f32[0];
    if (!firstClient
        || !nglProfileEvalShader(reinterpret_cast<nglShader*>(gCDAepsShader)))
        return;
    apsCommon::SubmitSpawnedEffectQueue();
    UpdateLights((float)ServerTime_sInst.mTickMSec);
    float mTickDelta = ServerTime_sInst.mTickDelta;
    bool bUpdate = true;
    float dt = ServerTime_sInst.mTickDelta;
    if (gScreenshotInProgress != 0)
    {
        mTickDelta = 0.0f;
        dt = 0.0f;
        bUpdate = false;
    }
    else if (ServerTime_sInst.mTickDelta > 0.05f)
    {
        dt = 0.05f;
        mTickDelta = 0.05f;
    }
    else if (ServerTime_sInst.mTickDelta <= 0.0f)
    {
        bUpdate = false;
    }
    gFXTime = gFXTime + mTickDelta;
    if (currCl != LocalClient_FirstLocalClientIndex())
        gFXTime = gFXTime - dt;
    const math::Mat43* worldToView = nglGetMatrix_WorldToView(nglBuildScene);
    apsCommon::SetupFrame(*worldToView, -1.0f);
    apsCommon::GetPlayerViewPort(0)->mActive = false;  // dword_F6A290[0] == 2 placeholder
    RemoveDeadEffects();
    ProcessEffectsCollisions();
    FX_UpdateRainDrops(dt);
    cdl_proftimer_fx_update.start();
    UpdateEffects(bUpdate);
    cdl_proftimer_fx_update.stop();
}


// ============================================================================
// ProcessEffectsCollisions - ea: 0x006DA3F0
// ============================================================================
struct apsCollisionData {
    uint8_t _pad[0x10];
    unsigned int mNumRaycastRequests;  // +0x10
    struct RaycastRequest {
        math::Position3 start;      // +0x00
        math::Position3 end;        // +0x10
        math::Position3 result;     // +0x20
        float fraction;             // +0x30
        math::Dir3 normal;          // +0x34
    } mRaycastRequests[1];          // +0x14
};
struct ParticleRaycastData {
    uint8_t _pad[0x20];
    struct proximity_data_tLocal {
        uint8_t _pad[0x20];
    } mProximityData;               // +0x20
};
struct trace_tFx {
    math::Position3 endpos;          // +0x00
    math::Dir3 normal;               // +0x10
    float fraction;                  // +0x20
    int surfaceFlags;                // +0x24
    int contents;                    // +0x28
};
extern void ProximityUpdate(ParticleRaycastData& data,
                            const apsCollisionData& col);  // ?ProximityUpdate@@YAXAAURaycastData@ParticleEffect@@ABUCollisionData@apsEffect@@@Z
extern void TracePoint(const void* proximity, trace_tFx* trace,
                       const math::Position3& start,
                       const math::Position3& end, int contentmask);

void ProcessEffectsCollisions()  // ?ProcessEffectsCollisions@@YAXXZ @ 0x6DA3F0
{
    ParticleEffect** it = gParticleEffectList.mElements;
    ParticleEffect** end = &gParticleEffectList.mElements[gParticleEffectList.mSize];
    while (it != end)
    {
        ParticleEffect* v2 = *it;
        if (v2 == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
            AeAssert::gCurrentLine = 1002;
            AeAssert::gCurrentExpr = "effect";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("ParticleEffect pointer invalid. How did we get here?"))
                __debugbreak();
        }
        apsCollisionData* col = (apsCollisionData*)v2->mEffect->mCollisionData;
        if (col != nullptr)
        {
            if (v2->mRaycastData == nullptr)
            {
                ParticleRaycastData* rd =
                    (ParticleRaycastData*)tlMemAlloc(0x1870u, 0x10u, 0);
                if (rd != nullptr)
                {
                    new (rd) ParticleRaycastData();
                }
                v2->mRaycastData = rd;
            }
            if (col->mNumRaycastRequests != 0)
            {
                ProximityUpdate(*(ParticleRaycastData*)v2->mRaycastData, *col);
                unsigned int n = col->mNumRaycastRequests;
                for (unsigned int i = 0; i < n; ++i)
                {
                    apsCollisionData::RaycastRequest& req = col->mRaycastRequests[i];
                    trace_tFx trace;
                    memset(&trace, 0, sizeof(trace));
                    TracePoint(&((ParticleRaycastData*)v2->mRaycastData)->mProximityData,
                               &trace, req.start, req.end, 41951377);
                    if (trace.fraction == 1.0f || trace.contents == 0)
                    {
                        req.fraction = -1.0f;
                    }
                    else
                    {
                        req.result = trace.endpos;
                        req.normal = trace.normal;
                        req.fraction = trace.fraction;
                    }
                }
                col->mNumRaycastRequests = 0;
            }
        }
        ++it;
    }
}

// ============================================================================
// ThreadedUpdateEffects - ea: 0x006DA720
// ============================================================================
class tagInfoLocal2 {
public:
    class Entity2* parent;           // +0x00
    class Entity2* next;             // +0x04
};
class Entity2 {
public:
    uint8_t _pad0[0xE0];
    struct refEntityLocal {
        math::Position3 currentOrigin;  // +0x00
        math::Position3 currentAngles;  // +0x10
    } r;                             // +0xE0
    uint8_t _pad1[0x2C4 - 0x110];
    int flags;                       // +0x2C4
};
class DObjHandleDbLocal2 {
public:
    struct DbElement {
        void* mObject;               // +0x00
        int mKey;                    // +0x04
    };
    uint8_t _pad[0xA8];
    DbElement mElements[0x540];      // +0xA8
    static DObjHandleDbLocal2 sInst; // ?sInst@DObjHandleDb@@0V1@A
};
class EntityHandleDbLocal3 {
public:
    struct DbElement {
        void* mObject;               // +0x00
        int mKey;                    // +0x04
    };
    uint8_t _pad[0xA8];
    DbElement mElements[0x540];      // +0xA8
    static EntityHandleDbLocal3 sInst;  // ?sInst@EntityHandleDb@@0V1@A
};
DObjHandleDbLocal2 DObjHandleDbLocal2::sInst;
EntityHandleDbLocal3 EntityHandleDbLocal3::sInst;
extern void apsMemory_ClearBlockAllocator();   // ?ClearBlockAllocator@apsMemory@@SAXXZ
extern void apsMemory_SetBlockAllocator();     // ?SetBlockAllocator@apsMemory@@SAXXZ
extern void apsEffect_SetCulled(apsEffect* effect, int v);    // ?SetCulled@apsEffect@@QAEXH@Z
extern bool FX_GetBoneOrientation2(unsigned int handle, short bone,
                                   float* ori);  // ?FX_GetBoneOrientation@@YA_NV?$DbLinkedHandle@VDObjHandleDb@@VDObj@@@@HPAUorientation_t@@@Z
extern void AnglesToAxis2(const math::Position3& angles,
                          const math::Position3& origin, float* out);  // ?AnglesToAxis@@YAXABVPosition3@math@@0AAVnalMatrix4x4@@@Z
extern void View_IsSplitScreen();               // ?IsSplitScreen@View@@YA_NXZ
struct jqBatch {
    void* Input;                     // +0x00
    void* Output;                    // +0x04
    void* Scratch;                   // +0x08
    void* Static;                    // +0x0C
};
struct level_locals_t;
extern level_locals_t level;
class AbstractEffectParticle;
extern AbstractEffectParticle* gLastAbstractEffectParticle;
extern int g_scr_data_debris_bro_func;
extern void apsCommon_SetCurrentPakId(int pakId);  // ?SetCurrentPakId@apsCommon@@SAXH@Z
extern void apsCommon_SetPakAllocs(int v);         // ?SetPakAllocs@apsCommon@@SAXH@Z

void ThreadedUpdateEffects(jqBatch* batch)  // ?ThreadedUpdateEffects@@YAXPAUjqBatch@@@Z @ 0x6DA720
{
    char bUpdate = *(char*)batch->Static;
    View_IsSplitScreen();
    ParticleEffect** it = gParticleEffectList.mElements;
    ParticleEffect** end = &gParticleEffectList.mElements[gParticleEffectList.mSize];
    while (it != end)
    {
        ParticleEffect* v4 = *it;
        if (v4 != nullptr && (v4->mFlags & 2) != 0)
        {
            ++it;
            continue;
        }
        apsEffect* effect = v4->mEffect;
        void* mObject = nullptr;
        Entity2* v10 = nullptr;
        unsigned int mVal = (unsigned int)v4->mDObjHandle;
        unsigned int idx = mVal & 0xFFF;
        if (idx < 0x540
            && mVal >> 12 == (unsigned int)DObjHandleDbLocal2::sInst.mElements[idx].mKey)
            mObject = DObjHandleDbLocal2::sInst.mElements[idx].mObject;
        unsigned int eidx = (unsigned int)v4->mEntHandle & 0xFFF;
        if (eidx < 0x540
            && (unsigned int)v4->mEntHandle >> 12
                   == (unsigned int)EntityHandleDbLocal3::sInst.mElements[eidx].mKey)
            v10 = (Entity2*)EntityHandleDbLocal3::sInst.mElements[eidx].mObject;
        if ((v4->mFlags & 1) != 0)
        {
            if (mObject == nullptr && v10 == nullptr)
            {
                effect->StopEmitting();
                if (v4->mPoPtr == nullptr)
                    goto skipUpdate;
            }
        }
        else if (mObject == nullptr && v10 == nullptr)
        {
            if (v4->mPoPtr == nullptr)
                goto skipUpdate;
        }
        if (v4->mPoPtr != nullptr)
        {
            if ((v4->mFlags & 0x10) != 0)
            {
                effect->SetLocalToWorldTransform(*v4->mPoPtr);
                v4->mFlags &= (unsigned short)~0x10u;
            }
            goto skipUpdate;
        }
        if (v4->mBoneIndex < 0)
        {
            if (v10 != nullptr)
            {
                math::Mat43 mat;
                mat.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
                mat.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
                mat.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
                mat.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 0.0f);
                if ((v4->mFlags & 4) != 0)
                {
                    mat.w.v = v10->r.currentOrigin.v;
                }
                else
                {
                    AnglesToAxis2(v10->r.currentAngles, v10->r.currentOrigin,
                                  (float*)&mat);
                }
                effect->SetLocalToWorldTransform(mat);
            }
        }
        else
        {
            float ori[13];
            if (FX_GetBoneOrientation2((unsigned int)v4->mDObjHandle,
                                       v4->mBoneIndex, ori))
            {
                math::Mat43 mat;
                memcpy(&mat, ori, 52);
                effect->SetLocalToWorldTransform(mat);
            }
        }
    skipUpdate:
        if ((v4->mFlags & 8) != 0)
        {
            v4->mFlags &= (unsigned short)~8u;
            const char* levelBytes = reinterpret_cast<const char*>(&level);
            if (*(const int*)(levelBytes + 0xACC) == 0
                && *(const int*)(levelBytes + 0xAC8) == 0)
            {
                gLastAbstractEffectParticle =
                    (AbstractEffectParticle*)v4->mAbstractEffectParticle;
                ((void (__cdecl*)())(intptr_t)g_scr_data_debris_bro_func)();
            }
            gLastAbstractEffectParticle = nullptr;
        }
        if (bUpdate != 0)
        {
            int mPakId = v4->mPakId;
            if (mPakId == -1)
            {
                effect->Update(gFXTime);
            }
            else
            {
                apsMemory_ClearBlockAllocator();
                apsCommon_SetCurrentPakId(mPakId);
                apsCommon_SetPakAllocs(1);
                v4->culled = 0;
                apsEffect_SetCulled(effect, 0);
                effect->Update(gFXTime);
                apsMemory_SetBlockAllocator();
                apsCommon_SetCurrentPakId(-1);
                apsCommon_SetPakAllocs(0);
            }
        }
        effect->CalcSortKey();
        ++it;
    }
    apsCommon::GetPlayerViewPort(0);  // mBuildScene=nullptr equivalent skipped
}
