// ============================================================================
// COD3 Game Types — Entity, EntityState, EntityShared
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "core/math_types.h"
#include "engine/broc_types.h"
#include "core/ae_fixed_string.h"
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// TPakId â€” pak archive id enum
// ============================================================================
// TPakId (IDA local enum).
enum TPakId {
    PAK_ID_INVALID = 0xFFFFFFFFu,
    PAK_ID_MIN = 0,
    PAK_ID_MAX = 0x63,
};
#define PAK_ID_INVALID ((TPakId)-1)
#define PAK_ID_MIN ((TPakId)0)

// Forward declarations
class Entity;
class DObj;
class EntityNotifySet;
class ScriptEventParams;
struct ScriptEventHandler;
struct biped_system;
// biped_phys_info (IDA local type, size 0x580 / 1408 bytes).
// The Bitmask member is represented by its 32-bit storage here; the owning
// physics translation unit provides the inline Bitmask methods.
class biped_phys_info {
public:
    Entity*             m_owner;                   // +0x000
    uint8_t             _pad04[0x0C];              // +0x004..+0x00F
    math::Mat43         m_cur_mat[10];             // +0x010
    math::Mat43         m_last_mat[10];            // +0x290
    math::Position3     m_cur_angles;              // +0x510
    math::Position3     m_cur_origin;              // +0x520
    math::Position3     m_last_angles;             // +0x530
    math::Position3     m_last_origin;             // +0x540
    int16_t             m_bone[10];                // +0x550
    float               m_delta_t;                 // +0x564
    struct biped_system* m_bp_sys;                 // +0x568
    int32_t             m_current_debug_joint;     // +0x56C
    uint32_t            m_render_flags;            // +0x570 (Bitmask<unsigned int>)
    biped_system* get_bp_sys();                    // game.o 0x0065C5D0
};
static_assert(sizeof(biped_phys_info) == 0x580, "biped_phys_info size mismatch");
static_assert(offsetof(biped_phys_info, m_bp_sys) == 0x568,
              "biped_phys_info::m_bp_sys offset mismatch");
class Destructible;
struct Client;
struct scr_vehicle_t;
struct turretInfo_t;
struct trRefEntity;
struct refEntity_t;
class EntityNotify;
class XAnimTree;
class XModel;
class PoolAllocator;
struct gitem_s;
struct Curve;
struct tagInfo_t;
struct animscripted_t {
    struct NalPositionOrientation {
        math::Quaternion quat;  // +0x00
        math::Position3  pos;   // +0x10
    };
    NalPositionOrientation origin;  // +0x00 (32 bytes)
    NalPositionOrientation offset;  // +0x20 (32 bytes)
    unsigned int    anim;       // +0x40 (scr_anim_s mHandle)
    void*           root;       // +0x44
    int             bStarted;   // +0x48
    int             mode;       // +0x4C
    float           fBlendOutTime;  // +0x50
    unsigned int    notifyName; // +0x54
    float           fHeightOfs; // +0x58
    float           fEndPitch;  // +0x5C
    float           fEndRoll;   // +0x60
    float           fOrientLerp;// +0x64
};
struct proximity_data_t;
struct actor_s;
struct sentient_s;
class DCGSet;
struct WorldSector;
class EntityHandleDb;

// ============================================================================
// Handle — generic object handle (4 bytes) — verified against IDA
// ============================================================================
class Handle {
public:
    unsigned int mVal;  // +0x00

    Handle() : mVal(0) {}              // implicit default preserved
    Handle(int v);                     // ??0Handle@@QAE@H@Z (g.o 0x4A9100)
    static Handle NullHandle();        // ?NullHandle@Handle@@SA?AV1@XZ (g.o 0x4A9120)
    bool IsUnassigned() const;         // ?IsUnassigned@Handle@@QBE_NXZ (g.o 0x4A9140)
    unsigned int GetVal() const;       // ?GetVal@Handle@@QBEIXZ (g.o 0x4A9150)
};
static_assert(sizeof(Handle) == 4, "Handle size mismatch");

bool operator==(Handle lhs, Handle rhs);  // ??8@YA_NVHandle@@0@Z (g.o 0x4A9160)
bool operator!=(Handle lhs, Handle rhs);  // ??9@YA_NVHandle@@0@Z (g.o 0x4A9180)

// Handle type — wraps a DbLinkedHandle
template <typename HandleDb, typename T>
class DbLinkedHandle {
public:
    Handle mHandle;  // +0x00 — wrapped handle

