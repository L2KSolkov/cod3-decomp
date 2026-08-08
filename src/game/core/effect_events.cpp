// ============================================================================
// effect_events.cpp - EffectEventSys / ActiveEffectSet + effect free functions
// (core.o EffectEvents.cpp family)
// ============================================================================

#include "game/core/core_systems.h"
#include "game/core/core_globals.h"
#include "core/PoolAllocator.h"
#include "aeps/apsEffect.h"

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Warning(const char* fmt, ...);
}

#define ASSERT_IDX(idx, cap, line)                                         \
    do {                                                                   \
        if ((idx) < 0 || (idx) >= (cap)) {                                 \
            AeAssert::gCurrentAuthor = AeAssert::COD3;                     \
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";             \
            AeAssert::gCurrentLine = (line);                               \
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";        \
            if (!AeAssert::IsIgnored()                                     \
                && AeAssert::Assert("out of bounds"))                      \
                __debugbreak();                                            \
        }                                                                  \
    } while (0)

// nsl voice enumeration (stub env; game reads srcId at +0x114)
typedef unsigned int nslSourceID;
enum nslSourceState { NSL_SOURCE_STATE_INVALID = 0 };
struct nslVoice;
extern unsigned int nslGetNumVoices();
extern nslVoice* nslGetVoice(unsigned int a);
extern nslSourceState nslGetSourceState(nslSourceID sid);

extern PoolAllocator* ActiveEffectSet_sAllocator;  // 0x00F00E84

namespace EffectEventSysStatics {
EffectEventSys* sInst = nullptr;  // 0x012F0380
}

PoolAllocator* ActiveEffectSet_sAllocator = nullptr;

// ============================================================================
// Entity lookup (HandleDb<Entity,1344,SizedHandle<12,20>>; elements at +0xA8)
// ============================================================================
class EntityHandleDb {
public:
    struct DbElement {
        Entity* mObject;  // +0x00
        int     mKey;     // +0x04
    };
    unsigned char _pad[0xA8];
    DbElement     mElements[0x540];
    static EntityHandleDb sInst;  // ?sInst@EntityHandleDb@@0V1@A
};
EntityHandleDb EntityHandleDb::sInst;

class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A
    Entity* GetPlayer(int idx);
    bool IsLocalPlayer(Entity* entity);
    int GetPlayerIndex(Entity* entity);
};
EntityManager* EntityManager::sInst = nullptr;

static const char defaultFileName[] = "";

// ============================================================================
// Camera shake support types (layout from IDA)
// ============================================================================
struct CameraShakeInstance {
    int   m_type;          // +0x00
    float m_magnitude;     // +0x04
    float m_magnitudeInc;  // +0x08
    float m_noiseFloats[8];// +0x0C
    float m_invDistance;   // +0x2C
    float m_time;          // +0x30
    float m_frequency;     // +0x34
    float m_movement;      // +0x38
    int   m_active;        // +0x3C
};
static_assert(sizeof(CameraShakeInstance) == 0x40,
              "CameraShakeInstance size mismatch");

struct CameraShake {
    float m_scale3D;          // +0x00
    float m_scaleCOD;         // +0x04
    int   m_scaleCOD_onlyADS; // +0x08
    CameraShakeInstance m_instanceData[5];  // +0x0C
};
static_assert(sizeof(CameraShake) == 0x14C, "CameraShake size mismatch");

extern CameraShakeInstance* CameraShake_StartCameraShake(
    CameraShake* self, int type, math::Position3* worldPos, float size,
    float timeOverride, float nextDelay);
extern void CameraShake_StopCameraShake(CameraShake* self,
                                        CameraShakeInstance* pShake);
extern void CameraShakeInstance_OverrideSettings(CameraShakeInstance* self,
                                                 float frequency,
                                                 float movement);
static Entity* AbstractEffectGetOwner(const AbstractEffect* effect);

extern math::Position3 GetTagFlashPos(Entity* cent);
extern CameraShake* g_cameraShake;  // 0x00F056E8
extern int dword_F6A290[4 * 0x322];  // per-client table, 0xC88-byte stride
extern void FX_ClearFX();
extern void Scr_Notify(Entity* ent, HashString hashValue,
                       unsigned int paramcount);
extern int g_debug_sync_queries;  // 0x00F00E78
extern TPakId CurPakId();
extern void* PakManager_sInst;
extern unsigned int AeHash(const char* str);
extern int FX_RegisterEffect(const char* name);
extern bool IsInSceneAnim();
extern int FX_GetBoneIndex(void* dobj, unsigned int bone_name_hash);
extern PoolAllocator* gCommonPoolAllocator;  // 0x00F00A18
extern float sNaN;  // 0x10F19D0
extern Broc::string gNULLString;  // 0x00F00EEC
extern unsigned int s_ImpactMessage;  // 0x00F00F30
extern void* CurveManager_sInst;  // 0x00F4F430
extern void CurveManager_PostEvent(void* self, unsigned int entityHandle,
                                   unsigned int hash, float value);
extern void AnglesToAxis(const math::Position3* angles,
                         const math::Position3* origin, math::Mat43* mat);
extern Entity* GetPlayer(int idx);
extern int currCl;
extern float clamp_0_to_1(float f);
extern void Entity_Notify(Entity* ent, unsigned int h);
extern unsigned int SoundDevice_PlaySound(
    unsigned int wave, unsigned int entHandle, bool important, int a5,
    const math::Position3* pos, const math::Position3* dir, float volume,
    float pitch, float minRange, float maxRange);
extern unsigned int SoundDevice_QueueSound(
    unsigned int wave, unsigned int entHandle, bool important, int a5,
    const math::Position3* pos, const math::Position3* dir, float volume,
    float pitch, float minRange, float maxRange);
extern float Sound_GetStartingVolume(void* sound);
struct SoundDeviceInst {
    float mVolScale;  // +0x00 (name-accessed)
};
extern SoundDeviceInst* SoundDevice_sInst;

CameraShake* g_cameraShake = nullptr;
int dword_F6A290[4 * 0x322];
int g_debug_sync_queries = 0;
PoolAllocator* gCommonPoolAllocator = nullptr;
float sNaN = 0.0f;
Broc::string gNULLString("");
unsigned int s_ImpactMessage = 0;

// snd_wait (Entity +0x3C8): two HashStrings
struct SndWait {
    HashString notifyHash;  // +0x00
    HashString soundName;   // +0x04
};

// ============================================================================
// SoundDevice surface (sound.o; opaque, only the fields the effects touch)
// ============================================================================
namespace SoundDevice {
struct Sound {
    int          mSource;        // +0x00 nslSourceID (-1 = invalid)
    unsigned int mWave;          // +0x04 nslWaveID
    HashString   mDialogNotify;  // +0x08
};
struct SoundHandleDb {
    struct DbElement {
        Sound*       mObject;  // +0x00
        unsigned int mKey;     // +0x04
    };
    DbElement mElements[512];
    static SoundHandleDb sInst;  // ?sInst@SoundHandleDb@SoundDevice@@0V12@A
};
SoundHandleDb SoundHandleDb::sInst;

extern void Sound_Stop(Sound* s);
extern void Sound_PlayQueued(Sound* s);
extern bool Sound_IsQueued(const Sound* s);
extern bool Sound_IsLooped(const Sound* s);
extern bool Sound_IsFinished(const Sound* s);
extern const char* Sound_GetSourceName(const Sound* s);
extern float Sound_GetVolume(const Sound* s);
extern float Sound_GetLength(const Sound* s);
extern void Sound_SetPoPtr(Sound* s, const math::Mat43* po);
extern void Sound_SetPitch(Sound* s, float pitch);
extern void Sound_SetVolume(Sound* s, float vol);
extern float nslGetWaveParam(unsigned int wave, int b, float c);
extern bool subtitle_manager_play_subtitle(const char* tag,
                                           const char* prefix);

static Sound* SoundFromHandle(unsigned int handleVal)
{
    unsigned int v = handleVal & 0xFFF;
    if (v < 0x200
        && handleVal >> 12 == SoundHandleDb::sInst.mElements[v].mKey)
        return SoundHandleDb::sInst.mElements[v].mObject;
    return nullptr;
}
}  // namespace SoundDevice

// Effect context / query type constants (placeholder values from disasm)
enum {
    EEffectContextInvalid = 0,
    kEffectContextFootstep = 1,
    kEffectContextGearRattle = 2,
    kEffectContextLanding = 3,
    kEffectContextScriptCall = 4,
    kEffectContextWeapon = 5,
    kEffectContextBulletHit = 6,
    kEffectContextGrenadeBounce = 7,
    kEffectContextProjExplode = 8,
    kEffectContextVehicle = 9,
    kEffectContextLight = 10,
    kEffectContextEIMelee = 11,
    EEffectContextCount = 12,
};

static unsigned int holdrand = 1;
static unsigned int RandNext()
{
    holdrand = holdrand * 214013 + 2531011;
    return (holdrand >> 16) & 0x7FFF;
}

extern SoundOptions gSoundOptions;  // 0x00F00EF0

struct Client {
    unsigned char _pad[0x7F0];
    int bFrozen;  // +0x7F0
    unsigned char _pad2[0xAE4 - 0x7F4];
    int mServerClientIndex;  // +0xAE4
};

struct rb_vehicle {
    unsigned int m_flags;  // +0x00
};
struct VehicleSeat {
    DbLinkedHandle<void, void> occupant;  // +0x00
};
struct scr_vehicle_t {
    rb_vehicle* mRBVeh;        // +0x00
    VehicleSeat seats[11];     // +0x04
};

static void BitSetAdd(BitSet<49>& bs, int v)
{
    bs.mBits[v >> 3] |= (unsigned char)(1u << (v & 7));
}
static void BitSetRmv(BitSet<49>& bs, int v)
{
    bs.mBits[v >> 3] &= (unsigned char)~(1u << (v & 7));
}

static void SetScriptIdField(EffectEventSys::CachedQuery& cq,
                             const char* scriptId)
{
    strncpy(cq.mSCRIPT_ID, scriptId, 127);
    cq.mSCRIPT_ID[127] = 0;
    BitSetAdd(cq.mSpecifiedFields, 7);
    BitSetRmv(cq.mWeakFields, 7);
}

static void SetWeaponIdField(EffectEventSys::CachedQuery& cq,
                             const char* weaponType)
{
    strncpy(cq.mWEAPON_ID, weaponType, 127);
    cq.mWEAPON_ID[127] = 0;
    BitSetAdd(cq.mSpecifiedFields, 8);
    BitSetRmv(cq.mWeakFields, 8);
}

static void SetVehicleIdField(EffectEventSys::CachedQuery& cq,
                              const char* vehicleType)
{
    strncpy(cq.mVEHICLE_ID, vehicleType, 127);
    cq.mVEHICLE_ID[127] = 0;
    BitSetAdd(cq.mSpecifiedFields, 9);
    BitSetRmv(cq.mWeakFields, 9);
}

static void BeginScriptCallQuery(EffectEventSys* sys, const Entity* ent,
                                 int context, const char* scriptId,
                                 TPakId pakid)
{
    sys->BeginEffectQuery(ent, pakid);
    sys->mCurrentQuery->mType = kEffectContextScriptCall;
    EffectEventSys::CachedQuery& cq = sys->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = context;
    if (scriptId != nullptr)
        SetScriptIdField(cq, scriptId);
}

// ea: 0x004D1D80
Handle PostEffectEventFootstep(const Entity* ent, int stanceType,
                               const CollisionDesc* col_desc)
{
    Handle result;
    if (IsInSceneAnim() || gSoundOptions.mFxDontPlayFootSteps != 0
        || (ent->client != nullptr && ent->client->bFrozen != 0))
    {
        result.mVal = 0;
        return result;
    }
    if (col_desc->material > 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 105;
        AeAssert::gCurrentExpr =
            "( mat_type >= kCollisionMaterialMin && mat_type <= "
            "kCollisionMaterialMax )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    EffectEventSys* v6 = EffectEventSysStatics::sInst;
    v6->BeginEffectQuery(ent, (TPakId)-1);
    v6->mCurrentQuery->mType = kEffectContextFootstep;
    EffectEventSys::CachedQuery& cq = v6->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 0;
    BitSetAdd(cq.mSpecifiedFields, 2);
    BitSetRmv(cq.mWeakFields, 2);
    v6->SetScriptId(Broc::string(gNULLString));
    v6->CollisionInfo(col_desc, true);
    v6->mCurrentQuery->mQueryType = 1;
    result = v6->ExecEffectQuery();
    return result;
}

// ea: 0x004D1EC0
Handle PostEffectEventGearRattle(const Entity* ent, int stanceType,
                                 const CollisionDesc* col_desc)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayGearRattle != 0 || IsInSceneAnim()
        || (ent->client != nullptr && ent->client->bFrozen != 0))
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v6 = EffectEventSysStatics::sInst;
    v6->BeginEffectQuery(ent, (TPakId)-1);
    v6->mCurrentQuery->mType = kEffectContextGearRattle;
    EffectEventSys::CachedQuery& cq = v6->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 1;
    BitSetAdd(cq.mSpecifiedFields, 2);
    BitSetRmv(cq.mWeakFields, 2);
    v6->SetScriptId(Broc::string(gNULLString));
    v6->mCurrentQuery->mFlags.mVal |= 4u;
    memcpy(&v6->mCurrentQuery->mCollisionInfo, col_desc,
           sizeof(v6->mCurrentQuery->mCollisionInfo));
    v6->mCurrentQuery->mQueryType = 2;
    result = v6->ExecEffectQuery();
    return result;
}

// ea: 0x004D1FD0
Handle PostEffectEventLanding(const Entity* ent,
                              const CollisionDesc* col_desc)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayLanding != 0)
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v4 = EffectEventSysStatics::sInst;
    v4->BeginEffectQuery(ent, (TPakId)-1);
    v4->mCurrentQuery->mType = kEffectContextLanding;
    EffectEventSys::CachedQuery& cq = v4->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 2;
    v4->SetScriptId(Broc::string(gNULLString));
    v4->CollisionInfo(col_desc, true);
    v4->mCurrentQuery->mQueryType = 3;
    result = v4->ExecEffectQuery();
    return result;
}

