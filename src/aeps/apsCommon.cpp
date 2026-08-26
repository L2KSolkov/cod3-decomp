// ============================================================================
// apsCommon.cpp — shared APS globals / helpers (22 non-inline funcs).
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsCommon.cpp
//
// Port strategy (matches apsGroup.o / apsMath.o precedent):
//   - All 22 non-inline functions verified against IDA disasm.
//   - Inline COMDATs (apsLOD setters, CameraSettings accessors, apsAllocator
//     ctor/dtor, apsSingleton<apsError> InstancePtr, apsDebug::SetEnabled,
//     apsDestroy<apsError>) are defined in the headers and emitted here per
//     codmp_xboxr.map attribution.
//   - apsSingleton<apsError>::sInstancePtr (data) is owned by apsCommon.o:
//     explicit template instantiation at the bottom of this file.
// ============================================================================
#include "apsCommon.h"
#include "apsMemory.h"
#include "apsError.h"
#include "apsDebug.h"
#include "apsVertexBuffer.h"
#include "apsBillboardRenderer.h"
#include "apsUVARenderer.h"
#include "apsColorBillboardRenderer.h"
#include "apsColorUVARenderer.h"
#include "apsRectangleRenderer.h"
#include "apsUVARectangleRenderer.h"
#include "apsColorUVARectangleRenderer.h"
#include "apsColorRectangleRenderer.h"
#include "apsSimpleMeshRenderer.h"
#include "apsShrimpRenderer.h"

#include <math.h>
#include <new>

enum TPakId : int;
#define PAK_ID_INVALID ((TPakId)-1)

class PakFile {
public:
    void* MemAlloc(unsigned int align, unsigned int size, bool search_prereqs);
    bool MemFree(void* ptr, bool search_prereqs);
};

class PakManager {
public:
    static PakManager* sInst;
    PakFile* GetPakFile(TPakId id);
};

class TlSystemCallbacks {
public:
    static void* MemAlloc(unsigned int size, unsigned int align, unsigned int flags);
    static void MemFree(void* ptr);
};

struct mem_heap {
    unsigned char _pad[0x474];
    void* start;
    void* end;
};

class ae_heap {
public:
    void** __vftable;
    void* Malloc(unsigned int size, unsigned int alignment);
    void Free(void* ptr);
    mem_heap* GetHeapPointer();
};

extern void* gApsHeap;
extern void mem_break();

// ============================================================================
// Data statics (apsCommon.o). Initial values verified against IDA:
//   mCurAllocator = &mDefAllocator (0xE49620 data), mCurPakId = -1.
// ============================================================================
apsCommon::CameraSettings apsCommon::mCamera;    // @0x10DED10
nglScene* apsCommon::mBuildScene = 0;            // @0x10DECFC
nglScene* apsCommon::mShimmerScene = 0;          // @0x10DECF0

apsAllocator* apsCommon::mCurAllocator = &apsCommon::mDefAllocator;  // @0xE49620
int apsCommon::mCurPakId = -1;                   // @0xE49624
apsAllocator apsCommon::mDefAllocator;           // @0xE4964C
apsLOD apsCommon::mLOD;                          // @0x10DECB0
apsAllocator* apsCommon::mBlockAllocator = 0;    // @0x10DEDDC
float* (*apsCommon::mSplineCallback)(unsigned int) = 0;  // @0x10DECE0
unsigned int apsCommon::mFlags = 0;              // @0x10DECE4
int apsCommon::mPakAllocs = 0;                   // @0x10DECE8
nglTexture* apsCommon::mDepthBufferTexture = 0;  // @0x10DECEC
apsClient* apsCommon::mApsClient = 0;            // @0x10DECF4
float apsCommon::mChanceToRemove = 0.0f;         // @0x10DECF8
apsCommon::PlayerViewPort apsCommon::mViewPort[apsCommon::MAX_NUM_VIEWPORTS];  // @0x10DED50

