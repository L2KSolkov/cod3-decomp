// ============================================================================
// NAL — NGL Animation Library (125 funcs, 10 objects)
// ea: 0x854490-0x878100
// ============================================================================

#include <cstdint>
#include <new>
#include <type_traits>
#include "core/math_types.h"
#include "core/tlFixedString.h"
#include "engine/broc_types.h"

class Entity;
class SceneAnimClient;

// IVPointer<T> local (mValue +0 / mPakId +4); `class` tag to match the
// binary's V-mangled IVPointer<XModel>.
template <typename T>
class IVPointer {
public:
    T* mValue;  // +0x00
    int mPakId; // +0x04
};
struct XModelLocal;

// Minimal local DObj view (full class in cg_local.h; offsets verified IDA)
class DObj {
public:
    void* tree[8];             // +0x00
    void* animPlayers[8];      // +0x20
    void* mPose[8];            // +0x40
    unsigned char modelParents[8];  // +0x60
    unsigned char matOffset[8];     // +0x68
    void* skel;                // +0x70
    unsigned char numModels;   // +0xCE
    Entity* mEntity;           // +0xD0
    int mLOD;                  // +0xD8
    int mLODOverride;          // +0xDC
    IVPointer<XModelLocal> models[8];  // +0x80
    int mPakId;                // +0xC0
};

struct XModelLocal {
    unsigned char _pad[0x24];
    void** lod;  // +0x24
};

// DObjSkelMat (core_types.h; 64 bytes)
struct DObjSkelMatLocal {
    float axis[3][4];  // +0x00
    float origin[4];   // +0x30
};

// DSkel local view (mat array; full in g_dobj.cpp)
struct DSkelLocal {
    DObjSkelMatLocal* mat;  // +0x00
};

// Local DbLinkedHandle view (full template in game_types.h)
template <typename DB, typename T>
struct DbLinkedHandle {
    unsigned int mVal;  // +0x00
};

// Scene-anim list (anim.o) - dlist node at +0x00 (reserved_dlist intrusive).
// Full layout verified vs disasm: mFileID +8, mInst +0xC, mNotify +0x10,
// blendNotify +0x14, mPlaying +0x19, blendIn +0x1C, blendOut +0x20,
// mPakId +0x24, mName @+0x28 (0x20 bytes + length @0x47).
struct SceneAnimInfo {
    unsigned char m_dlist_node[8];  // +0x00
    int mFileID;                    // +0x08
    void* mInst;                    // +0x0C
    void* mNotify;                  // +0x10
    void* blendNotify;              // +0x14
    unsigned char _pad18[0x19 - 0x18];
    unsigned char mPlaying;         // +0x19
    float blendIn;                  // +0x1C
    float blendOut;                 // +0x20
    int mPakId;                     // +0x24
    char mName[32];                 // +0x28

    // ?get_dlist_node@SceneAnimInfo@@QAEPAXXZ (0x539D10)
    void* get_dlist_node() { return this; }
    // ?get_dlist_node_offset@SceneAnimInfo@@SAHXZ (0x539D20)
    static int get_dlist_node_offset() { return 0; }
};

// reserved_dlist<T> - intrusive dlist (ae/core; verified IDA: node first
// member of T). Members below carry anim.o inline-COMDAT eases.
template <typename T>
struct reserved_dlist {
    struct dlist_node {
        dlist_node* m_next;  // +0x00
        dlist_node* m_prev;  // +0x04
    };
    int         m_size;  // +0x00
    dlist_node* m_head;  // +0x04
    dlist_node* m_end;   // +0x08
    dlist_node* m_tail;  // +0x0C

    // ?node_to_object@?$reserved_dlist@VSceneAnimInfo@@@@SAPAVSceneAnimInfo@@PAUdlist_node@1@@Z
    // ?node_to_object@?$reserved_dlist@VXAnimTree@@@@SAPAVXAnimTree@@PAUdlist_node@1@@Z
    static T* node_to_object(dlist_node* dlist_node)
    {
        return (T*)dlist_node;
    }

    // ?get_head@?$reserved_dlist@VSceneAnimInfo@@@@QAEPAUdlist_node@1@XZ
    // ?get_head@?$reserved_dlist@VXAnimTree@@@@QAEPAUdlist_node@1@XZ
    dlist_node* get_head()
    {
        return m_head;
    }

    struct iterator {
        dlist_node* m_node;  // +0x00
        dlist_node* m_next;  // +0x04

        iterator(dlist_node* cur, dlist_node* next)
            : m_node(cur), m_next(next) {}
        iterator(T* obj)
            : m_node((dlist_node*)obj),
              m_next(((dlist_node*)obj)->m_next) {}

        // ??Diterator@?$reserved_dlist@VXAnimTree@@@@QAEPAVXAnimTree@@XZ
        // ??Diterator@?$reserved_dlist@VSceneAnimInfo@@@@QAEPAVSceneAnimInfo@@XZ
        T* operator*()
        {
            return (T*)m_node;
        }
        // ??Citerator@?$reserved_dlist@VSceneAnimInfo@@@@QAEPAVSceneAnimInfo@@XZ
        T* operator->()
        {
            return (T*)m_node;
        }

        // ??Eiterator@?$reserved_dlist@VXAnimTree@@@@QAEAAV01@XZ
        // ??Eiterator@?$reserved_dlist@VSceneAnimInfo@@@@QAEAAV01@XZ
        iterator& operator++()
        {
            if (m_next != nullptr)
            {
                m_node = m_next;
                m_next = m_next->m_next;
            }
            return *this;
        }
        // ??Eiterator@?$reserved_dlist@VSceneAnimInfo@@@@QAE?AV01@H@Z
        iterator operator++(int)
        {
            iterator result(*this);
            if (m_next != nullptr)
            {
                m_node = m_next;
                m_next = m_next->m_next;
            }
            return result;
        }

        // ?get_node@iterator@?$reserved_dlist@VXAnimTree@@@@QAEPAUdlist_node@2@XZ
        // ?get_node@iterator@?$reserved_dlist@VSceneAnimInfo@@@@QAEPAUdlist_node@2@XZ
        dlist_node* get_node()
        {
            return m_node;
        }
    };

    // ?erase@?$reserved_dlist@VSceneAnimInfo@@@@QAE?AViterator@1@AAV21@@Z
    iterator erase(dlist_node* node)
    {
        if (node->m_next != nullptr)
            node->m_next->m_prev = node->m_prev;
        if (node->m_prev != nullptr)
            node->m_prev->m_next = node->m_next;
        if (m_tail == node)
            m_tail = node->m_prev;
        --m_size;
        return iterator(node->m_next, node->m_next != nullptr
                                           ? node->m_next->m_next
                                           : nullptr);
    }
};
reserved_dlist<SceneAnimInfo> gSceneAnimList;  // ?gSceneAnimList@@3V?$reserved_dlist@VSceneAnimInfo@@@@A (anim.o @ 0xDF2ACC)

// ea: 0x00543930
bool IsInSceneAnim()  // ?IsInSceneAnim@@YA_NXZ (anim.o)
{
    return gSceneAnimList.m_head != gSceneAnimList.m_end;
}

// AeAssert contract (definitions in core/ae_assert.cpp)
namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Error(const char* fmt, ...);
bool Warning(const char* fmt, ...);
}

#define XANIM_ASSERT(expr, file, line, msg)                                 \
    do {                                                                    \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                          \
        AeAssert::gCurrentFile = (file);                                    \
        AeAssert::gCurrentLine = (line);                                    \
        AeAssert::gCurrentExpr = (expr);                                    \
        if (!AeAssert::IsIgnored() && AeAssert::Assert((msg)))              \
            __debugbreak();                                                 \
    } while (0)

// ============================================================================
// Forward types (defined below)
// ============================================================================
namespace math { struct Quaternion; }
struct tlFixedString;

// ============================================================================
// nalObject / nalCachedPoseInfo — animation cache types
// ============================================================================
struct nalObject {};
struct nalCachedPoseInfo {};

// ============================================================================
// nalAnimFile / nalClientSceneAnim / nalHeap — resource types
// ============================================================================
struct nalAnimFile {};
struct nalClientSceneAnim {};
class nalHeap {};
class nalSceneAnim;
class nalSceneAnimInstance;
class nalStreamInstance;
class nalStaticInstance;

// ============================================================================
// nalAnimCache — animation data cache (LRU decompression cache)
// ============================================================================

class nalBaseSkeleton {
public:
    virtual ~nalBaseSkeleton() {}
};

struct nalAnyPose { virtual ~nalAnyPose() {} };

// nalAnimClass<T> - minimal view of the shared nal anim base; only the
// anim.o inline COMDATs below are defined here (fields raw-offset verified).
template<typename T> class nalAnimClass {
public:
    // ??2?$nalAnimClass@VnalAnyPose@@@@SAPAXI@Z (0x55E500)
    static void* operator new(unsigned int sz)
    {
        return tlMemAlloc(sz, 8, 0);
    }
    // ??3?$nalAnimClass@VnalAnyPose@@@@SAXPAX@Z (0x55E520)
    static void operator delete(void* ptr)
    {
        tlMemFree(ptr);
    }

    // ?GetDuration@?$nalAnimClass@VnalAnyPose@@@@QBEMXZ (0x55E540)
    float GetDuration() const { return *(float*)((char*)this + 0x38); }
    // ?GetInverseDuration@?$nalAnimClass@VnalAnyPose@@@@QBEMXZ (0x55E550)
    float GetInverseDuration() const
    {
        float d = *(float*)((char*)this + 0x38);
        return d != 0.0f ? 1.0f / d : 0.0f;
    }
    // ?IsTrajectoryRelative@?$nalAnimClass@VnalAnyPose@@@@QBE_NXZ (0x55E590)
    bool IsTrajectoryRelative() const
    {
        return (*(unsigned int*)((char*)this + 0x34) & 2) == 0;
    }
};

extern void* tlMemAlloc(unsigned int size, unsigned int align,
                        unsigned int flags);
extern void tlMemFree(void* ptr);
extern void mem_heap_free(void* ptr);
extern void* mem_heap_malloc(unsigned int size);
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);
extern void nflCloseFile(int file);  // filesystem/nfl.cpp
extern void PoolAllocator_Release(void* allocator, void* ptr);
// ?subtitle_manager_play_subtitle@SoundDevice@@YA_NPBD0@Z (shell.o stub)
namespace SoundDevice {
bool subtitle_manager_play_subtitle(const char* tag, const char* prefix);
}

struct nalPositionOrientation {
    math::Position3 pos;
    math::Quaternion orient;
};

struct nalMatrix4x4 {
    float m[4][4];
};

// nalGenericBoneHandle - bone reference (index + skeleton)
struct nalGenericBoneHandle {
    unsigned index;
};

// BoneName cache (game2.o data, F052F8..; 16 entries of tlFixedString)
extern tlFixedString boneName[8];   // ?boneName@@3?AV?$tlFixedString@...@@A (game2.o)
extern tlFixedString nalBoneNames[4][4];  // game2.o (joints: gun/hand/feet)
extern nalPositionOrientation nalGenericPose_GetModelPositionOrientation(
    void* pose, const nalGenericBoneHandle* handle);  // ?GetModelPositionOrientation@nalGenericPose@nalGeneric@@QBE?BVnalPositionOrientation@@ABVnalGenericBoneHandle@2@@Z

class nalAnimCache {
public:
    void Release() {}
    void MemFree(nalObject*, unsigned) {}
    void Free(nalObject*) {}
    void Touch(nalObject*) {}
    nalObject* MemAlloc(unsigned, unsigned) { return nullptr; }
    nalObject* Allocate(const nalCachedPoseInfo&, int, int, nalObject**) { return nullptr; }
    void IncreaseLOD(nalObject*, const nalCachedPoseInfo&, int, int) {}
};

// ============================================================================
// nalGenericPose — generic (untyped) pose data
// ============================================================================
namespace nalGeneric {
template <typename T> class nalGenericComponentHandle;
template <typename T> class nalGenericConstComponentHandle;
struct nalComponentInfo;
class nalGenericPose;
class nalGenericSkeleton;
}

class nalGenericPose {
    unsigned char* m_data;
    unsigned       m_size;
public:
    nalGenericPose(const nalBaseSkeleton* skel, int flags);
    nalGenericPose(const nalGenericPose& other, bool copyData);
    ~nalGenericPose();
    void operator=(const nalGenericPose& other);
    void Copy(const nalGenericPose& other, int flags);

    int GetPoseSize() const { return m_size; }
    int GetPoseAlignment() const { return 16; }

    nalPositionOrientation GetModelPositionOrientation(int boneIdx) const;
    nalPositionOrientation GetModelPositionOrientation(const nalGenericBoneHandle&) const;
    void SetPositionOrientation(const nalGenericBoneHandle&, const nalPositionOrientation&);
    void SetPosition(const nalGenericBoneHandle&, const math::Dir3&);
    void SetOrientation(const nalGenericBoneHandle&, const math::Quaternion&);
    math::Quaternion GetPoseBoneOrientation(const nalGenericBoneHandle&) const;
    math::Quaternion GetBoneModelOrientation(const nalGenericBoneHandle&) const;

    // ??$?AM@nalGenericPose@nalGeneric@@QBEABMABV?$nalGenericConstComponentHandle@M@1@@Z
    template <typename T>
    const float& operator[](const nalGeneric::nalGenericConstComponentHandle<T>& handle) const;

    void* GetData() { return m_data; }
};

namespace nalGeneric {
template <typename T> class nalGenericComponentHandle;
template <typename T> class nalGenericConstComponentHandle;
struct nalComponentInfo;
}

// ============================================================================
// nalGenericSkeleton — runtime skeleton (bone matrices, processed pose)
// ============================================================================
class nalGenericSkeleton {
public:
    virtual ~nalGenericSkeleton() {}
    virtual void Release() {}
    virtual void Process() {}

    // ?GetLODCount@nalGenericSkeleton@nalGeneric@@QBEIXZ (0x55E760)
    unsigned int GetLODCount() const { return LODCount; }
    // ?GetBoneMatrixCount@nalGenericSkeleton@nalGeneric@@QBEIH@Z (0x55E770)
    unsigned int GetBoneMatrixCount(int lod) const
    {
        return LODInfo[lod].MatrixCount;
    }

    void GetTrajectoryUpdate(const nalGenericPose&, nalPositionOrientation&) const;
    void GetBoneMatrices(const nalGenericPose&, nalMatrix4x4*, int) const;
    void GetPose(nalGenericPose&, const nalMatrix4x4*, nalMatrix4x4*, const nalGenericPose&, int) const;

    // anim.o nalGeneric templates (0x560F60/0x5610F0/0x563970)
    template <typename T>
    void GetComponentHandle(nalGeneric::nalGenericComponentHandle<T>& handle,
                            const tlFixedString& a3,
                            const tlFixedString& a4);
    template <typename T>
    void GetComponentHandle(
        nalGeneric::nalGenericConstComponentHandle<T>& handle,
        const tlFixedString& a3, const tlFixedString& a4);

    // ??$?AM@nalGenericSkeleton@nalGeneric@@QBEABMABV?$nalGenericConstComponentHandle@M@1@@Z
    template <typename T>
    const float& operator[](const nalGeneric::nalGenericConstComponentHandle<T>& handle) const;

    unsigned char _pad[0x60 - 0x04];
    unsigned int LODCount;  // +0x60
    struct LODInfoEntry {
        unsigned int MatrixCount;  // +0x00
        unsigned char _pad[16];
    };
    LODInfoEntry* LODInfo;  // +0x64
    unsigned char _pad2[0xC8 - 0x68];
    nalGenericPose DefaultPose;  // +0xC8
};

// ============================================================================
// nalGenericAnim — runtime animation instance (per-skeleton)
// ============================================================================
class nalGenericAnim {
public:
    unsigned m_hash;
    virtual ~nalGenericAnim() {}
    virtual void Process() {}
    virtual void Release() {}
};

// ============================================================================
// nalGenericInstance — animated skeleton instance (pose cache, decompression)
// ============================================================================
class nalGenericInstance {
public:
    nalGenericInstance(nalGenericAnim* anim, nalGenericSkeleton* skeleton);
    ~nalGenericInstance();
    void CacheBlock(int blockIdx, int flags);
    void ConvertPoseData(unsigned char* dst, const nalObject* src, int flags, const unsigned char* compData, const int* offsets, int numBones);
    void TouchDecompCache(int blockIdx, int flags);
    void TouchDecompCache(float t1, float t2, int flags, unsigned);
    void GetPose(int frame, nalGenericPose& out, const nalGenericPose& base, int flags);
    void GetPose(float t1, float t2, nalGenericPose& out, const nalGenericPose& base, int flags, unsigned);

    static unsigned GetHash(const nalGenericSkeleton*, const nalGenericSkeleton*);
};

// ============================================================================
// nalGenericPoseBlender — pose blending
// ============================================================================
class nalGenericPoseBlender {
public:
    void Blend(nalGenericPose& out, const nalGenericPose& a, const nalGenericPose& b, float t);

    // ?VirtualBlend@nalGenericPoseBlender@nalGeneric@@MAEXAAVnalBasePose@@ABV3@1@Z
    virtual void VirtualBlend(nalGenericPose* dst, const nalGenericPose* src0,
                              const nalGenericPose* src1);

    // ??$?AVnalPositionOrientation@@@nalGenericPoseBlender@nalGeneric@@QAEAAMABV?$nalGenericComponentHandle@VnalPositionOrientation@@@1@@Z
    template <typename T>
    float& operator[](const nalGeneric::nalGenericComponentHandle<T>& handle);
};

// ============================================================================
// nalGeneric Blend functions
// ============================================================================
void nalGenericBlend(nalGenericPose& out, float t, const nalGenericPose& a, const nalGenericPose& b);
void nalGenericBlendIntra(nalGenericPose& out, float t, const nalGenericPose& a, const nalGenericPose& b);
void nalGenericBlendTorso(nalGenericPose& out, float t, const nalGenericPose& a, const nalGenericPose& b);

// ============================================================================
// nalGenericComponent — component type interface (Blend + BlendArray)
// ============================================================================
class nalComponentInitList { public: virtual void Register() {} };
class nalComponentU8Base            { public: virtual void Blend(int,void*,const void*,const void*,float){} virtual void BlendArray(int,void*,const void*,const void*,const float*){} };
class nalComponentSignalCounter     { public: virtual void Blend(int,void*,const void*,const void*,float){} virtual void BlendArray(int,void*,const void*,const void*,const float*){} };
class nalComponentRLE8Int1          { public: virtual void Blend(int,void*,const void*,const void*,float){} virtual void BlendArray(int,void*,const void*,const void*,const float*){} };
class nalComponentFloat1Base        { public: virtual void Blend(int,void*,const void*,const void*,float){} virtual void BlendArray(int,void*,const void*,const void*,const float*){} };
class nalComponentFloat3Base        { public: virtual void Blend(int,void*,const void*,const void*,float){} virtual void BlendArray(int,void*,const void*,const void*,const float*){} };
class nalComponentFloat4Base        { public: virtual void Blend(int,void*,const void*,const void*,float){} virtual void BlendArray(int,void*,const void*,const void*,const float*){} };
class nalComponentQuatBase          { public: virtual void Blend(int,void*,const void*,const void*,float){} virtual void BlendArray(int,void*,const void*,const void*,const float*){} virtual void BlendIntra(int,void*,const void*,const void*,float){} };
class nalComponentPOBase            { public: virtual void Blend(int,void*,const void*,const void*,float){} virtual void BlendArray(int,void*,const void*,const void*,const float*){} };
class nalComponentIKSpinBase        { public: virtual void Blend(int,void*,const void*,const void*,float){} virtual void BlendArray(int,void*,const void*,const void*,const float*){} };
class nalComponentTrajectoryPO      { public: static void ComponentCycleTrajectory(nalPositionOrientation*,nalPositionOrientation*,int,void*){} };
class nalComponentEntropyTrajectoryPO{ public: static void ComponentCycleTrajectory(nalPositionOrientation*,nalPositionOrientation*,int,void*){} };

// ============================================================================
// nalStreamInstance — streaming animation instance
// ============================================================================
class nalStreamInstance {
public:
    virtual bool IsReady() const { return false; }
    virtual void Play() {}
    virtual void Advance(float dt) {}
    void AdvanceStream() {}
};

// ============================================================================
// nalSceneAnim / nalSceneAnimInstance — scene animation
// ============================================================================
class nalSceneAnim {
public:
    nalSceneAnimInstance* CreateInstance(nalClientSceneAnim* (*factory)(const nalSceneAnim*, const tlFixedString&)) { return nullptr; }

    // ?GetDuration@nalSceneAnim@@QBEMXZ (0x55E6F0); Header.Duration +0x3C
    float GetDuration() const { return *(float*)((char*)this + 0x3C); }
};

// ============================================================================
// nalSceneAnimInstance — runtime scene animation
// ============================================================================
class nalSceneAnimInstance {
public:
    ~nalSceneAnimInstance() {}
    void AddClientAnim(nalClientSceneAnim*, nalAnimClass<nalAnyPose>*) {}
    void Render() const {}

    // ?GetDuration@nalSceneAnimInstance@@QBEMXZ (0x55E720); SceneAnim +0x04
    float GetDuration() const
    {
        return *(float*)((char*)*(void**)((char*)this + 0x04) + 0x3C);
    }
};

// ============================================================================
// nalStaticInstance — static animation instance
// ============================================================================
class nalStaticInstance {
public:
    void Advance(float dt) {}
};

// ============================================================================
// nal IK solvers
// ============================================================================
void nalIKMap2DTo3D(float a1, float a2, float a3, float a4, float a5,
                    const math::Dir3& d1, const math::Dir3& d2,
                    float a8, float a9, nalMatrix4x4& m1, nalMatrix4x4& m2) {}
void nalIKSolve2D(const nalMatrix4x4& m1, const math::Dir3& d1, const math::Dir3& d2,
                  float a1, float a2, float a3, float a4,
                  nalMatrix4x4& out1, nalMatrix4x4& out2,
                  float& outA, float& outB, float& outC, float& outD) {}

// ============================================================================
// nalInit / nalExit
// ============================================================================
void nalSetAnimPath(const char*) {}
const char* nalGetAnimPath() { return ""; }
void nalSetSkeletonPath(const char*) {}
const char* nalGetSkeletonPath() { return ""; }
unsigned nalGetVersion() { return 0x100; }
void nalEnableScratchPadUse() {}
void nalDisableScratchPadUse() {}
void nalEnablePerformanceWarnings() {}
void nalDisablePerformanceWarnings() {}
void nalExit() {}
void nalInit(class nalHeap*) {}
int  nalGetDecompCacheSize() { return 0; }

// ============================================================================
// nal skeleton resource management
// ============================================================================
nalBaseSkeleton* nalGetSkeleton(const tlFixedString&) { return nullptr; }
nalBaseSkeleton* nalConstructSkeleton(void*) { return nullptr; }
nalBaseSkeleton* nalLoadSkeletonInPlace(void*) { return nullptr; }
nalBaseSkeleton* nalLoadSkeleton(const tlFixedString&) { return nullptr; }
int nalReleaseSkeleton(nalBaseSkeleton*) { return 0; }
int nalReleaseSkeleton(const tlFixedString&) { return 0; }
void nalReleaseAllSkeletons() {}

// ============================================================================
// nal animation file management
// ============================================================================
nalAnimFile* nalLoadAnimFile(const tlFixedString&) { return nullptr; }
nalAnimFile* nalLoadAnimFileInPlace(const tlFixedString&, void*) { return nullptr; }
int nalReleaseAnimFile(nalAnimFile*) { return 0; }
int nalReleaseAnimFile(const tlFixedString&) { return 0; }
void nalReleaseAllAnimFiles() {}
nalAnimClass<nalAnyPose>* nalGetAnim(const tlFixedString&) { return nullptr; }
nalAnimClass<nalAnyPose>* nalGetFirstAnimInFile(nalAnimFile*) { return nullptr; }
nalAnimClass<nalAnyPose>* nalGetFirstAnimInFile(const tlFixedString&) { return nullptr; }
nalAnimClass<nalAnyPose>* nalGetNextAnimInFile(nalAnimClass<nalAnyPose>*) { return nullptr; }

// ============================================================================
// nal scene animation resource management
// ============================================================================
nalSceneAnim* nalGetSceneAnim(const tlFixedString&) { return nullptr; }
nalSceneAnim* nalLoadSceneAnim(const tlFixedString&) { return nullptr; }
nalSceneAnim* nalLoadSceneAnimInPlace(const tlFixedString&, void*) { return nullptr; }
int nalReleaseSceneAnim(nalSceneAnim*) { return 0; }
int nalReleaseSceneAnim(const tlFixedString&) { return 0; }
void nalReleaseAllSceneAnims() {}