// ea: 0x004D2080
Handle PostEffectEventScriptCall(const Entity* ent, const char* scriptId,
                                 const Broc::vector* pos,
                                 const Broc::vector* facing, bool queue,
                                 TPakId pakid, bool important)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayScriptCall != 0)
    {
        result.mVal = 0;
        return result;
    }
    if (scriptId == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 177;
        AeAssert::gCurrentExpr = "scriptId";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Empty script id passed in"))
        {
            __debugbreak();
            result.mVal = 0;
            return result;
        }
        result.mVal = 0;
        return result;
    }
    if (pos->x != sNaN && pos->y != sNaN && pos->z != sNaN
        && facing->x != sNaN && facing->y != sNaN && facing->z != sNaN)
    {
        math::Mat43* v24 = (math::Mat43*)gCommonPoolAllocator->Allocate(
            0x40, false);
        math::Position3 v25;
        v25.v.m128_f32[0] = facing->x;
        v25.v.m128_f32[1] = facing->y;
        v25.v.m128_f32[2] = facing->z;
        v25.v.m128_f32[3] = 0.0f;
        math::Position3 angles;
        angles.v.m128_f32[0] = pos->x;
        angles.v.m128_f32[1] = pos->y;
        angles.v.m128_f32[2] = pos->z;
        angles.v.m128_f32[3] = 0.0f;
        AnglesToAxis(&angles, &v25, v24);
        EffectEventSys* v16 = EffectEventSysStatics::sInst;
        BeginScriptCallQuery(v16, ent, 3, scriptId, pakid);
        v16->SetScriptId(Broc::string(scriptId));
        v16->mCurrentQuery->mMatrix = v24;
        v16->mCurrentQuery->mQueryType = 8;
        v16->SetQueryImportance(important);
        result = v16->ExecEffectQuery();
        return result;
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
    AeAssert::gCurrentLine = 183;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning(
            "Undefined position or angles passed to PostEffectEventScriptCall"))
        __debugbreak();
    result.mVal = 0;
    return result;
}

// ea: 0x004D2340
Handle PostEffectEventScriptCall(const Entity* ent, const char* scriptId,
                                 bool queue, TPakId pakid, bool important)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayScriptCall != 0)
    {
        result.mVal = 0;
        return result;
    }
    if (scriptId == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 227;
        AeAssert::gCurrentExpr = "scriptId";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Empty script id passed in"))
            __debugbreak();
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v7 = EffectEventSysStatics::sInst;
    BeginScriptCallQuery(v7, ent, 3, scriptId, pakid);
    if (queue)
        v7->mCurrentQuery->mFlags.mVal |= 2u;
    v7->SetScriptId(Broc::string(scriptId));
    v7->mCurrentQuery->mQueryType = 8;
    v7->SetQueryImportance(important);
    result = v7->ExecEffectQuery();
    return result;
}

// ea: 0x004D24C0
Handle PostEffectEventQueueDialog(const Entity* ent, const char* scriptId,
                                  int notifyHash)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayScriptCall != 0)
    {
        result.mVal = 0;
        return result;
    }
    if (scriptId == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 268;
        AeAssert::gCurrentExpr = "scriptId";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Empty script id passed in"))
            __debugbreak();
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v5 = EffectEventSysStatics::sInst;
    BeginScriptCallQuery(v5, ent, 3, scriptId, (TPakId)-1);
    v5->mCurrentQuery->mFlags.mVal |= 2u;
    v5->SetScriptId(Broc::string(scriptId));
    v5->mCurrentQuery->mDialogNotify = notifyHash;
    v5->mCurrentQuery->mQueryType = 7;
    v5->mCurrentQuery->mFlags.mVal |= 8u;
    result = v5->ExecEffectQuery();
    return result;
}

// ea: 0x004D2630
Handle PostEffectEventScriptCall_Dir(const Entity* ent, const char* scriptId,
                                     const float* dir, bool queue)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayScriptCall_Dir != 0)
    {
        result.mVal = 0;
        return result;
    }
    if (scriptId == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 295;
        AeAssert::gCurrentExpr = "scriptId";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Empty script id passed in"))
            __debugbreak();
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v6 = EffectEventSysStatics::sInst;
    BeginScriptCallQuery(v6, ent, 3, scriptId, (TPakId)-1);
    if (queue)
        v6->mCurrentQuery->mFlags.mVal |= 2u;
    v6->DirectionInfo(dir);
    v6->SetScriptId(Broc::string(scriptId));
    v6->mCurrentQuery->mQueryType = 8;
    result = v6->ExecEffectQuery();
    return result;
}

// ea: 0x004D2780
Handle PostEffectEventWeaponFire1st(const Entity* ent, const char* weaponType,
                                    int weaponAction, int16_t cacheSound,
                                    int barrel)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayWeapon != 0)
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v7 = EffectEventSysStatics::sInst;
    v7->BeginEffectQuery(ent, (TPakId)-1);
    v7->mCurrentQuery->mType = kEffectContextWeapon;
    EffectEventSys::CachedQuery& cq = v7->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 4;
    SetWeaponIdField(cq, weaponType);
    BitSetAdd(cq.mSpecifiedFields, 10);
    BitSetRmv(cq.mWeakFields, 10);
    cq.mACTION = weaponAction;
    BitSetAdd(cq.mSpecifiedFields, 12);
    BitSetAdd(cq.mWeakFields, 12);
    cq.mBARREL = barrel;
    v7->SetScriptId(Broc::string(gNULLString));
    v7->mCurrentQuery->mCacheSoundType =
        ent->scr_vehicle != nullptr ? -1 : cacheSound;
    v7->mCurrentQuery->mQueryType = 4;
    result = v7->ExecEffectQuery();
    return result;
}

// ea: 0x004D28F0
Handle PostEffectEventWeaponFire3rd(const Entity* ent, const char* weaponType,
                                    int weaponAction, int16_t cacheSound)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayWeapon != 0)
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v6 = EffectEventSysStatics::sInst;
    v6->BeginEffectQuery(ent, (TPakId)-1);
    v6->mCurrentQuery->mType = kEffectContextWeapon;
    EffectEventSys::CachedQuery& cq = v6->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 4;
    SetWeaponIdField(cq, weaponType);
    BitSetAdd(cq.mSpecifiedFields, 10);
    BitSetRmv(cq.mWeakFields, 10);
    cq.mACTION = weaponAction;
    v6->SetScriptId(Broc::string(gNULLString));
    v6->mCurrentQuery->mCacheSoundType =
        ent->scr_vehicle != nullptr ? -1 : cacheSound;
    v6->mCurrentQuery->mQueryType = 5;
    result = v6->ExecEffectQuery();
    return result;
}

// ea: 0x004D2A30
Handle PostEffectEventWeaponReload(const Entity* ent, const char* weaponType,
                                   int weaponAction, bool queue)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayWeapon != 0)
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v6 = EffectEventSysStatics::sInst;
    v6->BeginEffectQuery(ent, (TPakId)-1);
    v6->mCurrentQuery->mType = kEffectContextWeapon;
    EffectEventSys::CachedQuery& cq = v6->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 4;
    SetWeaponIdField(cq, weaponType);
    BitSetAdd(cq.mSpecifiedFields, 10);
    BitSetRmv(cq.mWeakFields, 10);
    cq.mACTION = weaponAction;
    v6->SetScriptId(Broc::string(gNULLString));
    if (queue)
        v6->mCurrentQuery->mFlags.mVal |= 2u;
    v6->mCurrentQuery->mCacheSoundType = -1;
    v6->mCurrentQuery->mQueryType = 6;
    result = v6->ExecEffectQuery();
    return result;
}

// ea: 0x004D2B70
Handle PostEffectEventWeapon(const Entity* ent, const char* weaponType,
                             int weaponAction)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayWeapon != 0)
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v5 = EffectEventSysStatics::sInst;
    v5->BeginEffectQuery(ent, (TPakId)-1);
    v5->mCurrentQuery->mType = kEffectContextWeapon;
    EffectEventSys::CachedQuery& cq = v5->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 4;
    SetWeaponIdField(cq, weaponType);
    BitSetAdd(cq.mSpecifiedFields, 10);
    BitSetRmv(cq.mWeakFields, 10);
    cq.mACTION = weaponAction;
    v5->SetScriptId(Broc::string(gNULLString));
    v5->mCurrentQuery->mCacheSoundType = -1;
    v5->mCurrentQuery->mQueryType = -1;
    result = v5->ExecEffectQuery();
    return result;
}

// ea: 0x004D2C90
Handle PostEffectEventBulletHit(const Entity* ent, int weaponClass,
                                CollisionDesc* col_desc)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayBulletHit != 0)
    {
        result.mVal = 0;
        return result;
    }
    if (col_desc->material > 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 406;
        AeAssert::gCurrentExpr =
            "( mat_type >= kCollisionMaterialMin && mat_type <= "
            "kCollisionMaterialMax )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    if ((ent->flags & 0x2000000) != 0)
        col_desc->material = 7;  // kCollisionMaterialFLESH
    EffectEventSys* v5 = EffectEventSysStatics::sInst;
    v5->BeginEffectQuery(ent, (TPakId)-1);
    v5->mCurrentQuery->mType = kEffectContextBulletHit;
    EffectEventSys::CachedQuery& cq = v5->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 5;
    BitSetAdd(cq.mSpecifiedFields, 11);
    BitSetRmv(cq.mWeakFields, 11);
    cq.mWEAPON_CLASS = weaponClass;
    v5->SetScriptId(Broc::string(gNULLString));
    v5->mCurrentQuery->mQueryType = 0;
    v5->CollisionInfo(col_desc, true);
    result = v5->ExecEffectQuery();
    return result;
}

// ea: 0x004D2DD0
Handle PostEffectEventGrenadeBounce(const Entity* ent, const char* weaponType,
                                    const CollisionDesc* col_desc)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayGrenadeBounce != 0)
    {
        result.mVal = 0;
        return result;
    }
    if (col_desc->material > 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 434;
        AeAssert::gCurrentExpr =
            "( mat_type >= kCollisionMaterialMin && mat_type <= "
            "kCollisionMaterialMax )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    EffectEventSys* v5 = EffectEventSysStatics::sInst;
    v5->BeginEffectQuery(ent, (TPakId)-1);
    v5->mCurrentQuery->mType = kEffectContextGrenadeBounce;
    EffectEventSys::CachedQuery& cq = v5->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 6;
    SetWeaponIdField(cq, weaponType);
    v5->SetScriptId(Broc::string(gNULLString));
    v5->CollisionInfo(col_desc, true);
    v5->mCurrentQuery->mQueryType = -1;
    result = v5->ExecEffectQuery();
    return result;
}

// ea: 0x004D2F20
Handle PostEffectEventProjExplode(const Entity* ent, const char* weaponType,
                                  const CollisionDesc* col_desc)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayProjExplode != 0)
    {
        result.mVal = 0;
        return result;
    }
    if (col_desc->material > 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 459;
        AeAssert::gCurrentExpr =
            "( mat_type >= kCollisionMaterialMin && mat_type <= "
            "kCollisionMaterialMax )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    EffectEventSys* v5 = EffectEventSysStatics::sInst;
    v5->BeginEffectQuery(ent, (TPakId)-1);
    v5->mCurrentQuery->mType = kEffectContextProjExplode;
    EffectEventSys::CachedQuery& cq = v5->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 7;
    SetWeaponIdField(cq, weaponType);
    v5->SetScriptId(Broc::string(gNULLString));
    v5->CollisionInfo(col_desc, true);
    v5->mCurrentQuery->mQueryType = -1;
    result = v5->ExecEffectQuery();
    return result;
}

// ea: 0x004D3070
Handle PostEffectEventVehicle(const Entity* ent, const char* vehicleType,
                              int action)
{
    Handle result;
    int mFxDontPlayTurret;
    if (action == 45 || action == 46)
        mFxDontPlayTurret = gSoundOptions.mFxDontPlayTurret;
    else
        mFxDontPlayTurret = gSoundOptions.mFxDontPlayVehicle;
    if (mFxDontPlayTurret != 0)
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v6 = EffectEventSysStatics::sInst;
    v6->BeginEffectQuery(ent, (TPakId)-1);
    v6->mCurrentQuery->mType = kEffectContextVehicle;
    EffectEventSys::CachedQuery& cq = v6->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 8;
    SetVehicleIdField(cq, vehicleType);
    BitSetAdd(cq.mSpecifiedFields, 10);
    BitSetRmv(cq.mWeakFields, 10);
    cq.mACTION = action;
    v6->SetScriptId(Broc::string(gNULLString));
    v6->mCurrentQuery->mQueryType = -1;
    result = v6->ExecEffectQuery();
    return result;
}

// ea: 0x004D3190
Handle PostEffectEventVehicleExplosion(const Entity* ent,
                                       const char* vehicleType, int action,
                                       const CollisionDesc* col_desc)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayVehicle != 0)
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v6 = EffectEventSysStatics::sInst;
    v6->BeginEffectQuery(ent, (TPakId)-1);
    v6->mCurrentQuery->mType = kEffectContextVehicle;
    EffectEventSys::CachedQuery& cq = v6->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 8;
    SetVehicleIdField(cq, vehicleType);
    BitSetAdd(cq.mSpecifiedFields, 10);
    BitSetRmv(cq.mWeakFields, 10);
    cq.mACTION = action;
    v6->SetScriptId(Broc::string(gNULLString));
    v6->mCurrentQuery->mFlags.mVal |= 4u;
    memcpy(&v6->mCurrentQuery->mCollisionInfo, col_desc,
           sizeof(v6->mCurrentQuery->mCollisionInfo));
    v6->mCurrentQuery->mQueryType = -1;
    result = v6->ExecEffectQuery();
    return result;
}

