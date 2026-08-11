// ============================================================================
// NAL — NGL Animation Library (125 funcs, 10 objects)
// ea: 0x854490-0x878100
// ============================================================================

#include <cstdint>
#include "core/math_types.h"
#include "core/tlFixedString.h"

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