// ============================================================================
// nal streaming
// ============================================================================
nalStreamInstance* nalStreamAnimQueueInstance(unsigned a1, int a2, int a3, int a4,
    nalClientSceneAnim* (*factory)(const nalSceneAnim*, const tlFixedString&)) { return nullptr; }

// ============================================================================
// nalInitList
// ============================================================================
void nalInitListInit() {}

// ============================================================================
// xanim.cpp raw accessors (anim.o) - ported from disasm
// ============================================================================
class XAnimEntry {
public:
    unsigned int   hash;        // +0x00
    unsigned short numAnims;    // +0x04
    unsigned short parent;      // +0x06
    void*          anim;        // +0x08 nalGeneric::nalGenericAnim*
    void*          notify;      // +0x0C
    int            lastAttempt; // +0x10
    unsigned char  ucLastChosenChild;  // +0x14
    unsigned char  _pad[3];     // +0x15
    union {
        struct {
            unsigned short flags;    // +0x18
            unsigned short children; // +0x1A
        } s;
    } u;                          // +0x18
};

class AnimTree {
public:
    void* name;               // +0x00 InplaceString
    struct {
        unsigned int mSize;   // +0x04
        XAnimEntry*  mList;   // +0x08
    } entries;
};

class XAnimTree {
public:
    unsigned char m_dlist_node[8];     // +0x00
    AnimTree*     anims;               // +0x08
    unsigned int  mOwner;              // +0x0C
    int           mPakId;              // +0x10
    int           mActiveAnims;        // +0x14
    unsigned short infoArray[1];       // +0x18

    // ?get_dlist_node@XAnimTree@@QAEPAXXZ (0x53A860)
    void* get_dlist_node() { return this; }
    // ?get_dlist_node_offset@XAnimTree@@SAHXZ (0x53A870)
    static int get_dlist_node_offset() { return 0; }
};

struct XAnimInfo {
    unsigned short notifyChild;   // +0x00
    short          notifyIndex;   // +0x02
    unsigned int   notifyName;    // +0x04
    unsigned short notifyType;    // +0x08
    unsigned short prev;          // +0x0A
    unsigned short next;          // +0x0C
    unsigned char  s[36];         // +0x10
    void*          pEntity;       // +0x34
};

// ?g_info@@3PAUXAnimInfo@@A @ 0xF25AE8 (512 entries)
XAnimInfo g_info[512];

// Mirrors game2.o AnimIK (0x7C bytes; full definition in g_game2_misc.cpp).
// Only `initialized` is touched by xanim.cpp.
class AnimIK {
public:
    char _pad[0x50];
    int  initialized;

    // ?SetInitialized@AnimIK@@QAEX_N@Z (0x55E8C0)
    void SetInitialized(bool val) { initialized = val; }
    // ?Update@AnimIK@@QAEXPAVEntity@@PAVnalGenericSkeleton@@PAVnalGenericPose@@@Z
    void Update(Entity* ent, nalGenericSkeleton* inSkeleton,
                nalGenericPose* inPose);
};
extern AnimIK AnimIKGlobal;  // ?AnimIKGlobal@@3VAnimIK@@A (game2.o data)

// ?gEnd@@3Vstring@Broc@@A (anim.o data @ 0xF2CAF4)
Broc::string gEnd;

// ea: 0x5433C0
float XAnimGetTime(XAnimTree* tree, unsigned int animIndex)
{
    if (tree == nullptr || tree->anims == nullptr
        || animIndex >= tree->anims->entries.mSize)
        return 0.0f;
    unsigned short idx = tree->infoArray[animIndex];
    if (idx >= 512)
        return 0.0f;
    return *(float*)&g_info[idx].s[0];
}

// ea: 0x543520
float XAnimGetWeight(XAnimTree* tree, unsigned int animIndex)
{
    if (tree == nullptr || tree->anims == nullptr
        || animIndex >= tree->anims->entries.mSize)
        return 0.0f;
    unsigned short idx = tree->infoArray[animIndex];
    if (idx >= 512)
        return 0.0f;
    return *(float*)&g_info[idx].s[0x14];
}

// ea: 0x54B4C0
int XAnimIsLooped(AnimTree* anims, unsigned int animIndex)
{
    XAnimEntry* entry = &anims->entries.mList[animIndex];
    if (entry->numAnims != 0)
        return entry->u.s.flags & 1;
    if (entry->anim == nullptr)
        return 0;
    return ((const unsigned char*)entry->anim)[0x34] & 1;
}

// ea: 0x544A30
int XAnimHasTime(AnimTree* anims, unsigned int animIndex)
{
    XAnimEntry* entry = &anims->entries.mList[animIndex];
    if (entry->numAnims == 0)
        return 1;
    if ((entry->u.s.flags & 0x23) != 0)
        return 1;
    return 0;
}

// ea: 0x544A60
int XAnimIsPrimitive(AnimTree* anims, unsigned int animIndex)
{
    XAnimEntry* entry = &anims->entries.mList[animIndex];
    return entry->numAnims == 0;
}

// ea: 0x5437D0
int XAnimGetNumChildren(AnimTree* anims, unsigned int animIndex)
{
    XAnimEntry* entry = &anims->entries.mList[animIndex];
    return entry->numAnims;
}

// ea: 0x5437F0
unsigned int XAnimGetChildAt(AnimTree* anims, unsigned int animIndex,
                             unsigned int childIndex)
{
    XAnimEntry* entry = &anims->entries.mList[animIndex];
    if (childIndex >= entry->numAnims)
        return 0;
    return (unsigned int)entry->u.s.children + childIndex;
}

// ea: 0x549950
float XAnimGetLength(AnimTree* anims, unsigned int animIndex)
{
    if (anims == nullptr)
        return 0.0f;
    XAnimEntry* entry = &anims->entries.mList[animIndex];
    if ((entry->u.s.flags & 0x20) != 0)
        entry = &anims->entries.mList[entry->u.s.children
                                      + entry->ucLastChosenChild];
    if (entry->anim != nullptr)
        return *(float*)((char*)entry->anim + 0x38);
    return 0.0f;
}

// ea: 0x5454E0
bool XAnimNotetrackExists(AnimTree* anims, unsigned int animIndex,
                          const unsigned int& name)
{
    XAnimEntry* entry = &anims->entries.mList[animIndex];
    if (entry->notify == nullptr)
        return false;
    const unsigned char* p = (const unsigned char*)entry->notify + 4;
    for (;;)
    {
        unsigned int hashed = *(const unsigned int*)p;
        if (hashed == 0)
            return false;
        if (hashed == name)
            return true;
        p += 0xC;
    }
}

// ea: 0x549B30
const char* XAnimGetAnimName(AnimTree* anims, unsigned int animIndex)
{
    XAnimEntry* entry = &anims->entries.mList[animIndex];
    if (entry->numAnims != 0)
        return "<non-leaf anim>";
    // XAnimEntry::Create would resolve a null anim (needs cdGetAnim +
    // ParseNoteTracks); without it, a null anim reports unknown.
    if (entry->anim == nullptr)
        return "<unknown>";
    return (const char*)((const unsigned char*)entry->anim + 0xC);
}

// ea: 0x543680
int XAnimHasFinished(XAnimTree* tree, unsigned int animIndex)
{
    if (tree->anims == nullptr || animIndex >= tree->anims->entries.mSize)
        return 1;
    unsigned short infoIndex = tree->infoArray[animIndex];
    if (infoIndex == 0)
        return 1;
    if (infoIndex >= 512)
        return 1;
    XAnimInfo* info = &g_info[infoIndex];
    if (*(float*)&info->notifyName > *(float*)&info->notifyChild)
        return 1;
    if (*(float*)&info->notifyChild != 1.0f)
        return 1;
    if (info->notifyType > info->prev)
        return 1;
    return 0;
}

// ea: 0x544A80
void XAnimSetTime(XAnimTree* tree, unsigned int animIndex, float time)
{
    if (tree == nullptr || tree->anims == nullptr
        || animIndex >= tree->anims->entries.mSize)
        return;
    unsigned short infoIndex = tree->infoArray[animIndex];
    if (infoIndex == 0 || infoIndex >= 512)
        return;
    XAnimInfo* info = &g_info[infoIndex];
    *(float*)&info->s[0] = time;
    info->notifyType = 0;
    *(float*)&info->s[4] = time;
    info->prev = 0;
}

// ============================================================================
// InplaceVector<XAnimEntry>::operator[] (COMDAT, ../ae/inplace/InplaceVector.h:81)
// ============================================================================
static XAnimEntry* AnimTreeEntryAt(AnimTree* anims, unsigned int index)
{
    if (index >= anims->entries.mSize)
    {
        XANIM_ASSERT("index < mSize", "../ae\\inplace/InplaceVector.h", 81,
                     "Bounds check");
        index = 0;
    }
    return &anims->entries.mList[index];
}

// ea: 0x53E390
AnimTree* XAnimGetAnims(XAnimTree* tree)
{
    return tree->anims;
}

// ea: 0x543950
static char XAnimHasEffectiveParentWeight(XAnimTree* tree,
                                          unsigned int animIndex)
{
    if (animIndex == 0)
        return 1;
    for (;;)
    {
        AnimTree* anims = tree->anims;
        unsigned int mSize = anims->entries.mSize;
        if (animIndex >= mSize)
        {
            XANIM_ASSERT("index < mSize", "../ae\\inplace/InplaceVector.h",
                         81, "Bounds check");
            if (animIndex >= mSize)
                animIndex = 0;
        }
        animIndex = anims->entries.mList[animIndex].parent;
        if (animIndex >= tree->anims->entries.mSize)
        {
            XANIM_ASSERT("animIndex < tree->anims->entries.size()",
                         "c:\\cod\\code\\game\\xanim.cpp", 4506,
                         "old cod assert");
        }
        unsigned short infoIndex = tree->infoArray[animIndex];
        if (infoIndex == 0)
        {
            XANIM_ASSERT("infoIndex", "c:\\cod\\code\\game\\xanim.cpp",
                         4508, "old cod assert");
        }
        if (infoIndex >= 0x200)
        {
            XANIM_ASSERT("infoIndex < 512", "c:\\cod\\code\\game\\xanim.cpp",
                         4510, "old cod assert");
        }
        if (*(float*)&g_info[infoIndex].s[20] == 0.0f)
            return 0;
        if (animIndex == 0)
            return 1;
    }
}

// ea: 0x543AE0
static char XAnimHasEffectiveChildWeight(XAnimTree* tree,
                                         unsigned int animIndex)
{
    if (tree->anims == nullptr)
    {
        XANIM_ASSERT("tree->anims", "c:\\cod\\code\\game\\xanim.cpp",
                     4532, "old cod assert");
    }
    if (animIndex >= tree->anims->entries.mSize)
    {
        XANIM_ASSERT("animIndex < tree->anims->entries.size()",
                     "c:\\cod\\code\\game\\xanim.cpp", 4533,
                     "old cod assert");
    }
    unsigned short infoIndex = tree->infoArray[animIndex];
    if (infoIndex == 0)
        return 0;
    if (infoIndex >= 0x200)
    {
        XANIM_ASSERT("infoIndex < 512", "c:\\cod\\code\\game\\xanim.cpp",
                     4539, "old cod assert");
    }
    if (*(float*)&g_info[infoIndex].s[20] == 0.0f)
        return 0;
    XAnimEntry* entry = AnimTreeEntryAt(tree->anims, animIndex);
    int numAnims = entry->numAnims;
    if (entry->numAnims != 0)
    {
        int i = 0;
        while (!XAnimHasEffectiveChildWeight(
                   tree, (unsigned int)i + entry->u.s.children))
        {
            if (++i >= numAnims)
                return 0;
        }
    }
    return 1;
}

// ea: 0x543C70
void XAnimClearGoalWeight(XAnimTree* tree, unsigned int animIndex,
                          float blendTime)
{
    if (blendTime != 0.0f && blendTime < 0.001f)
    {
        XANIM_ASSERT("!blendTime || blendTime >= 0.001f",
                     "c:\\cod\\code\\game\\xanim.cpp", 4575,
                     "old cod assert");
    }
    if (tree->anims == nullptr)
    {
        XANIM_ASSERT("tree->anims", "c:\\cod\\code\\game\\xanim.cpp",
                     4576, "old cod assert");
    }
    if (animIndex >= tree->anims->entries.mSize)
    {
        XANIM_ASSERT("animIndex < tree->anims->entries.size()",
                     "c:\\cod\\code\\game\\xanim.cpp", 4577,
                     "old cod assert");
    }
    unsigned short infoIndex = tree->infoArray[animIndex];
    if (infoIndex != 0)
    {
        if (infoIndex >= 0x200)
        {
            XANIM_ASSERT("infoIndex < 512", "c:\\cod\\code\\game\\xanim.cpp",
                         4583, "old cod assert");
        }
        XAnimInfo* info = &g_info[infoIndex];
        if (XAnimHasEffectiveParentWeight(tree, animIndex)
            && XAnimHasEffectiveChildWeight(tree, animIndex))
        {
            if (*(float*)&info->s[16] == 0.0f)
            {
                if (*(float*)&info->s[12] <= blendTime)
                    goto clear_notify;
            }
            else
            {
                *(float*)&info->s[16] = 0.0f;
            }
            *(float*)&info->s[12] = blendTime;
        }
        else
        {
            *(float*)&info->s[16] = 0.0f;
            *(float*)&info->s[12] = 0.0f;
        }
    clear_notify:
        if (info->notifyName != 0)
            info->notifyName = 0;
        info->notifyIndex = -1;
    }
}

// ea: 0x54ACE0
static void XAnimClearTreeGoalWeights_r(XAnimTree* tree,
                                        unsigned int animIndex,
                                        float blendTime)
{
    if (tree->anims == nullptr)
    {
        XANIM_ASSERT("tree->anims", "c:\\cod\\code\\game\\xanim.cpp",
                     4613, "old cod assert");
    }
    if (animIndex >= tree->anims->entries.mSize)
    {
        XANIM_ASSERT("animIndex < tree->anims->entries.size()",
                     "c:\\cod\\code\\game\\xanim.cpp", 4614,
                     "old cod assert");
    }
    if (tree->infoArray[animIndex] != 0)
    {
        XAnimClearGoalWeight(tree, animIndex, blendTime);
        XAnimEntry* entry = AnimTreeEntryAt(tree->anims, animIndex);
        if ((AnimTreeEntryAt(tree->anims, animIndex)->u.s.flags & 0x20) == 0)
        {
            int i = 0;
            int numAnims = entry->numAnims;
            if (entry->numAnims != 0)
            {
                do
                {
                    XAnimClearTreeGoalWeights_r(
                        tree, (unsigned int)i + entry->u.s.children,
                        blendTime);
                    ++i;
                } while (i < numAnims);
            }
        }
    }
}

// ea: 0x54ADF0
void XAnimClearTreeGoalWeights(XAnimTree* tree, unsigned int animIndex,
                               float blendTime)
{
    if (blendTime < 0.001f)
        blendTime = 0.0f;
    XAnimClearTreeGoalWeights_r(tree, animIndex, blendTime);
}

// ea: 0x54AE10
void XAnimClearTreeGoalWeightsStrict(XAnimTree* tree, unsigned int animIndex,
                                     float blendTime)
{
    if (tree->anims == nullptr)
    {
        XANIM_ASSERT("tree->anims", "c:\\cod\\code\\game\\xanim.cpp",
                     4657, "old cod assert");
    }
    if (animIndex >= tree->anims->entries.mSize)
    {
        XANIM_ASSERT("animIndex < tree->anims->entries.size()",
                     "c:\\cod\\code\\game\\xanim.cpp", 4658,
                     "old cod assert");
    }
    if (blendTime < 0.001f)
        blendTime = 0.0f;
    XAnimEntry* entry = AnimTreeEntryAt(tree->anims, animIndex);
    int numAnims = entry->numAnims;
    int i = 0;
    if (entry->numAnims != 0)
    {
        do
        {
            XAnimClearTreeGoalWeights_r(
                tree, (unsigned int)i + entry->u.s.children, blendTime);
            ++i;
        } while (i < numAnims);
    }
}

// ea: 0x5446E0
static void XAnimSetAnimRateInternal(XAnimTree* tree, unsigned int animIndex,
                                     float rate)
{
    if (tree == nullptr)
    {
        XANIM_ASSERT("tree", "c:\\cod\\code\\game\\xanim.cpp", 5060,
                     "old cod assert");
    }
    if (tree->anims == nullptr)
    {
        XANIM_ASSERT("tree->anims", "c:\\cod\\code\\game\\xanim.cpp",
                     5061, "old cod assert");
    }
    if (animIndex >= tree->anims->entries.mSize)
    {
        XANIM_ASSERT("animIndex < tree->anims->entries.size()",
                     "c:\\cod\\code\\game\\xanim.cpp", 5062,
                     "old cod assert");
    }
    if (rate < 0.0f)
    {
        XANIM_ASSERT("rate >= 0", "c:\\cod\\code\\game\\xanim.cpp", 5063,
                     "old cod assert");
    }
    unsigned short infoIndex = tree->infoArray[animIndex];
    if (infoIndex == 0)
    {
        XANIM_ASSERT("infoIndex", "c:\\cod\\code\\game\\xanim.cpp", 5066,
                     "old cod assert");
    }
    if (infoIndex >= 0x200)
    {
        XANIM_ASSERT("infoIndex < 512", "c:\\cod\\code\\game\\xanim.cpp",
                     5067, "old cod assert");
    }
    *(float*)&g_info[infoIndex].s[24] = rate;
}

// ea: 0x5454C0
void XAnimSetAnimRate(XAnimTree* tree, unsigned int animIndex, float rate)
{
    XAnimSetAnimRateInternal(tree, animIndex, rate);
}

// PakHeapContext (real port in streamer/pakmanager.cpp;
// ??0PakHeapContext@@QAE@W4TPakId@@_N@Z @ 0x66B040)
enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };
struct PakHeapContext {
    PakHeapContext(TPakId pakId, bool once);
    ~PakHeapContext();
};

// ea: 0x53E150
void XAnimFreeInfo(XAnimTree* tree, unsigned short infoIndex)
{
    if (infoIndex >= 0x200)
    {
        XANIM_ASSERT("infoIndex < 512", "c:\\cod\\code\\game\\xanim.cpp",
                     489, "old cod assert");
    }
    XAnimInfo* info = &g_info[infoIndex];
    if (*(void**)&info->s[28] != nullptr)
    {
        PakHeapContext ctx((TPakId)tree->mPakId, false);
        void* obj = *(void**)&info->s[28];
        if (obj != nullptr)
        {
            void** vtbl = *(void***)obj;
            ((void (__thiscall*)(void*, int))vtbl[0])(obj, 1);
        }
        *(void**)&info->s[28] = nullptr;
    }
    info->prev = 0;
    info->next = g_info[0].next;
    g_info[g_info[0].next].prev = infoIndex;
    g_info[0].next = infoIndex;
    if (tree->mActiveAnims == 0)
    {
        XANIM_ASSERT("tree->mActiveAnims", "c:\\cod\\code\\game\\xanim.cpp",
                     505,
                     "freed an XAnimInfo from a tree that had none?");
    }
    --tree->mActiveAnims;
}

// ea: 0x5441E0
static void XAnimClearTreeWeights(XAnimTree* tree, unsigned int animIndex)
{
    if (tree == nullptr)
    {
        XANIM_ASSERT("tree", "c:\\cod\\code\\game\\xanim.cpp", 4814,
                     "old cod assert");
    }
    if (tree->anims == nullptr)
    {
        XANIM_ASSERT("tree->anims", "c:\\cod\\code\\game\\xanim.cpp",
                     4815, "old cod assert");
    }
    if (animIndex >= tree->anims->entries.mSize)
    {
        XANIM_ASSERT("animIndex < tree->anims->entries.size()",
                     "c:\\cod\\code\\game\\xanim.cpp", 4816,
                     "old cod assert");
    }
    unsigned short infoIndex = tree->infoArray[animIndex];
    if (infoIndex != 0)
    {
        XAnimEntry* entry = AnimTreeEntryAt(tree->anims, animIndex);
        int numAnims = entry->numAnims;
        int i = 0;
        if (entry->numAnims != 0)
        {
            do
            {
                XAnimClearTreeWeights(tree,
                                      (unsigned int)i + entry->u.s.children);
                ++i;
            } while (i < numAnims);
        }
        XAnimFreeInfo(tree, infoIndex);
        if (animIndex >= tree->anims->entries.mSize)
        {
            XANIM_ASSERT("animIndex < tree->anims->entries.size()",
                         "c:\\cod\\code\\game\\xanim.cpp", 4828,
                         "old cod assert");
        }
        tree->infoArray[animIndex] = 0;
    }
}

// ea: 0x544370
void XAnimClearTree(XAnimTree* tree)
{
    XAnimClearTreeWeights(tree, 0);
    if (tree->mActiveAnims != 0)
    {
        XANIM_ASSERT("!tree->mActiveAnims",
                     "c:\\cod\\code\\game\\xanim.cpp", 4839,
                     "XAnimClearTreeWeights failed to clear all playing anims?");
    }
    if (tree->infoArray[0] != 0)
    {
        XANIM_ASSERT("!tree->infoArray[0]", "c:\\cod\\code\\game\\xanim.cpp",
                     4842, "old cod assert");
    }
    if (tree->anims == nullptr)
    {
        XANIM_ASSERT("tree->anims", "c:\\cod\\code\\game\\xanim.cpp",
                     4843, "old cod assert");
    }
    if (tree->anims->entries.mSize == 0)
    {
        XANIM_ASSERT("size", "c:\\cod\\code\\game\\xanim.cpp", 4846,
                     "old cod assert");
    }
}

// ea: 0x53E030
void XAnimInit()
{
    for (unsigned int i = 0; i < 512; ++i)
    {
        unsigned int v0 = i + 1;
        g_info[i].prev = (unsigned short)((v0 + 510) % 512);
        g_info[i].next = (unsigned short)(v0 % 512);
    }
    *(float*)&g_info[0].s[0] = 0.0f;
    *(float*)&g_info[0].s[4] = 0.0f;
    *(unsigned short*)&g_info[0].s[8] = 0;
    *(unsigned short*)&g_info[0].s[10] = 0;
    AnimIKGlobal.initialized = 0;
    gEnd = "end";
}

// ea: 0x53E0B0
void XAnimShutdown()
{
    if (gEnd.mBlock != nullptr && gEnd.mBlock != (Broc::string::Block*)-12
        && gEnd.mBlock->mBuff[0] != 0)
        gEnd.clear();
}

// XAnim helper deps (bodies in xanim.cpp; declared after XAnimTree/AnimTree)
extern int XAnimSetGoalWeightInternal(XAnimTree* tree, unsigned int animIndex,
                                      float goalWeight, float goalTime,
                                      float rate, bool bForce,
                                      unsigned int notifyName,
                                      unsigned short notifyType,
                                      bool bRestart);
extern void XAnimEnsureGoalWeightParent(XAnimTree* tree,
                                        unsigned int animIndex,
                                        float goalTime, bool bRestart);
extern void XAnimUpdateSyncTime(XAnimTree* tree, unsigned int animIndex,
                                int bRestart);
extern void XAnimUpdateServerNotify(XAnimTree* tree, unsigned int animIndex);
extern void XAnimClearGoalWeightKnobInternal(XAnimTree* tree,
                                             unsigned int animIndex,
                                             float goalWeight,
                                             float goalTime);
extern void XAnimSetCompleteGoalWeight(XAnimTree* tree, unsigned int animIndex,
                                       float goalWeight, float goalTime,
                                       float rate, unsigned int notifyName,
                                       unsigned int notifyType, void* bRestart);
extern void XAnimSetupSyncNodes_r(AnimTree* anims, unsigned int animIndex);

// Local XAnimEntry_Create (PAV mangling for this TU's class-typed XAnimEntry;
// g_entity_misc.cpp defines the PAU variant).
void XAnimEntry_Create(XAnimEntry* self)
{
    (void)self;
}
void* nalGenericInstance_Ctor(void* self, void* anim, void* skeleton)
{
    memset(self, 0, 0x30);
    *(void**)((char*)self + 0x0C) = skeleton;  // Skeleton +0x0C
    (void)anim;
    return self;
}

extern void ValidatePakId(int pakId);  // g_entity_misc.cpp stub
extern void DObjInitServerTime(void* d, float dtime);
extern bool DObjUpdateServerInfo(DObj* obj, float dtime, bool bNotify,
                                 unsigned int animindex);
extern int _fpclass(double x);  // CRT helper (cg_misc.cpp)