// ea: 0x004D32D0
Handle PostEffectEventVehicleWheel(const Entity* ent, const char* vehicleType,
                                   int action, int mat_type,
                                   unsigned int wheel_tag_hash)
{
    Handle result;
    int boneID = -1;
    if (gSoundOptions.mFxDontPlayVehicleWheel != 0
        || (wheel_tag_hash != 0
            && (boneID = FX_GetBoneIndex(ent->mDObj, wheel_tag_hash)) == -1))
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v7 = EffectEventSysStatics::sInst;
    v7->BeginEffectQuery(ent, (TPakId)-1);
    v7->mCurrentQuery->mType = kEffectContextVehicle;
    EffectEventSys::CachedQuery& cq = v7->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 8;
    SetVehicleIdField(cq, vehicleType);
    BitSetAdd(cq.mSpecifiedFields, 10);
    BitSetRmv(cq.mWeakFields, 10);
    cq.mACTION = action;
    if (mat_type > 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 545;
        AeAssert::gCurrentExpr =
            "( mat_type >= kCollisionMaterialMin && mat_type <= "
            "kCollisionMaterialMax )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    BitSetAdd(cq.mSpecifiedFields, 3);
    BitSetAdd(cq.mWeakFields, 3);
    cq.mMATERIAL = mat_type;
    v7->SetScriptId(Broc::string(gNULLString));
    v7->mCurrentQuery->mBoneIndex = boneID;
    v7->mCurrentQuery->mQueryType = -1;
    result = v7->ExecEffectQuery();
    return result;
}

// ea: 0x004D3490
Handle PostEffectEventPointLightFlash(const Entity* ent,
                                      const char* weaponType, int weaponAction)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayLightFlash != 0)
    {
        result.mVal = 0;
        return result;
    }
    EffectEventSys* v5 = EffectEventSysStatics::sInst;
    v5->BeginEffectQuery(ent, (TPakId)-1);
    v5->mCurrentQuery->mType = kEffectContextLight;
    EffectEventSys::CachedQuery& cq = v5->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 9;
    BitSetAdd(cq.mSpecifiedFields, 10);
    BitSetRmv(cq.mWeakFields, 10);
    cq.mACTION = weaponAction;
    v5->SetScriptId(Broc::string(gNULLString));
    v5->mCurrentQuery->mQueryType = -1;
    result = v5->ExecEffectQuery();
    return result;
}

// ea: 0x004D3560
Handle PostEffectEventEIMelee(const Entity* ent, int action)
{
    Handle result;
    EffectEventSys* v3 = EffectEventSysStatics::sInst;
    v3->BeginEffectQuery(ent, (TPakId)-1);
    v3->mCurrentQuery->mType = kEffectContextEIMelee;
    EffectEventSys::CachedQuery& cq = v3->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 10;
    BitSetAdd(cq.mSpecifiedFields, 10);
    BitSetRmv(cq.mWeakFields, 10);
    cq.mACTION = action;
    v3->SetScriptId(Broc::string(gNULLString));
    v3->mCurrentQuery->mCacheSoundType = -1;
    v3->mCurrentQuery->mQueryType = -1;
    result = v3->ExecEffectQuery();
    return result;
}

// ea: 0x004D3620
Handle PostEffectEventPhysicsImpact(const Entity* ent, int myColMat,
                                    const CollisionDesc* col_desc,
                                    float intensity)
{
    Handle result;
    if (gSoundOptions.mFxDontPlayBulletHit != 0)
    {
        result.mVal = 0;
        return result;
    }
    if (col_desc->material > 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
        AeAssert::gCurrentLine = 595;
        AeAssert::gCurrentExpr =
            "( mat_type >= kCollisionMaterialMin && mat_type <= "
            "kCollisionMaterialMax )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    EffectEventSys* v6 = EffectEventSysStatics::sInst;
    v6->BeginEffectQuery(ent, (TPakId)-1);
    v6->mCurrentQuery->mType = EEffectContextCount;
    EffectEventSys::CachedQuery& cq = v6->mCurrentQuery->mCachedQuery;
    BitSetAdd(cq.mSpecifiedFields, 0);
    BitSetRmv(cq.mWeakFields, 0);
    cq.mCONTEXT = 11;
    BitSetAdd(cq.mSpecifiedFields, 4);
    BitSetRmv(cq.mWeakFields, 4);
    cq.mMYMATERIAL = myColMat;
    v6->SetScriptId(Broc::string(gNULLString));
    v6->mCurrentQuery->mQueryType = -1;
    v6->CollisionInfo(col_desc, true);
    result = v6->ExecEffectQuery();
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (scr_vehicle != nullptr)
    {
        if (scr_vehicle->mRBVeh == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEvent.cpp";
            AeAssert::gCurrentLine = 608;
            AeAssert::gCurrentExpr = "ent->scr_vehicle->mRBVeh";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Bad vehicle in "
                                    "PostEffectEventPhysicsImpact"))
                __debugbreak();
        }
        if ((scr_vehicle->mRBVeh->m_flags & 0x80) != 0)
        {
            for (int i = 0; i < 11; ++i)
            {
                unsigned int v15 =
                    scr_vehicle->seats[i].occupant.mHandle.mVal & 0xFFF;
                if (v15 < 0x540
                    && scr_vehicle->seats[i].occupant.mHandle.mVal >> 12
                           == EntityHandleDb::sInst.mElements[v15].mKey)
                {
                    Entity* mObject =
                        EntityHandleDb::sInst.mElements[v15].mObject;
                    if (mObject != nullptr
                        && EntityManager::sInst->IsLocalPlayer(mObject))
                    {
                        int PlayerIndex =
                            EntityManager::sInst->GetPlayerIndex(mObject);
                        if (RumbleManager::Inst(PlayerIndex) != nullptr)
                        {
                            RumbleEffect rumbleEffect;
                            memset(&rumbleEffect, 0, sizeof(rumbleEffect));
                            rumbleEffect.mRumbleDataArray[0].enabled = true;
                            rumbleEffect.mRumbleDataArray[1].enabled = true;
                            rumbleEffect.mRumbleDataArray[0].intensity = 1.0f;
                            rumbleEffect.mRumbleDataArray[0].steady_duration =
                                0.2f;
                            rumbleEffect.mRumbleDataArray[0].delay = 0.0f;
                            rumbleEffect.mRumbleDataArray[1].intensity = 1.0f;
                            rumbleEffect.mRumbleDataArray[1].delay = 0.0f;
                            rumbleEffect.mRumbleDataArray[1].steady_duration =
                                0.2f;
                            rumbleEffect.mRumbleDataArray[1].ramp_up_duration =
                                0.0f;
                            rumbleEffect.mRumbleDataArray[1]
                                .ramp_down_duration = 0.0f;
                            RumbleManager* v18 =
                                RumbleManager::Inst(PlayerIndex);
                            v18->Play(&rumbleEffect, intensity);
                        }
                    }
                }
            }
            CurveManager_PostEvent(CurveManager_sInst,
                                   ent->mHandle.mHandle.mVal, s_ImpactMessage,
                                   intensity);
        }
    }
    return result;
}

// ============================================================================
// EffectEventSys - query param setters
// ============================================================================

// ea: 0x004BCE20
int EffectEventSys::NumberOfVoicesUsed()
{
    unsigned int NumVoices = nslGetNumVoices();
    int v2 = 0;
    for (unsigned int i = 0; i < NumVoices; ++i)
    {
        nslVoice* Voice = nslGetVoice(i);
        if (nslGetSourceState(*(nslSourceID*)((char*)Voice + 0x114))
            != NSL_SOURCE_STATE_INVALID)
            ++v2;
    }
    return v2;
}

// ea: 0x004BCE60
void EffectEventSys::TagNameIndexInfo(int index)
{
    mCurrentQuery->mBoneIndex = index;
}

// ea: 0x004BCE80
void EffectEventSys::SetEffectMatrix(math::Mat43* pMat)
{
    mCurrentQuery->mMatrix = pMat;
}

// ea: 0x004BCEA0
void EffectEventSys::SetScriptId(Broc::string val)
{
    mCurrentQuery->mScriptId = val;
}

// ea: 0x004BCF00
void EffectEventSys::SetDialogNotify(int notify)
{
    mCurrentQuery->mDialogNotify = notify;
}

// ea: 0x004BCF20
void EffectEventSys::SetQueryType(unsigned int val)
{
    mCurrentQuery->mQueryType = val;
}

// ea: 0x004BCF40
void EffectEventSys::SoundCacheType(int val)
{
    mCurrentQuery->mCacheSoundType = (int16_t)val;
}

// ea: 0x004C11A0
void EffectEventSys::IsSoundToBeQueued(bool val)
{
    if (val)
        mCurrentQuery->mFlags.mVal |= 2u;
}

// ea: 0x004C11C0
void EffectEventSys::SetQueryImportance(bool val)
{
    if (val)
        mCurrentQuery->mFlags.mVal |= 8u;
    else
        mCurrentQuery->mFlags.mVal &= ~8u;
}

// ea: 0x004C11F0
void EffectEventSys::DirectionInfo(const float* dir)
{
    mCurrentQuery->mFlags.mVal |= 1u;
    mCurrentQuery->mCollisionInfo.simple.normal.v.m128_f32[0] = dir[0];
    mCurrentQuery->mCollisionInfo.simple.normal.v.m128_f32[1] = dir[1];
    mCurrentQuery->mCollisionInfo.simple.normal.v.m128_f32[2] = dir[2];
}

// ============================================================================
// EffectEventSys - sound notify + fades
// ============================================================================

// ea: 0x004BCD90
void EffectEventSys::SendSpecificSoundNotify(Entity* pEnt,
                                             HashString soundName)
{
    if (pEnt != nullptr)
    {
        SndWait* sndWait = (SndWait*)&pEnt->snd_wait;
        if (sndWait->notifyHash.mHash != 0
            && sndWait->soundName.mHash == soundName.mHash)
        {
            Scr_Notify(pEnt, sndWait->notifyHash, 0);
            sndWait->notifyHash.mHash = 0;
            sndWait->soundName.mHash = 0;
        }
    }
}

// ea: 0x004BCDE0
void EffectEventSys::SendSoundNotify(Entity* pEnt)
{
    if (pEnt != nullptr)
    {
        SndWait* sndWait = (SndWait*)&pEnt->snd_wait;
        if (sndWait->notifyHash.mHash != 0)
        {
            Scr_Notify(pEnt, sndWait->notifyHash, 0);
            sndWait->notifyHash.mHash = 0;
            sndWait->soundName.mHash = 0;
        }
    }
}

// ea: 0x004C0FE0
void EffectEventSys::FadeOutEffect(AbstractEffect* effect, float seconds)
{
    effect->StartFadeOut(seconds);
    mFadingEffects.push_back(effect);
}

// ea: 0x004C1010
void EffectEventSys::AdvanceFades(float delta)
{
    for (unsigned int v3 = 0; v3 < (unsigned int)mFadingEffects.m_size; ++v3)
    {
        ASSERT_IDX(v3, 128, 154);
        if ((mFadingEffects[v3]->mCodeFlags.mVal & 1) != 0)
        {
            ASSERT_IDX(v3, 128, 154);
            AbstractEffect* effect = mFadingEffects[v3];
            effect->FrameAdvance(delta);
            ASSERT_IDX(v3, 128, 154);
            if ((effect->mCodeFlags.mVal & 1) != 0
                && effect->mFadeTime < 0.001f)
            {
                ASSERT_IDX(v3, 128, 154);
                AbstractEffect* p = mFadingEffects[v3];
                if (p != nullptr)
                    delete p;
                mFadingEffects[v3] = mFadingEffects[mFadingEffects.m_size - 1];
                if (mFadingEffects.m_size != 0)
                    --mFadingEffects.m_size;
                --v3;
            }
        }
    }
}

// ea: 0x004C1120
void EffectEventSys::SetSoundParams(SoundParams& soundParams, PendingQuery& q,
                                    float useNslDefault)
{
    soundParams.mNameRef = nullptr;
    soundParams.mHashStr = 0;
    soundParams.mVolume = useNslDefault;
    soundParams.mPakId = q.mEffectsPak;
    soundParams.mEnt.mHandle.mVal = q.mQueryEnt.mHandle.mVal;
    soundParams.mQueue = (q.mFlags.mVal & 2) != 0;
    soundParams.mCd = (q.mFlags.mVal & 4) != 0 ? &q.mCollisionInfo : nullptr;
    soundParams.mFlags = 0;
    soundParams.mMaxVoices = 2;
    soundParams.mPitch = useNslDefault;
    soundParams.mDuration = useNslDefault;
    soundParams.mSubtitle = nullptr;
    soundParams.mDialogNotify = q.mDialogNotify;
}

