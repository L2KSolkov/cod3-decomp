// ============================================================================
// NAL — NGL Animation Library (125 funcs, 10 objects)
// ea: 0x854490-0x878100
// ============================================================================

#include <cstdint>
#include <new>
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

    void* GetData() { return m_data; }
};

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