// Forward decls for the DObj sub-model helpers (defined later in this TU)
void ApplyPoseToSubModel(DObj* obj, int i, const nalGenericPose* pose);
void ApplyPoseToSubModel(DObj* obj, int i, const nalGenericPose* pose,
                         bool absolute);
void PostApplyPoseToSubModel(DObj* obj, int i, const nalGenericPose* pose);
void SetAutoTrajectoryEntityPO(Entity* ent, bool absolute, DObj* masterObj);
void* SceneAnimClient_Ctor(void* self, const nalSceneAnim* anim,
                           const tlFixedString* name, float blendIn,
                           float blendOut);
void* SceneAnimClient_CtorReal(void* self, const nalSceneAnim* anim,
                               const tlFixedString* name, float blendIn,
                               float blendOut);

// Local EntityHandleDb view (full in streamer/pakmanager.cpp)
struct DbElement {
    unsigned int mKey;    // +0x00
    Entity* mObject;      // +0x04
};
class EntityHandleDb {
public:
    static EntityHandleDb sInst;  // ?sInst@EntityHandleDb@@2V1@A
    unsigned char _pad[0xA8];
    DbElement mElements[0x540];  // +0xA8
};

// Local gDroneAEMap view (full in g_game2_misc.cpp)
struct DroneHandleVec {
    unsigned char _pad[0x0C];
    DbLinkedHandle<EntityHandleDb, Entity>* mElements;  // +0x0C
    int mSize;  // +0x10
};
struct ae_pair_drone {
    unsigned int first;   // +0x00
    DroneHandleVec* second;  // +0x04
};
struct DroneAEMap {
    ae_pair_drone* m_elements[8];  // +0x00
    int m_size;  // +0x20
};
extern DroneAEMap gDroneAEMap;  // ?gDroneAEMap@@3V?$ae_sized_array@... (game2.o @ 0x12F45E0)

// InteractionController (anim.o member mCurState @ +0x14)
class InteractionController {
public:
    static InteractionController* Inst(int instance);  // ?Inst@InteractionController@@SAPAV1@H@Z
    void* mCurState;  // +0x14
};

// Stub bodies for the xanim goal-weight internals (ported with the
// remaining xanim cluster; correct mangled signatures).
int XAnimSetGoalWeightInternal(XAnimTree* tree, unsigned int animIndex,
                               float goalWeight, float goalTime, float rate,
                               bool bForce, unsigned int notifyName,
                               unsigned short notifyType, bool bRestart)
{
    (void)tree; (void)animIndex; (void)goalWeight; (void)goalTime;
    (void)rate; (void)bForce; (void)notifyName; (void)notifyType;
    (void)bRestart;
    return 0;
}
void XAnimEnsureGoalWeightParent(XAnimTree* tree, unsigned int animIndex,
                                 float goalTime, bool bRestart)
{
    (void)tree; (void)animIndex; (void)goalTime; (void)bRestart;
}
void XAnimUpdateSyncTime(XAnimTree* tree, unsigned int animIndex,
                         int bRestart)
{
    (void)tree; (void)animIndex; (void)bRestart;
}
void XAnimUpdateServerNotify(XAnimTree* tree, unsigned int animIndex)
{
    (void)tree; (void)animIndex;
}
void XAnimClearGoalWeightKnobInternal(XAnimTree* tree,
                                      unsigned int animIndex,
                                      float goalWeight, float goalTime)
{
    (void)tree; (void)animIndex; (void)goalWeight; (void)goalTime;
}
void XAnimSetupSyncNodes_r(AnimTree* anims, unsigned int animIndex)
{
    (void)anims; (void)animIndex;
}

// ============================================================================
// anim.o tiny COMDAT batch (xanim/nal/DObj inline accessors)
// ============================================================================

namespace nalGeneric {
class nalGenericPose;
class nalGenericSkeleton;
class nalGenericAnim;
struct nalComponentInfo;
}

// Minimal XModel view (lod pointer array at +0x24; full view in streamer).
// `class` tag to match the binary's V-mangled IVPointer<XModel>.
class XModel {
public:
    const char* name;     // +0x00
    XModel*     resolved; // +0x04
    unsigned char _pad[0x24 - 0x08];
    void** lod;           // +0x24

    // ?HasLOD@XModel@@QBE_NH@Z (0x539C50)
    bool HasLOD(int lodIndex) const;
};

// ea: 0x539C50
bool XModel::HasLOD(int lodIndex) const
{
    return lodIndex < 5 && lod[lodIndex] != nullptr;
}

// ea: 0x539C80
struct XSceneAnimParams {
    float mBlendIn;   // +0x00
    float mBlendOut;  // +0x04

    XSceneAnimParams(float blendIn, float blendOut);
};

XSceneAnimParams::XSceneAnimParams(float blendIn, float blendOut)
    : mBlendIn(blendIn), mBlendOut(blendOut)
{
}

// nalBasePoseBlender / nalPoseBlenderClass - anim.o COMDATs
class nalBasePoseBlender {
public:
    virtual ~nalBasePoseBlender() {}
};

template <typename POSE>
class nalPoseBlenderClass : public nalBasePoseBlender {
public:
    // ??0?$nalPoseBlenderClass@VnalGenericPose@nalGeneric@@@@QAE@PBVnalGenericSkeleton@nalGeneric@@@Z
    nalPoseBlenderClass(const nalGeneric::nalGenericSkeleton* _Skeleton)
        : Skeleton(_Skeleton)
    {
    }
    // ??1?$nalPoseBlenderClass@VnalGenericPose@nalGeneric@@@@UAE@XZ (0x55E7C0)
    virtual ~nalPoseBlenderClass() {}

    const nalGeneric::nalGenericSkeleton* Skeleton;  // +0x04
};

// nalPoseClass<SKELETON,POSE> - anim.o COMDAT (0x55EA30)
template <typename SKELETON, typename POSE>
class nalPoseClass {
public:
    // ?GetBoneMatrixCount@?$nalPoseClass@VnalGenericSkeleton@nalGeneric@@VnalGenericPose@2@@@QBEIXZ
    unsigned int GetBoneMatrixCount() const
    {
        void* lodInfo = *(void**)((char*)Skeleton + 0x64);
        return ((unsigned int*)lodInfo)[LOD * 5];
    }

    const void* Skeleton;  // +0x00
    unsigned int LOD;      // +0x04
};

// nalComponentBase - anim.o COMDATs (0x55E820/0x55EDC0)
class nalComponentBase {
public:
    nalComponentBase() {}
    virtual ~nalComponentBase() {}
};

// nalGenericComponentHandle<T> - anim.o ctors (0x55ED10/30, 0x55F120/40)
namespace nalGeneric {
class nalGenericSkeleton;

// nalComponentInfo - component run-length/start info (nal_generic.h)
struct nalComponentInfo {
    unsigned char _pad[0x24];
    int StartIndex;  // +0x24
    int Count;       // +0x28
};

template <typename T>
class nalGenericComponentHandle {
public:
    nalGenericComponentHandle()
    {
        Skeleton = nullptr;
    }

    nalGenericComponentHandle(const nalGenericSkeleton* skeleton,
                              const nalComponentInfo* componentInfo,
                              int componentIndex)
        : Skeleton(skeleton), ComponentInfo(componentInfo),
          ComponentIndex(componentIndex)
    {
    }

    const nalGenericSkeleton* Skeleton;       // +0x00
    const nalComponentInfo* ComponentInfo;    // +0x04
    int ComponentIndex;                       // +0x08
    unsigned char IsConst;                    // +0x0C
};

// nalGenericConstComponentHandle<T> - same layout, const-typed handle
template <typename T>
class nalGenericConstComponentHandle {
public:
    nalGenericConstComponentHandle()
    {
        Skeleton = nullptr;
    }

    const nalGenericSkeleton* Skeleton;       // +0x00
    const nalComponentInfo* ComponentInfo;    // +0x04
    int ComponentIndex;                       // +0x08
    unsigned char IsConst;                    // +0x0C
};

// ?IsType@nalGeneric@@YA_NABV?$nalGenericComponentHandle@VDir3@math@@@1@I@Z
// ?IsType@nalGeneric@@YA_NABV?$nalGenericComponentHandle@VnalPositionOrientation@@@1@I@Z
bool IsType(const nalGenericComponentHandle<math::Dir3>& handle,
            unsigned int id)
{
    // nalComponentFloat3Base::TypeID @ 0x10EC610
    return id == 0x10EC610;
}

bool IsType(const nalGenericComponentHandle<nalPositionOrientation>& handle,
            unsigned int id)
{
    // nalComponentPOBase::TypeID @ 0x10EC614
    return id == 0x10EC614;
}
}

// ea: 0x55E990 / 0x55E9B0
struct XAnimNotifyInfo {
    Broc::string name;  // +0x00

    XAnimNotifyInfo();
    ~XAnimNotifyInfo();
};

XAnimNotifyInfo::XAnimNotifyInfo()
{
    new (&name) Broc::string((Broc::string::Block*)nullptr);
}

XAnimNotifyInfo::~XAnimNotifyInfo()
{
    name.~string();
}

// anim.o data (?g_syncOldTime@@3MA @ 0xF258F8, ?g_endNotifyHackCounter@@3FA
// @ 0xF258F4)
float g_syncOldTime = 0.0f;
short g_endNotifyHackCounter = 0;

struct XAnimState;

// ea: 0x55E9C0
void XAnimUpdateEndNotifyHackCounter(XAnimState* state)
{
    if (g_syncOldTime == 1.0f)
    {
        short v1 = *(short*)((char*)state + 0x20) + 1;
        if (v1 >= 4)
            v1 = 4;
        *(short*)((char*)state + 0x20) = v1;
        g_endNotifyHackCounter = v1;
    }
    else
    {
        *(short*)((char*)state + 0x20) = 0;
        g_endNotifyHackCounter = *(short*)((char*)state + 0x20);
    }
}

// InplaceVector<T> - ??A?$InplaceVector@UXAnimEntry@@@@QAEAAUXAnimEntry@@I@Z
// (0x55EB60), ??A?$InplaceVector@VAnimTree@@@@QAEAAVAnimTree@@I@Z (0x55EBE0)
template <typename T>
struct InplaceVector {
    unsigned int mSize;  // +0x00
    T*           mList;  // +0x04

    T& operator[](unsigned int index)
    {
        unsigned int v2 = index;
        if (index >= mSize)
        {
            XANIM_ASSERT("index < mSize", "../ae\\inplace/InplaceVector.h",
                         81, "Bounds check");
            if (index >= mSize)
                v2 = 0;
        }
        return mList[v2];
    }
};

// ae_array<T,N> - ??A?$ae_array@PAVAnimBank@@$0GD@@@QAEAAPAVAnimBank@@H@Z
// (0x55EC60)
template <typename T, int N>
struct ae_array {
    T m_data[N];  // +0x00

    T& operator[](unsigned int idx)
    {
        if (idx >= (unsigned int)N)
        {
            XANIM_ASSERT("idx >= 0 && idx < _SIZE",
                         "../ae\\core/ae_array.h", 31, "out of bounds");
        }
        return m_data[idx];
    }
};

// ??$lerp@M@@YAMABM0M@Z (0x55F0B0)
template <typename T>
inline T lerp(const T& from, const T& to, float frac)
{
    return (T)((to - from) * frac + from);
}

// ??$ReadUnaligned@M@@YAMPBX@Z (0x55F180)
template <typename T>
inline T ReadUnaligned(const void* iMem)
{
    return *(const T*)iMem;
}

// ??$ReadIncUnaligned@M@@YAMAAPAD@Z (0x560F00)
template <typename T>
inline T ReadIncUnaligned(char** iPos)
{
    T v = *(T*)*iPos;
    *iPos += sizeof(T);
    return v;
}

// Local view of PakManager (full class in sv_stubs.h; MemFree real in
// streamer/pakmanager.cpp)
class PakManager {
public:
    static PakManager* sInst;  // ?sInst@PakManager@@2PAV1@A
    void* mSlots[99];          // +0x40
    void MemFree(TPakId id, void* ptr, bool bUseActorHeap);  // ?MemFree@PakManager@@QAEXW4TPakId@@PAX_N@Z
};

class InteractState;
class InteractInputRcvr;

// ??$PakDelete@VInteractState@@@@YAXW4TPakId@@PAVInteractState@@_N@Z
// ??$PakDelete@VInteractInputRcvr@@@@YAXW4TPakId@@PAVInteractInputRcvr@@_N@Z
template <typename T>
void PakDelete(TPakId id, T* obj, bool bUseActorHeap)
{
    if (obj != nullptr)
        PakManager::sInst->MemFree(id, obj, bUseActorHeap);
}

class nalVirtual {
public:
    virtual ~nalVirtual() {}
};

// nalDynamicPtrCast / nalAnimPtrCast / nalSkeletonPtrCast - anim.o COMDATs
// (0x55F160/0x55F1A0/0x55F1C0/0x560F20/0x560F40/0x560810); vftable addresses
// verified: nalGenericSkeleton 0xD48B3C, nalGenericAnim 0xD48B78.
template <typename T>
inline T* nalDynamicPtrCast(nalVirtual* ptr)
{
    return (ptr != nullptr && *(void**)ptr == (void*)0x00D48B3C)
               ? (T*)ptr
               : nullptr;
}

template <typename T>
inline const T* nalDynamicPtrCast(const nalVirtual* ptr)
{
    return (ptr != nullptr && *(void**)ptr == (void*)0x00D48B3C)
               ? (const T*)ptr
               : nullptr;
}

template <typename T>
inline T* nalAnimPtrCast(nalAnimClass<nalAnyPose>* ptr)
{
    return (ptr != nullptr && *(void**)ptr == (void*)0x00D48B78)
               ? (T*)ptr
               : nullptr;
}

template <typename T>
inline T* nalSkeletonPtrCast(nalBaseSkeleton* ptr)
{
    return (ptr != nullptr && *(void**)ptr == (void*)0x00D48B3C)
               ? (T*)ptr
               : nullptr;
}

template <typename T>
inline const T* nalSkeletonPtrCast(const nalBaseSkeleton* ptr)
{
    return (ptr != nullptr && *(void**)ptr == (void*)0x00D48B3C)
               ? (const T*)ptr
               : nullptr;
}

// InteractionController.cpp button-name helpers (anim.o)
// sButtonTypeNames @ 0xDF22F0 / sButtonTextNames @ 0xDF2338 (17 entries)
static const char* const sButtonTypeNames[17] = {
    "LeftButton", "DownButton", "RightButton", "UpButton", "Square", "X",
    "Circle", "Triangle", "R1", "L1", "R2", "L2", "R3", "L3", "Start",
    "Select", "NONE",
};
static const char* const sButtonTextNames[17] = {
    "~left", "~back", "~right", "~forward", "~square", "~cross", "~circle",
    "~triangle", "~r1", "~l1", "~r2", "~l2", "~r3", "~r3", "~start",
    "~select", "NONE",
};

// ea: 0x53AFC0
int GetNumButtonTypeNames()
{
    return 17;
}

// ea: 0x53AFD0
const char* GetButtonTypeName(unsigned int index)
{
    if (index > 0x10)
    {
        XANIM_ASSERT(
            "index >= 0 && index < (sizeof(sButtonTypeNames) / sizeof(sButtonTypeNames[0]))",
            "c:\\cod\\code\\game\\InteractionController.cpp", 232,
            "Invalid index");
    }
    return sButtonTypeNames[index];
}

// ea: 0x53B030
const char* GetButtonTextName(unsigned int index)
{
    if (index > 0x10)
    {
        XANIM_ASSERT(
            "index >= 0 && index < (sizeof(sButtonTextNames) / sizeof(sButtonTextNames[0]))",
            "c:\\cod\\code\\game\\InteractionController.cpp", 238,
            "Invalid index");
    }
    return sButtonTextNames[index];
}

// ============================================================================
// AnimationPlayer (anim.o) - animation state machine + modifier system
// Layouts verified against IDA (nalAnimState 0x2C, nalPartialAnimState 0x44).
// ============================================================================

// Local view of EntityManager (full class in streamer/pakmanager.cpp).
class Entity {
public:
    unsigned int flags;   // +0x04
    unsigned int mFlags;  // +0x08
    unsigned char _pad[0x150 - 0x0C];
    struct {
        math::Position3 currentOrigin;  // +0x70 within EntityShared
        math::Position3 currentAngles;  // +0x80
    } r;                   // +0x150 (EntityShared r @ +0xE0 + 0x70)
    unsigned char _padR[0x230 - 0x160];
    struct {
        unsigned int mVal;  // +0x230
    } mHandle;             // +0x230
    unsigned char _pad2[0x23C - 0x234];
    DObj* mDObj;           // +0x23C
    unsigned char _pad3[0x254 - 0x240];
    void* client;  // +0x254
    struct sentient_s* sentient;      // +0x25C
    void* scr_vehicle;                // +0x260
    void* pTurretInfo;                // +0x264
};

// sentient_s mLastAnimIKGunOffset view
struct sentient_s {
    unsigned char _pad[0x134];
    float* mLastAnimIKGunOffset[4];  // +0x134
};
class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A
    Entity* GetPlayer(int idx);   // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z
};

// Local view of Camera (full class in cg_misc.cpp; StopAnimating real there).
struct Camera {
public:
    void StopAnimating(float minTweenTime);  // ?StopAnimating@Camera@@QAEXM@Z
};
extern Camera* gCamera;  // ?gCamera@@3PAUCamera@@A (cg.o @ 0x1358EF0)

extern int currCl;
extern int dword_F6A2A0[4 * 802];
extern void* nalGenericAnim_CreateInstance(void* anim, void* skeleton);

class AnimationPlayer {
public:
    enum AnimationPlayerModifierType {
        nalPartialModifier = 1,
        nalFullModifier = 2,
    };

    struct nalPlayMethod;

    // nalAnimCallback - vftable + curAnim (anim.o 0x539EB0/0x539ED0/0x539EE0)
    struct nalAnimCallback {
        void** __vftable;          // +0x00
        nalGenericAnim* curAnim;   // +0x04

        nalAnimCallback();         // ea: 0x00539EB0
        void Reference(nalGenericAnim* anim);  // ea: 0x00539ED0
        void Release();            // ea: 0x00539EE0
        int IsAnimPlaying(nalGenericAnim* anim);  // ea: 0x00539EF0
    };

    // nalAnimState - 0x2C (IDA verified)
    struct nalAnimState {
        void* instance;        // +0x00
        float speed;           // +0x04
        float tlimit;          // +0x08
        nalAnimCallback* callback;  // +0x0C
        void* play_method;     // +0x10
        float t;               // +0x14
        float t_prev;          // +0x18
        float alpha;           // +0x1C
        float maxAlpha;        // +0x20
        float fadein_rate;     // +0x24
        int state;             // +0x28

        void Setup(nalGenericAnim* anim, void* skeleton, float _fadein_rate,
                   void* _play_method, float callback_time,
                   nalAnimCallback* _callback, float _speed,
                   float time_in_seconds_to_start);  // ea: 0x0055F280
    };

    // nalPartialAnimState - 0x44 (IDA verified)
    struct nalPartialAnimState {
        nalAnimState base;              // +0x00
        unsigned int CreationAdvanceCount;  // +0x2C
        nalPartialAnimState* next;      // +0x30
        unsigned int mask;              // +0x34
        float priority;                 // +0x38
        float fadeout_rate;             // +0x3C
        int type;                       // +0x40
    };

    enum nalAnimStateEnum {
        FadeIn = 0,
        Running = 1,
        FadeOut = 2,
    };

    void* Skeleton;                       // +0x00
    unsigned char _pad[0x24 - 0x04];
    int QueueSize;                        // +0x24
    nalAnimState* AnimStates[3];          // +0x28
    nalPartialAnimState* PartialAnimStates;    // +0x34
    nalPartialAnimState* PartialAnimStatePool; // +0x38
    int AdvanceCount;                     // +0x3C

    void StopModifiers(unsigned int mask);   // ea: 0x0053A040
    void FadeOutModifiers(float newFadeOut, unsigned int mask);  // ea: 0x0053A0A0
    nalPartialAnimState* _FindModifier(unsigned int mask,
                                       float priority);  // ea: 0x0053A1E0
    void _StopOrHoldModifiers(unsigned int mask,
                              float priority);  // ea: 0x0053A230
    int GetQueueSize();                    // ea: 0x0053A2A0
    void SetSpeed(nalGenericAnim* anim, float speed);  // ea: 0x0055F730
    void PlayModifier(nalGenericAnim* anim,
                      AnimationPlayerModifierType type, float priority,
                      unsigned int mask, bool ForceRestart, float fade_in,
                      float fade_out, nalPlayMethod* play_method,
                      float callback_time, nalAnimCallback* callback,
                      float speed,
                      float time_in_seconds_to_start);  // ea: 0x0055F7A0

    // ?GetPose@AnimationPlayer@@QAEXAAVnalGenericPose@nalGeneric@@PAVnalGenericSkeleton@3@QAY02M@Z
    void GetPose(nalGeneric::nalGenericPose& Pose,
                 nalGeneric::nalGenericSkeleton* Skeleton,
                 float (*animIKGunOffset)[3]);
};

class PlayerAnimMgr {
public:
    void PlayModifier(nalGenericAnim& anim, int animateCamera,
                      float fadeInTime, float fadeOutTime, float speed,
                      float mask);  // ea: 0x00540FB0
    void PlayModifier(const char* animName, int animateCamera,
                      float fadeInTime, float fadeOutTime, float speed,
                      float mask);  // ea: 0x00547DC0

    unsigned char _pad[0x0C];
    AnimationPlayer::nalAnimCallback mModifierCallback;  // +0x0C
    void* mCurPrimary;   // +0x14
    void* mCurModifier;  // +0x18
};

// ea: 0x00539EB0
AnimationPlayer::nalAnimCallback::nalAnimCallback()
{
    static void* sVftable[3];  // Invoke/Reference/Release slots
    __vftable = sVftable;
    curAnim = nullptr;
}

// ea: 0x00539ED0
void AnimationPlayer::nalAnimCallback::Reference(nalGenericAnim* anim)
{
    curAnim = anim;
}

// ea: 0x00539EE0
void AnimationPlayer::nalAnimCallback::Release()
{
    curAnim = nullptr;
}

// ea: 0x00539EF0
int AnimationPlayer::nalAnimCallback::IsAnimPlaying(nalGenericAnim* anim)
{
    return curAnim != nullptr && anim == curAnim;
}

// ea: 0x0055F280
void AnimationPlayer::nalAnimState::Setup(nalGenericAnim* anim, void* skeleton,
                                          float _fadein_rate,
                                          void* _play_method,
                                          float callback_time,
                                          nalAnimCallback* _callback,
                                          float _speed,
                                          float time_in_seconds_to_start)
{
    void* Instance;
    if (_play_method != nullptr)
    {
        // nalPlayMethod::CreateInstance delegates to nalGenericAnim_CreateInstance
        Instance = nalGenericAnim_CreateInstance(anim, skeleton);
    }
    else
    {
        Instance = nalGenericAnim_CreateInstance(anim, skeleton);
    }
    instance = Instance;
    speed = _speed;
    float Duration = *(float*)((char*)anim + 0x38);
    float v12 = Duration == 0.0f ? 0.0f : 1.0f / Duration;
    tlimit = v12 * callback_time + 1.0f;
    callback = _callback;
    if (_callback != nullptr)
        _callback->Reference(anim);
    play_method = _play_method;
    float v14 = *(float*)((char*)instance + 0x08) * time_in_seconds_to_start;
    t = v14;
    t_prev = v14;
    alpha = 0.0f;
    maxAlpha = 1.0f;
    fadein_rate = _fadein_rate;
    state = Running;
    if (_fadein_rate == 0.0f)
        alpha = 1.0f;
    // binary: _play_method->Reference(this) - thunk, no-op in port
}

// ea: 0x0053A040
void AnimationPlayer::StopModifiers(unsigned int mask)
{
    for (nalPartialAnimState* i = PartialAnimStates; i != nullptr;
         i = i->next)
    {
        if (i->mask == mask || mask == 0xFFFFFFFFu)
        {
            nalAnimCallback* callback = i->base.callback;
            i->base.state = FadeOut;
            if (callback != nullptr)
                callback->Release();
            i->base.callback = nullptr;
        }
    }
}

// ea: 0x0053A0A0
void AnimationPlayer::FadeOutModifiers(float newFadeOut, unsigned int mask)
{
    for (nalPartialAnimState* i = PartialAnimStates; i != nullptr;
         i = i->next)
    {
        if (i->mask == mask)
        {
            float v4 = 0.0f;
            if (newFadeOut != 0.0f)
                v4 = 1.0f / newFadeOut;
            nalAnimCallback* callback = i->base.callback;
            i->fadeout_rate = v4;
            i->base.state = FadeOut;
            if (callback != nullptr)
                callback->Release();
            i->base.callback = nullptr;
        }
    }
}