// ea: 0x004CA820
int EffectEventSys::IsSoundAlreadyPlaying(unsigned int mSoundNameHashStr,
                                          Entity* pEnt, int maxEffects)
{
    int playCount = 0;
    AbstractEffect* oldestEffect = nullptr;
    int oldestEffectIndex = 0;
    unsigned int oldestAbstractEffectIndex = 0;
    int highestDelayCountSoFar = 0;
    bool stopSound = false;
    if (mEffectSets.m_size == 0)
        return 0;
    for (unsigned int v6 = 0; v6 < (unsigned int)mEffectSets.m_size; ++v6)
    {
        ASSERT_IDX(v6, 512, 154);
        ActiveEffectSet* v8 = mEffectSets[v6];
        for (unsigned int v7 = 0;
             v7 < (unsigned int)v8->mEffects.m_size; ++v7)
        {
            ASSERT_IDX(v6, 512, 154);
            ASSERT_IDX(v7, 6, 148);
            AbstractEffect* v9 = v8->mEffects.m_elements[v7];
            if (v9 == nullptr || (v9->mCodeFlags.mVal & 4) == 0)
                continue;
            unsigned int v10 = v9->mEntity.mHandle.mVal & 0xFFF;
            Entity* mObject = nullptr;
            if (v10 < 0x540
                && v9->mEntity.mHandle.mVal >> 12
                       == EntityHandleDb::sInst.mElements[v10].mKey)
                mObject = EntityHandleDb::sInst.mElements[v10].mObject;
            if (pEnt == mObject
                && mSoundNameHashStr == v9->mEffectNameHashStr)
            {
                if (++playCount >= maxEffects)
                    stopSound = true;
                if (oldestEffect != nullptr)
                {
                    if (v9->mCountSinceStarted > highestDelayCountSoFar)
                    {
                        oldestEffect = v9;
                        highestDelayCountSoFar = v9->mCountSinceStarted;
                        oldestEffectIndex = (int)v6;
                        oldestAbstractEffectIndex = v7;
                    }
                    continue;
                }
                oldestEffect = v9;
                oldestAbstractEffectIndex = v7;
                highestDelayCountSoFar = v9->mCountSinceStarted;
                oldestEffectIndex = (int)v6;
            }
        }
    }
    if (stopSound)
    {
        if (oldestEffect == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
            AeAssert::gCurrentLine = 403;
            AeAssert::gCurrentExpr = "oldestEffect";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Oldest effect not setup"))
                __debugbreak();
        }
        oldestEffect->StopEffect();
        ActiveEffectSet* set = mEffectSets[oldestEffectIndex];
        ASSERT_IDX(oldestAbstractEffectIndex, 6, 154);
        AbstractEffect* p = set->mEffects[oldestAbstractEffectIndex];
        if (p != nullptr)
            delete p;
        unsigned int v14 = set->mEffects.m_size - 1;
        ASSERT_IDX(v14, 6, 154);
        set->mEffects[oldestAbstractEffectIndex] = set->mEffects[v14];
        if (set->mEffects.m_size != 0)
            --set->mEffects.m_size;
    }
    return playCount;
}

// ============================================================================
// HandleDb lookups
// ============================================================================

// ea: 0x004CACD0
bool EffectEventSys::IsEffectActive(Handle handle)
{
    int v2 = handle.mVal & 0x1FF;
    bool result = false;
    if (v2 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v2].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v2].mObject;
        if (mObject != nullptr
            && ((mObject->mFlags.mVal & 2) != 0
                || mObject->mEffects.m_size != 0))
            return true;
    }
    return result;
}

// ea: 0x004CB890
bool EffectEventIsPlaying(Handle effect)
{
    int v1 = effect.mVal & 0x1FF;
    bool result = false;
    if (v1 < 0x200
        && effect.mVal >> 9
               == EffectEventSysStatics::sInst->mHandleDb.mElements[v1].mKey)
    {
        ActiveEffectSet* mObject =
            EffectEventSysStatics::sInst->mHandleDb.mElements[v1].mObject;
        if (mObject != nullptr
            && ((mObject->mFlags.mVal & 2) != 0
                || mObject->mEffects.m_size != 0))
            return true;
    }
    return result;
}

// ea: 0x004CB8E0
void EffectEventAdjustEffect_Scale(Handle effect, const char* param,
                                   float scale)
{
    int v3 = effect.mVal & 0x1FF;
    if (v3 < 0x200
        && effect.mVal >> 9
               == EffectEventSysStatics::sInst->mHandleDb.mElements[v3].mKey)
    {
        ActiveEffectSet* mObject =
            EffectEventSysStatics::sInst->mHandleDb.mElements[v3].mObject;
        if (mObject != nullptr)
            mObject->AdjustEffect_Scale(param, scale);
    }
}

// ea: 0x004CB930
void EffectEventFF(Handle effect, float deltaT)
{
    int v2 = effect.mVal & 0x1FF;
    if (v2 < 0x200
        && effect.mVal >> 9
               == EffectEventSysStatics::sInst->mHandleDb.mElements[v2].mKey)
    {
        ActiveEffectSet* mObject =
            EffectEventSysStatics::sInst->mHandleDb.mElements[v2].mObject;
        if (mObject != nullptr)
            mObject->FastForward(deltaT);
    }
}

// ea: 0x004CB980
void EffectEventPlayQueuedEffect(Handle effect)
{
    int v1 = effect.mVal & 0x1FF;
    if (v1 < 0x200
        && effect.mVal >> 9
               == EffectEventSysStatics::sInst->mHandleDb.mElements[v1].mKey)
    {
        ActiveEffectSet* mObject =
            EffectEventSysStatics::sInst->mHandleDb.mElements[v1].mObject;
        if (mObject != nullptr)
            mObject->PlayQueuedEffect();
    }
}

// ea: 0x004CF2D0
void EffectEventStopEmitting(Handle effect)
{
    EffectEventSysStatics::sInst->StopEffect(effect, false);
}

// ea: 0x004CF2F0
void EffectEventKill(Handle effect)
{
    EffectEventSysStatics::sInst->StopEffect(effect, false);
}

// ea: 0x004CEF20
void EffectEventSys::StopEffect(Handle handle, bool kill)
{
    bool saveStoppingAll = mStoppingAll;
    if (kill)
        mStoppingAll = true;
    for (unsigned int v5 = 0; v5 < (unsigned int)mEffectSets.m_size; ++v5)
    {
        ASSERT_IDX(v5, 512, 154);
        if (mEffectSets.m_elements[v5]->mId.mVal == handle.mVal)
        {
            ActiveEffectSet* v7 = mEffectSets[v5];
            if (v7 != nullptr)
            {
                v7->~ActiveEffectSet();
                ActiveEffectSet_sAllocator->Release(v7);
            }
            mEffectSets[v5] = mEffectSets[mEffectSets.m_size - 1];
            if (mEffectSets.m_size != 0)
                --mEffectSets.m_size;
            --v5;
        }
    }
    mStoppingAll = saveStoppingAll;
}

// ============================================================================
// HandleDb
// ============================================================================

// ea: 0x004E8D50
Handle HandleDb::AllocateHandle()
{
    int m_cur_val = -1;
    for (int i = 0; i < 512; ++i)
    {
        if ((mFreeBits[i >> 3] >> (i & 7)) & 1)
        {
            m_cur_val = i;
            break;
        }
    }
    if (m_cur_val >= 0x200)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
        AeAssert::gCurrentLine = 98;
        AeAssert::gCurrentExpr = "nextIndex >= 0 && nextIndex < _MaxEltements";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "index out of bounds!!! ILLEGAL array access!"))
            __debugbreak();
    }
    mFreeBits[m_cur_val >> 3] &= (unsigned char)~(1u << (m_cur_val & 7));
    Handle result;
    result.mVal = (mElements[m_cur_val].mKey << 9) | m_cur_val;
    return result;
}

// ea: 0x004E3DB0
void HandleDb::BindObjectToHandle(Handle handle, ActiveEffectSet* obj)
{
    int v3 = handle.mVal & 0x1FF;
    if (mElements[v3].mKey != (unsigned int)(handle.mVal >> 9))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
        AeAssert::gCurrentLine = 123;
        AeAssert::gCurrentExpr = "element.GetKey() == h.GetKey()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("handle was not allocated for this object"))
            __debugbreak();
    }
    mElements[v3].mObject = obj;
}

// ea: 0x004E6430
void HandleDb::ReleaseHandle(Handle h)
{
    if (h.mVal != 0)
    {
        int v3 = h.mVal & 0x1FF;
        if (mElements[v3].mKey == (unsigned int)(h.mVal >> 9))
        {
            mFreeBits[v3 >> 3] |= (unsigned char)(1u << (v3 & 7));
            mElements[v3].mObject = nullptr;
            mElements[v3].mKey = mElements[v3].mKey + 1;
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
            AeAssert::gCurrentLine = 170;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("freeing invalid handle"))
                __debugbreak();
        }
    }
}

// ============================================================================
// EffectEventSys - handle plumbing + lifecycle
// ============================================================================

// ea: 0x004C56B0
ActiveEffectSet* EffectEventSys::GetActiveEffectSet(Handle handle)
{
    int v2 = handle.mVal & 0x1FF;
    if (v2 < 0x200
        && handle.mVal >> 9 == mHandleDb.mElements[v2].mKey)
        return mHandleDb.mElements[v2].mObject;
    return nullptr;
}

// ea: 0x004CABD0
void EffectEventSys::AdjustEffect_Scale(Handle handle, const char* param,
                                        float scale)
{
    int v4 = handle.mVal & 0x1FF;
    if (v4 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v4].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v4].mObject;
        if (mObject != nullptr)
            mObject->AdjustEffect_Scale(param, scale);
    }
}

// ea: 0x004CAC10
void EffectEventSys::FastForward(Handle handle, float deltaT)
{
    int v3 = handle.mVal & 0x1FF;
    if (v3 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v3].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v3].mObject;
        if (mObject != nullptr)
            mObject->FastForward(deltaT);
    }
}

// ea: 0x004CAC50
void EffectEventSys::PlayQueuedEffect(Handle handle)
{
    int v2 = handle.mVal & 0x1FF;
    if (v2 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v2].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v2].mObject;
        if (mObject != nullptr)
            mObject->PlayQueuedEffect();
    }
}

// ea: 0x004CAC90
void EffectEventSys::StopLoopingEffects(Handle handle)
{
    int v2 = handle.mVal & 0x1FF;
    if (v2 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v2].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v2].mObject;
        if (mObject != nullptr)
            mObject->mFlags.mVal |= 1u;
    }
}

// ea: 0x004CB840
void EffectEventSys::ReleaseHandle(ActiveEffectSet* t)
{
    if (t->mId.mVal != 0)
    {
        mHandleDb.ReleaseHandle(t->mId);
        t->mId.mVal = 0;
    }
}

// ea: 0x004CB870
void EffectEventSys::ReleaseHandle(Handle h)
{
    if (h.mVal != 0)
        mHandleDb.ReleaseHandle(h);
}

// ea: 0x004CEDC0
void EffectEventSys::StopAll()
{
    mStoppingAll = true;
    while (mEffectSets.m_size != 0)
    {
        ActiveEffectSet* v3 = mEffectSets[0];
        if (v3 != nullptr)
        {
            v3->~ActiveEffectSet();
            ActiveEffectSet_sAllocator->Release(v3);
        }
        unsigned int v4 = mEffectSets.m_size - 1;
        ASSERT_IDX(v4, 512, 154);
        mEffectSets[0] = mEffectSets[v4];
        if (mEffectSets.m_size != 0)
            --mEffectSets.m_size;
    }
    while (mFadingEffects.m_size != 0)
    {
        AbstractEffect* v6 = mFadingEffects[0];
        if (v6 != nullptr)
            delete v6;
        unsigned int v7 = mFadingEffects.m_size - 1;
        ASSERT_IDX(v7, 128, 154);
        mFadingEffects[0] = mFadingEffects[v7];
        if (mFadingEffects.m_size != 0)
            --mFadingEffects.m_size;
    }
    FX_ClearFX();
    mStoppingAll = false;
}

// ea: 0x004CF020
void EffectEventSys::KillEffectsWithPakId(TPakId pak_id)
{
    for (unsigned int v3 = 0; v3 < (unsigned int)mEffectSets.m_size;)
    {
        ASSERT_IDX(v3, 512, 154);
        if (mEffectSets[v3]->mPakId == pak_id)
        {
            ASSERT_IDX(v3, 512, 154);
            ActiveEffectSet* v4 = mEffectSets[v3];
            if (v4 != nullptr)
            {
                v4->~ActiveEffectSet();
                ActiveEffectSet_sAllocator->Release(v4);
            }
            unsigned int v5 = mEffectSets.m_size - 1;
            ASSERT_IDX(v5, 512, 154);
            ASSERT_IDX(v3, 512, 154);
            mEffectSets[v3] = mEffectSets[v5];
            if (mEffectSets.m_size != 0)
                --mEffectSets.m_size;
        }
        else
        {
            ++v3;
        }
    }
}

// ea: 0x004CF1D0
void EffectEventSys::CollisionInfo(const CollisionDesc* col_desc, bool set_mat)
{
    mCurrentQuery->mFlags.mVal |= 4u;
    memcpy(&mCurrentQuery->mCollisionInfo, col_desc,
           sizeof(mCurrentQuery->mCollisionInfo));
    if (set_mat)
    {
        int material = col_desc->material;
        if (material > 0)  // kCollisionMaterialASPHALT
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
            AeAssert::gCurrentLine = 1558;
            AeAssert::gCurrentExpr =
                "( mat_type >= kCollisionMaterialMin && mat_type <= "
                "kCollisionMaterialMax )";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("value not in enum range"))
                __debugbreak();
        }
        CachedQuery* p_mCachedQuery = &mCurrentQuery->mCachedQuery;
        p_mCachedQuery->mSpecifiedFields.mBits[3 >> 3] |=
            (unsigned char)(1u << (3 & 7));
        p_mCachedQuery->mWeakFields.mBits[3 >> 3] |=
            (unsigned char)(1u << (3 & 7));
        p_mCachedQuery->mMATERIAL = material;
    }
}

// ea: 0x004CF290
Handle EffectEventSys::AssignHandle(ActiveEffectSet* t)
{
    Handle v4 = mHandleDb.AllocateHandle();
    mHandleDb.BindObjectToHandle(v4, t);
    t->mId = v4;
    return v4;
}

// ============================================================================
// ActiveEffectSet
// ============================================================================

// ea: 0x004C0C00
void ActiveEffectSet::AddEffect(AbstractEffect* effect)
{
    math::Mat43* mPoPtr = this->mPoPtr;
    if (mPoPtr != nullptr)
        effect->SetPoPtr(mPoPtr);
    mEffects.push_back(effect);
}

// ea: 0x004C0C30
bool ActiveEffectSet::IsFinished() const
{
    return (mFlags.mVal & 2) == 0 && mEffects.m_size == 0;
}

// ea: 0x004C0C50
bool ActiveEffectSet::IsQueued() const
{
    unsigned int v2 = 0;
    if (mEffects.m_size == 0)
        return true;
    while (true)
    {
        ASSERT_IDX(v2, 6, 148);
        if (!mEffects.m_elements[v2]->IsQueued())
            break;
        if (++v2 >= (unsigned int)mEffects.m_size)
            return true;
    }
    return false;
}