    DbLinkedHandle() { mHandle.mVal = 0; }
    DbLinkedHandle(int v) { mHandle.mVal = (unsigned int)v; }  // ??0?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAE@H@Z (g.o 0x4AC6C0)
    DbLinkedHandle(Handle h) { mHandle = h; }                  // ??0?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAE@VHandle@@@Z (g.o 0x4AC6E0)
    DbLinkedHandle& operator=(Handle rhs)                      // ??4?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAEAAV0@VHandle@@@Z (g.o 0x4AC700)
    {
        mHandle = rhs;
        return *this;
    }
    template <typename OtherHandleDb, typename OtherT>
    DbLinkedHandle& operator=(
        const DbLinkedHandle<OtherHandleDb, OtherT>& rhs)
    {
        mHandle.mVal = rhs.mHandle.mVal;
        return *this;
    }
    T* operator*() const;  // ??D?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QBEPAVEntity@@XZ (g.o 0x4B2670)
    T* operator->() const; // ??C?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QBEPAVEntity@@XZ (g.o 0x4B26B0)
    bool IsValid() const { return mHandle.mVal != 0; }
};
static_assert(sizeof(DbLinkedHandle<void, void>) == 4, "DbLinkedHandle size mismatch");

// ============================================================================
// tagInfo_t - entity tag attachment info (112 bytes) - verified against IDA
// ============================================================================
struct tagInfo_t {
    Entity*    parent;          // +0x00
    Entity*    next;            // +0x04
    HashString name;            // +0x08
    int16_t    index;           // +0x0C
    int16_t    useAngles;       // +0x0E
    float      axis[4][3];      // +0x10
    float      parentInvAxis[4][3];  // +0x40

    static class PoolAllocator* sAllocator;  // ?sAllocator@tagInfo_t@@2PAVPoolAllocator@@A
    static void SetAllocator(PoolAllocator* allocator);
    static void* operator new(size_t size, bool forceHeapAlloc,
                              const char* file, int line);  // ??2tagInfo_t@@SAPAXI_NPBDH@Z (g.o 0x4A7760)
    static void operator delete(void* ptr, bool forceHeapAlloc,
                                const char* file, int line);  // ??3tagInfo_t@@SAXPAX_NPBDH@Z (g.o 0x4A7780)
    static void operator delete(void* ptr);  // ??3tagInfo_t@@SAXPAX@Z (g.o 0x4A77A0)

    tagInfo_t();  // ??0tagInfo_t@@QAE@XZ (g.o 0x4AC460)
};
static_assert(sizeof(tagInfo_t) == 0x70, "tagInfo_t size mismatch");
static_assert(offsetof(tagInfo_t, name) == 0x08, "tagInfo_t::name offset mismatch");
static_assert(offsetof(tagInfo_t, index) == 0x0C, "tagInfo_t::index offset mismatch");

// Inplace vector
template <typename T>
class InplaceVector {
public:
    unsigned int mSize;  // +0x00
    T*           mList;  // +0x04

    T& operator[](unsigned int i) { return mList[i]; }
    const T& operator[](unsigned int i) const { return mList[i]; }
    unsigned int size() const { return mSize; }
};
static_assert(sizeof(InplaceVector<char>) == 8, "InplaceVector size mismatch");

// IVPointer — intrusive counted pointer (8 bytes)
// Layout: { T* mValue; TPakId mPakId; } — verified against IDA
template <typename T>
class IVPointer {
public:
    T*           mValue;   // +0x00 — actual pointer data
    unsigned int mPakId;   // +0x04 — pak id (TPakId)

    IVPointer() : mValue(nullptr), mPakId(PAK_ID_INVALID) {}  // ??0?$IVPointer@VPhysData@@@@QAE@XZ (g.o 0x4ACE80)
    void clear() { mValue = nullptr; mPakId = PAK_ID_INVALID; }  // ?clear@?$IVPointer@VXModel@@@@QAEXXZ (g.o 0x4ACE20)