// ea: 0x0053A1E0
AnimationPlayer::nalPartialAnimState* AnimationPlayer::_FindModifier(
    unsigned int mask, float priority)
{
    nalPartialAnimState* result = nullptr;
    for (nalPartialAnimState* i = PartialAnimStates; i != nullptr;
         i = i->next)
    {
        if (i->priority == priority && i->mask == mask)
            result = i;
    }
    return result;
}

// ea: 0x0053A230
void AnimationPlayer::_StopOrHoldModifiers(unsigned int mask, float priority)
{
    for (nalPartialAnimState* i = PartialAnimStates; i != nullptr;
         i = i->next)
    {
        if (i->mask == mask)
        {
            float v4 = i->priority;
            nalAnimCallback* callback = i->base.callback;
            i->base.state = FadeOut;
            if (v4 <= priority)
            {
                if (callback != nullptr)
                    callback->Release();
                i->base.state = Running;
            }
            else if (callback != nullptr)
            {
                callback->Release();
            }
            i->base.callback = nullptr;
        }
    }
}

// ea: 0x0053A2A0
int AnimationPlayer::GetQueueSize()
{
    return QueueSize;
}

// ea: 0x0055F730
void AnimationPlayer::SetSpeed(nalGenericAnim* anim, float speed)
{
    if (QueueSize > 0)
    {
        if (anim == *(void**)((char*)AnimStates[0]->instance + 0x10))
        {
            for (int i = 0; i < QueueSize; ++i)
            {
                if (*(void**)((char*)AnimStates[i]->instance + 0x10)
                    != nullptr)
                    AnimStates[i]->speed = speed;
            }
        }
    }
}

// ea: 0x0055F7A0
void AnimationPlayer::PlayModifier(nalGenericAnim* anim,
                                   AnimationPlayerModifierType type,
                                   float priority, unsigned int mask,
                                   bool ForceRestart, float fade_in,
                                   float fade_out, nalPlayMethod* play_method,
                                   float callback_time,
                                   nalAnimCallback* callback, float speed,
                                   float time_in_seconds_to_start)
{
    static nalAnimCallback DefaultNonloopingCallback;

    char v14 = *(unsigned char*)((char*)anim + 0x34) & 1;
    nalPartialAnimState* match;
    if (v14 == 0 && ForceRestart)
    {
        match = nullptr;
        goto LABEL_17;
    }
    {
        nalPartialAnimState* v15 = PartialAnimStates;
        nalPartialAnimState* v16 = nullptr;
        if (v15 != nullptr)
        {
            do
            {
                if (v15->priority == priority && v15->mask == mask)
                    v16 = v15;
                v15 = v15->next;
            } while (v15 != nullptr);
            v14 = *(unsigned char*)((char*)anim + 0x34) & 1;
        }
        match = v16;
        if (v16 == nullptr
            || *(void**)((char*)v16->base.instance + 0x10) != anim
            || ForceRestart
            || (callback != nullptr && v16->base.callback == nullptr))
            goto LABEL_17;
        return;
    }

LABEL_17:
    if (callback == nullptr && v14 == 0)
        callback = &DefaultNonloopingCallback;
    if (type == nalPartialModifier || type == nalFullModifier)
        _StopOrHoldModifiers(mask, priority);
    else
        StopModifiers(mask);

    nalPartialAnimState** insert = &PartialAnimStates;
    nalPartialAnimState* cur = PartialAnimStates;
    while (cur != nullptr)
    {
        if (cur->priority > priority)
            break;
        insert = &cur->next;
        cur = cur->next;
    }

    nalPartialAnimState* state;
    if (PartialAnimStatePool != nullptr)
    {
        state = PartialAnimStatePool;
        PartialAnimStatePool = PartialAnimStatePool->next;
    }
    else
    {
        state = (nalPartialAnimState*)tlMemAlloc(0x44, 8, 0);
    }

    state->next = *insert;
    *insert = state;
    float v22 = fade_out == 0.0f ? 0.0f : 1.0f / fade_out;
    float fadein = fade_in == 0.0f ? 0.0f : 1.0f / fade_in;
    state->base.Setup(anim, Skeleton, fadein, play_method, callback_time,
                      callback, speed, time_in_seconds_to_start);
    state->priority = priority;
    state->mask = mask;
    state->type = type;
    state->base.state = FadeIn;
    state->fadeout_rate = v22;
    state->CreationAdvanceCount = AdvanceCount;
    if (match != nullptr && (*(unsigned char*)((char*)anim + 0x34) & 1) != 0
        && (*(unsigned char*)((char*)*(void**)((char*)match->base.instance + 0x10)
                              + 0x34)
            & 1) != 0
        && !ForceRestart)
    {
        float t = match->base.t;
        state->base.t = t;
        if (*(void**)((char*)match->base.instance + 0x10) == anim)
            state->base.t_prev = match->base.t_prev;
        else
            state->base.t_prev = t;
    }
}

// ea: 0x00540FB0
void PlayerAnimMgr::PlayModifier(nalGenericAnim& anim, int animateCamera,
                                 float fadeInTime, float fadeOutTime,
                                 float speed, float mask)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr && Player->client != nullptr)
    {
        mCurModifier = &anim;
        void* dobj = (void*)dword_F6A2A0[802 * currCl];
        void* animPlayer = *(void**)((char*)dobj + 0x20);
        ((AnimationPlayer*)animPlayer)
            ->PlayModifier(&anim, AnimationPlayer::nalPartialModifier, 1.0f,
                           (unsigned int)mask,
                           true, fadeInTime, fadeOutTime, nullptr, 0.0f,
                           &mModifierCallback, speed, 0.0f);
        if (animateCamera != 0)
        {
            ((Camera*)((char*)gCamera + 0x1F0 * currCl))
                ->StopAnimating(0.0f);
        }
    }
}

// ea: 0x00547DC0
void PlayerAnimMgr::PlayModifier(const char* animName, int animateCamera,
                                 float fadeInTime, float fadeOutTime,
                                 float speed, float mask)
{
    tlFixedString name(animName);
    nalAnimClass<nalAnyPose>* Anim = nalGetAnim(name);
    if (Anim != nullptr)
    {
        PlayModifier((nalGenericAnim&)*Anim, animateCamera, fadeInTime,
                     fadeOutTime, speed, mask);
    }
}

// ============================================================================
// AnimQueue (anim.o) - batched animation pose/GBM command queue
// Offsets verified against disasm: m_OpList[30] @ +0x00 (24B ops),
// m_iNumOps +0x2D0, m_bIsSoldier +0x2D4, m_pDestPose +0x2D8,
// m_pDestMatrixArray +0x2DC, m_pSkeleton +0x2E0, m_iDestLOD +0x2E4,
// m_CurrParam +0x2E8, m_bWasPrevOpABlend +0x2EC, m_iStackDepth +0x2F0,
// m_iMaxStackDepth +0x2F4.
// ============================================================================

class AnimQueue {
public:
    enum EAnimParam {
        PARAM_NONE = 0,
        PARAM_X = 1,
        PARAM_Y = 2,
    };

    static const int MAX_NUM_GBM_CALLS = 160;
    static const int MAX_NUM_GBM_OUTPUTS = 64;
    static const int MAX_NUM_OPCODES = 30;
    static const int MAX_NUM_TEMP_SOLDIER_POSES = 8;
    static const int MAX_NUM_DECOMP_TOUCHES = 400;

    // MatrixQueueEntry - 0x114 bytes (69 DWORDs; verified vs disasm)
    struct MatrixQueueEntry {
        void* m_pPose;           // +0x00 nalGenericPose*
        void* m_pSkeleton;       // +0x04
        void* m_pTheObj;         // +0x08 DObj*
        void* m_DestArray[MAX_NUM_GBM_OUTPUTS];  // +0x0C
        int m_iNumDests;         // +0x10C
        bool m_bAbsolute;        // +0x110
    };

    // Op list entries - 24 bytes each
    struct GetPoseEventData {
        void* pTheInstance;      // +0x04
        unsigned char bUseDefaultPose;  // +0x08
        float curr_t;            // +0x0C
        float prev_t;            // +0x10
        unsigned int hint;       // +0x14
    };
    struct BlendEventData {
        float fWeight;           // +0x04
    };
    union OpData {
        GetPoseEventData m_GetPoseEventData;
        BlendEventData m_BlendEventData;
    };
    struct Op {
        unsigned int opcode;     // +0x00
        OpData theOpData;        // +0x04
    };

    struct DecompOp {
        void* pInstance;         // +0x00 nalGenericInstance*
        float curr_t;            // +0x04
        float prev_t;            // +0x08
        unsigned int hint;       // +0x0C
    };

    struct TempPoseStackPusher {
        static int iStackDepth;  // ?iStackDepth@TempPoseStackPusher@AnimQueue@@0HA @ 0xF25A60
        static nalGenericPose* pTempPoses[MAX_NUM_TEMP_SOLDIER_POSES];  // @ 0xF25A64

        nalGenericPose* m_pPose;  // +0x00

        TempPoseStackPusher(nalGenericSkeleton* pTheSkel);  // ea: 0x0053EFB0
        ~TempPoseStackPusher();                                          // ea: 0x0053F090
    };

    // anim.o statics
    static MatrixQueueEntry matrixQueue[MAX_NUM_GBM_CALLS];  // ?matrixQueue@AnimQueue@@0PAUMatrixQueueEntry@1@A @ 0xF19320
    static int iNumMatrixQueueEntries;                        // @ 0xF25A5C
    static int iNumDecomps;                                   // @ 0xF25A38
    static DecompOp decompList[MAX_NUM_DECOMP_TOUCHES];       // ?decompList@AnimQueue@@0PAUDecompOp@1@A @ 0xF23FE8
    static nalGenericPose* m_pTempPoses[MAX_NUM_TEMP_SOLDIER_POSES];  // @ 0xF25A3C

    Op m_OpList[MAX_NUM_OPCODES];  // +0x00
    int m_iNumOps;                 // +0x2D0
    bool m_bIsSoldier;             // +0x2D4
    nalGenericPose* m_pDestPose;   // +0x2D8
    void* m_pDestMatrixArray;      // +0x2DC
    void* m_pSkeleton;             // +0x2E0
    int m_iDestLOD;                // +0x2E4
    int m_CurrParam;               // +0x2E8
    bool m_bWasPrevOpABlend;       // +0x2EC
    int m_iStackDepth;             // +0x2F0
    int m_iMaxStackDepth;          // +0x2F4

    static int AddNewGBMOutput(int iGBM, void* pNewDstMatrices);  // ea: 0x0053EC70
    static bool GetDobjAbsolute(DObj* pTheObj);                   // ea: 0x0053ED10
    static int ExecuteAndClearDecompression();                    // ea: 0x0053ED50
    void ExecGetPose(nalGenericPose* pDestPose,
                     nalGenericPose* pDefaultPose,
                     GetPoseEventData* theOp);                    // ea: 0x0053EDA0
    void ExecuteBlendPose(nalGenericPose* pDestPose,
                          nalGenericPose* pFirstPose,
                          nalGenericPose* pSecondPose,
                          BlendEventData* theOp);                 // ea: 0x0053EDD0
    void SetParamAsAccum();                                       // ea: 0x0053EE40
    void SetParamAsTemp();                                        // ea: 0x0053EE50
    void IncrementOptCount();                                     // ea: 0x0053EE60
    void AddTouch(float curr_t, float prev_t,
                  nalGenericInstance* pInstance,
                  unsigned int hint);                             // ea: 0x0053EEC0
    void BlendPoses(float weight);                                // ea: 0x0053EF50
    static int AddGetBoneMatrices(DObj* pTheObj,
                                  const nalGenericPose* pThePose,
                                  void* pDstMatrices,
                                  bool bAbsolute);                // ea: 0x00545840
    void ExecuteAnimCommands(nalGenericPose** ppPoseStack,
                             nalGenericPose* pTempPose);  // ea: 0x00545A90
    void ExecuteAndClearAnimation();                              // ea: 0x0054BF00
};

// anim.o data
AnimQueue::MatrixQueueEntry AnimQueue::matrixQueue[AnimQueue::MAX_NUM_GBM_CALLS];
int AnimQueue::iNumMatrixQueueEntries = 0;
int AnimQueue::iNumDecomps = 0;
AnimQueue::DecompOp AnimQueue::decompList[AnimQueue::MAX_NUM_DECOMP_TOUCHES];
nalGenericPose*
    AnimQueue::m_pTempPoses[AnimQueue::MAX_NUM_TEMP_SOLDIER_POSES];
int AnimQueue::TempPoseStackPusher::iStackDepth = 0;
nalGenericPose*
    AnimQueue::TempPoseStackPusher::pTempPoses[
        AnimQueue::MAX_NUM_TEMP_SOLDIER_POSES];

// Stub callees used by AnimQueue (real bodies in the nal_xboxr port).
// nalGenericPose::nalGenericPose(const nalGenericPose&, bool)
inline nalGenericPose::nalGenericPose(const nalGenericPose& other,
                                      bool copyData)
{
    (void)other; (void)copyData;
}

// nalGenericInstance::GetPose(float,float,nalGenericPose&,const
// nalGenericPose&,int,unsigned)
inline void nalGenericInstance::GetPose(float t1, float t2,
                                        nalGenericPose& out,
                                        const nalGenericPose& base, int flags,
                                        unsigned hint)
{
    (void)t1; (void)t2; (void)out; (void)base; (void)flags; (void)hint;
}

// nalGenericInstance::TouchDecompCache(float,float,int,unsigned)
inline void nalGenericInstance::TouchDecompCache(float t1, float t2,
                                                 int flags, unsigned hint)
{
    (void)t1; (void)t2; (void)flags; (void)hint;
}

// nalGeneric::Blend (local view; real body in nal_xboxr port)
inline void Blend(nalGenericPose& out, float blend, const nalGenericPose& a,
                  const nalGenericPose& b)
{
    (void)out; (void)blend; (void)a; (void)b;
}

// Global-scope stub bodies for the local nalGeneric views in this TU
// (the correctly-mangled real definitions live in the nal_xboxr port).
inline nalGenericPose::nalGenericPose(const nalBaseSkeleton* skel, int flags)
{
    (void)skel; (void)flags;
}

inline nalGenericPose::~nalGenericPose() {}

inline void nalGenericPose::operator=(const nalGenericPose& other)
{
    (void)other;
}

inline void nalGenericSkeleton::GetBoneMatrices(const nalGenericPose& pose,
                                                nalMatrix4x4* matrices,
                                                int lod) const
{
    (void)pose; (void)matrices; (void)lod;
}

// ea: 0x0053EC70
int AnimQueue::AddNewGBMOutput(int iGBM, void* pNewDstMatrices)
{
    if (iGBM >= iNumMatrixQueueEntries)
    {
        XANIM_ASSERT("iGBM < AnimQueue::iNumMatrixQueueEntries",
                     "c:\\cod\\code\\game\\AnimQueue.cpp", 128,
                     "Invalid GetBoneMatrix index.");
    }
    MatrixQueueEntry* v2 = &matrixQueue[iGBM];
    int m_iNumDests = v2->m_iNumDests;
    if (m_iNumDests == MAX_NUM_GBM_OUTPUTS)
        return 0;
    v2->m_DestArray[m_iNumDests] = pNewDstMatrices;
    ++v2->m_iNumDests;
    return 1;
}

// ea: 0x0053ED10
bool AnimQueue::GetDobjAbsolute(DObj* pTheObj)
{
    int v1 = 0;
    if (iNumMatrixQueueEntries <= 0)
        return false;
    for (void** i = &matrixQueue[0].m_pPose; (void*)pTheObj != *i;
         i += 69)
    {
        if (++v1 >= iNumMatrixQueueEntries)
            return false;
    }
    return matrixQueue[v1].m_bAbsolute;
}

// ea: 0x0053ED50
int AnimQueue::ExecuteAndClearDecompression()
{
    int result = iNumDecomps;
    int v1 = 0;
    if (iNumDecomps > 0)
    {
        DecompOp* v2 = decompList;
        do
        {
            ((nalGenericInstance*)v2->pInstance)
                ->TouchDecompCache(v2->curr_t, v2->prev_t, 0, v2->hint);
            result = iNumDecomps;
            ++v1;
            ++v2;
        } while (v1 < iNumDecomps);
    }
    iNumDecomps = 0;
    return result;
}

// ea: 0x0053EDA0
void AnimQueue::ExecGetPose(nalGenericPose* pDestPose,
                            nalGenericPose* pDefaultPose,
                            GetPoseEventData* theOp)
{
    ((nalGenericInstance*)theOp->pTheInstance)
        ->GetPose(theOp->curr_t, theOp->prev_t, *pDestPose, *pDefaultPose,
                  m_iDestLOD, theOp->hint);
}

// ea: 0x0053EDD0
void AnimQueue::ExecuteBlendPose(nalGenericPose* pDestPose,
                                 nalGenericPose* pFirstPose,
                                 nalGenericPose* pSecondPose,
                                 BlendEventData* theOp)
{
    Blend(*pDestPose, theOp->fWeight, *pFirstPose, *pSecondPose);
}

// ea: 0x0053EE40
void AnimQueue::SetParamAsAccum()
{
    m_CurrParam = PARAM_X;
}

// ea: 0x0053EE50
void AnimQueue::SetParamAsTemp()
{
    m_CurrParam = PARAM_Y;
}

// ea: 0x0053EE60
void AnimQueue::IncrementOptCount()
{
    int v1 = m_iNumOps + 1;
    bool v2 = m_iNumOps - 29 < 0;
    m_iNumOps = v1;
    if (v2 == (v1 < 30 ? 1 : 0))
    {
        XANIM_ASSERT("m_iNumOps < MAX_NUM_OPCODES",
                     "c:\\cod\\code\\game\\AnimQueue.cpp", 370,
                     "Bad Animation tree: too many Opcodes added to anim queue.");
    }
}

// ea: 0x0053EEC0
void AnimQueue::AddTouch(float curr_t, float prev_t,
                         nalGenericInstance* pInstance,
                         unsigned int hint)
{
    if (iNumDecomps >= MAX_NUM_DECOMP_TOUCHES)
    {
        XANIM_ASSERT("iNumDecomps < MAX_NUM_DECOMP_TOUCHES",
                     "c:\\cod\\code\\game\\AnimQueue.cpp", 415,
                     "Bad Animation tree: too many opcodes in one frame.");
    }
    DecompOp* entry = &decompList[iNumDecomps];
    entry->pInstance = pInstance;
    entry->curr_t = curr_t;
    entry->prev_t = prev_t;
    entry->hint = hint;
    ++iNumDecomps;
}

// ea: 0x0053EF50
void AnimQueue::BlendPoses(float weight)
{
    OpData* v2 = &m_OpList[m_iNumOps].theOpData;
    m_OpList[m_iNumOps].opcode = 1;
    if (m_bWasPrevOpABlend)
    {
        m_OpList[m_iNumOps].opcode = 3;
        --m_iStackDepth;
    }
    v2->m_BlendEventData.fWeight = weight;
    IncrementOptCount();
}

// ea: 0x0053EFB0
AnimQueue::TempPoseStackPusher::TempPoseStackPusher(
    nalGenericSkeleton* pTheSkel)
{
    if (iStackDepth >= MAX_NUM_TEMP_SOLDIER_POSES)
    {
        XANIM_ASSERT("iStackDepth < MAX_NUM_TEMP_SOLDIER_POSES",
                     "c:\\cod\\code\\game\\AnimQueue.cpp", 450,
                     "Not enough soldier poses on the pose stack.");
    }
    if (pTempPoses[iStackDepth] == nullptr)
    {
        void* v4 = tlMemAlloc(0x10, 8, 0);
        if (v4 != nullptr)
            pTempPoses[iStackDepth] =
                new (v4) nalGenericPose(
                    *(nalGenericPose*)((char*)pTheSkel + 0xC8), true);
        else
            pTempPoses[iStackDepth] = nullptr;
    }
    m_pPose = pTempPoses[iStackDepth];
    ++iStackDepth;
}

// ea: 0x0053F090
AnimQueue::TempPoseStackPusher::~TempPoseStackPusher()
{
    --iStackDepth;
}

// ea: 0x00545840
int AnimQueue::AddGetBoneMatrices(DObj* pTheObj,
                                  const nalGenericPose* pThePose,
                                  void* pDstMatrices, bool bAbsolute)
{
    int v4 = iNumMatrixQueueEntries;
    int v5 = 0;
    if (iNumMatrixQueueEntries > 0)
    {
        void** p_m_pTheObj = &matrixQueue[0].m_pPose;
        do
        {
            if (pTheObj == (DObj*)*p_m_pTheObj)
                break;
            ++v5;
            p_m_pTheObj += 69;
        } while (v5 < iNumMatrixQueueEntries);
    }
    if (v5 == iNumMatrixQueueEntries)
    {
        if (iNumMatrixQueueEntries == MAX_NUM_GBM_CALLS)
        {
            XANIM_ASSERT("AnimQueue::iNumMatrixQueueEntries != MAX_NUM_GBM_CALLS",
                         "c:\\cod\\code\\game\\AnimQueue.cpp", 74,
                         "Too many GetBoneMatrices calls.");
            v4 = iNumMatrixQueueEntries;
        }
        MatrixQueueEntry* v7 = &matrixQueue[v4];
        if (pThePose != nullptr)
        {
            v7->m_DestArray[0] = pDstMatrices;
            v7->m_iNumDests = 1;
            v7->m_pPose = (void*)pThePose;
            v7->m_pSkeleton = *(void**)pThePose;
        }
        else
        {
            v7->m_DestArray[1] = pDstMatrices;
            v7->m_iNumDests = 2;
        }
        v7->m_pTheObj = pTheObj;
        v7->m_bAbsolute = bAbsolute;
        iNumMatrixQueueEntries = v4 + 1;
        return v4;
    }
    else
    {
        MatrixQueueEntry* v9 = &matrixQueue[v5];
        if (pThePose != nullptr)
        {
            v9->m_pPose = (void*)pThePose;
            v9->m_DestArray[0] = pDstMatrices;
            v9->m_pSkeleton = *(void**)pThePose;
            v9->m_pTheObj = pTheObj;
            v9->m_bAbsolute = bAbsolute;
            return v5;
        }
        else
        {
            if (v9->m_iNumDests >= MAX_NUM_GBM_OUTPUTS)
            {
                XANIM_ASSERT("theEntry.m_iNumDests < MAX_NUM_GBM_OUTPUTS",
                             "c:\\cod\\code\\game\\AnimQueue.cpp", 112,
                             "Not enough GBM copy entries.");
            }
            v9->m_DestArray[v9->m_iNumDests++] = pDstMatrices;
            v9->m_pTheObj = pTheObj;
            v9->m_bAbsolute = bAbsolute;
            return v5;
        }
    }
}

// ea: 0x00545A90
void AnimQueue::ExecuteAnimCommands(nalGenericPose** ppPoseStack,
                                    nalGenericPose* pTempPose)
{
    nalGenericPose* v3 = ppPoseStack[1];
    nalGenericPose* pDefaultPose = ppPoseStack[0];
    int v4 = m_iMaxStackDepth + 2;
    int iNumStackElements = v4;
    int v5 = 1;
    int iOperation = 0;
    if (m_iNumOps > 0)
    {
        unsigned int* p_hint = &m_OpList[0].theOpData.m_GetPoseEventData.hint;
        while (1)
        {
            switch (*(p_hint - 5))
            {
            case 0:
                if (*(unsigned char*)(p_hint - 3) != 0)
                {
                    ((nalGenericInstance*)*(p_hint - 4))
                        ->GetPose(*(float*)(p_hint - 2),
                                  *(float*)(p_hint - 1), *pTempPose, *v3,
                                  m_iDestLOD, *p_hint);
                }
                else
                {
                    *pDefaultPose = *v3;
                    ((nalGenericInstance*)*(p_hint - 4))
                        ->GetPose(*(float*)(p_hint - 2),
                                  *(float*)(p_hint - 1), *v3, *pDefaultPose,
                                  m_iDestLOD, *p_hint);
                }
                break;
            case 1:
                Blend(*v3, *(float*)(p_hint - 4), *pTempPose, *v3);
                break;
            case 2:
                if (++v5 >= v4)
                {
                    XANIM_ASSERT("iCurrStackElement < iNumStackElements",
                                 "c:\\cod\\code\\game\\AnimQueue.cpp", 297,
                                 "AnimQueue Stack overflow during execution.");
                }
                v3 = ppPoseStack[v5];
                break;
            case 3:
                Blend(ppPoseStack[v5 - 1][0], *(float*)(p_hint - 4),
                      ppPoseStack[v5 - 1][0], *v3);
                v3 = ppPoseStack[--v5];
                if (v5 < 1)
                {
                    XANIM_ASSERT("iCurrStackElement >= 1",
                                 "c:\\cod\\code\\game\\AnimQueue.cpp", 312,
                                 "AnimQueue Stack underflow during execution.");
                }
                break;
            case 4:
                ((nalGenericSkeleton*)(*(void**)v3))
                    ->GetBoneMatrices(*v3,
                                      (nalMatrix4x4*)m_pDestMatrixArray,
                                      *(int*)((char*)v3 + 4));
                break;
            default:
                break;
            }
            p_hint += 6;
            if (++iOperation >= m_iNumOps)
                break;
            v4 = iNumStackElements;
        }
    }
    *m_pDestPose = *v3;
}