// ea: 0x004CA660
void ActiveEffectSet::FrameAdvance(float delta)
{
    if ((mFlags.mVal & 2) != 0 && mEffects.m_size != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
        AeAssert::gCurrentLine = 169;
        AeAssert::gCurrentExpr =
            "!mFlags.Test( kActiveEffectFlag_Pending ) || "
            "mEffects.size() == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("effect should be ready"))
            __debugbreak();
    }
    if ((mFlags.mVal & 2) == 0)
    {
        if ((mFlags.mVal & 1) != 0)
            DoStopLoopingEffects();
        int m_size = mEffects.m_size;
        int size = m_size;
        for (unsigned int v4 = 0; v4 < (unsigned int)m_size; ++v4)
        {
            ASSERT_IDX(v4, 6, 154);
            mEffects.m_elements[v4]->FrameAdvance(delta);
            if (m_size <= mEffects.m_size)
            {
                ASSERT_IDX(v4, 6, 154);
                AbstractEffect* effect = mEffects[v4];
                if (effect->IsFinished())
                {
                    unsigned int mVal =
                        mEffects[v4]->mEntity.mHandle.mVal;
                    unsigned int v7 = mVal & 0xFFF;
                    if (v7 < 0x540
                        && mVal >> 12 == EntityHandleDb::sInst.mElements[v7].mKey)
                    {
                        Entity* mObject =
                            EntityHandleDb::sInst.mElements[v7].mObject;
                        if (mObject != nullptr)
                        {
                            SndWait* sndWait = (SndWait*)&mObject->snd_wait;
                            if (sndWait->notifyHash.mHash != 0)
                            {
                                Scr_Notify(mObject, sndWait->notifyHash, 0);
                                sndWait->notifyHash.mHash = 0;
                                sndWait->soundName.mHash = 0;
                            }
                        }
                    }
                    ASSERT_IDX(v4, 6, 154);
                    AbstractEffect* p = mEffects[v4];
                    if (p != nullptr)
                        delete p;
                    mEffects[v4] = mEffects[mEffects.m_size - 1];
                    if (mEffects.m_size != 0)
                        --mEffects.m_size;
                    --v4;
                    m_size = --size;
                }
            }
            else
            {
                --v4;
                size = --m_size;
            }
        }
    }
}

// ea: 0x004C0CD0
void ActiveEffectSet::AdjustEffect_Scale(const char* param, float scale)
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        mEffects.m_elements[i]->AdjustEffect_Scale(param, scale);
    }
}

// ea: 0x004C0D50
void ActiveEffectSet::FastForward(float deltaT)
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        mEffects.m_elements[i]->FastForward(deltaT);
    }
}

// ea: 0x004C0DD0
void ActiveEffectSet::PlayQueuedEffect()
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        mEffects.m_elements[i]->PlayQueuedEffect();
    }
}

// ea: 0x004C0E40
void ActiveEffectSet::SetPoPtr(math::Mat43* po)
{
    int m_size = mEffects.m_size;
    unsigned int v4 = 0;
    mPoPtr = po;
    if (m_size != 0)
    {
        do
        {
            ASSERT_IDX(v4, 6, 154);
            mEffects.m_elements[v4]->SetPoPtr(po);
            ++v4;
        } while (v4 < (unsigned int)mEffects.m_size);
    }
}

// ea: 0x004C0EC0
void ActiveEffectSet::StopLoopingEffects()
{
    mFlags.mVal |= 1u;
}

// ea: 0x004C0ED0
void ActiveEffectSet::DoStopLoopingEffects()
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        AbstractEffect* effect = mEffects.m_elements[i];
        if (effect->IsLooping())
        {
            ASSERT_IDX(i, 6, 154);
            if ((effect->mFlags & 1) == 0)
            {
                AbstractEffect* p = mEffects[i];
                if (p != nullptr)
                    delete p;
                mEffects[i] = mEffects[mEffects.m_size - 1];
                if (mEffects.m_size != 0)
                    --mEffects.m_size;
                --i;
            }
        }
    }
}

// ============================================================================
// AbstractEffect virtuals / getters
// ============================================================================

// ea: 0x004CC230
math::Position3 AbstractEffect::GetPosition() const
{
    math::Position3 result;
    const math::Mat43* mPoPtr = this->mPoPtr;
    if (mPoPtr != nullptr)
    {
        result.v.m128_f32[0] = mPoPtr->w.v.m128_f32[0];
        result.v.m128_f32[1] = mPoPtr->w.v.m128_f32[1];
        result.v.m128_f32[2] = mPoPtr->w.v.m128_f32[2];
        result.v.m128_f32[3] = mPoPtr->w.v.m128_f32[3];
        return result;
    }
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v7 = mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v7 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v7].mKey)
        mObject = EntityHandleDb::sInst.mElements[v7].mObject;
    result.v = mObject->r.currentOrigin.v;
    return result;
}

// ea: 0x004CC2F0
bool AbstractEffect::IsFinished()
{
    unsigned int v1 = mEntity.mHandle.mVal & 0xFFF;
    if (v1 < 0x540
        && mEntity.mHandle.mVal >> 12
               == EntityHandleDb::sInst.mElements[v1].mKey
        && EntityHandleDb::sInst.mElements[v1].mObject != nullptr)
    {
        return false;
    }
    return (mFlags & 1) == 0;
}

// ea: 0x004CC330
Broc::string AbstractEffect::GetEntityDebugString() const
{
    Broc::string r((Broc::string::Block*)nullptr);
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v4 = mVal & 0xFFF;
    if (v4 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != nullptr)
        {
            r += " ENT:";
            Broc::string::Block* mBlock = mObject->mClassName.mBlock;
            const char* v7 = mBlock ? (const char*)(mBlock + 1)
                                    : defaultFileName;
            r += v7;
        }
    }
    return r;
}

// ea: 0x004BD300
Broc::string AbstractEffectLight::GetDebugString() const
{
    return Broc::string((Broc::string::Block*)nullptr);
}

// ea: 0x004BD370
Broc::string AbstractEffectShakeAndRumble::GetDebugString() const
{
    return Broc::string((Broc::string::Block*)nullptr);
}

// ea: 0x004CE0B0
math::Position3 AbstractEffectShakeAndRumble::GetPositionOnEntity() const
{
    math::Position3 result;
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v3 = mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v3 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey)
        mObject = EntityHandleDb::sInst.mElements[v3].mObject;
    result.v = mObject->r.currentOrigin.v;
    return result;
}

// ea: 0x004CE110
float AbstractEffectShakeAndRumble::GetDistanceScale(int client)
{
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v5 = mVal & 0xFFF;
    if (v5 >= 0x540 || mVal >> 12 != EntityHandleDb::sInst.mElements[v5].mKey
        || EntityHandleDb::sInst.mElements[v5].mObject == nullptr)
        return 1.0f;
    if (mMinDist2 <= 0.0f || mMaxDist2 <= mMinDist2)
        return 1.0f;
    Entity* Player = EntityManager::sInst->GetPlayer(client);
    math::Position3 v11 = GetPositionOnEntity();
    __m128 v7 = _mm_sub_ps(Player->r.currentOrigin.v, v11.v);
    __m128 v8 = _mm_mul_ps(v7, v7);
    float v12 = v8.m128_f32[0]
                + (_mm_shuffle_ps(v8, v8, 0x55).m128_f32[0]
                   + _mm_shuffle_ps(v8, v8, 0xAA).m128_f32[0]);
    if (mMinDist2 > v12)
        return 1.0f;
    if (mMaxDist2 <= v12)
        return 0.0f;
    return 1.0f - (v12 - mMinDist2) / (mMaxDist2 - mMinDist2);
}

// ============================================================================
// AbstractEffectParticle virtuals
// ============================================================================

// ea: 0x004BD200
void AbstractEffectParticle::SetPoPtr(math::Mat43* po)
{
    mPoPtr = po;
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle != nullptr)
        mParticle->mPoPtr = po;
}

// ea: 0x004BD220
bool AbstractEffectParticle::IsLooping() const
{
    return false;
}

// ea: 0x004BD230
void AbstractEffectParticle::AdjustEffect_Scale(const char* param, float scale)
{
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle != nullptr)
    {
        apsEffect* mEffect = mParticle->mEffect;
        if (mEffect != nullptr)
        {
            int ModifierId = mEffect->GetModifierId(param);
            if (ModifierId >= 0)
            {
                float value =
                    this->mParticle->mEffect->GetModifierTemplateValue(
                        ModifierId);
                float iVal = value * scale;
                this->mParticle->mEffect->SetModifierValue(ModifierId, iVal);
            }
        }
    }
}

// ea: 0x004BD280
void AbstractEffectParticle::FastForward(float deltaT)
{
    apsEffect* mEffect = mParticle->mEffect;
    if (mEffect != nullptr)
        mEffect->FastForward(deltaT, 100);
}

// ea: 0x004C1350
void AbstractEffectParticle::StartFadeOut(float seconds)
{
    mCodeFlags.mVal |= 3u;
    mFadeStart = seconds;
    mFadeTime = seconds;
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle != nullptr)
    {
        apsEffect* mEffect = mParticle->mEffect;
        if (mEffect != nullptr)
            mEffect->StopEmitting();
    }
}

// ea: 0x004C59C0
Broc::string AbstractEffectParticle::GetDebugString() const
{
    Broc::string r((Broc::string::Block*)nullptr);
    if (mDelayTrigger <= mDelayCount)
    {
        Broc::string::Block* mBlock = mEffectName.mBlock;
        const char* v8 = mBlock ? (const char*)(mBlock + 1)
                                : defaultFileName;
        ae_formatted_string<1024, unsigned short> v10("PFX: %s", v8);
        r = (const char*)v10.mBuff;
    }
    else
    {
        Broc::string::Block* v4 = mEffectName.mBlock;
        const char* v5 = v4 ? (const char*)(v4 + 1) : defaultFileName;
        ae_formatted_string<1024, unsigned short> v10(
            "PFX: %s DELAYED %f/%f", v5, mDelayCount, mDelayTrigger);
        r = (const char*)v10.mBuff;
    }
    return r;
}

// ============================================================================
// AbstractEffectSound virtuals
// ============================================================================

// ea: 0x004C12A0
void AbstractEffectSound::StartFadeOut(float seconds)
{
    mCodeFlags.mVal |= 3u;
    mFadeStart = seconds;
    mFadeTime = seconds;
}

// ============================================================================
// AbstractEffectLight virtuals
// ============================================================================

// ea: 0x004CDEF0
math::Position3 AbstractEffectLight::GetPositionOnEntity(Entity* e) const
{
    math::Position3 result;
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v4 = mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v4 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey)
        mObject = EntityHandleDb::sInst.mElements[v4].mObject;
    result.v = mObject->r.currentOrigin.v;
    return result;
}

// ea: 0x004CDF50
void AbstractEffectLight::FrameAdvance(float delta_t)
{
    LightEffect* mVertLight = this->mVertLight;
    if (mVertLight != nullptr)
    {
        gdLight* mLight = this->mLight;
        mTime = delta_t + mTime;
        if (mTime <= ((mLight->rampdown + mLight->rampup) + mLight->duration))
        {
            if (mTime < (mLight->rampup + mLight->duration))
            {
                if (mTime < mLight->rampup)
                    mVertLight->SetScale(mTime / mLight->rampup);
                else
                    mVertLight->SetScale(1.0f);
            }
            else
            {
                mVertLight->SetScale(
                    1.0f - ((mTime - mLight->duration) - mLight->rampup)
                               / mLight->rampdown);
            }
        }
        else
        {
            mVertLight->mActive = false;
            this->mVertLight = nullptr;
        }
        if (this->mVertLight != nullptr)
        {
            unsigned int v7 = mEntity.mHandle.mVal & 0xFFF;
            if (v7 < 0x540
                && mEntity.mHandle.mVal >> 12
                       == EntityHandleDb::sInst.mElements[v7].mKey)
            {
                Entity* mObject = EntityHandleDb::sInst.mElements[v7].mObject;
                if (mObject != nullptr)
                    this->mVertLight->mLightPos.v =
                        GetTagFlashPos(mObject).v;
            }
        }
    }
}

// ea: 0x004CE060
bool AbstractEffectLight::IsFinished()
{
    if (mLight->projlight != 0)
        return true;
    LightEffect* pProj = mProjLight;
    if (pProj == nullptr || pProj->IsLightFinished())
    {
        LightEffect* pVert = mVertLight;
        if (pVert == nullptr || pVert->IsLightFinished())
            return true;
    }
    if ((mCodeFlags.mVal & 2) != 0)
        return AbstractEffect::IsFinished();
    return false;
}

// ============================================================================
// AbstractEffectShakeAndRumble virtuals
// ============================================================================

// ea: 0x004CE720
bool AbstractEffectShakeAndRumble::IsFinished()
{
    if ((mCodeFlags.mVal & 2) == 0)
        return true;
    for (int i = 0; i < 4; ++i)
    {
        if (dword_F6A290[i * 0x322] == 2 && mShake[i] != nullptr
            && mShake[i]->m_active != 0)
        {
            RumbleManager* v4 = RumbleManager::Inst(i);
            if (v4->IsPlaying(mRumbleHandle[i]))
                return false;
        }
    }
    return true;
}

// ea: 0x004CE780
void AbstractEffectShakeAndRumble::AdjustEffect_Scale(const char* param,
                                                      float scale)
{
    if (dword_F6A290[0] == 2 && mRumbleHandle[0].mVal != 0)
    {
        RumbleManager* v3 = RumbleManager::Inst(0);
        v3->SetIntensity(mRumbleHandle[0], scale);
    }
}