    T* operator*() { ValidatePakId((TPakId)mPakId); return mValue; }  // ??D?$IVPointer@VXModel@@@@QAEPAVXModel@@XZ (g.o 0x4B1240)
    T* operator->() { ValidatePakId((TPakId)mPakId); return mValue; }  // ??C?$IVPointer@VXModel@@@@QAEPAVXModel@@XZ (g.o 0x4B1260)
    const T* operator->() const { ValidatePakId((TPakId)mPakId); return mValue; }  // ??C?$IVPointer@VXModel@@@@QBEPBVXModel@@XZ (g.o 0x4B18C0)
    operator bool() const { ValidatePakId((TPakId)mPakId); return mValue != nullptr; }  // ??B?$IVPointer@VXModel@@@@QBE_NXZ (g.o 0x4B27B0)
    bool operator!() const { ValidatePakId((TPakId)mPakId); return mValue == nullptr; }  // ??7?$IVPointer@VXModel@@@@QBE_NXZ (g.o 0x4B27D0)

private:
    T* Deref() const;  // ?Deref@?$IVPointer@VXModel@@@@ABEPAVXModel@@XZ (g.o 0x4AE4C0)
};
static_assert(sizeof(IVPointer<char>) == 8, "IVPointer size mismatch");
extern void ValidatePakId(TPakId pakId);  // core.o

// ============================================================================
// DObjModel — model slot for Entity::CreateDObj (20 bytes) - verified IDA
// ============================================================================
struct IVPointerRaw {
    void* mValue;  // +0x00
    int   mPakId;  // +0x04
};
class DObjModel {
public:
    IVPointerRaw  model;            // +0x00
    Broc::string      boneName;         // +0x08
    int               ignoreCollision;  // +0x0C
    XAnimTree*        animTree;         // +0x10
};
static_assert(sizeof(DObjModel) == 0x14, "DObjModel size mismatch");

// ============================================================================
// trType_t — trajectory type enumeration (from IDA, all values verified)
// ============================================================================
enum trType_t {
    TR_STATIONARY = 0,
    TR_INTERPOLATE = 1,
    TR_LINEAR = 2,
    TR_LINEAR_STOP = 3,
    TR_SINE = 4,
    TR_GRAVITY = 5,
    TR_GRAVITY_LOW = 6,
    TR_GRAVITY_FLOAT = 7,
    TR_GRAVITY_PAUSED = 8,
    TR_ACCELERATE = 9,
    TR_DECCELERATE = 10,
};

// ============================================================================
// trajectory_t — entity position/angle interpolation (40 bytes)
// Size: 0x28 (40 bytes) — verified against IDA
// Note: trBase/trDelta are float[3] (12 bytes), NOT math::Position3.
// ============================================================================
struct trajectory_t {
    trType_t trType;             // +0x00
    int32_t  trTime;             // +0x04
    int32_t  trDuration;         // +0x08
    float    trBase[3];          // +0x0C
    float    trDelta[3];         // +0x18
    int32_t  trGravityOverride;  // +0x24

    trajectory_t();  // ??0trajectory_t@@QAE@XZ (g.o 0x4A9B10)
};
static_assert(sizeof(trajectory_t) == 0x28, "trajectory_t size mismatch");
static_assert(offsetof(trajectory_t, trBase) == 0x0C, "trajectory_t::trBase offset mismatch");
static_assert(offsetof(trajectory_t, trDelta) == 0x18, "trajectory_t::trDelta offset mismatch");

// ============================================================================
// EntityState — network-replicated entity state (224 bytes)
// Size: 0xE0 (224 bytes) — verified against IDA
// ============================================================================
class EntityState {
public:
    EntityState();                            // ??0EntityState@@QAE@XZ (game.o 0x620480)
    void SetLerpAngles(const math::Position3& angles);  // ?SetLerpAngles@EntityState@@QAEXABVPosition3@math@@@Z (cg.o 0x6BBAC0)
    const math::Position3 GetLerpAngles() const;  // ?GetLerpAngles@EntityState@@QBE?BVPosition3@math@@XZ (g.o 0x4A5750)
    const math::Position3* GetLerpOrigin(math::Position3* result);  // shell.o 0x5AD2D0
    void SetLerpOrigin(const math::Position3& origin);  // ?SetLerpOrigin@EntityState@@QAEXABVPosition3@math@@@Z (g.o 0x4AF2A0)