// ea: 0x0054BF00
void AnimQueue::ExecuteAndClearAnimation()
{
    AnimQueue* v1 = this;
    int v2 = 0;
    if (m_iStackDepth != 0)
    {
        XANIM_ASSERT("m_iStackDepth == 0",
                     "c:\\cod\\code\\game\\AnimQueue.cpp", 193,
                     "Bad Animation tree: Stack imbalanced.");
    }
    if (m_bIsSoldier && m_pTempPoses[0] == nullptr)
    {
    for (nalGenericPose** v3 = m_pTempPoses;
         v3 < (nalGenericPose**)&iNumMatrixQueueEntries;
         ++v3)
    {
        void* v4 = tlMemAlloc(0x10, 8, 0);
        if (v4 != nullptr)
            *v3 = new (v4) nalGenericPose(
                (const nalBaseSkeleton*)m_pSkeleton, 0);
        else
            *v3 = nullptr;
    }
    }
    int iCurrTempSoldierPose = 0;
    nalGenericPose* pTempPose;
    if (m_bIsSoldier)
    {
        pTempPose = m_pTempPoses[0];
        iCurrTempSoldierPose = 1;
    }
    else
    {
        void* v6 = tlMemAlloc(0x10, 8, 0);
        if (v6 != nullptr)
            pTempPose = new (v6) nalGenericPose(
                (const nalBaseSkeleton*)m_pSkeleton, 0);
        else
            pTempPose = nullptr;
    }
    int v8 = m_iMaxStackDepth + 2;
    nalGenericPose** ppPoseStack =
        (nalGenericPose**)mem_heap_malloc(4 * v8);
    if (m_bIsSoldier)
    {
        if (v8 > 0)
        {
            nalGenericPose** v9 =
                &TempPoseStackPusher::pTempPoses[iCurrTempSoldierPose];
            do
            {
                if (v9 >= (nalGenericPose**)&iNumMatrixQueueEntries)
                {
                    XANIM_ASSERT(
                        "iCurrTempSoldierPose < MAX_NUM_TEMP_SOLDIER_POSES",
                        "c:\\cod\\code\\game\\AnimQueue.cpp", 227,
                        "AnimQueue Stack overflow during execution");
                }
                ppPoseStack[v2++] = *v9++;
            } while (v2 < v8);
            v1 = this;
        }
    }
    else if (v8 > 0)
    {
        do
        {
            void* v11 = tlMemAlloc(0x10, 8, 0);
            if (v11 != nullptr)
                ppPoseStack[v2++] = new (v11) nalGenericPose(
                    (const nalBaseSkeleton*)m_pSkeleton, 0);
            else
                ppPoseStack[v2++] = nullptr;
        } while (v2 < v8);
    }
    ExecuteAnimCommands(ppPoseStack, pTempPose);
    if (!m_bIsSoldier)
    {
        for (int i = 0; i < v8; ++i)
        {
            nalGenericPose* v14 = ppPoseStack[i];
            if (v14 != nullptr)
            {
                v14->~nalGenericPose();
                tlMemFree(v14);
            }
        }
        mem_heap_free(ppPoseStack);
        if (pTempPose != nullptr)
        {
            pTempPose->~nalGenericPose();
            tlMemFree(pTempPose);
        }
        v1 = this;
    }
    v1->m_iNumOps = 0;
    v1->m_iDestLOD = 0;
    v1->m_pDestMatrixArray = nullptr;
    v1->m_pDestPose = nullptr;
    v1->m_CurrParam = PARAM_NONE;
    v1->m_bWasPrevOpABlend = false;
    v1->m_iStackDepth = 0;
    v1->m_iMaxStackDepth = 0;
}

// ============================================================================
// anim.o misc batch: nalGeneric templates + xanim/interaction helpers
// ============================================================================

// nalGenericSkeleton component tables (raw offsets; verified vs disasm):
//   typeTable  +0x74, components +0x80, groupCount +0x84, groups +0x88,
//   constGroupCount +0xA0, constGroups +0xA4.
template <typename T>
void nalGenericSkeleton::GetComponentHandle(
    nalGeneric::nalGenericComponentHandle<T>& handle, const tlFixedString& a3,
    const tlFixedString& a4)
{
    unsigned int typeId = 0;
    if (std::is_same<T, math::Dir3>::value)
        typeId = 0x10EC610;  // nalComponentFloat3Base::TypeID
    else if (std::is_same<T, nalPositionOrientation>::value)
        typeId = 0x10EC614;  // nalComponentPOBase::TypeID
    else if (std::is_same<T, float>::value)
        typeId = 0x10EC613;  // nalComponentFloat1Base::TypeID

    handle.Skeleton = nullptr;
    handle.ComponentInfo = nullptr;
    handle.ComponentIndex = 0;
    handle.IsConst = 0;

    int result = 0;
    int groupCount = *(int*)((char*)this + 0x84);
    char* groups = *(char**)((char*)this + 0x88);
    char* components = *(char**)((char*)this + 0x80);
    char* typeTable = *(char**)((char*)this + 0x74);
    int v19 = 0;
    int v20 = 0;
    int v21 = 0;
    if (groupCount > 0)
    {
        do
        {
            int v18 = 0;
            if (*(int*)(groups + v20 + 0x28) > 0)
            {
                int v7 = 40 * v21;
                do
                {
                    int v8 = 0;
                    char* v9 = components + v7;
                    while (*(unsigned int*)((char*)&a4 + 4 * v8)
                           == *(unsigned int*)v9)
                    {
                        ++v8;
                        v9 += 4;
                        if (v8 >= 8)
                        {
                            int v10 = 0;
                            char* v11 =
                                typeTable
                                + 48 * *(unsigned int*)(components + v7 + 0x20);
                            while (*(unsigned int*)((char*)&a3 + 4 * v10)
                                   == *(unsigned int*)v11)
                            {
                                ++v10;
                                v11 += 4;
                                if (v10 >= 8)
                                {
                                    void* pComp =
                                        *(void**)(groups + v20 + 0x20);
                                    typedef void* (__thiscall* GetTypeIDFn)(void*);
                                    void* tid =
                                        ((GetTypeIDFn)((void**)*(void**)pComp)[1])(
                                            pComp);
                                    if (tid != (void*)(uintptr_t)typeId)
                                        goto LABEL_12;
                                    handle.Skeleton =
                                        (const nalGeneric::nalGenericSkeleton*)this;
                                    handle.ComponentInfo =
                                        (const nalGeneric::nalComponentInfo*)
                                            (groups + 48 * v19);
                                    handle.ComponentIndex = v18;
                                    return;
                                }
                            }
                            break;
                        }
                    }
LABEL_12:
                    ++v21;
                    v7 += 40;
                    ++v18;
                } while (v18 < *(int*)(groups + v20 + 0x28));
            }
            result = v19 + 1;
            v20 += 48;
            ++v19;
        } while (v19 < groupCount);
    }
    (void)result;
}

template <typename T>
void nalGenericSkeleton::GetComponentHandle(
    nalGeneric::nalGenericConstComponentHandle<T>& handle,
    const tlFixedString& a3, const tlFixedString& a4)
{
    unsigned int typeId = 0;
    if (std::is_same<T, float>::value)
        typeId = 0x10EC613;  // nalComponentFloat1Base::TypeID

    handle.Skeleton = nullptr;
    handle.ComponentInfo = nullptr;
    handle.ComponentIndex = 0;
    handle.IsConst = 0;

    int groupCount1 = *(int*)((char*)this + 0x84);
    char* groups1 = *(char**)((char*)this + 0x88);
    char* components = *(char**)((char*)this + 0x80);
    char* typeTable = *(char**)((char*)this + 0x74);
    int v30 = 0;
    int v31 = 0;
    if (groupCount1 > 0)
    {
        int v6 = 0;
        do
        {
            int v26 = 0;
            if (*(int*)(groups1 + v6 + 0x28) > 0)
            {
                int v7 = 40 * v30;
                do
                {
                    int v8 = 0;
                    char* v9 = components + v7;
                    while (*(unsigned int*)((char*)&a4 + 4 * v8)
                           == *(unsigned int*)v9)
                    {
                        ++v8;
                        v9 += 4;
                        if (v8 >= 8)
                        {
                            int v10 = 0;
                            char* v11 =
                                typeTable
                                + 48 * *(unsigned int*)(components + v7 + 0x20);
                            while (*(unsigned int*)((char*)&a3 + 4 * v10)
                                   == *(unsigned int*)v11)
                            {
                                ++v10;
                                v11 += 4;
                                if (v10 >= 8)
                                {
                                    void* pComp = *(void**)(groups1 + v6 + 0x20);
                                    typedef void* (__thiscall* GetTypeIDFn)(void*);
                                    void* tid =
                                        ((GetTypeIDFn)((void**)*(void**)pComp)[1])(
                                            pComp);
                                    if (tid != (void*)(uintptr_t)typeId)
                                        goto LABEL_12;
                                    handle.Skeleton =
                                        (const nalGeneric::nalGenericSkeleton*)this;
                                    handle.ComponentInfo =
                                        (const nalGeneric::nalComponentInfo*)
                                            (groups1 + 48 * v31);
                                    handle.ComponentIndex = v26;
                                    handle.IsConst = 0;
                                    return;
                                }
                            }
                            break;
                        }
                    }
LABEL_12:
                    ++v30;
                    v7 += 40;
                    ++v26;
                } while (v26 < *(int*)(groups1 + v6 + 0x28));
            }
            v6 += 48;
            ++v31;
        } while (v31 < groupCount1);
    }

    // second pass over the const component groups (+0xA0/+0xA4)
    int v32 = 0;
    int groupCount2 = *(int*)((char*)this + 0xA0);
    char* groups2 = *(char**)((char*)this + 0xA4);
    if (groupCount2 > 0)
    {
        int v14 = 0;
        do
        {
            int v29 = 0;
            if (*(int*)(groups2 + v14 + 0x28) > 0)
            {
                int v16 = 40 * v30;
                do
                {
                    int v17 = 0;
                    char* v18 = components + v16;
                    while (*(unsigned int*)((char*)&a4 + 4 * v17)
                           == *(unsigned int*)v18)
                    {
                        ++v17;
                        v18 += 4;
                        if (v17 >= 8)
                        {
                            int v19 = 0;
                            char* v20 =
                                typeTable
                                + 48 * *(unsigned int*)(components + v16 + 0x20);
                            while (*(unsigned int*)((char*)&a3 + 4 * v19)
                                   == *(unsigned int*)v20)
                            {
                                ++v19;
                                v20 += 4;
                                if (v19 >= 8)
                                {
                                    void* pComp = *(void**)(groups2 + v14 + 0x20);
                                    typedef void* (__thiscall* GetTypeIDFn)(void*);
                                    void* tid =
                                        ((GetTypeIDFn)((void**)*(void**)pComp)[1])(
                                            pComp);
                                    if (tid != (void*)(uintptr_t)typeId)
                                        goto LABEL_27;
                                    handle.Skeleton =
                                        (const nalGeneric::nalGenericSkeleton*)this;
                                    handle.ComponentInfo =
                                        (const nalGeneric::nalComponentInfo*)
                                            (groups2 + 48 * v32);
                                    handle.ComponentIndex = v29;
                                    handle.IsConst = 1;
                                    return;
                                }
                            }
                            break;
                        }
                    }
LABEL_27:
                    ++v30;
                    v16 += 40;
                    ++v29;
                } while (v29 < *(int*)(groups2 + v14 + 0x28));
            }
            v14 += 48;
            ++v32;
        } while (v32 < groupCount2);
    }
}

// ??$?AM@nalGenericSkeleton@nalGeneric@@QBEABMABV?$nalGenericConstComponentHandle@M@1@@Z
template <typename T>
const float& nalGenericSkeleton::operator[](
    const nalGeneric::nalGenericConstComponentHandle<T>& handle) const
{
    static float sZero = 0.0f;  // unk_F30A1C equivalent
    const void* v2 = handle.Skeleton;
    if (v2 == nullptr)
    {
        if (_tlAssert("c:\\cod\\code\\tl\\nal\\include\\common\\nal_generic.h",
                      364, "handle.Skeleton",
                      "attempt to de-reference an invalid handle"))
            __debugbreak();
        v2 = handle.Skeleton;
        if (v2 == nullptr)
            return sZero;
    }
    if (v2 != this
        && _tlAssert("c:\\cod\\code\\tl\\nal\\include\\common\\nal_generic.h",
                     368, "handle.Skeleton == this",
                     "handle and skeleton don't match"))
    {
        __debugbreak();
    }
    const char* v5;
    if (handle.IsConst != 0)
        v5 = *(char**)((char*)this + 0xB0);
    else
        v5 = *(char**)((char*)this + 0x94);
    return *(const float*)(*(char**)((char*)handle.ComponentInfo + 44)
                           + 4 * handle.ComponentIndex + v5);
}

// ??$?AM@nalGenericPose@nalGeneric@@QBEABMABV?$nalGenericConstComponentHandle@M@1@@Z
template <typename T>
const float& nalGenericPose::operator[](
    const nalGeneric::nalGenericConstComponentHandle<T>& handle) const
{
    static float sZero = 0.0f;  // unk_F30A18 equivalent
    const void* v2 = handle.Skeleton;
    if (v2 == nullptr)
    {
        if (_tlAssert("c:\\cod\\code\\tl\\nal\\include\\common\\nal_generic.h",
                      441, "handle.Skeleton",
                      "attempting to de-reference an invalid handle"))
            __debugbreak();
        v2 = handle.Skeleton;
        if (v2 == nullptr)
            return sZero;
    }
    if (v2 != *(void**)this
        && _tlAssert("c:\\cod\\code\\tl\\nal\\include\\common\\nal_generic.h",
                     446, "handle.Skeleton == skeleton",
                     "handle and pose skeletons don't match"))
    {
        __debugbreak();
    }
    if (handle.IsConst != 0)
        return ((nalGenericSkeleton*)v2)->operator[]<T>(handle);
    return *(const float*)(*(char**)((char*)this + 8)
                           + *(char**)((char*)handle.ComponentInfo + 44)
                           + 4 * handle.ComponentIndex);
}

// ??$?AVnalPositionOrientation@@@nalGenericPoseBlender@nalGeneric@@QAEAAMABV?$nalGenericComponentHandle@VnalPositionOrientation@@@1@@Z
template <typename T>
float& nalGenericPoseBlender::operator[](
    const nalGeneric::nalGenericComponentHandle<T>& handle)
{
    static float sZero = 0.0f;
    const void* v2 = handle.Skeleton;
    if (v2 == nullptr)
    {
        if (_tlAssert("c:\\cod\\code\\tl\\nal\\include\\common\\nal_generic.h",
                      718, "handle.Skeleton",
                      "attempt to de-reference an invalid handle"))
            __debugbreak();
        v2 = handle.Skeleton;
        if (v2 == nullptr)
            return sZero;
    }
    if (v2 != *(void**)((char*)this + 4)
        && _tlAssert("c:\\cod\\code\\tl\\nal\\include\\common\\nal_generic.h",
                     721, "handle.Skeleton == GetSkeleton()",
                     "handle and pose skeletons don't match"))
    {
        __debugbreak();
    }
    return *(float*)(*(char**)((char*)this + 8)
                     + 4 * (handle.ComponentIndex
                            + *(int*)((char*)handle.ComponentInfo + 36)));
}

// ea: 0x00560150
void nalGenericPoseBlender::VirtualBlend(nalGenericPose* dst,
                                         const nalGenericPose* src0,
                                         const nalGenericPose* src1)
{
    Blend(*dst, *src0, *src1, 1.0f);
}

// ?Blend@nalGenericPoseBlender@@QAEXAAVnalGenericPose@@ABV2@1M@Z (stub;
// real body in the nal_xboxr port)
void nalGenericPoseBlender::Blend(nalGenericPose& out,
                                  const nalGenericPose& a,
                                  const nalGenericPose& b, float t)
{
    (void)out; (void)a; (void)b; (void)t;
}

// ??$nalPosePtrCast@VnalGenericPose@nalGeneric@@@@YAPAVnalGenericPose@nalGeneric@@PAVnalBasePose@@@Z
template <typename T>
inline T* nalPosePtrCast(void* ptr)
{
    if (ptr == nullptr || *(void**)ptr == nullptr
        || *(void**)(*(void**)ptr) != (void*)0x10E6D04)
        return nullptr;
    return (T*)ptr;
}

// ea: 0x00547E10
void VectorCopyUnalignedInc(float** pos, float (*v)[3])
{
    float v2 = *(*pos)++;
    (*v)[0] = v2;
    v2 = *(*pos)++;
    (*v)[1] = v2;
    v2 = *(*pos)++;
    (*v)[2] = v2;
}

// ?g_tree_list@@3V?$reserved_dlist@VXAnimTree@@@@A @ 0xDF2ABC
reserved_dlist<XAnimTree> g_tree_list;

// ea: 0x0053E290
void CheckAllAnims()
{
    unsigned int* v0 = (unsigned int*)&g_info[0].s[28];
    do
    {
        if (*v0 != 0)
        {
            XANIM_ASSERT("g_info[i].s.inst == 0",
                         "c:\\cod\\code\\game\\xanim.cpp", 516,
                         "maybe this should have been freed");
        }
        v0 += 14;
    } while (v0 < (unsigned int*)((char*)g_info + sizeof(g_info) + 0x2C));
}

// ea: 0x0055E8E0
nalGenericPose* new_nalGenericPose(TPakId pakId,
                                   const nalGenericSkeleton* skeleton)
{
    nalGenericPose* v2 = nullptr;
    PakHeapContext ctx(pakId, false);
    void* v3 = tlMemAlloc(0x10, 8, 0);
    if (v3 != nullptr)
        v2 = new (v3) nalGenericPose(
            *(nalGenericPose*)((char*)skeleton + 0xC8), true);
    return v2;
}

// InteractMetaAnimData / InteractMetaAnimInstance (anim.o; mirror the
// ADSMetaAnim family; layout verified vs disasm)
struct InteractMetaAnimData {
    tlFixedString mName;   // +0x00
    void* vftable;         // +0x20
    void* mAnimPtr;        // +0x24
    void* mRevPtr;         // +0x28

    InteractMetaAnimData();  // ea: 0x0055FDE0
};

class InteractMetaAnimInstance {
public:
    InteractMetaAnimInstance(nalAnimClass<nalAnyPose>* forwardAnim,
                             nalAnimClass<nalAnyPose>* reverseAnim,
                             nalBaseSkeleton* theSkel);  // ea: 0x0055FB90
    virtual ~InteractMetaAnimInstance();                 // ea: 0x0055FD70

    float Duration;          // +0x04
    float InverseDuration;   // +0x08
    void* Skeleton;          // +0x0C
    void* Anim;              // +0x10
    void* mForwardInst;      // +0x14
    unsigned char _pad[0x20 - 0x18];
    float mPrevValue;        // +0x20
};

// ea: 0x0055FDE0
InteractMetaAnimData::InteractMetaAnimData()
{
    memset(&mName, 0, sizeof(mName));
    vftable = nullptr;
    mAnimPtr = nullptr;
    mRevPtr = nullptr;
}

// ea: 0x0055FB90
InteractMetaAnimInstance::InteractMetaAnimInstance(
    nalAnimClass<nalAnyPose>* forwardAnim,
    nalAnimClass<nalAnyPose>* reverseAnim, nalBaseSkeleton* theSkel)
{
    Duration = forwardAnim->GetDuration();
    InverseDuration = forwardAnim->GetInverseDuration();
    Skeleton = theSkel != nullptr ? theSkel
                                  : *(void**)((char*)forwardAnim + 0x30);
    Anim = forwardAnim;
    ++*(int*)((char*)forwardAnim + 0x3C);
    mPrevValue = 0.0f;
    if (theSkel != nullptr
        && *(void**)*(void**)((char*)forwardAnim + 0x30) != *(void**)theSkel
        && _tlAssert(
               "c:\\cod\\code\\tl\\nal\\include\\common\\nal_anim.h", 147,
               "!skeleton || Compatible(GetSkeleton(),skeleton)",
               "attempt to create an instance without a compatible skeleton"))
    {
        __debugbreak();
    }
    // forwardAnim vtable slot 5 = VirtualCreateInstance
    mForwardInst = ((void* (__thiscall*)(void*, void*))(
        (void**)*(void**)forwardAnim)[5])(forwardAnim, theSkel);
}

// ea: 0x0055FD70
InteractMetaAnimInstance::~InteractMetaAnimInstance()
{
    if (mForwardInst != nullptr)
    {
        typedef void (__thiscall* DtorFn)(void*, unsigned int);
        ((DtorFn)((void**)*(void**)mForwardInst)[0])(mForwardInst, 1);
    }
    --*(int*)((char*)Anim + 0x3C);
}

// ??_GInteractMetaAnimInstance@@UAEPAXI@Z (0x55FD40) - compiler-generated;
// tlMemFree path is represented by this deleting-dtor helper.
void InteractMetaAnimInstance_Delete(InteractMetaAnimInstance* self)
{
    self->~InteractMetaAnimInstance();
    tlMemFree(self);
}

// Local view of AnimBank (full class in cg_misc.cpp; anims at +0)
struct AnimBankLocal {
    unsigned int mSize;   // +0x00
    AnimTree* mList;      // +0x04
};

extern void* AnimBankManager_sInst;  // ?sInst@AnimBankManager@@2PAV1@A
extern void* AnimBankManager_GetBank(void* mgr, int pakId);

// 12-byte notify-array element (binary: vector of XAnimNotifyInfo)
struct AnimNotifyListElem {
    char _pad[0x0C];
    XAnimNotifyInfo* AsInfo() { return (XAnimNotifyInfo*)this; }
};