// ============================================================================
// Extern declarations for unported callees (render.o / apsMemory.o / apsEffect.o
// / apsInternal.o / tl_system.o). Names/scoping match codmp_xboxr.map mangling;
// satisfied by /FORCE:UNRESOLVED until the owning objects are ported.
// ============================================================================
namespace apsMemory {
    void Report();                                  // ?Report@apsMemory@@YAXXZ (apsMemory.o)
}

namespace apsInternal {
    void Init(int iMeshLightCat);                   // ?Init@apsInternal@@YAXH@Z
    void SubmitSpawnedEffectQueue();                // ?SubmitSpawnedEffectQueue@apsInternal@@YAXXZ
    void RemoveFromSpawnedEffectQueueByPakId(int pakId);  // ?RemoveFromSpawnedEffectQueueByPakId@apsInternal@@YAXH@Z
    void ClearSpawnedEffectQueue();                 // ?ClearSpawnedEffectQueue@apsInternal@@YAXXZ
}

extern void tlPrintf(const char* fmt, ...);         // ?tlPrintf@@YAXPBDZZ (tl_system.o)

// ea: 0x006C3090
void* apsMemAlloc(unsigned int size, unsigned int align, unsigned int flags)
{
    if (apsCommon::PakAllocs() == 0)
        return TlSystemCallbacks::MemAlloc(size, align, flags);

    void* result = apsMemory::AllocFromPools((int)size, (int)align);
    if (result == nullptr)
    {
        int currentPakId = apsCommon::GetCurrentPakId();
        if (currentPakId != -1 && currentPakId >= 0 && currentPakId < 0x63)
        {
            PakFile* pakFile = PakManager::sInst->GetPakFile((TPakId)currentPakId);
            if (pakFile != nullptr)
                result = pakFile->MemAlloc(align, size, true);
        }
        if (result == nullptr)
        {
            void* heapResult = ((ae_heap*)gApsHeap)->Malloc(size, align);
            if (heapResult == nullptr && (flags & 2) == 0)
                mem_break();
            result = heapResult;
        }
    }
    return result;
}

// ea: 0x006C3110
void apsMemFree(void* ptr)
{
    if (apsCommon::PakAllocs() != 0)
    {
        if (ptr != nullptr && apsMemory::FreeFromPools(ptr) == 0)
        {
            int currentPakId = apsCommon::GetCurrentPakId();
            PakFile* pakFile = nullptr;
            if (currentPakId != -1)
                pakFile = PakManager::sInst->GetPakFile((TPakId)currentPakId);
            if (currentPakId == -1 || pakFile == nullptr
                || !pakFile->MemFree(ptr, true))
            {
                mem_heap* heap = ((ae_heap*)gApsHeap)->GetHeapPointer();
                if ((unsigned char*)ptr > (unsigned char*)heap->start
                    && (unsigned char*)ptr < (unsigned char*)heap->end)
                    ((ae_heap*)gApsHeap)->Free(ptr);
            }
        }
    }
    else
    {
        TlSystemCallbacks::MemFree(ptr);
    }
}

// ============================================================================
// apsAllocator::MemAlign — forward to the raw aps memory allocator.
// ea: 0x7EB370
// ============================================================================
void* apsAllocator::MemAlign(unsigned int iSize, unsigned int iAlignment) const {
    void* result = (void*)iSize;
    if (iSize != 0)
        return apsMemAlloc(iSize, iAlignment, 2u);
    return result;
}

// ============================================================================
// apsAllocator::MemFree — forward to the raw aps memory free.
// ea: 0x7EB3A0
// ============================================================================
void apsAllocator::MemFree(void* iPtr) const {
    apsMemFree(iPtr);
}