// ea: 0x004CE7B0
void AbstractEffectShakeAndRumble::StopEffect()
{
    if (dword_F6A290[0] == 2)
    {
        if (mRumbleHandle[0].mVal != 0)
        {
            RumbleManager* v2 = RumbleManager::Inst(0);
            v2->Remove(mRumbleHandle[0]);
            mRumbleHandle[0].mVal = 0;
        }
        if (mShake[0] != nullptr)
        {
            CameraShake_StopCameraShake(g_cameraShake, mShake[0]);
            mShake[0] = nullptr;
        }
    }
}

// ea: 0x004CE230
void AbstractEffectShakeAndRumble::FrameAdvance(float delta_t)
{
    mDelayCount = delta_t + mDelayCount;
    if (mDelayTrigger > mDelayCount)
        return;
    unsigned int mVal = mEntity.mHandle.mVal;
    Client* ownerClient = nullptr;
    Client* v6 = nullptr;
    unsigned int v7 = mVal & 0xFFF;
    if (v7 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v7].mKey
        && EntityHandleDb::sInst.mElements[v7].mObject != nullptr)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v7].mObject;
        if (mObject->client != nullptr)
        {
            ownerClient = AbstractEffectGetOwner(this)->client;
            v6 = ownerClient;
        }
    }
    if ((mCodeFlags.mVal & 2) == 0)
    {
        mCodeFlags.mVal |= 2u;
        int instance = 0;
        CameraShake* v32 = g_cameraShake;
        for (int v10 = 0; v10 < 1; ++v10)
        {
            if (dword_F6A290[v10 * 0x322] == 2
                && (v6 == nullptr
                    || v6->mServerClientIndex == v10))
            {
                float distanceScale = GetDistanceScale(v10);
                if (mFreq > 0.0f && mMovement > 0.0f && distanceScale > 0.0f)
                {
                    CameraShakeInstance* started =
                        CameraShake_StartCameraShake(
                            v32, 1, nullptr, 1.0f, mTime, mNextDelay);
                    mShake[0] = started;
                    if (started != nullptr)
                        CameraShakeInstance_OverrideSettings(
                            started, mFreq * distanceScale,
                            mMovement * distanceScale);
                }
                if (!mRumbleEnabled)
                    return;
                RumbleEffect rumbleEffect;
                for (int i = 0; i < 2; ++i)
                {
                    rumbleEffect.mRumbleDataArray[i].enabled = true;
                    rumbleEffect.mRumbleDataArray[i].delay = 0.0f;
                    rumbleEffect.mRumbleDataArray[i].intensity = 1.0f;
                    rumbleEffect.mRumbleDataArray[i].ramp_up_duration = 0.0f;
                    rumbleEffect.mRumbleDataArray[i].steady_duration = 1.0f;
                    rumbleEffect.mRumbleDataArray[i].ramp_down_duration = 0.0f;
                }
                rumbleEffect.mRumbleDataArray[0].intensity =
                    mUseHighFreqVibrator ? 1.0f : 0.0f;
                if (mSteadyDuration < 0.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\RumbleEffect.h";
                    AeAssert::gCurrentLine = 124;
                    AeAssert::gCurrentExpr = "new_duration >= 0.0f";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Please add a descriptive string"))
                        __debugbreak();
                }
                rumbleEffect.mRumbleDataArray[0].steady_duration =
                    mSteadyDuration;
                float rumbleIntensity = mRumble;
                if (mRumble >= 0.0f)
                {
                    if (mRumble > 1.0f)
                        rumbleIntensity = 1.0f;
                }
                else
                {
                    rumbleIntensity = 0.0f;
                }
                rumbleEffect.mRumbleDataArray[1].intensity = rumbleIntensity;
                if (mSteadyDuration < 0.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\RumbleEffect.h";
                    AeAssert::gCurrentLine = 124;
                    AeAssert::gCurrentExpr = "new_duration >= 0.0f";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Please add a descriptive string"))
                        __debugbreak();
                }
                rumbleEffect.mRumbleDataArray[1].steady_duration =
                    mSteadyDuration;
                if (mRampUpTime < 0.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\RumbleEffect.h";
                    AeAssert::gCurrentLine = 117;
                    AeAssert::gCurrentExpr = "new_duration >= 0.0f";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Please add a descriptive string"))
                        __debugbreak();
                }
                rumbleEffect.mRumbleDataArray[1].ramp_up_duration = mRampUpTime;
                if (mRampDownTime < 0.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\RumbleEffect.h";
                    AeAssert::gCurrentLine = 110;
                    AeAssert::gCurrentExpr = "new_duration >= 0.0f";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Please add a descriptive string"))
                        __debugbreak();
                }
                rumbleEffect.mRumbleDataArray[1].ramp_down_duration =
                    mRampDownTime;
                RumbleManager* mgr = RumbleManager::Inst(instance);
                if (mgr != nullptr)
                {
                    RumbleEffectInstanceHandle h =
                        mgr->Play(&rumbleEffect, distanceScale);
                    mRumbleHandle[0].mVal = h.mVal;
                }
            }
            ++instance;
        }
    }
    if (dword_F6A290[0] == 2
        && (ownerClient == nullptr || ownerClient->mServerClientIndex == 0))
    {
        float rumbleIntensitya = GetDistanceScale(0);
        if (mShake[0] != nullptr)
            CameraShakeInstance_OverrideSettings(
                mShake[0], mFreq * rumbleIntensitya,
                mMovement * rumbleIntensitya);
        RumbleManager* v22 = RumbleManager::Inst(0);
        if (v22 != nullptr)
            v22->SetIntensity(mRumbleHandle[0], rumbleIntensitya);
    }
}

// ============================================================================
// AbstractEffect fades + particle IsFinished
// ============================================================================

// ea: 0x004E2D40
bool AbstractEffect::IsFinishedFading()
{
    return (mCodeFlags.mVal & 1) != 0 && mFadeTime < 0.001f;
}

// ea: 0x004CDC90
bool AbstractEffectParticle::IsFinished()
{
    if ((mCodeFlags.mVal & 2) == 0)
        return false;
    unsigned int v2 = mEntity.mHandle.mVal & 0xFFF;
    if ((v2 >= 0x540
         || mEntity.mHandle.mVal >> 12
                != EntityHandleDb::sInst.mElements[v2].mKey
         || EntityHandleDb::sInst.mElements[v2].mObject == nullptr)
        && (mFlags & 1) == 0)
    {
        return true;
    }
    if ((mCodeFlags.mVal & 1) != 0 && mFadeTime <= 0.0f)
        return true;
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle == nullptr || mParticle->mEffect == nullptr)
        return true;
    if ((mCodeFlags.mVal & 1) != 0 && !IsFinishedFading())
        return false;
    return mParticle->mEffect->IsDone() != 0;
}

// ============================================================================
// ActiveEffectSet ctor
// ============================================================================

// ea: 0x004D00E0
ActiveEffectSet::ActiveEffectSet(TPakId pak_id)
{
    mEffects.m_size = 0;
    mPakId = pak_id;
    mPoPtr = nullptr;
    mId.mVal = 0;
    mFlags.mVal = 0;
    mFlags.mVal = 2;
    HandleDb* p_mHandleDb = &EffectEventSysStatics::sInst->mHandleDb;
    Handle v5 = p_mHandleDb->AllocateHandle();
    p_mHandleDb->BindObjectToHandle(v5, this);
    mId = v5;
}

// ============================================================================
// AbstractEffectSound virtuals
// ============================================================================

// ea: 0x004CD050
void AbstractEffectSound::SetPoPtr(math::Mat43* po)
{
    mPoPtr = po;
    SoundDevice::Sound* mObject =
        SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
    if (mObject != nullptr)
        SoundDevice::Sound_SetPoPtr(mObject, po);
}

// ea: 0x004CD0C0
bool AbstractEffectSound::IsQueued() const
{
    SoundDevice::Sound* mObject =
        SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
    if (mObject == nullptr)
        return true;
    return SoundDevice::Sound_IsQueued(mObject);
}

// ea: 0x004CD120
bool AbstractEffectSound::IsFinished()
{
    unsigned int mMask = mCodeFlags.mVal;
    if ((mMask & 2) == 0)
        return false;
    unsigned int v3 = mEntity.mHandle.mVal & 0xFFF;
    bool entInvalid = (v3 >= 0x540
                       || mEntity.mHandle.mVal >> 12
                              != EntityHandleDb::sInst.mElements[v3].mKey
                       || EntityHandleDb::sInst.mElements[v3].mObject == nullptr);
    if ((entInvalid && (mFlags & 1) == 0)
        || ((mMask & 1) != 0 && mFadeTime <= 0.0f))
    {
        return true;
    }
    if ((mMask & 0x20) != 0)
        return false;
    SoundDevice::Sound* mSound =
        SoundDevice::SoundFromHandle(this->mSound.mHandle.mVal);
    if (mSound == nullptr)
        return true;
    return SoundDevice::Sound_IsFinished(mSound);
}

// ea: 0x004CD1B0
bool AbstractEffectSound::IsLooping() const
{
    if ((mCodeFlags.mVal & 2) == 0)
        return false;
    SoundDevice::Sound* mObject =
        SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
    if (mObject == nullptr)
        return false;
    return SoundDevice::Sound_IsLooped(mObject);
}

// ea: 0x004CD230
void AbstractEffectSound::AdjustEffect_Scale(const char* param, float scale)
{
    SoundDevice::Sound* mObject =
        SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
    if (mObject != nullptr && mObject->mSource != -1)
    {
        if (strcmp(param, "SOUND_PITCH") == 0)
        {
            SoundDevice::Sound* v8 =
                SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
            float pitch = SoundDevice::nslGetWaveParam(v8->mWave, 1, 1.0f);
            pitch = pitch * scale;
            SoundDevice::Sound_SetPitch(
                SoundDevice::SoundFromHandle(mSound.mHandle.mVal), pitch);
        }
        else if (strcmp(param, "SOUND_VOLUME") == 0)
        {
            SoundDevice::Sound* v9 =
                SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
            float volume = SoundDevice::nslGetWaveParam(v9->mWave, 0, 1.0f);
            volume = volume * scale;
            SoundDevice::Sound_SetVolume(
                SoundDevice::SoundFromHandle(mSound.mHandle.mVal), volume);
        }
    }
}

// ea: 0x004CD350
void AbstractEffectSound::PlayQueuedEffect()
{
    SoundDevice::Sound* mObject =
        SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
    if (mObject != nullptr)
    {
        if (mObject == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
            AeAssert::gCurrentLine = 496;
            AeAssert::gCurrentExpr = "*mSound";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("The sound has not been queued!!!!!"))
                __debugbreak();
        }
        if (SoundDevice::Sound_IsQueued(mObject))
        {
            SoundDevice::Sound_PlayQueued(
                SoundDevice::SoundFromHandle(mSound.mHandle.mVal));
            const char* mSubtitle = this->mSubtitle;
            if (mSubtitle != nullptr)
                SoundDevice::subtitle_manager_play_subtitle(mSubtitle, nullptr);
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
            AeAssert::gCurrentLine = 501;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("The sound is not ready yet!!!!!"))
                __debugbreak();
        }
    }
}

// ea: 0x004CD4C0
void AbstractEffectSound::StopEffect()
{
    SoundDevice::Sound* mObject =
        SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
    if (mObject != nullptr)
    {
        SoundDevice::Sound_Stop(mObject);
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
        AeAssert::gCurrentLine = 517;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            Broc::string::Block* mBlock = mEffectName.mBlock;
            const char* v7 =
                mBlock ? (const char*)(mBlock + 1) : defaultFileName;
            if (AeAssert::Warning("The sound %s has not been queued!!!", v7))
                __debugbreak();
        }
    }
}

// ea: 0x004CD580
Broc::string AbstractEffectSound::GetDebugString() const
{
    Broc::string r((Broc::string::Block*)nullptr);
    if ((mCodeFlags.mVal & 2) == 0)
    {
        ae_formatted_string<128, unsigned char> v17(
            "SFX: %d DELAYED %f/%f", mEffectNameHashStr, mDelayCount,
            mDelayTrigger);
        r = (const char*)v17.mBuff;
    }
    else
    {
        const char* SourceName;
        float vol;
        float len;
        int mSource;
        unsigned int v6 = mSound.mHandle.mVal & 0xFFF;
        SoundDevice::Sound* mObject = nullptr;
        if (v6 < 0x200
            && mSound.mHandle.mVal >> 12
                   == SoundDevice::SoundHandleDb::sInst.mElements[v6].mKey)
            mObject = SoundDevice::SoundHandleDb::sInst.mElements[v6].mObject;
        if (mObject != nullptr)
        {
            SourceName = SoundDevice::Sound_GetSourceName(mObject);
            vol = SoundDevice::Sound_GetVolume(mObject);
            len = SoundDevice::Sound_GetLength(mObject);
            mSource = mObject->mSource;
        }
        else
        {
            SourceName = "<no sound>";
            vol = -1.0f;
            len = -1.0f;
            mSource = -1;
        }
        SoundDevice::Sound* v13 =
            SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
        char v14 = (v13 != nullptr && !SoundDevice::Sound_IsLooped(v13))
                       ? 'L'
                       : ' ';
        ae_formatted_string<128, unsigned char> v17(
            "SFX: %d [0x%08x] %.2f %.2f %c", SourceName, mSource, vol, len,
            v14);
        r = (const char*)v17.mBuff;
    }
    return r;
}

// ============================================================================
// AbstractEffectSound::FrameAdvance
// ============================================================================

static Entity* AbstractEffectGetOwner(const AbstractEffect* effect)
{
    unsigned int v = effect->mEntity.mHandle.mVal & 0xFFF;
    if (v < 0x540
        && effect->mEntity.mHandle.mVal >> 12
               == EntityHandleDb::sInst.mElements[v].mKey)
        return EntityHandleDb::sInst.mElements[v].mObject;
    return nullptr;
}