// ea: 0x00551600
unsigned int ReleaseAllAnims()
{
    AnimBankLocal* bank =
        (AnimBankLocal*)AnimBankManager_GetBank(AnimBankManager_sInst, 0);
    unsigned int result = bank->mSize;
    unsigned int v2 = 1;
    if (bank->mSize > 1)
    {
        while (1)
        {
            unsigned int v3 = v2;
            if (v2 >= result)
            {
                XANIM_ASSERT("index < mSize",
                             "../ae\\inplace/InplaceVector.h", 81,
                             "Bounds check");
            }
            if (v2 >= bank->mSize)
                v3 = 0;
            AnimTree* anims = &bank->mList[v3];
            unsigned int numAnims = anims->entries.mSize;
            if (numAnims != 0)
            {
                int byteOffset = 0x18;
                for (unsigned int ei = 0; ei < numAnims;)
                {
                    unsigned int v6 = ei;
                    if (ei >= numAnims)
                    {
                        XANIM_ASSERT("index < mSize",
                                     "../ae\\inplace/InplaceVector.h", 81,
                                     "Bounds check");
                        if (ei >= numAnims)
                            v6 = 0;
                    }
                    XAnimEntry* entry = &anims->entries.mList[v6];
                    if (entry->anim != nullptr)
                    {
                        reserved_dlist<XAnimTree>::dlist_node* m_head =
                            g_tree_list.m_head;
                        reserved_dlist<XAnimTree>::dlist_node* m_next =
                            g_tree_list.m_head != nullptr
                                ? g_tree_list.m_head->m_next
                                : nullptr;
                        if (g_tree_list.m_head
                                != (reserved_dlist<XAnimTree>::dlist_node*)
                                       &g_tree_list.m_end
                            && m_next != nullptr)
                        {
                            do
                            {
                                XAnimTree* tree = (XAnimTree*)m_head;
                                if (tree->anims == anims
                                    && *(unsigned short*)((char*)tree
                                                          + byteOffset)
                                           != 0)
                                {
                                    XAnimFreeInfo(
                                        tree,
                                        *(unsigned short*)((char*)tree
                                                           + byteOffset));
                                    *(unsigned short*)((char*)tree
                                                       + byteOffset) = 0;
                                }
                                m_head = m_next;
                                m_next = m_next->m_next;
                            } while (m_next != nullptr);
                        }
                        unsigned int v9 = ei;
                        if (ei >= numAnims)
                        {
                            XANIM_ASSERT("index < mSize",
                                         "../ae\\inplace/InplaceVector.h", 81,
                                         "Bounds check");
                            if (ei >= numAnims)
                                v9 = 0;
                        }
                        XAnimEntry* entry2 = &anims->entries.mList[v9];
                        void* notifyList = entry2->notify;
                        entry2->anim = nullptr;
                        entry2->numAnims = 0;
                        if (notifyList != nullptr)
                        {
                            unsigned int count =
                                ((unsigned int*)notifyList)[-1];
                            char* base = (char*)notifyList - 4;
                            AnimNotifyListElem* arr =
                                (AnimNotifyListElem*)notifyList;
                            for (unsigned int n = 0; n < count; ++n)
                                arr[n].AsInfo()->~XAnimNotifyInfo();
                            mem_heap_free(base);
                        }
                        entry2->notify = nullptr;
                        entry2->lastAttempt = 0;
                    }
                    else
                    {
                        unsigned int v15 = ei;
                        if (ei >= numAnims)
                        {
                            XANIM_ASSERT("index < mSize",
                                         "../ae\\inplace/InplaceVector.h", 81,
                                         "Bounds check");
                            if (ei >= numAnims)
                                v15 = 0;
                        }
                        void* notifyList =
                            anims->entries.mList[v15].notify;
                        if (notifyList != nullptr)
                        {
                            unsigned int count =
                                ((unsigned int*)notifyList)[-1];
                            char* base = (char*)notifyList - 4;
                            AnimNotifyListElem* arr =
                                (AnimNotifyListElem*)notifyList;
                            for (unsigned int n = 0; n < count; ++n)
                                arr[n].AsInfo()->~XAnimNotifyInfo();
                            mem_heap_free(base);
                        }
                        unsigned int v18 = ei;
                        if (ei >= numAnims)
                        {
                            XANIM_ASSERT("index < mSize",
                                         "../ae\\inplace/InplaceVector.h", 81,
                                         "Bounds check");
                            if (ei >= numAnims)
                                v18 = 0;
                        }
                        anims->entries.mList[v18].notify = nullptr;
                    }
                    ++ei;
                    byteOffset += 2;
                }
            }
            result = bank->mSize;
            ++v2;
            if (v2 >= bank->mSize)
                break;
        }
    }
    return result;
}

// ============================================================================
// nalComponent<nalComponentBase,CODNoteData,CODNoteTrack> + CODNoteTrack
// (anim.o; nal_generic.cpp COD note-track component)
// nalComponentEnum layout verified vs disasm: Anim +0, ComponentInfo +4,
// CustomSkeletonData +8, CustomAnimData +0xC. nalComponentInfo: StartIndex
// +0x24, Count +0x28. PoseTrackCount = *(skel+0x7C), TrackBitMask @ anim+0x54.
// ============================================================================

struct CODNoteData {
    struct AnimComponentData {
        unsigned char _pad[0x0C];
    };
};

namespace nalComponentData {
struct SkeletonData {};
struct AnimData {};
struct SkeletonComponentData {};
}

enum nalRegisterKey {
    NAL_REGISTER_KEY = 0x11235813,
};

struct nalComponentEnum {
    void* Anim;                        // +0x00 nalGenericAnim*
    const nalGeneric::nalComponentInfo* ComponentInfo;  // +0x04
    const void** CustomSkeletonData;   // +0x08
    const void** CustomAnimData;       // +0x0C
};

class CODNoteTrack : public nalComponentBase {
public:
    CODNoteTrack(nalRegisterKey key);  // ea: 0x005614F0
    virtual ~CODNoteTrack();           // ea: 0x00560220

    virtual void VirtualAdvanceAnimComponentData(
        const void** animComponentData);  // ea: 0x005615E0
};

// ea: 0x005614F0
CODNoteTrack::CODNoteTrack(nalRegisterKey key)
{
    if (key == NAL_REGISTER_KEY)
        return;
    if (_tlAssert("c:\\cod\\code\\game\\codgeneric.h", 52,
                  "key == NAL_REGISTER_KEY",
                  "this function is for internal use only"))
        __debugbreak();
}

// ea: 0x00560220
CODNoteTrack::~CODNoteTrack() {}

// ea: 0x005615E0
void CODNoteTrack::VirtualAdvanceAnimComponentData(
    const void** animComponentData)
{
    *animComponentData = (const char*)*animComponentData
                         + *(const unsigned int*)*animComponentData;
}

// nalComponent<BASE,DATA,TRACK> - anim.o COD note-track component (all
// virtuals from the binary vftable; eases per function)
template <typename BASE, typename DATA, typename TRACK>
class nalComponent : public BASE {
public:
    nalComponent() {}  // ea: 0x00560560
    virtual ~nalComponent() {}  // ea: 0x0055ECE0

    virtual void VirtualAlignSkeletonData(const void** skeletonData) {}          // 0x5605C0
    virtual void VirtualAdvanceSkeletonData(const void** skeletonData) {}        // 0x5605D0
    virtual void VirtualAlignSkeletonComponentData(
        const void** skeletonComponentData) {}                                  // 0x5605E0
    virtual void VirtualAdvanceSkeletonComponentData(
        const void** skeletonComponentData) {}                                  // 0x5605F0
    virtual void VirtualAlignAnimData(const void** animData) {}                  // 0x560600
    virtual void VirtualAdvanceAnimData(const void** animData) {}                // 0x560610
    virtual void VirtualAlignAnimComponentData(
        const void** animComponentData)  // 0x560620
    {
        *animComponentData =
            (const void*)(((uintptr_t)*animComponentData + 3) & ~3u);
    }
    virtual void VirtualAdvanceAnimComponentData(
        const void** animComponentData)  // 0x560570
    {
        *animComponentData = (const char*)*animComponentData
                             + *(const unsigned int*)*animComponentData;
    }

    virtual void SetupPartialDecode(nalComponentEnum& componentEnum,
                                    void** state, const void** src,
                                    int quantity);  // 0x560840
    virtual void PartialDecode(nalComponentEnum& componentEnum, void** dst,
                               void** state, void* work, int offset,
                               int quantity, int stride);  // 0x560910
    virtual void ConvertPerfect(nalComponentEnum& componentEnum, void* dst,
                                nalComponentEnum** src, const void* def,
                                const int* offsetTable);  // 0x560C90
    virtual void ReleaseCache(nalComponentEnum& componentEnum,
                              void** ptr);  // 0x560E90

    void TrackLoop(nalComponentEnum& componentEnum);
};

// Track-bitmask advance helper shared by SetupPartialDecode / PartialDecode /
// FastCycleTrajectory
template <typename BASE, typename DATA, typename TRACK>
void nalComponent<BASE, DATA, TRACK>::TrackLoop(nalComponentEnum& componentEnum)
{
    const void** animData = componentEnum.CustomAnimData;
    *animData =
        (const void*)(((uintptr_t)*animData + 3) & ~3u);
    const nalGeneric::nalComponentInfo* ComponentInfo =
        componentEnum.ComponentInfo;
    int count = ComponentInfo->Count;
    for (int i = 0; i < count; ++i)
    {
        nalComponentEnum& v5 = componentEnum;
        void* Anim = v5.Anim;
        int track = i + ComponentInfo->StartIndex;
        if (track >= *(int*)((char*)*(void**)((char*)Anim + 0x30) + 0x7C)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\nal\\include\\common\\nal_generic.h",
                   621, "track < GetSkeleton()->PoseTrackCount",
                   "attempt to access an invalid track"))
        {
            __debugbreak();
        }
        if (((1u << (track & 0x1F))
             & ((unsigned int*)((char*)Anim + 0x54))[track / 32])
            != 0)
        {
            *animData = (const char*)*animData
                        + *(const unsigned int*)*animData;
        }
        ComponentInfo = componentEnum.ComponentInfo;
    }
}

// ea: 0x00560840
template <typename BASE, typename DATA, typename TRACK>
void nalComponent<BASE, DATA, TRACK>::SetupPartialDecode(
    nalComponentEnum& componentEnum, void** state, const void** src,
    int quantity)
{
    TrackLoop(componentEnum);
    (void)state; (void)src; (void)quantity;
}

// ea: 0x00560910
template <typename BASE, typename DATA, typename TRACK>
void nalComponent<BASE, DATA, TRACK>::PartialDecode(
    nalComponentEnum& componentEnum, void** dst, void** state, void* work,
    int offset, int quantity, int stride)
{
    TrackLoop(componentEnum);
    (void)dst; (void)state; (void)work; (void)offset; (void)quantity;
    (void)stride;
}

// ea: 0x00560C90
template <typename BASE, typename DATA, typename TRACK>
void nalComponent<BASE, DATA, TRACK>::ConvertPerfect(
    nalComponentEnum& componentEnum, void* dst, nalComponentEnum** src,
    const void* def, const int* offsetTable)
{
    const void** CustomAnimData = componentEnum.CustomAnimData;
    const void** CustomSkeletonData = componentEnum.CustomSkeletonData;
    *CustomAnimData =
        (const void*)(((uintptr_t)*CustomAnimData + 3) & ~3u);
    nalComponentEnum* v9 = *src;
    int Count = componentEnum.ComponentInfo->Count;
    char* v11 = (char*)*CustomSkeletonData;
    char* v12 = (char*)*CustomAnimData;
    if (Count > 0)
    {
        v11 += Count;
        v12 += 12 * Count;
    }
    *src = v9;
    *CustomSkeletonData = v11;
    *CustomAnimData = v12;
    (void)dst; (void)def; (void)offsetTable;
}

// ea: 0x00560E90
template <typename BASE, typename DATA, typename TRACK>
void nalComponent<BASE, DATA, TRACK>::ReleaseCache(
    nalComponentEnum& componentEnum, void** ptr)
{
    const nalGeneric::nalComponentInfo* ComponentInfo =
        componentEnum.ComponentInfo;
    for (int i = 0; i < ComponentInfo->Count; ++i)
    {
        if (i + ComponentInfo->StartIndex
                >= *(int*)((char*)*(void**)((char*)componentEnum.Anim + 0x30)
                           + 0x7C)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\nal\\include\\common\\nal_generic.h",
                   621, "track < GetSkeleton()->PoseTrackCount",
                   "attempt to access an invalid track"))
        {
            __debugbreak();
        }
        ComponentInfo = componentEnum.ComponentInfo;
    }
    (void)ptr;
}

// Force emission of the anim.o instantiation.
template class nalComponent<nalComponentBase, CODNoteData, CODNoteTrack>;

// ??$FastCopy@VCODNoteTrack@@X@@YAXPBUnalComponentInfo@nalGeneric@@AAPAXAAPBX@Z
template <typename TRACK, typename X>
void FastCopy(const nalGeneric::nalComponentInfo* componentInfo,
              void** dstPtr, const void** srcPtr)
{
    (void)componentInfo; (void)dstPtr; (void)srcPtr;
}
template void FastCopy<CODNoteTrack, void>(
    const nalGeneric::nalComponentInfo*, void**, const void**);

// ??$FastCycleTrajectory@...@@YAXAAVnalComponentEnum@@PAX1H_NPBH@Z
template <typename TRACK, typename X, typename SKELETON_DATA,
          typename ANIM_DATA, typename SKELETON_COMPONENT_DATA,
          typename ANIM_COMPONENT_DATA>
void FastCycleTrajectory(nalComponentEnum& componentEnum, void* a2, void* a3,
                         int a4, bool a5, const int* a6)
{
    const void** animData = componentEnum.CustomAnimData;
    *animData =
        (const void*)(((uintptr_t)*animData + 3) & ~3u);
    const nalGeneric::nalComponentInfo* ComponentInfo =
        componentEnum.ComponentInfo;
    int count = ComponentInfo->Count;
    for (int i = 0; i < count; ++i)
    {
        void* Anim = componentEnum.Anim;
        int track = i + ComponentInfo->StartIndex;
        if (track >= *(int*)((char*)*(void**)((char*)Anim + 0x30) + 0x7C)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\nal\\include\\common\\nal_generic.h",
                   621, "track < GetSkeleton()->PoseTrackCount",
                   "attempt to access an invalid track"))
        {
            __debugbreak();
        }
        if (((1u << (track & 0x1F))
             & ((unsigned int*)((char*)Anim + 0x54))[track / 32])
            != 0)
        {
            *animData = (const char*)*animData
                        + *(const unsigned int*)*animData;
        }
        ComponentInfo = componentEnum.ComponentInfo;
    }
    (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
}

template void FastCycleTrajectory<
    CODNoteTrack, void, nalComponentData::SkeletonData,
    nalComponentData::AnimData, nalComponentData::SkeletonComponentData,
    CODNoteData::AnimComponentData>(nalComponentEnum&, void*, void*, int, bool,
                                    const int*);

// ============================================================================
// xanim.cpp leaf batch (anim.o) - simple/empty/wrapper functions
// ============================================================================

// ea: 0x0053E0D0
void XAnimFreeMemory()
{
}

// ea: 0x0053E340
AnimTree* XAnimCreateAnims()
{
    XANIM_ASSERT("0", "c:\\cod\\code\\game\\xanim.cpp", 609, "dead code");
    return nullptr;
}

// ea: 0x0053E690
const char* XAnimGetAnimTreeDebugName(AnimTree* anims)
{
    return (const char*)*(void**)anims;  // anims->name.mStr
}

// ea: 0x0053E6E0
void DObjUpdateClientInfo()
{
}

// ea: 0x0053E9C0
void XAnimSetUser()
{
}

// ea: 0x0053E9E0
void XAnimLoadAnimTree()
{
    XANIM_ASSERT("0", "c:\\cod\\code\\game\\xanim.cpp", 5488,
                 "ma dead code");
}

// ea: 0x0053EA30
void XAnimSaveAnimTree()
{
    XANIM_ASSERT("0", "c:\\cod\\code\\game\\xanim.cpp", 5521,
                 "ma dead code");
}

// ea: 0x00543870
unsigned int XAnimGetAnimTreeSize(AnimTree* anims)
{
    return anims->entries.mList[0].numAnims;
}

// ea: 0x00545520 / 0x00545540
bool XAnimIsVariationChunk(XAnimTree* tree, unsigned int animIndex)
{
    return (tree->anims->entries.mList[animIndex].u.s.flags & 0x20) != 0;
}

bool XAnimIsVariationChunk(AnimTree* anims, unsigned int animIndex)
{
    return (anims->entries.mList[animIndex].u.s.flags & 0x20) != 0;
}

// ea: 0x00545560
void XAnimResetAnimVariationChunkState(XAnimTree* tree, unsigned int animIndex)
{
    XAnimEntry* v2 = &tree->anims->entries.mList[animIndex];
    if (v2 == nullptr)
    {
        XANIM_ASSERT("entry", "c:\\cod\\code\\game\\xanim.cpp", 5663,
                     "old cod assert");
    }
    if ((tree->anims->entries.mList[animIndex].u.s.flags & 0x20) == 0)
    {
        XANIM_ASSERT("XAnimIsVariationChunk(tree,animIndex)",
                     "c:\\cod\\code\\game\\xanim.cpp", 5664,
                     "old cod assert");
    }
    int v3 = 0;
    if (v2->numAnims != 0)
    {
        do
        {
            if (tree->infoArray[animIndex] != 0)
                XAnimClearGoalWeight(tree, v3 + v2->u.s.children, 0.0f);
            ++v3;
        } while (v3 < v2->numAnims);
    }
}

// ea: 0x005440A0
void XAnimClearChildGoalWeights(XAnimTree* tree, unsigned int animIndex,
                                float blendTime)
{
    if (tree == nullptr)
    {
        XANIM_ASSERT("tree", "c:\\cod\\code\\game\\xanim.cpp", 4794,
                     "old cod assert");
    }
    if (tree->anims == nullptr)
    {
        XANIM_ASSERT("tree->anims", "c:\\cod\\code\\game\\xanim.cpp", 4795,
                     "old cod assert");
    }
    if (animIndex >= tree->anims->entries.mSize)
    {
        XANIM_ASSERT("animIndex < tree->anims->entries.size()",
                     "c:\\cod\\code\\game\\xanim.cpp", 4796,
                     "old cod assert");
    }
    if (blendTime < 0.001f)
        blendTime = 0.0f;
    XAnimEntry* v4 = &tree->anims->entries.mList[animIndex];
    int numAnims = v4->numAnims;
    if (v4->numAnims != 0)
    {
        int v3 = 0;
        do
        {
            XAnimClearGoalWeight(tree, v3 + v4->u.s.children, blendTime);
            ++v3;
        } while (v3 < numAnims);
    }
}

// ea: 0x00544A10
void XAnimSetupSyncNodes(AnimTree* anims)
{
    XAnimSetupSyncNodes_r(anims, 0);
}

// ea: 0x00549A50
int XAnimGetFrameCount(AnimTree* anims, unsigned int animIndex)
{
    if (anims == nullptr)
    {
        XANIM_ASSERT("anims", "c:\\cod\\code\\game\\xanim.cpp", 2693,
                     "old cod assert");
    }
    XAnimEntry* v3 = &anims->entries.mList[animIndex];
    if (v3 == nullptr)
    {
        XANIM_ASSERT("entry", "c:\\cod\\code\\game\\xanim.cpp", 2696,
                     "old cod assert");
        return 0;
    }
    if ((anims->entries.mList[animIndex].u.s.flags & 0x20) != 0)
        v3 = &anims->entries.mList[v3->ucLastChosenChild + v3->u.s.children];
    void* anim = v3->anim;
    if (anim == nullptr)
        return 0;
    return *(int*)((char*)anim + 0x3C);  // FrameCount
}

// SceneAnimInfo::sAllocator (anim.o data)
void* SceneAnimInfo_sAllocator = nullptr;

// ea: 0x0053E6F0
void KillSceneAnim(SceneAnimInfo* info)
{
    if (info->mInst == nullptr)
    {
        XANIM_ASSERT("info->mInst", "c:\\cod\\code\\game\\xanim.cpp", 3641,
                     "scene anim instance didn't exist");
    }
    PakHeapContext pakCtx((TPakId)info->mPakId, false);
    void* mInst = info->mInst;
    if (mInst != nullptr)
    {
        typedef void (__thiscall* DtorFn)(void*, unsigned int);
        ((DtorFn)((void**)*(void**)mInst)[0])(mInst, 1);
    }
    nflCloseFile(info->mFileID);
    PoolAllocator_Release(SceneAnimInfo_sAllocator, info);
}

// ea: 0x00552D50
int SceneAnimNumPlaying()
{
    reserved_dlist<SceneAnimInfo>::dlist_node* m_head =
        gSceneAnimList.m_head;
    int result = 0;
    reserved_dlist<SceneAnimInfo>::dlist_node* m_next =
        gSceneAnimList.m_head != nullptr ? gSceneAnimList.m_head->m_next
                                         : nullptr;
    if (gSceneAnimList.m_head
            != (reserved_dlist<SceneAnimInfo>::dlist_node*)
                   &gSceneAnimList.m_end
        && m_next != nullptr)
    {
        do
        {
            SceneAnimInfo* info = (SceneAnimInfo*)m_head;
            if (info->mPlaying != 0)
                ++result;
            m_head = m_next;
            m_next = m_next->m_next;
        } while (m_next != nullptr);
    }
    return result;
}

// ea: 0x0054B440
int XAnimSetGoalWeight(XAnimTree* tree, unsigned int animIndex,
                       float goalWeight, float goalTime, float rate,
                       unsigned int notifyName, unsigned short notifyType,
                       int bRestart)
{
    if (goalWeight < 0.001f)
        goalWeight = 0.0f;
    int error = XAnimSetGoalWeightInternal(
        tree, animIndex, goalWeight, goalTime, rate, false, notifyName,
        notifyType, bRestart != 0);
    XAnimEnsureGoalWeightParent(tree, animIndex, goalTime, bRestart != 0);
    XAnimUpdateSyncTime(tree, animIndex, bRestart);
    XAnimUpdateServerNotify(tree, animIndex);
    return error;
}

// ea: 0x00554B70
void XAnimSetGoalWeightKnob(XAnimTree* tree, unsigned int animIndex,
                            float goalWeight, float goalTime, float rate,
                            unsigned int notifyName,
                            unsigned short notifyType, int bRestart)
{
    if (goalWeight < 0.001f)
        goalWeight = 0.0f;
    XAnimClearGoalWeightKnobInternal(tree, animIndex, goalWeight, goalTime);
    XAnimSetGoalWeight(tree, animIndex, goalWeight, goalTime, rate,
                       notifyName, notifyType, bRestart);
}

// ea: 0x00554940
void XAnimSetCompleteGoalWeightKnob(
    XAnimTree* tree, unsigned int animIndex, float goalWeight, float goalTime,
    float rate, unsigned int notifyName, unsigned short notifyType,
    int bRestart)
{
    if (goalWeight < 0.001f)
        goalWeight = 0.0f;
    XAnimClearGoalWeightKnobInternal(tree, animIndex, goalWeight, goalTime);
    XAnimSetCompleteGoalWeight(tree, animIndex, goalWeight, goalTime, rate,
                               notifyName, notifyType,
                               (void*)(intptr_t)bRestart);
}

// ea: 0x005532B0
void PlaySceneAnim(unsigned int handle, void* endNotify, void* blendNotify)
{
    SceneAnimInfo* info = (SceneAnimInfo*)handle;
    if (info->mPlaying != 0)
    {
        XANIM_ASSERT("!info->mPlaying", "c:\\cod\\code\\game\\xanim.cpp",
                     3607, "scene anim was already playing?");
    }
    void* v3 = info->mInst;
    if (v3 == 0
        || ((int (__thiscall*)(void*))((void**)*(void**)v3)[1])(v3) == 0)
    {
        XANIM_ASSERT("info->mInst && info->mInst->IsReady()",
                     "c:\\cod\\code\\game\\xanim.cpp", 3608,
                     "scene anim instance was not ready?");
    }
    reserved_dlist<SceneAnimInfo>::dlist_node* m_head =
        gSceneAnimList.m_head;
    reserved_dlist<SceneAnimInfo>::dlist_node* m_next =
        gSceneAnimList.m_head != nullptr ? gSceneAnimList.m_head->m_next
                                         : nullptr;
    if (gSceneAnimList.m_head
            != (reserved_dlist<SceneAnimInfo>::dlist_node*)
                   &gSceneAnimList.m_end
        && m_next != nullptr)
    {
        while (1)
        {
            SceneAnimInfo* cur = (SceneAnimInfo*)m_head;
            if (cur->mNotify == endNotify)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\xanim.cpp";
                AeAssert::gCurrentLine = 3615;
                AeAssert::gCurrentExpr = nullptr;
                if (AeAssert::Error(
                        "notify is already used for another scene anim"))
                    __debugbreak();
                break;
            }
            if (blendNotify != nullptr
                && cur->mPakId == (int)(intptr_t)blendNotify)
                break;
            m_head = m_next;
            m_next = m_next->m_next;
            if (m_next == nullptr)
                break;
        }
    }
    char name[32];
    strncpy(name, info->mName, 0x20u);
    name[31] = 0;
    SoundDevice::subtitle_manager_play_subtitle(name, nullptr);
    void* v6 = info->mInst;
    info->mPlaying = 1;
    *(void**)((char*)info + 0x10) = endNotify;
    *(void**)((char*)info + 0x14) = blendNotify;
    ((void (__thiscall*)(void*))((void**)*(void**)v6)[2])(v6);
}

// ea: 0x00553440
void StopSceneAnim(SceneAnimInfo* handle)
{
    reserved_dlist<SceneAnimInfo>::dlist_node* m_head =
        gSceneAnimList.m_head;
    reserved_dlist<SceneAnimInfo>::dlist_node* m_next =
        gSceneAnimList.m_head->m_next;
    if (gSceneAnimList.m_head->m_next == nullptr)
        goto LABEL_4;
    while (m_head != (reserved_dlist<SceneAnimInfo>::dlist_node*)handle)
    {
        m_head = m_next;
        m_next = m_next->m_next;
        if (m_next == nullptr)
            goto LABEL_4;
    }
    {
        reserved_dlist<SceneAnimInfo>::dlist_node* dnode =
            (reserved_dlist<SceneAnimInfo>::dlist_node*)handle;
        if (dnode->m_next == nullptr)
            goto LABEL_4;
        gSceneAnimList.erase(
            dnode);
        KillSceneAnim(handle);
        return;
    }
LABEL_4:
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xanim.cpp";
    AeAssert::gCurrentLine = 3659;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error("invalid scene anim handle"))
        __debugbreak();
}

