// ============================================================================
// NAL — NGL Animation Library (125 funcs, 10 objects)
// ea: 0x854490-0x878100
// ============================================================================

#include <cstdint>
#include "core/math_types.h"
#include "core/tlFixedString.h"
#include "engine/broc_types.h"

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
template<typename T> class nalAnimClass {};

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

    void GetTrajectoryUpdate(const nalGenericPose&, nalPositionOrientation&) const;
    void GetBoneMatrices(const nalGenericPose&, nalMatrix4x4*, int) const;
    void GetPose(nalGenericPose&, const nalMatrix4x4*, nalMatrix4x4*, const nalGenericPose&, int) const;
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
};

// ============================================================================
// nalSceneAnimInstance — runtime scene animation
// ============================================================================
class nalSceneAnimInstance {
public:
    ~nalSceneAnimInstance() {}
    void AddClientAnim(nalClientSceneAnim*, nalAnimClass<nalAnyPose>*) {}
    void Render() const {}
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

// TODO: PakHeapContext real port (streamer.o;
//       ??0PakHeapContext@@QAE@W4TPakId@@_N@Z @ 0x66B040). Minimal placeholder
//       so XAnimFreeInfo's heap-scoped release call compiles.
struct PakHeapContext {
    PakHeapContext(int pakId, bool once)
    {
        (void)pakId;
        (void)once;
    }
    ~PakHeapContext() {}
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
        PakHeapContext ctx(tree->mPakId, false);
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