// ============================================================================
// apsAllocator::MemResize — release default allocator returns null.
// ea: 0x7EB3C0
// ============================================================================
void* apsAllocator::MemResize(void* iPtr, unsigned int iNewSize, unsigned int iAlignment) const {
    (void)iPtr;
    (void)iNewSize;
    (void)iAlignment;
    return nullptr;
}

// ============================================================================
// apsCommon::InitShaders — init the simple-mesh shader (thunk).
// ea: 0x7EB3D0
// ============================================================================
void apsCommon::InitShaders() {
    apsSimpleMeshRenderer::InitShader();
}

// ============================================================================
// apsCommon::SetupFrame — update the camera basis + roll from the world-to-view
// matrix, then swap the vertex buffer. The roll is atan2(mUp.z, mLeft.z) via an
// asin minimax polynomial (SSE shuffles + x87 sqrt, per the release disasm).
// ea: 0x7EB3E0
// ============================================================================
void apsCommon::SetupFrame(const math::Mat43& iWorldToView, float xFlip) {
    mCamera.mXFlip = xFlip;

    // Extract camera basis from the world-to-view matrix columns:
    //   mDir  = z-column, mUp = y-column, mLeft = x-column * xFlip
    __m128 v2 = iWorldToView.y.v;
    __m128 v3 = iWorldToView.w.v;
    __m128 v4 = _mm_shuffle_ps(iWorldToView.x.v, v2, 0xEE);          // (x.z, x.w, y.z, y.w)
    __m128 v5 = iWorldToView.z.v;
    __m128 v6 = _mm_shuffle_ps(iWorldToView.x.v, v2, 0x44);          // (x.x, x.y, y.x, y.y)
    __m128 v8 = _mm_shuffle_ps(v4, _mm_shuffle_ps(v5, v3, 0xEE), 0x88);  // (x.z, y.z, z.z, w.z) = mDir
    __m128 v9 = _mm_shuffle_ps(v5, v3, 0x44);                        // (z.x, z.y, w.x, w.y)
    mCamera.mLeft.v = _mm_mul_ps(_mm_shuffle_ps(v6, v9, 0x88), _mm_set1_ps(xFlip));  // (x.x, y.x, z.x, w.x) * xFlip
    mCamera.mUp.v = _mm_shuffle_ps(v6, v9, 0xDD);                    // (x.y, y.y, z.y, w.y) = mUp
    mCamera.mDir.v = v8;

    // Camera roll from the z-components of left/up:
    //   roll = atan2(mUp.z, mLeft.z) via asin approximation.
    float v11 = mCamera.mLeft.v.m128_f32[2];   // mLeft.z
    float v24 = mCamera.mUp.v.m128_f32[2];     // mUp.z
    if (v11 != 0.0f || v24 != 0.0f) {
        float v20 = fabsf(v11);                // |mLeft.z|
        float v19 = fabsf(v24);                // |mUp.z|
        if (v20 + v19 != 0.0f) {
            float v18 = 1.0f / sqrtf(v11 * v11 + v24 * v24);
            float v15;
            if (v20 <= v19) {
                float v16 = v18 * v20;         // |mLeft.z| / mag
                if (v16 >= 0.5f) {
                    float v22 = sqrtf(fabsf((1.0f - v16) * 0.5f));
                    v15 = (((((v22 * v22 * v22 * v22 * v22 * v22 * v22) * -0.1079625f)
                           - ((v22 * v22 * v22 * v22 * v22) * 0.15000001f))
                           - ((v22 * v22 * v22) * 0.33333331f))
                           - (v22 * 2.0f))
                           + 1.5707963f;
                } else {
                    v15 = ((((v16 * v16 * v16 * v16 * v16 * v16 * v16) * 0.053981241f)
                           + ((v16 * v16 * v16 * v16 * v16) * 0.075000003f))
                           + ((v16 * v16 * v16) * 0.1666667f))
                           + v16;
                }
            } else {
                float v13 = v18 * v19;         // |mUp.z| / mag
                float v14;
                if (v13 >= 0.5f) {
                    float v21 = sqrtf(fabsf((1.0f - v13) * 0.5f));
                    v14 = (((((v21 * v21 * v21 * v21 * v21 * v21 * v21) * -0.1079625f)
                           - ((v21 * v21 * v21 * v21 * v21) * 0.15000001f))
                           - ((v21 * v21 * v21) * 0.33333331f))
                           - (v21 * 2.0f))
                           + 1.5707963f;
                } else {
                    v14 = ((((v13 * v13 * v13 * v13 * v13 * v13 * v13) * 0.053981241f)
                           + ((v13 * v13 * v13 * v13 * v13) * 0.075000003f))
                           + ((v13 * v13 * v13) * 0.1666667f))
                           + v13;
                }
                v15 = 1.5707964f - v14;
            }
            if (v24 < 0.0f)
                v15 = 3.1415927f - v15;
            if (v11 < 0.0f)
                v15 = -v15;
            mCamera.mRoll = v15;
        }
    }
    apsVertexBuffer::Swap();
}