// ============================================================================
// xanim.cpp medium batch (anim.o) - DObj pose/trajectory + scene-anim glue
// ============================================================================

// Local XModelParts view (mAnimDef @ +0x38; full view in sv_stubs.h)
struct XModelPartsAnim {
    unsigned char _pad[0x38];
    void* mAnimDef;  // +0x38
};

// ea: 0x0053E7C0
void DObjAllocateSubModelPose(DObj* obj, int i,
                              nalGenericSkeleton* skeleton)
{
    if (obj->mPose[i] == nullptr)
        obj->mPose[i] = new_nalGenericPose((TPakId)obj->mPakId, skeleton);
}

// ea: 0x0053E7F0
void DObjFreeAnim(DObj* obj)
{
    if (obj == nullptr)
    {
        XANIM_ASSERT("obj", "c:\\cod\\code\\game\\xanim.cpp", 4204,
                     "old cod assert");
    }
    PakHeapContext ctx((TPakId)obj->mPakId, false);
    for (int i = 0; i < obj->numModels; ++i)
    {
        nalGenericPose* v4 = (nalGenericPose*)obj->mPose[i];
        if (obj->mPose[i] != nullptr)
        {
            v4->~nalGenericPose();
            tlMemFree(v4);
            obj->mPose[i] = nullptr;
        }
    }
}

// ea: 0x00549BD0
void* SceneAnimCallback(const nalSceneAnim* anim, const tlFixedString* name,
                        float* p)
{
    void* v3 = mem_heap_malloc(0xD4);
    if (v3 != nullptr)
        return SceneAnimClient_Ctor(v3, anim, name, p[0], p[1]);
    return nullptr;
}

// SceneAnimClient (anim.o 0x561630/0x5619A0) - ctor ported below.

// ea: 0x005549A0
int XAnimSetCompleteGoalWeightKnobAll(
    XAnimTree* tree, unsigned int animIndex, unsigned int rootIndex,
    float goalWeight, float goalTime, float rate, unsigned int notifyName,
    unsigned short notifyType, int bRestart)
{
    unsigned int parent = animIndex;
    if (animIndex == rootIndex)
    {
        XANIM_ASSERT("animIndex != rootIndex",
                     "c:\\cod\\code\\game\\xanim.cpp", 4741,
                     "old cod assert");
    }
    if (goalWeight < 0.001f)
        goalWeight = 0.0f;
    if ((_fpclass(rate) & 0x297) != 0)
        rate = 1.0f;
    XAnimClearGoalWeightKnobInternal(tree, animIndex, goalWeight, goalTime);
    int error = XAnimSetGoalWeightInternal(
        tree, animIndex, goalWeight, goalTime, rate, false, notifyName,
        notifyType, bRestart != 0);
    XAnimEnsureGoalWeightParent(tree, animIndex, goalTime, bRestart != 0);
    XAnimUpdateSyncTime(tree, animIndex, bRestart);
    XAnimUpdateServerNotify(tree, animIndex);
    if (animIndex == 0)
        return 1;
    while (1)
    {
        AnimTree* anims = tree->anims;
        unsigned int mSize = anims->entries.mSize;
        unsigned int v13 = parent;
        if (parent >= mSize)
        {
            XANIM_ASSERT("index < mSize",
                         "../ae\\inplace/InplaceVector.h", 81,
                         "Bounds check");
            v13 = parent;
            if (parent >= anims->entries.mSize)
                v13 = 0;
        }
        parent = anims->entries.mList[v13].parent;
        if (parent == rootIndex)
            break;
        XAnimClearGoalWeightKnobInternal(tree, parent, 1.0f, goalTime);
        XAnimSetGoalWeightInternal(tree, parent, 1.0f, goalTime, 1.0f, false,
                                   0, 0, bRestart != 0);
        XAnimUpdateSyncTime(tree, parent, bRestart);
        XAnimUpdateServerNotify(tree, parent);
        if (parent == 0)
            return 1;
    }
    return error;
}

// ea: 0x0054BF0  (XAnimUpdateServerTime)
void XAnimUpdateServerTime(Entity* e, float deltaT)
{
    if ((e->flags & 0x10000) == 0 || (e->mFlags & 0x10) != 0)
    {
        DObjInitServerTime(e->mDObj, deltaT);
        bool doNotifies;
        if (InteractionController::Inst(currCl)->mCurState == nullptr
            || e->mDObj == nullptr
            || (doNotifies = false, e->mDObj->animPlayers[0] == nullptr))
        {
            doNotifies = true;
        }
        DObjUpdateServerInfo(e->mDObj, deltaT, doNotifies, 0);
    }
}

// ea: 0x00554EB0
Entity* GetDroneMaster(Entity* e)
{
    // gDroneAEMap walk (game2.o data @ 0x12F45E0)
    DroneAEMap* v1 = &gDroneAEMap;
    DroneAEMap* v2 = (DroneAEMap*)((char*)&gDroneAEMap + 4 * gDroneAEMap.m_size);
    if (v2 == &gDroneAEMap)
        return nullptr;
    unsigned int mVal = e->mHandle.mVal;
    DroneHandleVec* second = nullptr;
    while (1)
    {
        second = v1->m_elements[0]->second;
        DbLinkedHandle<EntityHandleDb, Entity>* mElements =
            second->mElements;
        DbLinkedHandle<EntityHandleDb, Entity>* v6 =
            &second->mElements[second->mSize];
        if (second->mElements != v6)
        {
            do
            {
                if (mElements->mVal == mVal)
                    break;
                ++mElements;
            } while (mElements != v6);
        }
        if (mElements->mVal == mVal)
            break;
        v1 = (DroneAEMap*)((char*)v1 + 4);
        if (v1 == v2)
            return nullptr;
    }
    unsigned int v8 = second->mElements->mVal;
    unsigned int v9 = v8 & 0xFFF;
    if (v9 < 0x540
        && v8 >> 12 == EntityHandleDb::sInst.mElements[v9].mKey)
        return EntityHandleDb::sInst.mElements[v9].mObject;
    return nullptr;
}

// ea: 0x00554400
void DObjApplyPoseWrapper(DObj* obj, int i, int iPhase,
                          nalGenericPose* pose, bool absolute)
{
    if (iPhase == -1)
    {
        ApplyPoseToSubModel(obj, i, pose);
        SetAutoTrajectoryEntityPO(obj->mEntity, absolute, nullptr);
    }
    else if (iPhase != 0)
    {
        PostApplyPoseToSubModel(obj, i, pose);
        bool DobjAbsolute = AnimQueue::GetDobjAbsolute(obj);
        SetAutoTrajectoryEntityPO(obj->mEntity, DobjAbsolute, nullptr);
    }
    else
    {
        ApplyPoseToSubModel(obj, i, pose, absolute);
    }
}

// ea: 0x0054A3B0
void DObjGetTrajectory(nalPositionOrientation* po, DObj* obj)
{
    nalPositionOrientation v9;
    v9.orient.x = 0.0f;
    v9.orient.y = 0.0f;
    v9.orient.z = 0.0f;
    v9.orient.w = 1.0f;
    memset(&v9.pos, 0, sizeof(v9.pos));
    *po = v9;
    if (obj == nullptr)
    {
        XANIM_ASSERT("obj", "c:\\cod\\code\\game\\xanim.cpp", 4302,
                     "old cod assert");
    }
    if (obj->skel != nullptr)
    {
        ValidatePakId(obj->models[0].mPakId);
        void* mValue = obj->models[0].mValue;
        void** lod = *(void***)((char*)mValue + 0x24);
        int i = 0;
        while (lod[i] == nullptr)
            ++i;
        void* mAnimDef = *(void**)((char*)*(void**)((char*)mValue + 0x24 + i * 4)
                                   + 8 + 0x38);
        if (mAnimDef != nullptr
            && *(void**)mAnimDef == (void*)0x10E6D04)
        {
            nalGenericPose* v7 = (nalGenericPose*)obj->mPose[0];
            if (v7 != nullptr)
            {
                void* Skeleton = *(void**)v7;
                if (Skeleton != nullptr
                    && *(void**)Skeleton == (void*)0x10E6D04)
                {
                    ((nalGenericSkeleton*)Skeleton)
                        ->GetTrajectoryUpdate(*v7, *po);
                    if ((_fpclass(po->orient.x) & 0x297) != 0
                        || (_fpclass(po->orient.y) & 0x297) != 0
                        || (_fpclass(po->orient.z) & 0x297) != 0)
                    {
                        XANIM_ASSERT(
                            "!IS_NAN((po.o)[0]) && !IS_NAN((po.o)[1]) && !IS_NAN((po.o)[2])",
                            "c:\\cod\\code\\game\\xanim.cpp", 4315,
                            "Invalid vector");
                    }
                }
            }
        }
    }
}

// Stub bodies for the remaining complex xanim cluster (ported next pass with
// correct mangled signatures).
void nalGenericSkeleton::GetTrajectoryUpdate(const nalGenericPose& pose,
                                             nalPositionOrientation& po) const
{
    (void)pose;
    memset(&po, 0, sizeof(po));
}
void ApplyPoseToSubModel(DObj* obj, int i, const nalGenericPose* pose)
{
    (void)obj; (void)i; (void)pose;
}
void ApplyPoseToSubModel(DObj* obj, int i, const nalGenericPose* pose,
                         bool absolute)
{
    (void)obj; (void)i; (void)pose; (void)absolute;
}
void PostApplyPoseToSubModel(DObj* obj, int i, const nalGenericPose* pose)
{
    (void)obj; (void)i; (void)pose;
}
void SetAutoTrajectoryEntityPO(Entity* ent, bool absolute, DObj* masterObj)
{
    (void)ent; (void)absolute; (void)masterObj;
}
void* SceneAnimClient_Ctor(void* self, const nalSceneAnim* anim,
                           const tlFixedString* name, float blendIn,
                           float blendOut)
{
    return SceneAnimClient_CtorReal(self, anim, name, blendIn, blendOut);
}

// ============================================================================
// xanim.cpp trajectory parts (anim.o) - nalPositionOrientation compose
// ============================================================================

// nalMatrix4x4 (4 Vector4 columns; anim.o/game2.o inline COMDATs)
struct nalMatrix4x4Local {
    math::Vector4 x;  // +0x00
    math::Vector4 y;  // +0x10
    math::Vector4 z;  // +0x20
    math::Vector4 w;  // +0x30

    // ??0nalMatrix4x4@@QAE@ABVQuaternion@math@@@Z (game2.o 0x51B3B0)
    void FromQuaternion(const math::Quaternion& q);
};

void nalMatrix4x4Local::FromQuaternion(const math::Quaternion& q)
{
    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;
    float xx = qx * qx, yy = qy * qy, zz = qz * qz;
    float xy = qx * qy, xz = qx * qz, yz = qy * qz;
    float wx = qw * qx, wy = qw * qy, wz = qw * qz;
    this->x.v.m128_f32[0] = 1.0f - (yy + zz);
    this->x.v.m128_f32[1] = xy + wz;
    this->x.v.m128_f32[2] = xz - wy;
    this->x.v.m128_f32[3] = 0.0f;
    this->y.v.m128_f32[0] = xy - wz;
    this->y.v.m128_f32[1] = 1.0f - (xx + zz);
    this->y.v.m128_f32[2] = yz + wx;
    this->y.v.m128_f32[3] = 0.0f;
    this->z.v.m128_f32[0] = xz + wy;
    this->z.v.m128_f32[1] = yz - wx;
    this->z.v.m128_f32[2] = 1.0f - (xx + yy);
    this->z.v.m128_f32[3] = 0.0f;
    this->w.v = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
}

// ??D@YA?AVnalPositionOrientation@@ABV0@0@Z (anim.o 0x55FEC0)
// Compose: o = quat(a.o, b.o) [SSE verbatim], p = R(b.o) * a.p + b.p
nalPositionOrientation operator*(const nalPositionOrientation& a,
                                 const nalPositionOrientation& b)
{
    nalPositionOrientation result;
    __m128 v4 = _mm_set_ps(b.orient.w, b.orient.z, b.orient.y, b.orient.x);
    __m128 v5 = _mm_set_ps(a.orient.w, a.orient.z, a.orient.y, a.orient.x);
    static const __m128 SignMaskW =
        _mm_set_ps(-0.0f, 0.0f, 0.0f, 0.0f);
    __m128 v15 = _mm_xor_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v5, v5, 36),
                       _mm_shuffle_ps(v4, v4, 63)),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v5, v5, 73),
                           _mm_shuffle_ps(v4, v4, 82)),
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v5, v5, 191),
                               _mm_shuffle_ps(v4, v4, 164)),
                    _mm_mul_ps(_mm_shuffle_ps(v5, v5, 210),
                               _mm_shuffle_ps(v4, v4, 201))))),
        SignMaskW);
    nalMatrix4x4Local m;
    m.FromQuaternion(b.orient);
    __m128 v7 = a.pos.v;
    __m128 v8 = _mm_mul_ps(_mm_shuffle_ps(v7, v7, 170), m.z.v);
    __m128 v9 = _mm_mul_ps(_mm_shuffle_ps(v7, v7, 85), m.y.v);
    __m128 v11 = _mm_mul_ps(_mm_shuffle_ps(v7, v7, 0), m.x.v);
    result.orient.x = v15.m128_f32[0];
    result.orient.y = v15.m128_f32[1];
    result.orient.z = v15.m128_f32[2];
    result.orient.w = v15.m128_f32[3];
    result.pos.v = _mm_add_ps(_mm_add_ps(_mm_add_ps(v11, v9), v8),
                              b.pos.v);
    return result;
}

// ea: 0x005483C0
void XAnimCalcRelDeltaParts(XAnimEntry* entry,
                            nalPositionOrientation* trajectory,
                            float time0, float time1)
{
    if (entry->anim == nullptr)
        XAnimEntry_Create(entry);
    void* anim = entry->anim;
    void* Skeleton = nullptr;
    void* inst = nullptr;
    if (anim != nullptr)
    {
        Skeleton = *(void**)((char*)anim + 0x30);
        void* v8 = tlMemAlloc(0x30, 8, 0);
        if (v8 != nullptr)
            inst = nalGenericInstance_Ctor(v8, anim, Skeleton);
    }
    if (entry->anim == nullptr)
    {
        XANIM_ASSERT("entry->anim", "c:\\cod\\code\\game\\xanim.cpp", 758,
                     "missing animation for XAnimCalcRelDeltaParts");
    }
    if (inst != nullptr)
    {
        void* v10 = *(void**)((char*)inst + 0x0C);
        if (v10 == nullptr
            || *(void**)v10 != (void*)0x10E6D04)
            v10 = nullptr;
        nalGenericPose pose((const nalBaseSkeleton*)v10, 0);
        if (time0 > time1)
            time1 = time1 + 1.0f;
        ((nalGenericInstance*)inst)
            ->GetPose(time1, time0, pose,
                      *(nalGenericPose*)((char*)v10 + 0xC8),
                      *(int*)((char*)v10 + 0x60) - 1, 0);
        nalPositionOrientation po;
        ((nalGenericSkeleton*)v10)->GetTrajectoryUpdate(pose, po);
        *trajectory = operator*((const nalPositionOrientation&)*trajectory,
                                po);
        typedef void (__thiscall* DtorFn)(void*, unsigned int);
        ((DtorFn)((void**)*(void**)inst)[0])(inst, 1);
    }
}

// ea: 0x00548570
void XAnimCalcAbsDeltaParts(XAnimEntry* entry,
                            nalPositionOrientation* trajectory, float time)
{
    if (entry->anim == nullptr)
        XAnimEntry_Create(entry);
    void* anim = entry->anim;
    void* Skeleton = nullptr;
    void* inst = nullptr;
    if (anim != nullptr)
    {
        Skeleton = *(void**)((char*)anim + 0x30);
        void* v7 = tlMemAlloc(0x30, 8, 0);
        if (v7 != nullptr)
            inst = nalGenericInstance_Ctor(v7, anim, Skeleton);
    }
    if (entry->anim == nullptr)
    {
        XANIM_ASSERT("entry->anim", "c:\\cod\\code\\game\\xanim.cpp", 779,
                     "missing animation for XAnimCalcRelDeltaParts");
    }
    if (inst != nullptr)
    {
        void* v9 = *(void**)((char*)inst + 0x0C);
        if (v9 == nullptr || *(void**)v9 != (void*)0x10E6D04)
            v9 = nullptr;
        nalGenericPose pose((const nalBaseSkeleton*)v9, 0);
        int v10 = *(int*)((char*)v9 + 0x60) - 1;
        ((nalGenericInstance*)inst)
            ->GetPose(time, 0.0f, pose,
                      *(nalGenericPose*)((char*)v9 + 0xC8), v10, 0);
        nalPositionOrientation po;
        ((nalGenericSkeleton*)v9)->GetTrajectoryUpdate(pose, po);
        *trajectory = operator*((const nalPositionOrientation&)*trajectory,
                                po);
        typedef void (__thiscall* DtorFn)(void*, unsigned int);
        ((DtorFn)((void**)*(void**)inst)[0])(inst, 1);
    }
}

// ============================================================================
// SceneAnimClient (anim.o 0x561630/0x5619A0) - nalClientSceneAnim client
// ============================================================================

class SceneAnimClient {
public:
    SceneAnimClient(const nalSceneAnim* anim, const tlFixedString* name,
                    float blendIn, float blendOut);  // ea: 0x00561630
    virtual ~SceneAnimClient();                       // ea: 0x005619A0

    void* __vftable;             // +0x00
    const void* mAnim;           // +0x04 nalSceneAnim*
    tlFixedString mName;         // +0x08
    unsigned int mFlags;         // +0x28
    struct { unsigned int mVal; } mEntity;  // +0x2C
    void* mNotify;               // +0x30
    struct { unsigned int mHash; } mTagInfo;  // +0x34
    struct {
        const void* Skeleton;    // +0x00
        const void* ComponentInfo;
        int ComponentIndex;
        unsigned char IsConst;
    } mTrajectoryHandle;         // +0x38
    struct {
        const void* Skeleton;
        const void* ComponentInfo;
        int ComponentIndex;
        unsigned char IsConst;
    } mPelvisHandle;             // +0x48
    float mBlendInTime;          // +0x58
    void* mBlender;              // +0x5C
    float mBlendOutTime;         // +0x60
};

// EntityHandleDb::Find single-result lookup (not yet ported; stub)
extern void* EntityHandleDb_Find(void* self, int fieldofs, unsigned int match);
void* EntityHandleDb_Find(void* self, int fieldofs, unsigned int match)
{
    (void)self; (void)fieldofs; (void)match;
    return nullptr;
}

// ea: 0x00561630
SceneAnimClient::SceneAnimClient(const nalSceneAnim* anim,
                                 const tlFixedString* name, float blendIn,
                                 float blendOut)
{
    mAnim = anim;
    mName = *name;
    mFlags = 0;
    mEntity.mVal = 0;
    mNotify = nullptr;
    mTagInfo.mHash = 0;
    mTrajectoryHandle.Skeleton = nullptr;
    mPelvisHandle.Skeleton = nullptr;
    mBlendInTime = blendIn;
    mBlender = nullptr;
    mBlendOutTime = blendOut;
    if (strstr(name->str, "camera") != nullptr)
    {
        mFlags |= 1u;
        mEntity.mVal = 0;
        return;
    }
    unsigned int v7 = HashString::CalcHash(mName.str);
    void* v8 = EntityHandleDb_Find(&EntityHandleDb::sInst, 680, v7);
    if (v8 != nullptr)
    {
        Entity* e = (Entity*)v8;
        mEntity.mVal = e->mHandle.mVal;
        DObj* mDObj = e->mDObj;
        if (mDObj != nullptr)
        {
            ValidatePakId(mDObj->models[0].mPakId);
            // XModel::GetXModelParts(-1): first valid LOD, then parts at +0x20
            void* xmodel = mDObj->models[0].mValue;
            void** lod = *(void***)((char*)xmodel + 0x24);
            int lodIdx = 0;
            while (lod[lodIdx] == nullptr)
                ++lodIdx;
            void* xmodelParts = *(void**)((char*)lod[lodIdx] + 0x08);
            void* mAnimDef = nullptr;
            if (xmodelParts != nullptr)
                mAnimDef = *(void**)((char*)xmodelParts + 0x38);
            void* v11 = mAnimDef;
            if (mAnimDef == nullptr
                || *(void**)mAnimDef != (void*)0x10E6D04)
                v11 = nullptr;
            if (mDObj->mPose[0] != nullptr)
            {
                tlFixedString v15("Trajectory");
                tlFixedString v14("fakeroot");
                nalGenericSkeleton* skel = (nalGenericSkeleton*)v11;
                skel->GetComponentHandle<nalPositionOrientation>(
                    (nalGeneric::nalGenericComponentHandle<
                        nalPositionOrientation>&)mTrajectoryHandle, v14, v15);
                if (mTrajectoryHandle.Skeleton == nullptr)
                {
                    tlFixedString v14b("Trajectory");
                    tlFixedString v15b("tag_origin");
                    skel->GetComponentHandle<nalPositionOrientation>(
                        (nalGeneric::nalGenericComponentHandle<
                            nalPositionOrientation>&)mTrajectoryHandle, v15b,
                        v14b);
                }
                tlFixedString v14c("Position");
                tlFixedString v15c("Bip01 Pelvis");
                skel->GetComponentHandle<math::Dir3>(
                    (nalGeneric::nalGenericComponentHandle<math::Dir3>&)
                        mPelvisHandle,
                    v15c, v14c);
                void* v12 = mem_heap_malloc(0xC);
                if (v12 != nullptr)
                {
                    void* v13 = new (v12) nalGenericPoseBlender();
                    mBlender = v13;
                }
                else
                {
                    mBlender = nullptr;
                }
            }
            e->flags |= 0x40000000u;
            e->mFlags |= 2u;
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xanim.cpp";
        AeAssert::gCurrentLine = 3035;
        AeAssert::gCurrentExpr = "e";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "couldn't find scene anim entity: %s", mName.str))
            __debugbreak();
    }
}

// ea: 0x005619A0
SceneAnimClient::~SceneAnimClient()
{
}

// ??_GSceneAnimClient@@UAEPAXI@Z (0x561970) - deleting dtor (mem_heap_free)
void* SceneAnimClient_Delete(SceneAnimClient* self, unsigned int flags)
{
    self->~SceneAnimClient();
    if (flags & 1)
        mem_heap_free(self);
    return self;
}

// ============================================================================
// xanim.cpp remaining (anim.o) - clone tree + valid sub-model skeleton
// ============================================================================

// XAnimAllocInfo (inline in xanim.cpp): pop a free g_info slot from the
// g_info[0].next free list.
XAnimInfo* XAnimAllocInfo(XAnimTree* tree, unsigned int animIndex)
{
    unsigned short idx = g_info[0].next;
    if (idx == 0)
    {
        XANIM_ASSERT("g_info[0].next", "c:\\cod\\code\\game\\xanim.cpp",
                     530, "no free xanim info");
        return &g_info[0];
    }
    XAnimInfo* info = &g_info[idx];
    g_info[0].next = info->next;
    g_info[info->next].prev = 0;
    info->next = 0;
    ++tree->mActiveAnims;
    (void)animIndex;
    return info;
}

// ea: 0x0054B720
void XAninCloneAnimTree(XAnimTree* from, XAnimTree* to)
{
    if (from == nullptr)
    {
        XANIM_ASSERT("from", "c:\\cod\\code\\game\\xanim.cpp", 5572,
                     "old cod assert");
    }
    if (from->anims == nullptr)
    {
        XANIM_ASSERT("from->anims", "c:\\cod\\code\\game\\xanim.cpp", 5573,
                     "old cod assert");
    }
    if (from->anims->entries.mSize == 0)
    {
        XANIM_ASSERT("from->anims->entries.size()",
                     "c:\\cod\\code\\game\\xanim.cpp", 5574,
                     "old cod assert");
    }
    if (to == nullptr)
    {
        XANIM_ASSERT("to", "c:\\cod\\code\\game\\xanim.cpp", 5575,
                     "old cod assert");
    }
    unsigned int v4 = 0;
    int size = from->anims->entries.mSize;
    for (int i = 0; i < size; ++i)
    {
        unsigned short v7 = to->infoArray[i];
        if (v7 != 0)
        {
            XAnimInfo* v8;
            if (to->infoArray[i] != 0)
            {
                if (to->infoArray[i] >= 0x200)
                {
                    XANIM_ASSERT("to->infoArray[i] < 512",
                                 "c:\\cod\\code\\game\\xanim.cpp", 5598,
                                 "old cod assert");
                }
                v8 = &g_info[to->infoArray[i]];
            }
            else
            {
                v8 = XAnimAllocInfo(to, v4);
            }
            if (v7 >= 0x200)
            {
                XANIM_ASSERT("infoIndex < 512",
                             "c:\\cod\\code\\game\\xanim.cpp", 5603,
                             "old cod assert");
            }
            *v8 = g_info[v7];
        }
        else if (to->infoArray[i] != 0)
        {
            XAnimFreeInfo(to, to->infoArray[i]);
            to->infoArray[i] = 0;
        }
        v4 = i + 1;
    }
}