    uint8_t  eType;                               // +0x00
    uint8_t  loopSound;                           // +0x01
    uint8_t  surfType;                            // +0x02
    uint8_t  weapon;                              // +0x03
    uint8_t  eventParm;                           // +0x04
    uint8_t  scale;                               // +0x05 (verified vs disasm G_SetSoundBlend)
    union {
        uint16_t index;       // +0x06 (truncated entity index)
        uint16_t brushmodel;  // item/brushmodel index
        uint16_t item;        // item table index
    };
    DbLinkedHandle<EntityHandleDb, Entity> mOtherEntity;    // +0x08
    DbLinkedHandle<EntityHandleDb, Entity> mGroundEntity;   // +0x0C
    int32_t  eFlags;                              // +0x10
    trajectory_t pos;                             // +0x14
    trajectory_t apos;                            // +0x3C
    math::Position3 lerpOrigin;                   // +0x70
    math::Position3 lerpAngles;                   // +0x80
    math::Position3 origin2;                      // +0x90
    math::Position3 angles2;                      // +0xA0
    int32_t  constantLight;                       // +0xB0
    int32_t  solid;                               // +0xB4
    int32_t  eventSequence;                       // +0xB8
    uint8_t  events[4];                           // +0xBC
    uint8_t  eventParms[4];                       // +0xC0
    float    leanf;                               // +0xC4
    int32_t  dmgFlags;                            // +0xC8
    int32_t  useCount;                            // +0xCC
    int32_t  eTeam;                               // +0xD0
};
static_assert(sizeof(EntityState) == 0xE0, "EntityState size mismatch");
static_assert(offsetof(EntityState, eFlags) == 0x10, "EntityState::eFlags offset mismatch");

// ============================================================================
// EntityShared — server-side shared entity data (336 bytes)
// Size: 0x150 (336 bytes) — verified against IDA
// ============================================================================
struct EntityShared {
    EntityShared();                           // ??0EntityShared@@QAE@XZ (game.o 0x620280)

    int32_t  linked;                              // +0x00
    int32_t  svFlags;                             // +0x04
    DbLinkedHandle<EntityHandleDb, Entity> mSingleClient;   // +0x08
    DCGSet*  bmodel;                              // +0x0C
    math::Position3 mins;                         // +0x10
    math::Position3 maxs;                         // +0x20
    math::Position3 absmin;                       // +0x30
    math::Position3 absmax;                       // +0x40
    math::Vector4 pos_cache;                      // +0x50
    int32_t  lastLeaf;                            // +0x60
    int32_t  contents;                            // +0x64
    // padding from 0x68 to 0x70 (alignment before Position3)
    uint8_t  _pad1[8];                            // +0x68
    math::Position3 currentOrigin;                // +0x70
    math::Position3 currentAngles;                // +0x80
    math::Mat43    currentMat;                    // +0x90
    DbLinkedHandle<EntityHandleDb, Entity> mOwner;          // +0xD0
    int32_t  eventType;                           // +0xD4
    int32_t  eventTime;                           // +0xD8
    WorldSector* worldSector;                     // +0xDC
    EntityShared* nextEntityInWorldSector;         // +0xE0
    int32_t  numClusters;                         // +0xE4
    int32_t  clusternums[16];                     // +0xE8
    int32_t  lastCluster;                         // +0x128
    int32_t  areanum;                             // +0x12C
    int32_t  areanum2;                            // +0x130
    int32_t  linkcontents;                        // +0x134
    float    linkmin[2];                          // +0x138
    float    linkmax[2];                          // +0x140
    // padding to 0x150
    uint8_t  _pad2[8];                            // +0x148
};
static_assert(sizeof(EntityShared) == 0x150, "EntityShared size mismatch");

// ============================================================================
// AttachModelInfo — attached model + tag (12 bytes)
// ============================================================================
struct AttachModelInfo {
    IVPointer<XModel> mModel;  // +0x00 (8 bytes)
    Broc::string      mTag;    // +0x08

    AttachModelInfo()  // ??0AttachModelInfo@@QAE@XZ (game.o inline 0x660280)
        : mTag((Broc::string::Block*)nullptr)
    {
        mModel.mValue = nullptr;
        mModel.mPakId = (unsigned int)PAK_ID_INVALID;
    }
};
static_assert(sizeof(AttachModelInfo) == 0x0C, "AttachModelInfo size mismatch");

struct EntityAnimationDebug;  // opaque — Entity::AnimationDebug

// Entity::AnimationDebug - 0x48 bytes (verified vs SetAnimDebug disasm)
struct EntityAnimationDebug {
    const char* lastAnimPlayed;      // +0x00
    const char* prev2lastAnimPlayed; // +0x04
    ae_fixed_string<64, unsigned char> lastAnimNamed;  // +0x08
};