// ea: 0x004CC4B0
void AbstractEffectSound::FrameAdvance(float delta_t)
{
    math::Position3 v79;
    v79.v.m128_f32[0] = v79.v.m128_f32[1] = v79.v.m128_f32[2] =
        v79.v.m128_f32[3] = 0.0f;
    math::Position3 v79b;
    v79b.v.m128_f32[0] = v79b.v.m128_f32[1] = v79b.v.m128_f32[2] =
        v79b.v.m128_f32[3] = 0.0f;
    float minRange = -1.0f;
    float maxRange = -1.0f;
    SoundDevice::Sound* mObject =
        SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
    if (mObject != nullptr)
    {
        maxRange = SoundDevice::nslGetWaveParam(mObject->mWave, 26, 1.0f);
        minRange = SoundDevice::nslGetWaveParam(mObject->mWave, 25, 1.0f);
    }
    mDelayCount = mDelayCount + delta_t;
    ++mCountSinceStarted;
    if (mDelayTrigger > mDelayCount)
        return;
    unsigned int v15 = mEntity.mHandle.mVal & 0xFFF;
    if (v15 >= 0x540
        || mEntity.mHandle.mVal >> 12 != EntityHandleDb::sInst.mElements[v15].mKey
        || EntityHandleDb::sInst.mElements[v15].mObject == nullptr)
    {
        return;
    }
    unsigned int mMask = mCodeFlags.mVal;
    if ((mMask & 0x20) != 0 && (mMask & 2) != 0
        && SoundDevice::SoundFromHandle(mSound.mHandle.mVal) == nullptr)
    {
        Entity* Player = GetPlayer(currCl);
        Entity* Owner = AbstractEffectGetOwner(this);
        float v81 = Owner->r.currentOrigin.v.m128_f32[0]
                    - Player->r.currentOrigin.v.m128_f32[0];
        Player = GetPlayer(currCl);
        float v82 = Owner->r.currentOrigin.v.m128_f32[1]
                    - Player->r.currentOrigin.v.m128_f32[1];
        Player = GetPlayer(currCl);
        Owner = AbstractEffectGetOwner(this);
        float dz = Owner->r.currentOrigin.v.m128_f32[2]
                   - Player->r.currentOrigin.v.m128_f32[2];
        if ((maxRange * maxRange) > ((dz * dz) + (v82 * v82) + (v81 * v81)))
        {
            math::Position3* p_mCdPos;
            if ((mMask & 0x40) != 0)
                p_mCdPos = &mCdPos;
            else
            {
                v79b = GetPosition();
                p_mCdPos = &v79b;
            }
            float v80 = p_mCdPos->v.m128_f32[0];
            v81 = p_mCdPos->v.m128_f32[1];
            v82 = p_mCdPos->v.m128_f32[2];
            float v83 = p_mCdPos->v.m128_f32[3];
            unsigned int v20;
            if ((mFlags & 1) != 0 || (mMask & 0x40) == 0)
                v20 = mEntity.mHandle.mVal;
            else
                v20 = 0;
            bool important = (mMask & 8) != 0;
            math::Position3 pos;
            pos.v.m128_f32[0] = v80;
            pos.v.m128_f32[1] = v81;
            pos.v.m128_f32[2] = v82;
            pos.v.m128_f32[3] = v83;
            mSound.mHandle.mVal = SoundDevice_PlaySound(
                mWaveHdl, v20, important, 0, &pos, &v79, mSoundParams.mVolume,
                mFinalPitch, minRange, maxRange);
        }
    }
    if ((mCodeFlags.mVal & 2) != 0)
        goto LABEL_124;
    float v21 = mSoundParams.mVolume;
    if (v21 != -1.0f && (v21 < -0.000001f || mSoundParams.mVolume > 2.0f))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
        AeAssert::gCurrentLine = 236;
        AeAssert::gCurrentExpr = "mSoundParams.mVolume == useNslDefault || "
                                 "(mSoundParams.mVolume >= -0.000001f && "
                                 "mSoundParams.mVolume <= 2.0f)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("unlikely volume"))
            __debugbreak();
    }
    float mPitch = mSoundParams.mPitch;
    if (mPitch != -1.0f && (mPitch < 0.1f || mSoundParams.mPitch >= 4.0f))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
        AeAssert::gCurrentLine = 237;
        AeAssert::gCurrentExpr = "mSoundParams.mPitch == useNslDefault || "
                                 "(mSoundParams.mPitch >= 0.1f && "
                                 "mSoundParams.mPitch < 4.0f)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("unlikely pitch"))
            __debugbreak();
    }
    if (minRange != -1.0f && (minRange < 0.0f || maxRange < minRange))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
        AeAssert::gCurrentLine = 239;
        AeAssert::gCurrentExpr = "minRange == useNslDefault || (minRange >= "
                                 "0.0f && minRange <= maxRange)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("strange min/max ranges"))
            __debugbreak();
    }
    if (AbstractEffectGetOwner(this) == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
        AeAssert::gCurrentLine = 240;
        AeAssert::gCurrentExpr = "GetOwner() != 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("all sounds must have an owner"))
            __debugbreak();
    }
    mFinalPitch = mSoundParams.mPitch;
    if (mWaveHdl == (unsigned int)-1
        || SoundDevice::nslGetWaveParam(mWaveHdl, 0, 1.0f) == 0.0f)
        goto LABEL_69;
    if ((mCodeFlags.mVal & 0x10) != 0)
    {
        math::Position3* p_currentOrigin;
        Entity* Owner = AbstractEffectGetOwner(this);
        if ((mFlags & 1) != 0)
        {
            p_currentOrigin = Owner != nullptr ? &Owner->r.currentOrigin : &v79;
        }
        else
        {
            if ((mCodeFlags.mVal & 0x40) != 0)
                p_currentOrigin = &mCdPos;
            else
                p_currentOrigin =
                    Owner != nullptr ? &Owner->r.currentOrigin : &v79;
        }
        math::Position3 pos;
        pos.v.m128_f32[0] = p_currentOrigin->v.m128_f32[0];
        pos.v.m128_f32[1] = p_currentOrigin->v.m128_f32[1];
        pos.v.m128_f32[2] = p_currentOrigin->v.m128_f32[2];
        pos.v.m128_f32[3] = p_currentOrigin->v.m128_f32[3];
        bool important = (mCodeFlags.mVal & 8) != 0;
        mSound.mHandle.mVal = SoundDevice_QueueSound(
            mWaveHdl, mEntity.mHandle.mVal, important, 0, &pos, &v79,
            mSoundParams.mVolume, mFinalPitch, minRange, maxRange);
        if (SoundDevice::SoundFromHandle(mSound.mHandle.mVal) == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
            AeAssert::gCurrentLine = 269;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Sound '%s' queue failed",
                                    mSoundParams.mNameRef))
                __debugbreak();
        LABEL_69:
            mCodeFlags.mVal |= 2u;
        LABEL_70:
            EffectEventSysStatics::sInst->SendSoundNotify(
                AbstractEffectGetOwner(this));
            if (mSoundParams.mDialogNotify != 0)
                Entity_Notify(AbstractEffectGetOwner(this),
                              mSoundParams.mDialogNotify);
            return;
        }
        SoundDevice::Sound* sound =
            SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
        if (sound->mSource != -1 || (mCodeFlags.mVal & 8) == 0)
            goto LABEL_117;
        mSound.mHandle.mVal = SoundDevice_QueueSound(
            mWaveHdl, mEntity.mHandle.mVal, false, 0, &pos, &v79,
            mSoundParams.mVolume, mFinalPitch, minRange, maxRange);
        if (SoundDevice::SoundFromHandle(mSound.mHandle.mVal)->mSource == -1)
        {
            EffectEventSysStatics::sInst->SendSoundNotify(
                AbstractEffectGetOwner(this));
            if (mSoundParams.mDialogNotify != 0)
                Entity_Notify(AbstractEffectGetOwner(this),
                              mSoundParams.mDialogNotify);
        }
        if (SoundDevice::SoundFromHandle(mSound.mHandle.mVal)->mSource == -1)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
            AeAssert::gCurrentLine = 286;
            AeAssert::gCurrentExpr = "mSound->IsSourceValid()";
            if (!AeAssert::IsIgnored())
            {
                Broc::string::Block* mBlock = mEffectName.mBlock;
                const char* v33 =
                    mBlock ? (const char*)(mBlock + 1) : defaultFileName;
                if (AeAssert::Assert(
                        "Sound '%s' queue failed (important)", v33))
                    __debugbreak();
            }
        }
    }
    else
    {
        EffectEventSysStatics::sInst->IsSoundAlreadyPlaying(
            mEffectNameHashStr,
            EntityHandleDb::sInst
                .mElements[mEntity.mHandle.mVal & 0xFFF]
                .mObject,
            mSoundParams.mMaxVoices);
        math::Position3* v41;
        Entity* Owner = AbstractEffectGetOwner(this);
        bool useCd = (mCodeFlags.mVal & 0x40) != 0;
        if ((mFlags & 1) != 0)
            v41 = Owner != nullptr ? &Owner->r.currentOrigin : &v79;
        else
            v41 = useCd ? &mCdPos
                        : (Owner != nullptr ? &Owner->r.currentOrigin : &v79);
        math::Position3 pos;
        pos.v.m128_f32[0] = v41->v.m128_f32[0];
        pos.v.m128_f32[1] = v41->v.m128_f32[1];
        pos.v.m128_f32[2] = v41->v.m128_f32[2];
        pos.v.m128_f32[3] = v41->v.m128_f32[3];
        unsigned int entVal;
        if ((mFlags & 1) != 0 || !useCd)
            entVal = mEntity.mHandle.mVal;
        else
            entVal = 0;
        bool important = (mCodeFlags.mVal & 8) != 0;
        mSound.mHandle.mVal = SoundDevice_PlaySound(
            mWaveHdl, entVal, important, 0, &pos, &v79, mSoundParams.mVolume,
            mFinalPitch, minRange, maxRange);
        SoundDevice::Sound* sound =
            SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
        if (sound != nullptr)
        {
            if (sound->mSource != -1 || (mCodeFlags.mVal & 8) == 0)
                goto LABEL_117;
            mSound.mHandle.mVal = SoundDevice_PlaySound(
                mWaveHdl, entVal, false, 0, &pos, &v79, mSoundParams.mVolume,
                mFinalPitch, minRange, maxRange);
            if (SoundDevice::SoundFromHandle(mSound.mHandle.mVal)->mSource
                    == -1
                && AbstractEffectGetOwner(this) != nullptr)
            {
                EffectEventSysStatics::sInst->SendSoundNotify(
                    AbstractEffectGetOwner(this));
                if (mSoundParams.mDialogNotify != 0)
                    Entity_Notify(AbstractEffectGetOwner(this),
                                  mSoundParams.mDialogNotify);
            }
            if (SoundDevice::SoundFromHandle(mSound.mHandle.mVal)->mSource
                != -1)
                goto LABEL_117;
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
            AeAssert::gCurrentLine = 342;
            AeAssert::gCurrentExpr = "mSound->IsSourceValid()";
            if (!AeAssert::IsIgnored())
            {
                Broc::string::Block* v44 = mEffectName.mBlock;
                const char* v45 =
                    v44 ? (const char*)(v44 + 1) : defaultFileName;
                if (AeAssert::Assert("Sound '%s' play failed (important)",
                                     v45))
                    __debugbreak();
            }
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AbstractEffect.cpp";
            AeAssert::gCurrentLine = 318;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Sound '%s' play failed",
                                    mSoundParams.mNameRef))
                __debugbreak();
            mCodeFlags.mVal |= 2u;
            if (AbstractEffectGetOwner(this) != nullptr)
                goto LABEL_70;
            return;
        }
    }
LABEL_117:
    if (mSoundParams.mDialogNotify != 0)
    {
        SoundDevice::Sound* s =
            SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
        if (s != nullptr)
            s->mDialogNotify.mHash = mSoundParams.mDialogNotify;
    }
    SoundDevice::Sound* v46 =
        SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
    if (v46 != nullptr && SoundDevice::Sound_IsLooped(v46))
        mCodeFlags.mVal |= 0x20u;
    if (SoundDevice::SoundFromHandle(mSound.mHandle.mVal) != nullptr)
    {
        float mVolScale = SoundDevice_sInst != nullptr
                              ? SoundDevice_sInst->mVolScale
                              : 1.0f;
        float StartingVolume =
            Sound_GetStartingVolume(
                SoundDevice::SoundFromHandle(mSound.mHandle.mVal));
        mVolScale = StartingVolume * mVolScale;
        SoundDevice::Sound_SetVolume(
            SoundDevice::SoundFromHandle(mSound.mHandle.mVal), mVolScale);
    }
    mCodeFlags.mVal |= 2u;
LABEL_124:
    if (mSoundParams.mDuration >= 0.0f)
    {
        float v50 = mSoundParams.mDuration - delta_t;
        mSoundParams.mDuration = v50;
        if (v50 < 0.0f)
        {
            mSoundParams.mDuration = -1.0f;
            StartFadeOut(2.0f);
        }
    }
    SoundDevice::Sound* v52 =
        SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
    if (v52 != nullptr && !SoundDevice::Sound_IsFinished(v52))
    {
        SoundDevice::Sound* v53 =
            SoundDevice::SoundFromHandle(mSound.mHandle.mVal);
        if (SoundDevice::Sound_IsQueued(v53)
            && AbstractEffectGetOwner(this) != nullptr)
        {
            EffectEventSysStatics::sInst->SendSoundNotify(
                AbstractEffectGetOwner(this));
            if (SoundDevice::SoundFromHandle(mSound.mHandle.mVal)
                    ->mDialogNotify.mHash != 0)
            {
                Entity_Notify(
                    AbstractEffectGetOwner(this),
                    SoundDevice::SoundFromHandle(mSound.mHandle.mVal)
                        ->mDialogNotify.mHash);
            }
        }
        if ((mCodeFlags.mVal & 1) != 0)
        {
            mFadeTime = mFadeTime - delta_t;
            if (SoundDevice::SoundFromHandle(mSound.mHandle.mVal) != nullptr)
            {
                float vol = clamp_0_to_1(mFadeTime / mFadeStart)
                            * mSoundParams.mVolume;
                SoundDevice::Sound_SetVolume(
                    SoundDevice::SoundFromHandle(mSound.mHandle.mVal), vol);
            }
        }
        if (mPoPtr != nullptr
            && SoundDevice::SoundFromHandle(mSound.mHandle.mVal) != nullptr)
        {
            SoundDevice::Sound_SetPoPtr(
                SoundDevice::SoundFromHandle(mSound.mHandle.mVal), mPoPtr);
        }
    }
}