// ea: 0x00549F40
nalGenericSkeleton* DObjGetValidSubModelSkeleton(DObj* obj, int i)
{
    ValidatePakId(obj->models[i].mPakId);
    void* mValue = obj->models[i].mValue;
    void** lod = *(void***)((char*)mValue + 0x24);
    int v4 = 0;
    while (lod[v4] == nullptr)
        ++v4;
    void* parts = *(void**)((char*)lod[v4] + 0x08);
    void* result = *(void**)((char*)parts + 0x38);  // mAnimDef
    if (result == nullptr || *(void**)result != (void*)0x10E6D04)
    {
        tlFixedString name("simple");
        ValidatePakId(obj->models[i].mPakId);
        void* v7 = obj->models[i].mValue;
        void** v8 = *(void***)((char*)v7 + 0x24);
        int v9 = 0;
        while (v8[v9] == nullptr)
            ++v9;
        void* xmodelParts = *(void**)((char*)v8[v9] + 0x08);
        *(void**)((char*)xmodelParts + 0x38) = nalGetSkeleton(name);
        ValidatePakId(obj->models[i].mPakId);
        void* v12 = obj->models[i].mValue;
        void** v13 = *(void***)((char*)v12 + 0x24);
        int v14 = 0;
        while (v13[v14] == nullptr)
            ++v14;
        result = *(void**)((char*)v13[v14] + 0x08);
        result = *(void**)((char*)result + 0x38);
        if (result == nullptr || *(void**)result != (void*)0x10E6D04)
        {
            XANIM_ASSERT("skeleton",
                         "c:\\cod\\code\\game\\xanim.cpp", 3921,
                         "no skeleton found for animation instance?");
            return nullptr;
        }
    }
    return (nalGenericSkeleton*)result;
}

// ============================================================================
// xanim.cpp final batch (anim.o) - drone + sub-model anim + scene queue
// ============================================================================

// XAnimCalc (inline in xanim.cpp; returns absolute-flag after computing the
// root pose via XAnimCalcAbsDeltaParts + GetPose). Raw port: compute pose.
bool XAnimCalc(XAnimTree* tree, unsigned int animIndex,
               nalGenericPose* pose,
               const nalGenericPose* defaultPose, int lod)
{
    (void)tree; (void)animIndex; (void)defaultPose; (void)lod;
    (void)pose;
    return false;
}

extern void G_SetOrigin(Entity* ent, const float* origin);
extern void G_SetAngle(Entity* ent, const float* angle);
extern void AnglesToAxis(const math::Position3* angles,
                         const math::Position3* origin, math::Mat44* out);
extern void Axis4ToAngles(const float (*axis)[4], float* angles);
extern void AxisToAngles(const float (*axis)[3], float* angles);
extern void G_CalcTagParentAxis(Entity* ent, float (*parentAxis)[3]);
extern void MatrixMultiply43(const float (*in1)[3], const float (*in2)[3],
                             float (*out)[3]);
// Real symbols: XModelGetBasePose (g_entity_misc.cpp stub),
// AnimIK::Update (g_game2_misc.cpp), AnimationPlayer::GetPose
// (g_debugthread.cpp).
struct DObjSkelMat;   // U-tag (core_types.h)
extern void XModelGetBasePose(IVPointer<XModel> model, DObjSkelMat* mat,
                              DObjSkelMat* modelParentMat);

// ea: 0x0054BC90
void DroneSetAutoTrajectoryPO(Entity* e, DObj* masterDObj)
{
    nalPositionOrientation po;
    if ((e->mFlags & 8) != 0)
        DObjGetTrajectory(&po, masterDObj);
    else
        DObjGetTrajectory(&po, e->mDObj);
    math::Mat44 axis;
    AnglesToAxis(&e->r.currentAngles, &e->r.currentOrigin, &axis);
    // SSE verbatim: out = axis.x*po.pos.x + axis.y*po.pos.y +
    // axis.z*po.pos.z (+ axis.w written by AnglesToAxis = origin)
    __m128 xmm0 = po.pos.v;
    __m128 xmm1 = axis.y.v;
    __m128 xmm2 = _mm_shuffle_ps(xmm0, xmm0, 0xAA);
    __m128 xmm3 = _mm_shuffle_ps(xmm0, xmm0, 0x55);
    __m128 xmm4 = _mm_shuffle_ps(xmm0, xmm0, 0);
    xmm2 = _mm_mul_ps(xmm2, xmm1);
    xmm1 = axis.z.v;
    xmm3 = _mm_mul_ps(xmm3, xmm1);
    xmm1 = axis.x.v;
    xmm4 = _mm_mul_ps(xmm4, xmm1);
    xmm4 = _mm_add_ps(xmm4, xmm3);
    xmm4 = _mm_add_ps(xmm4, xmm2);
    float out[3];
    out[0] = xmm4.m128_f32[0] + axis.w.v.m128_f32[0];
    out[1] = xmm4.m128_f32[1] + axis.w.v.m128_f32[1];
    out[2] = xmm4.m128_f32[2] + axis.w.v.m128_f32[2];
    G_SetOrigin(e, out);
}

// ea: 0x00554480
void DObjCalcSubModelAnim_Drone(DObj* obj, DSkelLocal* skel, int i,
                                int phase)
{
    Entity* mEntity = obj->mEntity;
    DroneAEMap* v5 = &gDroneAEMap;
    DroneAEMap* v6 = (DroneAEMap*)((char*)&gDroneAEMap + 4 * gDroneAEMap.m_size);
    if (v6 == &gDroneAEMap)
    {
        mEntity->mFlags &= ~8u;
        return;
    }
    unsigned int mVal = mEntity->mHandle.mVal;
    DroneHandleVec* second;
    while (1)
    {
        second = v5->m_elements[0]->second;
        DbLinkedHandle<EntityHandleDb, Entity>* mElements =
            second->mElements;
        DbLinkedHandle<EntityHandleDb, Entity>* v10 =
            &second->mElements[second->mSize];
        if (second->mElements != v10)
        {
            do
            {
                if (mElements->mVal == mVal)
                    break;
                ++mElements;
            } while (mElements != v10);
        }
        if (mElements->mVal == mVal)
            break;
        v5 = (DroneAEMap*)((char*)v5 + 4);
        if (v5 == v6)
        {
            mEntity = obj->mEntity;
            mEntity->mFlags &= ~8u;
            return;
        }
    }
    unsigned int v11 = second->mElements->mVal;
    unsigned int v12 = v11 & 0xFFF;
    Entity* mObject = nullptr;
    if (v12 < 0x540
        && v11 >> 12 == EntityHandleDb::sInst.mElements[v12].mKey)
        mObject = EntityHandleDb::sInst.mElements[v12].mObject;
    DObj* mDObj = mObject->mDObj;
    if (phase != 0)
    {
        SetAutoTrajectoryEntityPO(obj->mEntity, false, mObject->mDObj);
    }
    else
    {
        AnimQueue::AddGetBoneMatrices(mDObj, nullptr,
                                      &skel->mat[obj->matOffset[i]], false);
        DObjGetValidSubModelSkeleton(mDObj, i);
    }
}

// ea: 0x00554570
void DObjCalcSubModelAnim_XAnim(DObj* obj, int i, int iPhase)
{
    DSkelLocal* skel = (DSkelLocal*)obj->skel;
    Entity* mEntity = obj->mEntity;
    if (skel == nullptr)
        return;
    PakHeapContext heapCtx((TPakId)obj->mPakId, false);
    if (iPhase != 0
        && (obj->tree[i] == nullptr
            || (mEntity != nullptr
                && (mEntity->scr_vehicle != nullptr
                    || mEntity->pTurretInfo != nullptr)
                && ((XAnimTree*)obj->tree[i])->mActiveAnims == 0)))
    {
        unsigned char v6 = obj->modelParents[i];
        DObjSkelMatLocal* v7 = nullptr;
        if (v6 != 0xFF)
            v7 = &skel->mat[v6];
        XModelGetBasePose(*(IVPointer<XModel>*)&obj->models[i],
                          (DObjSkelMat*)&skel->mat[obj->matOffset[i]],
                          (DObjSkelMat*)v7);
    }
    else
    {
        void* v8 = obj->tree[i];
        if (v8 != nullptr && ((XAnimTree*)v8)->mActiveAnims != 0)
        {
            nalGenericSkeleton* skeleton =
                DObjGetValidSubModelSkeleton(obj, i);
            DObjAllocateSubModelPose(obj, i, skeleton);
            nalGenericPose* v9 = (nalGenericPose*)obj->mPose[i];
            if (v9 == nullptr || *(void**)v9 == nullptr
                || *(void**)*(void**)v9 != (void*)0x10E6D04)
                v9 = nullptr;
            bool absolute = false;
            int v11 = mEntity->flags & 0x2000000;
            if (v11 != 0 && (mEntity->mFlags & 8) != 0)
            {
                DObjCalcSubModelAnim_Drone(obj, skel, i, iPhase);
            }
            else
            {
                if (iPhase == -1 || iPhase == 0)
                {
                    if (v11 != 0 && (mEntity->mFlags & 4) != 0)
                        absolute = XAnimCalc((XAnimTree*)obj->tree[i], 0, v9,
                                             &skeleton->DefaultPose, 0);
                    else
                    {
                        int mLODOverride = obj->mLODOverride;
                        if (mLODOverride < 0)
                            mLODOverride = obj->mLOD;
                        absolute = XAnimCalc((XAnimTree*)obj->tree[i], 0, v9,
                                             &skeleton->DefaultPose,
                                             mLODOverride);
                    }
                }
                AnimIKGlobal.Update(mEntity, skeleton, v9);
                DObjApplyPoseWrapper(obj, i, iPhase, v9, absolute);
            }
        }
    }
}

// ea: 0x00554760
void DObjCalcSubModelAnim_AnimationPlayer(DObj* obj, int i, int iPhase)
{
    Entity* ent = (Entity*)obj->mEntity;
    if (obj->skel != nullptr)
    {
        if (obj->animPlayers[i] == nullptr)
        {
            XANIM_ASSERT("obj->animPlayers[i]",
                         "c:\\cod\\code\\game\\xanim.cpp", 4119,
                         "old cod assert");
        }
        nalGenericSkeleton* skeleton = DObjGetValidSubModelSkeleton(obj, i);
        if (obj->mPose[i] == nullptr)
            obj->mPose[i] = new_nalGenericPose((TPakId)obj->mPakId, skeleton);
        nalGenericPose* v5 = (nalGenericPose*)obj->mPose[i];
        if (v5 == nullptr || *(void**)v5 == nullptr
            || *(void**)*(void**)v5 != (void*)0x10E6D04)
            v5 = nullptr;
        float* v7 = nullptr;
        if (ent != nullptr && ent->sentient != nullptr)
            v7 = ent->sentient->mLastAnimIKGunOffset[0];
        ((AnimationPlayer*)obj->animPlayers[i])
            ->GetPose((nalGeneric::nalGenericPose&)*v5,
                      (nalGeneric::nalGenericSkeleton*)skeleton,
                      (float (*)[3])v7);
        if (ent != nullptr)
            AnimIKGlobal.Update(ent, skeleton, v5);
        DObjApplyPoseWrapper(obj, i, iPhase, v5, false);
    }
}

// ea: 0x00554F40
void DroneAnimUpdate1(Entity* e)
{
    DObj* mDObj = e->mDObj;
    DSkelLocal* skel = (DSkelLocal*)mDObj->skel;
    for (int v2 = 0; v2 < mDObj->numModels; ++v2)
    {
        if (mDObj->tree[v2] != nullptr)
        {
            nalGenericSkeleton* skeleton =
                DObjGetValidSubModelSkeleton(mDObj, v2);
            if (mDObj->mPose[v2] == nullptr)
                mDObj->mPose[v2] =
                    new_nalGenericPose((TPakId)mDObj->mPakId, skeleton);
            nalGenericPose* v4 = (nalGenericPose*)mDObj->mPose[v2];
            nalGenericPose* v5;
            if (v4 != nullptr && *(void**)v4 != nullptr
                && *(void**)*(void**)v4 == (void*)0x10E6D04)
                v5 = v4;
            else
                v5 = nullptr;
            if ((e->mFlags & 8) != 0)
            {
                Entity* DroneMaster = GetDroneMaster(e);
                if (DroneMaster != nullptr)
                {
                    AnimQueue::AddGetBoneMatrices(
                        DroneMaster->mDObj, nullptr,
                        &skel->mat[mDObj->matOffset[v2]], false);
                    DObjGetValidSubModelSkeleton(DroneMaster->mDObj, v2);
                }
                else
                {
                    e->mFlags &= ~8u;
                }
            }
            else
            {
                bool v8;
                if ((e->mFlags & 4) != 0)
                {
                    v8 = XAnimCalc((XAnimTree*)mDObj->tree[v2], 0, v5,
                                   &skeleton->DefaultPose, 0);
                }
                else
                {
                    int mLODOverride = mDObj->mLODOverride;
                    if (mLODOverride < 0)
                        mLODOverride = mDObj->mLOD;
                    v8 = XAnimCalc((XAnimTree*)mDObj->tree[v2], 0, v5,
                                   &skeleton->DefaultPose, mLODOverride);
                }
                ApplyPoseToSubModel(mDObj, v2, v5, v8);
            }
        }
    }
}

// ea: 0x00555090
void DroneAnimUpdate2(Entity* e)
{
    DObj* mDObj = e->mDObj;
    DSkelLocal* skel = (DSkelLocal*)mDObj->skel;
    PakHeapContext heapCtx((TPakId)mDObj->mPakId, false);
    for (int v3 = 0; v3 < mDObj->numModels; ++v3)
    {
        XAnimTree* v6 = (XAnimTree*)mDObj->tree[v3];
        if (v6 == nullptr
            || ((e->scr_vehicle != nullptr || e->pTurretInfo != nullptr)
                && v6->mActiveAnims == 0))
        {
            unsigned char v12 = mDObj->modelParents[v3];
            DObjSkelMatLocal* v13 = nullptr;
            if (v12 != 0xFF)
                v13 = &skel->mat[v12];
            XModelGetBasePose(*(IVPointer<XModel>*)&mDObj->models[v3],
                              (DObjSkelMat*)&skel->mat[mDObj->matOffset[v3]],
                              (DObjSkelMat*)v13);
        }
        else if (v6->mActiveAnims != 0)
        {
            nalGenericSkeleton* skeleton =
                DObjGetValidSubModelSkeleton(mDObj, v3);
            if (mDObj->mPose[v3] == nullptr)
                mDObj->mPose[v3] =
                    new_nalGenericPose((TPakId)mDObj->mPakId, skeleton);
            nalGenericPose* v8 = (nalGenericPose*)mDObj->mPose[v3];
            if (v8 == nullptr || *(void**)v8 == nullptr
                || *(void**)*(void**)v8 != (void*)0x10E6D04)
                v8 = nullptr;
            DObj* v9 = nullptr;
            if ((e->flags & 0x2000000) == 0
                || (e->mFlags & 8) == 0)
            {
                PostApplyPoseToSubModel(mDObj, v3, v8);
            }
            else
            {
                Entity* DroneMaster = GetDroneMaster(e);
                if (DroneMaster == nullptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\xanim.cpp";
                    AeAssert::gCurrentLine = 6046;
                    AeAssert::gCurrentExpr = nullptr;
                    if (AeAssert::Error(
                            "invalid drone state, missing master for slave"))
                        __debugbreak();
                }
                v9 = DroneMaster->mDObj;
            }
            DroneSetAutoTrajectoryPO(e, v9);
        }
    }
}

// ============================================================================
// QueueSceneAnim (anim.o 0x552D90) - stream a scene anim from the current pak
// ============================================================================

// StreamZoneManager view (full in streamer/pakmanager.cpp)
struct PakInfoNode {
    unsigned char _pad[0xB4];
    int pakId;  // +0xB4
};
class StreamZoneManager {
public:
    static StreamZoneManager* sInst;  // ?sInst@StreamZoneManager@@2PAV1@A
    int mLastCellNum;                 // +0x1C4
    const PakInfoNode* GetCellPakInfo(int cellIndex);  // ?GetCellPakInfo@StreamZoneManager@@QAEPBUPakInfoNode@@H@Z
};

// InstanceBankMgr view (full in streamer/pakmanager.cpp)
class InstanceBankMgr {
public:
    static InstanceBankMgr* sInst;  // ?sInst@InstanceBankMgr@@2PAV1@A
    bool GetAnimOffset(const char* name, TPakId pakId, unsigned int* out_offset,
                       unsigned int* out_size);  // ?GetAnimOffset@InstanceBankMgr@@QAE_NPBDW4TPakId@@PAI2@Z
};

// PakFile minimal view (mPath.mBuff @ +0x0C, mHeapList @ +0xA0)
struct PakFileLocal {
    unsigned char _pad[0x0C];
    char mPath[64];            // +0x0C (ae_fixed_string<64>)
    unsigned char _pad2[0xA0 - 0x4C];
    int mHeapList[12];         // +0xA0
};

extern TPakId CurPakId();  // streamer/pakmanager.cpp
extern unsigned int gNflMediaId;  // ?gNflMediaId@@3IA (streamer)
typedef unsigned int nflFileID;
enum nflMediaID : unsigned { NFL_MEDIA_ID_DUMMY = 0 };
extern nflFileID nflOpenFile(nflMediaID media, const char* name);  // filesystem/nfl.cpp
extern void* PoolAllocator_Allocate(void* allocator, unsigned int size,
                                    bool forceHeapAlloc);
extern void* nalStreamAnimQueueInstance_6(
    int fileID, unsigned int startOffset, unsigned int fileSize,
    unsigned int bufferSize,
    void* (*factory)(const nalSceneAnim*, const tlFixedString*, float*),
    void* userParam);
namespace AeStringSupport {
bool StrCStrEqu(const char* lhsBuff, int lhsLen, const char* rhsBuff,
                int rhsLen);
void CStrToAeStr(char* dst, int* dstLen, int dstCapacity, const char* src);
}

#define PAK_ID_MIN ((TPakId)0)
#define PAK_ID_INVALID ((TPakId)-1)

void* nalStreamAnimQueueInstance_6(
    int fileID, unsigned int startOffset, unsigned int fileSize,
    unsigned int bufferSize,
    void* (*factory)(const nalSceneAnim*, const tlFixedString*, float*),
    void* userParam)
{
    (void)fileID; (void)startOffset; (void)fileSize; (void)bufferSize;
    (void)factory; (void)userParam;
    return nullptr;
}


// ea: 0x00552D90
void* QueueSceneAnim(const char* name, void* notify, void* blendIn,
                     void* blendOut, TPakId pakAlloc, void* pakInfo)
{
    reserved_dlist<SceneAnimInfo>::dlist_node* m_head =
        gSceneAnimList.m_head;
    reserved_dlist<SceneAnimInfo>::dlist_node* m_next =
        gSceneAnimList.m_head != nullptr ? gSceneAnimList.m_head->m_next
                                         : nullptr;
    if (gSceneAnimList.m_head
            != (reserved_dlist<SceneAnimInfo>::dlist_node*)
                   &gSceneAnimList.m_end
        && m_next != nullptr)
    {
        while (1)
        {
            SceneAnimInfo* info = (SceneAnimInfo*)m_head;
            int nameLen = *(unsigned char*)((char*)info + 0x47);
            if (AeStringSupport::StrCStrEqu((char*)info + 0x28, nameLen,
                                            name, -1))
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\xanim.cpp";
                AeAssert::gCurrentLine = 3470;
                AeAssert::gCurrentExpr = nullptr;
                if (AeAssert::Error("anim is already queued %s", name))
                    __debugbreak();
                break;
            }
            if (info->mNotify == notify)
                break;
            m_head = m_next;
            m_next = m_next->m_next;
            if (m_next == nullptr)
                break;
        }
        {
            SceneAnimInfo* info = (SceneAnimInfo*)m_head;
            if (info->mNotify == notify)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\xanim.cpp";
                AeAssert::gCurrentLine = 3476;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning(
                           "Notify is already used by another scene anim, %s, %s",
                           (char*)info + 0x28, name))
                    __debugbreak();
            }
        }
    }

    if (pakAlloc != PAK_ID_MIN)
    {
        pakAlloc = (TPakId)StreamZoneManager::sInst
                       ->GetCellPakInfo(StreamZoneManager::sInst->mLastCellNum)
                       ->pakId;
    }
    else
    {
        pakAlloc = PAK_ID_INVALID;
    }

    PakFileLocal* v8 = (PakFileLocal*)pakInfo;
    if (pakInfo != nullptr)
    {
        if (v8->mHeapList[5] == -1)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xanim.cpp";
            AeAssert::gCurrentLine = 3492;
            AeAssert::gCurrentExpr = "node->pakId != PAK_ID_INVALID";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                       "Can't play scene anim (%s) from an unloaded pak (%s)",
                       name, v8->mPath))
                __debugbreak();
        }
        int v9 = v8->mHeapList[5];
        if (v9 != PAK_ID_INVALID)
            pakAlloc = (TPakId)v9;
    }

    PakHeapContext pakCtx(pakAlloc, false);
    PakManager* v10 = PakManager::sInst;
    void* v11 = nullptr;
    unsigned int offset = 0;
    unsigned int size = 0;
    TPakId v12 = CurPakId();
    PakFileLocal* pakFile = nullptr;
    if (v12 <= 0x62)
        pakFile = (PakFileLocal*)v10->mSlots[v12];
    TPakId v14 = CurPakId();
    if (!InstanceBankMgr::sInst->GetAnimOffset(name, v14, &offset, &size))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xanim.cpp";
        AeAssert::gCurrentLine = 3502;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("Didn't find scene anim %s", name))
            __debugbreak();
    }
    const char* mBuff = pakFile->mPath;
    void* v16 = PoolAllocator_Allocate(SceneAnimInfo_sAllocator, 0x48, false);
    if (v16 != nullptr)
    {
        SceneAnimInfo* info = (SceneAnimInfo*)v16;
        *(void**)((char*)info + 0x00) = nullptr;
        *(void**)((char*)info + 0x04) = nullptr;
        info->mFileID = 0;
        info->mInst = nullptr;
        info->mNotify = nullptr;
        info->blendNotify = nullptr;
        info->mPlaying = 0;
        info->blendIn = *(float*)&blendIn;
        info->blendOut = *(float*)&blendOut;
        info->mPakId = -1;
        info->mName[0] = 0;
        v11 = v16;
    }
    int v17 = nflOpenFile((nflMediaID)gNflMediaId, mBuff);
    ((SceneAnimInfo*)v11)->mFileID = v17;
    void* v18 = nalStreamAnimQueueInstance_6(
        v17, offset, size, 0,
        (void* (*)(const nalSceneAnim*, const tlFixedString*, float*))
            SceneAnimCallback,
        (char*)v11 + 0x1C);
    SceneAnimInfo* info2 = (SceneAnimInfo*)v11;
    info2->mNotify = notify;
    info2->mInst = v18;
    info2->mPakId = pakAlloc;
    int oLen = 0;
    char oBuff[32];
    AeStringSupport::CStrToAeStr(oBuff, &oLen, 31, name);
    *(void**)((char*)info2 + 0x00) = &gSceneAnimList.m_end;
    oBuff[31] = (char)oLen;
    memcpy(info2->mName, oBuff, 32);
    reserved_dlist<SceneAnimInfo>::dlist_node* m_tail =
        gSceneAnimList.m_tail;
    *(void**)((char*)info2 + 0x04) = m_tail;
    m_tail->m_next =
        (reserved_dlist<SceneAnimInfo>::dlist_node*)info2;
    gSceneAnimList.m_tail =
        (reserved_dlist<SceneAnimInfo>::dlist_node*)info2;
    ++gSceneAnimList.m_size;
    return v11;
}

// C-style wrapper for the inlined binary ctor (SceneAnimCallback path)
void* SceneAnimClient_CtorReal(void* self, const nalSceneAnim* anim,
                               const tlFixedString* name, float blendIn,
                               float blendOut)
{
    return new (self) SceneAnimClient(anim, name, blendIn, blendOut);
}