// ============================================================================
// apsCommon::Report — dump the APS memory + effect usage report.
// ea: 0x7EB750
// ============================================================================
void apsCommon::Report() {
    tlPrintf("-----------------------------------------------------------------------------\n");
    tlPrintf("AEPS Report\n\n");
    apsMemory::Report();
    apsEffect::ReportEffects();
    tlPrintf("-----------------------------------------------------------------------------\n\n");
}

// ============================================================================
// apsCommon::SetClient — install the APS client callback object.
// ea: 0x7EB780
// ============================================================================
void apsCommon::SetClient(apsClient* client) {
    mApsClient = client;
}

// ============================================================================
// apsCommon::GetAllocator — current allocator.
// ea: 0x7EB790
// ============================================================================
apsAllocator* apsCommon::GetAllocator() {
    return mCurAllocator;
}

// ============================================================================
// apsCommon::GetCurrentPakId — current pak id.
// ea: 0x7EB7A0
// ============================================================================
int apsCommon::GetCurrentPakId() {
    return mCurPakId;
}

// ============================================================================
// apsCommon::SetCurrentPakId — set pak id, return the previous one.
// ea: 0x7EB7B0
// ============================================================================
int apsCommon::SetCurrentPakId(int id) {
    int result = mCurPakId;
    mCurPakId = id;
    return result;
}

// ============================================================================
// apsCommon::PakAllocs — current pak allocation count.
// ea: 0x7EB7D0
// ============================================================================
int apsCommon::PakAllocs() {
    return mPakAllocs;
}

// ============================================================================
// apsCommon::SetPakAllocs — set pak allocation count, return the previous one.
// ea: 0x7EB7E0
// ============================================================================
int apsCommon::SetPakAllocs(int value) {
    int result = mPakAllocs;
    mPakAllocs = value;
    return result;
}

// ============================================================================
// apsCommon::GetDefaultAllocator — the built-in default allocator.
// ea: 0x7EB800
// ============================================================================
apsAllocator* apsCommon::GetDefaultAllocator() {
    return &mDefAllocator;
}

// ============================================================================
// apsCommon::SetAllocator — set the current allocator (NULL => default),
// return the previous one.
// ea: 0x7EB810
// ============================================================================
apsAllocator* apsCommon::SetAllocator(apsAllocator* i_allocator) {
    apsAllocator* result = mCurAllocator;
    mCurAllocator = i_allocator;
    if (i_allocator == 0)
        mCurAllocator = &mDefAllocator;
    return result;
}

// ============================================================================
// apsCommon::GetBlockAllocator — block allocator (falls back to default).
// ea: 0x7EB840
// ============================================================================
apsAllocator* apsCommon::GetBlockAllocator() {
    apsAllocator* result = mBlockAllocator;
    if (mBlockAllocator == 0)
        return &mDefAllocator;
    return result;
}