// ============================================================================
// EffectEventSys query pipeline
// ============================================================================

// ea: 0x004D1720
void EffectEventSys::BeginEffectQuery(const Entity* ent, TPakId override_pak)
{
    if (mCurrentQuery != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
        AeAssert::gCurrentLine = 548;
        AeAssert::gCurrentExpr = "mCurrentQuery == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("effect query still open"))
            __debugbreak();
    }
    int m_size = mPendingQueries.m_size;
    mCurrentQuery = nullptr;
    if (m_size >= 64)
    {
        int i = 0;
        while (mPendingQueries[i].mCachedQuery.mCONTEXT != 0)
        {
            if (++i >= mPendingQueries.m_size)
                goto LABEL_15;
        }
        if (mPendingQueries.m_size > 1 && i < mPendingQueries.m_size)
        {
            mPendingQueries[i] = mPendingQueries[mPendingQueries.m_size - 1];
        }
        if (mPendingQueries.m_size != 0)
            --mPendingQueries.m_size;
    }
LABEL_15:
    if (mPendingQueries.m_size >= 64)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
        AeAssert::gCurrentLine = 567;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                "max pending queries reached; executing queries now!"))
            __debugbreak();
        ExecutePendingQueries();
        if (mPendingQueries.m_size != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
            AeAssert::gCurrentLine = 569;
            AeAssert::gCurrentExpr = "mPendingQueries.size() == 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "effect query buffer should be clear now"))
                __debugbreak();
        }
    }
    if (mPendingQueries.m_size >= 64)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
        AeAssert::gCurrentLine = 572;
        AeAssert::gCurrentExpr = "mPendingQueries.size() < 64";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("too many effect queries this frame"))
            __debugbreak();
    }
    unsigned int v7 = mPendingQueries.m_size;
    ASSERT_IDX(v7, 64, 154);
    PendingQuery* v8 = &mPendingQueries.m_elements[v7];
    v8->mType = EEffectContextInvalid;
    v8->mCachedQuery.mSpecifiedFields.mBits[0] = 0;
    v8->mCachedQuery.mSpecifiedFields.mBits[1] = 0;
    v8->mCachedQuery.mWeakFields.mBits[0] = 0;
    v8->mCachedQuery.mWeakFields.mBits[1] = 0;
    v8->mQueryEnt.mHandle.mVal = 0;
    v8->mEffect.mVal = 0;
    v8->mBoneIndex = -1;
    v8->mCacheSoundType = -1;
    v8->mQueryType = -1;
    v8->mFlags.mVal = 0;
    v8->mDialogNotify = 0;
    v8->mMatrix = nullptr;
    ++mPendingQueries.m_size;
    mCurrentQuery = v8;
    v8->mQueryEnt.mHandle.mVal = ent->mHandle.mHandle.mVal;
    unsigned int v10 = mCurrentQuery->mQueryEnt.mHandle.mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v10 < 0x540
        && mCurrentQuery->mQueryEnt.mHandle.mVal >> 12
               == EntityHandleDb::sInst.mElements[v10].mKey)
        mObject = EntityHandleDb::sInst.mElements[v10].mObject;
    if (mObject != ent)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
        AeAssert::gCurrentLine = 586;
        AeAssert::gCurrentExpr = "*(mCurrentQuery->mQueryEnt) == ent";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Entity returned incorrect Handle"))
            __debugbreak();
    }
    TPakId mPakId = override_pak;
    if (override_pak == (TPakId)-1)
    {
        mPakId = (TPakId)ent->mPakId;
        if (mPakId == (TPakId)-1)
            mPakId = CurPakId();
    }
    mCurrentQuery->mEffectsPak = mPakId;
    mCurrentQuery->mFlags.mVal &= ~4u;
    mCurrentQuery->mEffect.mVal = 0;
}

// ea: 0x004D1A60
Handle EffectEventSys::ExecEffectQuery()
{
    Handle result;
    if (mCurrentQuery == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
        AeAssert::gCurrentLine = 874;
        AeAssert::gCurrentExpr = "mCurrentQuery";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no active effect query"))
            __debugbreak();
    }
    if (mCurrentQuery == nullptr)
    {
        result.mVal = 0;
        return result;
    }
    unsigned int mVal = mCurrentQuery->mQueryEnt.mHandle.mVal;
    unsigned int v5 = mVal & 0xFFF;
    if (v5 >= 0x540 || mVal >> 12 != EntityHandleDb::sInst.mElements[v5].mKey
        || EntityHandleDb::sInst.mElements[v5].mObject == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
        AeAssert::gCurrentLine = 877;
        AeAssert::gCurrentExpr = "*mCurrentQuery->mQueryEnt";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no Entity associated with this query"))
            __debugbreak();
    }
    unsigned int v6 = mCurrentQuery->mQueryEnt.mHandle.mVal & 0xFFF;
    if (v6 >= 0x540
        || mCurrentQuery->mQueryEnt.mHandle.mVal >> 12
               != EntityHandleDb::sInst.mElements[v6].mKey
        || EntityHandleDb::sInst.mElements[v6].mObject == nullptr)
    {
        if (mPendingQueries.m_size != 0)
            --mPendingQueries.m_size;
        mCurrentQuery = nullptr;
        result.mVal = 0;
        return result;
    }
    if (mEffectSets.m_size == 512)
    {
        result.mVal = 0;
        return result;
    }
    ActiveEffectSet* v9 = (ActiveEffectSet*)ActiveEffectSet_sAllocator->Allocate(
        0x2C, false);
    ActiveEffectSet* v10;
    if (v9 != nullptr)
        v10 = new (v9) ActiveEffectSet(mCurrentQuery->mEffectsPak);
    else
        v10 = nullptr;
    if (v10 != nullptr)
    {
        mCurrentQuery->mEffect.mVal = v10->mId.mVal;
        if (mCurrentQuery->mMatrix != nullptr)
        {
            v10->SetPoPtr(mCurrentQuery->mMatrix);
            v10->mFlags.mVal |= 4u;
        }
        mEffectSets.push_back(v10);
        mCurrentQuery = nullptr;
        if (g_debug_sync_queries != 0)
        {
            ExecPendingQuery(mPendingQueries[mPendingQueries.m_size - 1]);
            --mPendingQueries.m_size;
        }
        result.mVal = v10->mId.mVal;
        return result;
    }
    result.mVal = 0;
    return result;
}

// ea: 0x004D1CB0
Handle EffectEventSys::TriggerNamedEffect(const Entity* ent, const char* name,
                                          TPakId override_pak)
{
    Handle result;
    if (ent != nullptr)
    {
        BeginEffectQuery(ent, override_pak);
        mCurrentQuery->mType = kEffectContextScriptCall;
        CachedQuery* p_mCachedQuery = &mCurrentQuery->mCachedQuery;
        p_mCachedQuery->mSpecifiedFields.mBits[0] |= 1u;
        p_mCachedQuery->mWeakFields.mBits[0] &= ~1u;
        char Destination[128];
        strncpy(Destination, name, 0x7F);
        p_mCachedQuery->mCONTEXT = 3;
        Destination[127] = 0;
        CachedQuery* v8 = &mCurrentQuery->mCachedQuery;
        v8->mSpecifiedFields.mBits[7 >> 3] |= (unsigned char)(1u << (7 & 7));
        v8->mWeakFields.mBits[7 >> 3] &= (unsigned char)~(1u << (7 & 7));
        memcpy(&v8->mSCRIPT_ID, Destination, sizeof(v8->mSCRIPT_ID));
        result = ExecEffectQuery();
        return result;
    }
    result.mVal = 0;
    return result;
}

// ea: 0x004D13C0
void EffectEventSys::ExecPendingQuery(PendingQuery& q)
{
    unsigned int v4 = q.mEffect.mVal & 0x1FF;
    if (v4 < 0x200 && q.mEffect.mVal >> 9 == mHandleDb.mElements[v4].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v4].mObject;
        if (mObject != nullptr)
        {
            unsigned int v6 = q.mQueryEnt.mHandle.mVal & 0xFFF;
            Entity* v7 = nullptr;
            if (v6 < 0x540
                && q.mQueryEnt.mHandle.mVal >> 12
                       == EntityHandleDb::sInst.mElements[v6].mKey)
                v7 = EntityHandleDb::sInst.mElements[v6].mObject;
            if (v7 == nullptr)
            {
                mObject->mFlags.mVal &= ~2u;
                return;
            }
            float distSq = 99999.898f;
            if (dword_F6A290[0] == 2)
            {
                Entity* Player = EntityManager::sInst->GetPlayer(0);
                float v = v7->r.currentOrigin.v.m128_f32[0]
                          - Player->r.currentOrigin.v.m128_f32[0];
                float v14 = v7->r.currentOrigin.v.m128_f32[1]
                            - Player->r.currentOrigin.v.m128_f32[1];
                float dz = v7->r.currentOrigin.v.m128_f32[2]
                           - Player->r.currentOrigin.v.m128_f32[2];
                float d = (dz * dz) + (v14 * v14) + (v * v);
                if (d < 99999.898f)
                    distSq = d;
            }
            int GDEvents;
            if (q.mQueryType == 8)
            {
                Broc::string::Block* mBlock = q.mScriptId.mBlock;
                const char* v11 =
                    mBlock ? (const char*)(mBlock + 1) : defaultFileName;
                GDEvents = QueryGDEvents(v11, q, mObject, 0.0f);
            }
            else
            {
                GDEvents = QueryEventTable(q, mObject, distSq);
                if (GDEvents < 0)
                {
                    HashString v12;
                    v12.mHash = ((SndWait*)&v7->snd_wait)->notifyHash.mHash;
                    if (v12.mHash != 0)
                    {
                        Scr_Notify(v7, v12, 0);
                        ((SndWait*)&v7->snd_wait)->notifyHash.mHash = 0;
                        ((SndWait*)&v7->snd_wait)->soundName.mHash = 0;
                    }
                    mObject->mFlags.mVal &= ~2u;
                    return;
                }
            }
            if (GDEvents != 0)
            {
                mObject->mFlags.mVal &= ~2u;
                return;
            }
            HashString v12;
            v12.mHash = ((SndWait*)&v7->snd_wait)->notifyHash.mHash;
            if (v12.mHash != 0)
            {
                Scr_Notify(v7, v12, 0);
                ((SndWait*)&v7->snd_wait)->notifyHash.mHash = 0;
                ((SndWait*)&v7->snd_wait)->soundName.mHash = 0;
            }
            mObject->mFlags.mVal &= ~2u;
        }
    }
}

// ea: 0x004D1680
void EffectEventSys::ExecutePendingQueries()
{
    unsigned int v3 = 0;
    unsigned int v2 = 0;
    while (v3 < (unsigned int)mPendingQueries.m_size)
    {
        ASSERT_IDX(v2, 64, 154);
        ExecPendingQuery(mPendingQueries.m_elements[v2]);
        ++v3;
        ++v2;
    }
    mPendingQueries.m_size = 0;
}

// ea: 0x004D3930
void EffectEventSys::FrameAdvance(float delta)
{
    if (mCurrentQuery != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
        AeAssert::gCurrentLine = 443;
        AeAssert::gCurrentExpr = "mCurrentQuery == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("effect query still open"))
            __debugbreak();
    }
    AdvanceFades(delta);
    for (unsigned int i = 0; i < (unsigned int)mEffectSets.m_size; ++i)
    {
        ASSERT_IDX(i, 512, 154);
        mEffectSets[i]->FrameAdvance(delta);
        ASSERT_IDX(i, 512, 154);
        ActiveEffectSet* v4 = mEffectSets[i];
        if ((v4->mFlags.mVal & 2) == 0 && v4->mEffects.m_size == 0)
        {
            ActiveEffectSet* v6 = mEffectSets[i];
            if (v6 != nullptr)
            {
                v6->~ActiveEffectSet();
                ActiveEffectSet_sAllocator->Release(v6);
            }
            mEffectSets[i] = mEffectSets[mEffectSets.m_size - 1];
            if (mEffectSets.m_size != 0)
                --mEffectSets.m_size;
            --i;
        }
    }
    ExecutePendingQueries();
}

// ============================================================================
// Debug fx lists
// ============================================================================

// ea: 0x004D3AD0
void ActiveEffectSet::GetDebugFxList(Entity* ent,
                                     std::vector<std::string>* fx)
{
    for (unsigned int v5 = 0; v5 < (unsigned int)mEffects.m_size; ++v5)
    {
        if (ent == nullptr)
            goto LABEL_23;
        ASSERT_IDX(v5, 6, 148);
        unsigned int v6 = mEffects[v5]->mEntity.mHandle.mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v6 < 0x540
            && mEffects[v5]->mEntity.mHandle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v6].mKey)
            mObject = EntityHandleDb::sInst.mElements[v6].mObject;
        if (mObject != ent)
            continue;
    LABEL_23:
        ASSERT_IDX(v5, 6, 148);
        Broc::string v12 = mEffects[v5]->GetDebugString();
        Broc::string::Block* mBlock = v12.mBlock;
        const char* v9 = mBlock ? (const char*)(mBlock + 1) : defaultFileName;
        fx->push_back(std::string(v9));
    }
}

// ea: 0x004D3C70
void EffectEventSys::GetDebugFxList(Entity* ent,
                                    std::vector<std::string>* fx)
{
    for (unsigned int i = 0; i < (unsigned int)mEffectSets.m_size; ++i)
    {
        ASSERT_IDX(i, 512, 148);
        std::vector<std::string> s;
        mEffectSets[i]->GetDebugFxList(ent, &s);
        for (size_t k = 0; k < s.size(); ++k)
            fx->push_back(s[k]);
    }
}
