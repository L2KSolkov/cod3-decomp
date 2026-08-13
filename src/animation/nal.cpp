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

// Scene-anim list (anim.o) - dlist node at +0x00 (reserved_dlist intrusive)
struct SceneAnimInfo {
    unsigned char m_dlist_node[8];  // +0x00

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
};
reserved_dlist<SceneAnimInfo> gSceneAnimList;  // ?gSceneAnimList@@3V?$reserved_dlist@VSceneAnimInfo@@@@A (anim.o @ 0xDF2ACC)

// ea: 0x00543930
bool IsInSceneAnim()  // ?IsInSceneAnim@@YA_NXZ (anim.o)
{
    return gSceneAnimList.m_head != gSceneAnimList.m_end;
}

// AeAssert contract (definitions in core/ae_assert.cpp)
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
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

struct nalPositionOrientation {
    math::Position3 pos;
    math::Dir3       orient;
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
struct XModel {
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
struct nalComponentInfo;

template <typename T>
class nalGenericComponentHandle {
public:
    nalGenericComponentHandle()
    {
        Skeleton = nullptr;
    }

protected:
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
    unsigned char _pad[0x254];
    void* client;  // +0x254
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

class DObj;  // local forward decl (full class in cg_local.h)

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
                                    handle.Skeleton = this;
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
                                    handle.Skeleton = this;
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
                                    handle.Skeleton = this;
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