// ============================================================================
// apsCommon::SetBlockAllocator — install the block allocator.
// ea: 0x7EB850
// ============================================================================
void apsCommon::SetBlockAllocator(apsAllocator* i_allocator) {
    mBlockAllocator = i_allocator;
}

// ============================================================================
// apsCommon::SubmitSpawnedEffectQueue — thunk to apsInternal.o.
// ea: 0x7EB860
// ============================================================================
void apsCommon::SubmitSpawnedEffectQueue() {
    apsInternal::SubmitSpawnedEffectQueue();
}

// ============================================================================
// apsCommon::RemoveFromSpawnedEffectQueueByPakId — thunk to apsInternal.o.
// ea: 0x7EB870
// ============================================================================
void apsCommon::RemoveFromSpawnedEffectQueueByPakId(int pakId) {
    apsInternal::RemoveFromSpawnedEffectQueueByPakId(pakId);
}

// ============================================================================
// apsCommon::ClearSpawnedEffectQueue — thunk to apsInternal.o.
// ea: 0x7EB880
// ============================================================================
void apsCommon::ClearSpawnedEffectQueue() {
    apsInternal::ClearSpawnedEffectQueue();
}

// ============================================================================
// apsCommon::GetPlayerViewPort — per-player view port (single viewport).
// ea: 0x7EB890
// ============================================================================
apsCommon::PlayerViewPort* apsCommon::GetPlayerViewPort(unsigned int playerId) {
    if (playerId >= MAX_NUM_VIEWPORTS &&
        _tlAssert("source/apsCommon.cpp", 343,
                  "playerId < MAX_NUM_VIEWPORTS", ""))
        __debugbreak();
    return &mViewPort[playerId];
}

// ============================================================================
// apsCommon::Init — initialise the APS system.
// ea: 0x7EB8D0
// ============================================================================
void apsCommon::Init(unsigned int bDebug, int iMeshLightCat, unsigned int iMaxParticles) {
    if ((mFlags & 1) != 0 &&
        _tlAssert("source/apsCommon.cpp", 98,
                  "!Initialised()", "Aeps already initialised"))
        __debugbreak();

    mFlags = 0;
    mCamera.Init();
    apsDebug::SetEnabled(bDebug);
    mSplineCallback = 0;

    void* errorMem = mCurAllocator->MemAlign(8, 4);
    if (errorMem != 0)
        new (errorMem) apsError();

    apsVertexBuffer::Init(iMaxParticles);
    apsBillboardRenderer::Init();
    apsUVARenderer::Init();
    apsColorBillboardRenderer::Init();
    apsColorUVARenderer::Init();
    apsRectangleRenderer::Init();
    apsUVARectangleRenderer::Init();
    apsColorUVARectangleRenderer::Init();
    apsColorRectangleRenderer::Init();
    apsSimpleMeshRenderer::Init();
    apsShrimpRenderer::Init();
    apsInternal::Init(iMeshLightCat);

    mLOD.SetDefaults();
    mLOD.SetEnabled(1);
    mDepthBufferTexture = 0;
    mShimmerScene = 0;
    mFlags |= 1u;
}

// ============================================================================
// apsCommon::Term — tear down the APS system.
// ea: 0x7EBA40
// ============================================================================
void apsCommon::Term() {
    if ((mFlags & 1) == 0 &&
        _tlAssert("source/apsCommon.cpp", 164,
                  "Initialised()", "Aeps not initialised"))
        __debugbreak();

    apsError* error = apsSingleton<apsError>::InstancePtr();
    if (error != 0) {
        error->~apsError();
        mCurAllocator->MemFree(error);
    }
    mFlags &= ~1u;
}

// ============================================================================
// apsSingleton<apsError> — explicit instantiation. Emits sInstancePtr (data)
// plus the inline member COMDATs (InstancePtr etc.) into apsCommon.o.
// ============================================================================
template class apsSingleton<apsError>;