// ============================================================================
// Entity — main game entity (1136 bytes)
// Size: 0x470 (1136 bytes) — verified against IDA (107 members)
// ============================================================================
class Entity {
public:
    Entity();             // ?Entity@@QAE@XZ (core.o)
    Entity(TPakId pakId); // ?Entity@@QAE@W4TPakId@@@Z (core.o)
    ~Entity();            // ??1Entity@@QAE@XZ (core.o)
    static void* operator new(size_t s);  // ??2Entity@@SAPAXI@Z (core.o)
    static void operator delete(void* ptr);  // ??3Entity@@SAXPAX@Z (game.o 0x620210)
    void SetAnimDebug(int lastAnim);  // ?SetAnimDebug@Entity@@QAEXH@Z (game.o 0x62AFB0)
    void SetScriptEventHandler(ScriptEventHandler* n);  // ?SetScriptEventHandler@Entity@@QAEXPAVScriptEventHandler@@@Z (streamer.o 0x6816F0)
    bool AddScriptEvent(HashString h, HashString callback);  // ?AddScriptEvent@Entity@@QAE_NVHashString@@V2@@Z (core.o)
    bool RemoveScriptEvent(HashString h, HashString callback); // ?RemoveScriptEvent@Entity@@QAE_NVHashString@@V2@@Z (scr.o 0x5E9B90)
    static void FreeAllDObjs(bool deleteDObjs);  // ?FreeAllDObjs@Entity@@SAX_N@Z
    void SetAlwaysRender(bool r);                // ?SetAlwaysRender@Entity@@QAEX_N@Z
    void Notify(const char* n);                  // ?Notify@Entity@@QAEXPBD@Z (scr.o 0x5EE310)
    void Notify(HashString h);                       // ?Notify@Entity@@QAEXVHashString@@@Z
    void Notify(HashString h, unsigned int* e);      // ?Notify@Entity@@QAEXVHashString@@PAI@Z
    void Notify(HashString h, const unsigned int& e);  // ?Notify@Entity@@QAEXVHashString@@ABI@Z
    void Notify(HashString h, const int& d, const Broc::entity& e,
                const int& mod, const int& hitloc,
                const float* hit_normal);  // ?Notify@Entity@@QAEXVHashString@@ABHABVentity@Broc@@11QBM@Z (g.o 0x4B3B40)
    void Notify(HashString h, const int& d, const Broc::entity& e,
                const int& mod, const int& hitloc);  // ?Notify@Entity@@QAEXVHashString@@ABHABVentity@Broc@@11@Z
    void Notify(HashString h, const Broc::entity& e);  // ?Notify@Entity@@QAEXVHashString@@ABVentity@Broc@@@Z (g.o 0x4B38A0)
    template <typename T>
    void Notify(HashString h, const T& d);  // ??$Notify@Vstring@Broc@@@Entity@@QAEXVHashString@@ABVstring@Broc@@@Z (g.o 0x4B3560)
    template <typename T1, typename T2>
    void Notify(HashString h, const T1& d1, const T2& d2);  // ??$Notify@Vstring@Broc@@V12@@Entity@@QAEXVHashString@@ABVstring@Broc@@1@Z (g.o 0x4B3660)
    void FreeDObj(bool deleteDObjs);              // ?FreeDObj@Entity@@QAEX_N@Z (game.o)
    void CreateDObj(DObjModel* models, unsigned short numModels,
                    XAnimTree* tree, unsigned short gameId);  // ?CreateDObj@Entity@@QAEXPAVDObjModel@@GPAVXAnimTree@@G@Z (game.o)
    biped_phys_info* get_bp_info();             // ?get_bp_info@Entity@@QAEPAVbiped_phys_info@@XZ (game.o 0x00602210)
    void set_bp_info(biped_phys_info* bpInfo);  // ?set_bp_info@Entity@@QAEXPAVbiped_phys_info@@@Z (game.o)
    bool IsEnemy(Entity* ent);                   // ?IsEnemy@Entity@@QAE_NPAV1@@Z (game.o)
    const math::Mat43 CalcRotTranMat43();         // ?CalcRotTranMat43@Entity@@QAE?BVMat43@math@@XZ (game.o)
    void ExecScriptHandler(HashString h, ScriptEventParams* params);  // ?ExecScriptHandler@Entity@@QAEXVHashString@@PAVScriptEventParams@@@Z (game.o)
    void CalcOriginAnglesFromMat();               // ?CalcOriginAnglesFromMat@Entity@@QAEXXZ (game.o)
    int  GetParentBoneIndex(int boneIndex);       // ?GetParentBoneIndex@Entity@@QAEHH@Z (game.o)
    const math::Mat43::Packed& GetBaseRelMat(int boneIndex);  // ?GetBaseRelMat@Entity@@QAEABUPacked@Mat43@math@@H@Z (game.o)
    int IsVisible() const;                        // ?IsVisible@Entity@@QBEHXZ (game.o)
    bool IsDoingPhysics();                        // ?IsDoingPhysics@Entity@@QAE_NXZ (game.o)
    bool IsInRagdoll();                           // ?IsInRagdoll@Entity@@QAE_NXZ (game.o)
    bool IsInSnapshot() const;                    // ?IsInSnapshot@Entity@@QBE_NXZ (game.o)
    bool IsCameraTweening() const;                // ?IsCameraTweening@Entity@@QBE_NXZ (game.o)
    bool IsLocalPlayer() const;                   // ?IsLocalPlayer@Entity@@QBE_NXZ (game.o)
    int GetPlayerIndex() const;                   // ?GetPlayerIndex@Entity@@QBEHXZ (game.o)
    trRefEntity& GetRenderEntity();               // ?GetRenderEntity@Entity@@QAEAAVtrRefEntity@@XZ (game.o)
    refEntity_t& GetRefEntity();                  // ?GetRefEntity@Entity@@QAEAAUrefEntity_t@@XZ (game.o 0x62AFA0)
    const DObj* GetViewModelDObj() const;         // ?GetViewModelDObj@Entity@@QBEPBVDObj@@XZ (game.o 0x6205F0)
    DObj* GetViewModelDObj();                     // ?GetViewModelDObj@Entity@@QAEPAVDObj@@XZ (game.o 0x620630)
    math::Mat43 CalcAbsMat(int boneIndex);        // ?CalcAbsMat@Entity@@QAE?AVMat43@math@@H@Z (game.o 0x620670)
    math::Mat43 GetRelMat(int boneIndex);         // ?GetRelMat@Entity@@QAE?AVMat43@math@@H@Z (game.o 0x6208E0)
    bool has_zone_collision() const;              // ?has_zone_collision@Entity@@QBE_NXZ (game.o 0x620BE0)
    void FootStep();                              // ?FootStep@Entity@@QAEXXZ (game.o 0x620C20)
    void AddNotify(EntityNotify* notify);          // ?AddNotify@Entity@@QAEXPAVEntityNotify@@@Z (game.o 0x62AD70)
    void SetInSnapshot();                         // ?SetInSnapshot@Entity@@QAEXXZ (game.o)
    EntityState  s;                               // +0x000 (224 bytes)
    static void SetLerpOrigin(EntityState* s, const math::Position3* origin);  // ?SetLerpOrigin@EntityState@@QAEXABVPosition3@math@@@Z
    EntityShared r;                               // +0x0E0 (336 bytes)
    int32_t  mPakId;                              // +0x230
    DbLinkedHandle<EntityHandleDb, Entity> mHandle;         // +0x234
    int16_t  mEntityArrayIndex;                   // +0x238
    // pad 2
    uint8_t  _pad23A[2];                          // +0x23A
    DObj*    mDObj;                               // +0x23C
    EntityNotifySet* mNotifySet;                  // +0x240
    EntityNotifySet* GetNotifySet();              // ?GetNotifySet@Entity@@QAEPAVEntityNotifySet@@XZ (g.o 0x4A6620)
    void SetNotifySet(EntityNotifySet* n);        // ?SetNotifySet@Entity@@QAEXPAVEntityNotifySet@@@Z (core.o 0x4B5610)
    const math::Position3& GetPosition();         // ?GetPosition@Entity@@QAEABVPosition3@math@@XZ (core.o 0x4B5630)
    DbLinkedHandle<EntityHandleDb, Entity> GetHandle() const;  // ?GetHandle@Entity@@QBE?AV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@XZ (g.o 0x4A67B0)
    void SetEntityArrayIndex(int v);              // ?SetEntityArrayIndex@Entity@@QAEXH@Z (g.o 0x4A67D0)
    int GetEntityArrayIndex() const;              // ?GetEntityArrayIndex@Entity@@QBEHXZ (g.o 0x4A67F0)
    void AssignHandle(Handle h);                  // ?AssignHandle@Entity@@QAEXVHandle@@@Z (g.o 0x4AF3A0)
    TPakId GetPakId() const;                      // ?GetPakId@Entity@@QBE?AW4TPakId@@XZ (g.o 0x4A6800)
    DObj* GetDObj();                              // ?GetDObj@Entity@@QAEPAVDObj@@XZ (g.o 0x4A6820)
    void SetDestructible(IVPointer<Destructible> d);  // ?SetDestructible@Entity@@QAEXV?$IVPointer@VDestructible@@@@@Z (g.o 0x4A68B0)
    IVPointer<Destructible> GetDestructible();    // ?GetDestructible@Entity@@QAE?AV?$IVPointer@VDestructible@@@@XZ (g.o 0x4A68E0)
    ScriptEventHandler* mScriptEventHandler;      // +0x244
    biped_phys_info* mBPInfo;                     // +0x248
    IVPointer<Destructible> mDestructible;        // +0x24C (8 bytes)
    Client*  client;                              // +0x254
    actor_s* actor;                               // +0x258
    sentient_s* sentient;                         // +0x25C
    scr_vehicle_t* scr_vehicle;                   // +0x260
    turretInfo_t* pTurretInfo;                    // +0x264
    trRefEntity* mRenderEntity;                   // +0x268
    XAnimTree* pAnimTree;                         // +0x26C
    IVPointer<XModel> mModel;                     // +0x270 (8 bytes)
    float    modelscale;                          // +0x278
    Broc::string   mClassName;                    // +0x27C
    HashString     mClassNameHash;                // +0x280
    Broc::string   targetname;                    // +0x284
    unsigned int   targetnameHash;                // +0x288
    Broc::string   mTarget;                       // +0x28C
    unsigned int   mTargetHash;                   // +0x290
    Broc::string   mGroupName;                    // +0x294
    unsigned int   mGroupNameHash;                // +0x298
    Broc::string   mScriptNoteworthy;             // +0x29C
    unsigned int   mScriptNoteworthyHash;         // +0x2A0
    Broc::string   mAnimName;                     // +0x2A4
    unsigned int   mAnimNameHash;                 // +0x2A8
    unsigned int   mHintString;                   // +0x2AC
    uint8_t  physicsObject;                       // +0x2B0
    uint8_t  noise_index;                         // +0x2B1
    uint8_t  ctf_has_flag;                        // +0x2B2
    uint8_t  active;                              // +0x2B3
    uint8_t  moverState;                          // +0x2B4
    uint8_t  attachIgnoreCollision;               // +0x2B5
    // pad 2
    uint8_t  _pad2B6[2];                          // +0x2B6
    int32_t  takedamage;                          // +0x2B8
    unsigned int invulnerability_timeout;         // +0x2BC
    int32_t  spawnflags;                          // +0x2C0
    int32_t  flags;                               // +0x2C4
    unsigned int mFlags;                          // +0x2C8 (Bitmask<unsigned int>)
    int32_t  clipmask;                            // +0x2CC
    int32_t  processedFrame;                      // +0x2D0
    DbLinkedHandle<EntityHandleDb, Entity> parentHandle;    // +0x2D4
    // pad 4 (align to 0x2E0)
    uint8_t  _pad2D8[4];                          // +0x2D8
    math::Position3 pos1;                         // +0x2E0
    math::Position3 pos2;                         // +0x2F0
    math::Position3 pos3;                         // +0x300
    int32_t  timestamp;                           // +0x310
    float    angle;                               // +0x314
    Broc::string team;                            // +0x318
    float    speed;                               // +0x31C
    float    closespeed;                          // +0x320
    // pad 12 (align to 0x330)
    uint8_t  _pad324[12];                         // +0x324
    math::Position3 movedir;                      // +0x330
    int32_t  gDuration;                           // +0x340
    int32_t  gDurationBack;                       // +0x344
    int32_t  nextthink;                           // +0x348
    int32_t  think;                               // +0x34C (fn_think_e)
    uint8_t  reached;                             // +0x350
    uint8_t  blocked;                             // +0x351
    uint8_t  touch;                               // +0x352
    uint8_t  use;                                 // +0x353
    uint8_t  pain;                                // +0x354
    uint8_t  die;                                 // +0x355
    uint8_t  entinfo;                             // +0x356
    uint8_t  controller;                          // +0x357
    int32_t  health;                              // +0x358
    int32_t  maxHealth;                           // +0x35C
    int32_t  damage;                              // +0x360
    int32_t  methodOfDeath;                       // +0x364
    int32_t  splashMethodOfDeath;                 // +0x368
    int32_t  count;                               // +0x36C
    Entity*  enemy;                               // +0x370
    Entity*  activator;                           // +0x374
    Entity*  teamchain;                           // +0x378
    Entity*  teammaster;                          // +0x37C
    float    wait;                                // +0x380
    float    random;                              // +0x384
    float    delay;                               // +0x388
    // pad 4 (align to 0x390)
    uint8_t  _pad38C[4];                          // +0x38C
    math::Position3 rotate;                       // +0x390
    math::Position3 TargetAngles;                 // +0x3A0
    const gitem_s* item;                          // +0x3B0
    int32_t  key;                                 // +0x3B4
    Broc::string mSpawnItem;                      // +0x3B8
    int16_t  cell_index;                          // +0x3BC
    int16_t  mPersistentIndex;                    // +0x3BE
    int32_t  count2;                              // +0x3C0
    int32_t  grenadeExplodeTime;                  // +0x3C4
    struct {
        HashString notifyHash;                    // +0x3C8
        HashString soundName;                     // +0x3CC
    } snd_wait;                                   // +0x3C8 (8 bytes)
    Curve*   curve;                               // +0x3D0
    tagInfo_t* tagInfo;                           // +0x3D4
    Entity*  tagChildren;                         // +0x3D8
    animscripted_t* scripted;                     // +0x3DC
    AttachModelInfo mAttachModels[7];             // +0x3E0 (84 bytes)
    uint16_t disconnectedLinks;                   // +0x434
    int32_t  iDisconnectTime;                     // +0x438
    int32_t  currentValid;                        // +0x43C
    int32_t  fireSndDelay;                        // +0x440
    int32_t  isFiring;                            // +0x444
    Handle   effectLoopingFire;                   // +0x448
    int32_t  previousEventSequence;               // +0x44C
    int32_t  previousPreEventSequence;            // +0x450
    struct EntityAnimationDebug* mAnimDebug;       // +0x454
    void*    mBrocExtendedEntity;                 // +0x458
    proximity_data_t* proximity_data;             // +0x45C
    int32_t  uniqueIndex;                         // +0x460
    // pad to 0x470
    uint8_t  _pad464[12];                         // +0x464
};
static_assert(sizeof(Entity) == 0x470, "Entity size mismatch");
static_assert(offsetof(Entity, s) == 0x000, "Entity::s offset mismatch");
static_assert(offsetof(Entity, r) == 0x0E0, "Entity::r offset mismatch");
static_assert(offsetof(Entity, mPakId) == 0x230, "Entity::mPakId offset mismatch");
static_assert(offsetof(Entity, mHandle) == 0x234, "Entity::mHandle offset mismatch");
static_assert(offsetof(Entity, mDObj) == 0x23C, "Entity::mDObj offset mismatch");
static_assert(offsetof(Entity, mDestructible) == 0x24C, "Entity::mDestructible offset mismatch");
static_assert(offsetof(Entity, client) == 0x254, "Entity::client offset mismatch");
static_assert(offsetof(Entity, actor) == 0x258, "Entity::actor offset mismatch");
static_assert(offsetof(Entity, sentient) == 0x25C, "Entity::sentient offset mismatch");
static_assert(offsetof(Entity, mModel) == 0x270, "Entity::mModel offset mismatch");
static_assert(offsetof(Entity, mClassName) == 0x27C, "Entity::mClassName offset mismatch");
static_assert(offsetof(Entity, mClassNameHash) == 0x280, "Entity::mClassNameHash offset mismatch");
static_assert(offsetof(Entity, pos1) == 0x2E0, "Entity::pos1 offset mismatch");
static_assert(offsetof(Entity, movedir) == 0x330, "Entity::movedir offset mismatch");
static_assert(offsetof(Entity, health) == 0x358, "Entity::health offset mismatch");
static_assert(offsetof(Entity, enemy) == 0x370, "Entity::enemy offset mismatch");
static_assert(offsetof(Entity, mAttachModels) == 0x3E0, "Entity::mAttachModels offset mismatch");
static_assert(offsetof(Entity, uniqueIndex) == 0x460, "Entity::uniqueIndex offset mismatch");
