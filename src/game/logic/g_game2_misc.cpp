// ============================================================================
// g_game2_misc.cpp - game2.o misc functions (default pak, mission stats,
// multiplayer rank helpers, spline util)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <new>
#include <stdio.h>
#include <string.h>

#include "filesystem/apk.h"
#include "game/logic/g_inspector.h"
#include "core/fourcc.h"
#include "core/PoolAllocator.h"

class ScriptEventParams;

static const __m128 Float4_SinCoefs_1_game2 =
    _mm_setr_ps(-0.16666667f, 0.0083333338f, -0.00019841269f, 0.0f);

// ea: 0x00517440
math::Quaternion slerp(const math::Quaternion& q0,
                       const math::Quaternion& q1, float t)
{
    __m128 product = _mm_mul_ps(q0.v, q1.v);
    float dot = product.m128_f32[0]
        + (product.m128_f32[1]
           + (product.m128_f32[2] + product.m128_f32[3]));
    float adjustedDot = dot;
    __m128 weights = _mm_setr_ps(1.0f - t, t, 1.0f, 0.0f);
    if (dot < 0.0f)
    {
        adjustedDot = -dot;
        weights = _mm_setr_ps(1.0f - t, -t, 1.0f, 0.0f);
    }

    if (adjustedDot < 0.99999899f)
    {
        float theta;
        if (adjustedDot >= 0.5f)
        {
            float s = sqrtf((1.0f - adjustedDot) * 0.5f);
            float s2 = s * s;
            float s3 = s2 * s;
            float s5 = s3 * s2;
            theta = ((((s5 * s2) * 0.1079625f)
                      + (s5 * 0.15000001f))
                     + (s3 * 0.33333331f))
                + (s * 2.0f);
        }
        else
        {
            float dot2 = adjustedDot * adjustedDot;
            float dot3 = dot2 * adjustedDot;
            float dot4 = dot2 * dot2;
            theta = ((((dot4 * dot2) * -0.053981241f)
                      - (dot4 * 0.075000003f))
                     - (dot3 * 0.1666667f))
                - adjustedDot + 1.570796f;
        }

        __m128 angle = _mm_mul_ps(weights, _mm_set1_ps(theta));
        __m128 angle2 = _mm_mul_ps(angle, angle);
        __m128 angle3 = _mm_mul_ps(angle2, angle);
        __m128 angle5 = _mm_mul_ps(angle2, angle3);
        __m128 sine = _mm_add_ps(
            _mm_add_ps(
                _mm_add_ps(
                    angle,
                    _mm_mul_ps(
                        _mm_mul_ps(angle2, angle5),
                        _mm_shuffle_ps(Float4_SinCoefs_1_game2,
                                       Float4_SinCoefs_1_game2, 170))),
                _mm_mul_ps(angle5,
                           _mm_shuffle_ps(Float4_SinCoefs_1_game2,
                                          Float4_SinCoefs_1_game2, 85))),
            _mm_mul_ps(angle3,
                       _mm_shuffle_ps(Float4_SinCoefs_1_game2,
                                      Float4_SinCoefs_1_game2, 0)));
        float denominator = sine.m128_f32[2];
        weights = _mm_div_ps(sine, _mm_set1_ps(denominator));
    }

    math::Quaternion result;
    result.v = _mm_add_ps(
        _mm_mul_ps(q0.v, _mm_shuffle_ps(weights, weights, 0)),
        _mm_mul_ps(q1.v, _mm_shuffle_ps(weights, weights, 85)));
    return result;
}

// ea: 0x004EAAC0
FourCC::FourCC(int v)
    : mVal(static_cast<unsigned int>(v))
{
}

// ea: 0x004DE610
SmokeGrenadeInfo::SmokeGrenadeInfo()
    : mEffect(nullptr), mTime(0.0f), bHit{}
{
}

// ea: 0x004DE640
void* SmokeGrenadeMgr::operator new(size_t size, void* p)
{
    (void)size;
    return p;
}

// ============================================================================
// NAL surface used by AnimIK (animation/nal.cpp local views)
// ============================================================================
struct nalGenericBoneHandle {
    unsigned int index;  // +0x00
    void* skeleton;      // +0x04
};
struct nalPositionOrientation {
    math::Quaternion orient;  // +0x00
    math::Position3 pos;      // +0x10

    nalPositionOrientation() {}
};
static_assert(sizeof(nalPositionOrientation) == 0x20,
              "nalPositionOrientation size mismatch");

struct mpAnimJointPosOrientations_t {
    nalPositionOrientation child;  // +0x00
    nalPositionOrientation joint;  // +0x20
    nalPositionOrientation parent; // +0x40

    mpAnimJointPosOrientations_t(); // ea: 0x518CC0
};
static_assert(sizeof(mpAnimJointPosOrientations_t) == 0x60,
              "mpAnimJointPosOrientations_t size mismatch");

struct mpAnimJointBoneHandles_t {
    nalGenericBoneHandle child;  // +0x00
    nalGenericBoneHandle joint;  // +0x08
    nalGenericBoneHandle parent; // +0x10

    mpAnimJointBoneHandles_t(); // ea: 0x518CD0
};
static_assert(sizeof(mpAnimJointBoneHandles_t) == 0x18,
              "mpAnimJointBoneHandles_t size mismatch");

// ea: 0x00518C10
angles_t::angles_t()
{
}

// ea: 0x00518C20
float& angles_t::operator[](int index)
{
    if (index < 0 || index >= 3)
    {
        AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(0);
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\math_angles.h";
        AeAssert::gCurrentLine = 83;
        AeAssert::gCurrentExpr = "( index >= 0 ) && ( index < 3 )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return (&pitch)[index];
}

// ea: 0x00518CA0
angles_t& angles_t::Zero()
{
    pitch = 0.0f;
    yaw = 0.0f;
    roll = 0.0f;
    return *this;
}

// ea: 0x00518CC0
mpAnimJointPosOrientations_t::mpAnimJointPosOrientations_t()
{
}

// ea: 0x00518CD0
mpAnimJointBoneHandles_t::mpAnimJointBoneHandles_t()
{
    child.skeleton = nullptr;
    joint.skeleton = nullptr;
    parent.skeleton = nullptr;
}
namespace nalGeneric {
class nalGenericSkeleton;
class nalGenericPose;
}

// Cross-object externs
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void tlMemFree(void* ptr);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file,
                                 int line);
int default_apk_size = 0x11028;    // ?default_apk_size (game2.o)
// Release XBE bytes at ?default_apk@@3PAEA (IDA 0xDD94C0, 0x11028 bytes).
unsigned char default_apk[0x11028] = {
#include "game/logic/default_apk.inc"
};
unsigned char* default_pak_buf = nullptr;  // ?default_pak_buf (game2.o)
bool gMissionDataInitialized;    // ?gMissionDataInitialized (game2.o)
extern void BrocAddEntityThread(Entity* ent, unsigned int fcnHash,
                                void* params);  // ?BrocAddEntityThread (scr.o)
bool gTotalResetOfLevel;         // ?gTotalResetOfLevel (game2.o)
const char* notSet = "Not Set";         // ?notSet (game2.o)
extern int bg_iNumWeapons;              // ?bg_iNumWeapons (game.o)
extern vmCvar_t g_drawSmokeGren;        // ?g_drawSmokeGren (g.o)

// ============================================================================
// _xmission_data - 0x78 (IDA verified)
// ============================================================================
struct _xmission_data {
    char name[0x20];             // +0x00
    unsigned int weaponsUsed[3]; // +0x20
    int missionTime;             // +0x2C
    int missionLastTick;         // +0x30
    int missionStat[17];         // +0x34
};
static_assert(sizeof(_xmission_data) == 0x78, "_xmission_data size mismatch");

_xmission_data gXMissionData[32];         // ?gXMissionData@@3PAU_xmission_data@@A (game2.o @ 0x11D99F0)
_xmission_data* gMissionData = nullptr;   // ?gMissionData@@3PAU_xmission_data@@A (game2.o @ 0x12F3EC0)
_xmission_data gTempMissionData;          // ?gTempMissionData@@3U_xmission_data@@A (game2.o @ 0x12F2820)

// ============================================================================
// Mission stats enums/types (stat_support.cpp)
// ============================================================================
enum eMissionStats {
    eInvalid = -1,
    eTotalKills = 0,
    eTotalDeaths = 1,
    eGrenadeKillsInterval = 0x10,
    eNumStats = 0x11,
};

enum eWeaponCategory {
    eRiffle = 0,
    eNonGerman = 5,
};

struct _weapon_name {
    char name[0x20];  // +0x00
};

struct _weapon_category {
    _weapon_name* weapons;  // +0x00
};

_weapon_category gWeaponCategories[64];       // ?gWeaponCategories@@3PAU_weapon_category@@A (game2.o @ 0x11DA9F8)

#define STAT_ASSERT(which)                                                  \
    do {                                                                    \
        if ((int)(which) <= (int)eInvalid || (int)(which) >= (int)eNumStats) \
        {                                                                   \
            if (!AeAssert::IsIgnored()                                      \
                && AeAssert::Assert("invalid range access detected!"))      \
                __debugbreak();                                             \
        }                                                                   \
    } while (0)

// ============================================================================
// ScriptEventHandler - script event dispatch list (0x44, IDA verified)
// ============================================================================
struct ScriptEventHandler {
    struct ScriptEvent {
        HashString notify;    // +0x00
        HashString callback;  // +0x04

        ScriptEvent();        // ea: 0x518740
    };

    unsigned char m_dlist_node[8];      // +0x00
    ScriptEvent mEvents[7];             // +0x08
    ScriptEventHandler* mNext;          // +0x40

    ScriptEventHandler();               // ea: 0x4F9860
    static void* operator new(size_t size, bool forceHeapAlloc,
                              const char* file, int line);  // ea: 0x004EAC70
    static void* operator new(size_t size, void* placement)
    {
        (void)size;
        return placement;
    }
    static void operator delete(void* ptr, bool forceHeapAlloc,
                                const char* file, int line); // ea: 0x004EAC90
    static void operator delete(void* ptr);                  // ea: 0x004EACB0
    static void SetAllocator(PoolAllocator* allocator); // core.o 0x4B5570
    ~ScriptEventHandler();              // ea: 0x4F59D0
    bool AddEvent(HashString h, HashString callback);  // ea: 0x4F98D0
    bool AddEvent(HashString h, const char* callback); // ea: 0x4FF780
    bool RemoveEvent(HashString h, HashString callback);  // ea: 0x4F59F0
    bool ExecEvents(Entity* ent, HashString h, ScriptEventParams* params);  // ea: 0x4F5A50
};
using ScriptEvent = ScriptEventHandler::ScriptEvent;
static_assert(sizeof(ScriptEventHandler) == 0x44,
              "ScriptEventHandler size mismatch");

// ea: 0x00518740
ScriptEventHandler::ScriptEvent::ScriptEvent()
{
    notify.mHash = 0;
    callback.mHash = 0;
}

// ============================================================================
// ScriptEventHandler::~ScriptEventHandler - ea: 0x4F59D0
// ============================================================================
ScriptEventHandler::~ScriptEventHandler()
{
    ScriptEventHandler* mNext = this->mNext;
    if (mNext != nullptr)
    {
        mNext->~ScriptEventHandler();
        operator delete(mNext);
    }
}

// ============================================================================
// ScriptEventHandler::RemoveEvent - ea: 0x4F59F0
// ============================================================================
bool ScriptEventHandler::RemoveEvent(HashString h, HashString callback)
{
    ScriptEventHandler* cur = this;
    for (;;)
    {
        int v3 = 0;
        do
        {
            if (cur->mEvents[v3].notify.mHash == h.mHash
                && cur->mEvents[v3].callback.mHash == callback.mHash)
            {
                cur->mEvents[v3].callback.mHash = 0;
                cur->mEvents[v3].notify.mHash = 0;
                return true;
            }
            ++v3;
        } while (v3 < 7);
        if (cur->mNext == nullptr)
            break;
        cur = cur->mNext;
    }
    return false;
}

// ============================================================================
// ScriptEventHandler::ExecEvents - ea: 0x4F5A50
// ============================================================================
bool ScriptEventHandler::ExecEvents(Entity* ent, HashString h,
                                    ScriptEventParams* params)
{
    bool v4 = false;
    for (int i = 7; i != 0; --i)
    {
        if (mEvents[7 - i].notify.mHash == h.mHash)
        {
            BrocAddEntityThread(ent, mEvents[7 - i].callback.mHash, params);
            v4 = true;
        }
    }
    ScriptEventHandler* mNext = this->mNext;
    if (mNext != nullptr)
        return mNext->ExecEvents(ent, h, nullptr) || v4;
    return v4;
}

// ============================================================================
// SegmentSphereIntersection - segment/sphere test
// ea: 0x4F5AC0
// ============================================================================
bool SegmentSphereIntersection(const float* startPoint, const float* endPoint,
                               const float* sphereOrigin, float sphereRadius)
{
    struct DebugColor { float r, g, b, a; };
    extern const float VectorNormalize(float* const v);  // ?VectorNormalize (g.o)

    if (g_drawSmokeGren.integer == 2)
    {
        DebugColor col = { 1.0f, 1.0f, 0.0f, 1.0f };
        math::Position3 p1;
        math::Position3 p2;
        p1.v.m128_f32[0] = startPoint[0];
        p1.v.m128_f32[1] = startPoint[1];
        p1.v.m128_f32[2] = startPoint[2];
        p2.v.m128_f32[0] = endPoint[0];
        p2.v.m128_f32[1] = endPoint[1];
        p2.v.m128_f32[2] = endPoint[2];
        DebugRender::RenderLine(p1, p2,
                                Color(col.r, col.g, col.b, col.a), 5.0f);
    }
    float segDir[3];
    segDir[0] = endPoint[0] - startPoint[0];
    segDir[1] = endPoint[1] - startPoint[1];
    segDir[2] = endPoint[2] - startPoint[2];
    float rel[3] = {
        startPoint[0] - sphereOrigin[0],
        startPoint[1] - sphereOrigin[1],
        startPoint[2] - sphereOrigin[2],
    };
    float segLen = VectorNormalize(segDir);
    float v5 = (segDir[2] * segDir[2]) + (segDir[1] * segDir[1])
        + (segDir[0] * segDir[0]);
    float v6 = ((rel[2] * segDir[2]) + (rel[1] * segDir[1])
                + (rel[0] * segDir[0])) * 2.0f;
    float v7 = ((((rel[2] * rel[2]) + (rel[1] * rel[1])
                  + (rel[0] * rel[0])) - (sphereRadius * sphereRadius))
                * v5) * 4.0f;
    float v8 = (v6 * v6) - v7;
    if (v8 < 0.0f)
        return false;
    float inv = 1597463007.0f - ((float)((int)v8) * 0.5f);
    float t = ((0.0f - v6) - ((1.5f - (((v8 * 0.5f) * inv) * inv)) * inv))
        / (v5 * 2.0f);
    return t > 0.0f && segLen > t;
}

// ============================================================================
// nalMatrix4x4_to_Axis4 - copy the first three components of each row
// ea: 0x4F5ED0
// ============================================================================
struct nalMatrix4x4 {
    float x[4];  // rows
    float y[4];
    float z[4];
    float w[4];

};

static void nalMatrix4x4_FromPositionOrientation(
    const nalPositionOrientation& po, nalMatrix4x4* mat)
{
    const float qx = po.orient.x;
    const float qy = po.orient.y;
    const float qz = po.orient.z;
    const float qw = po.orient.w;
    const float xx = qx * qx;
    const float yy = qy * qy;
    const float zz = qz * qz;
    const float xy = qx * qy;
    const float xz = qx * qz;
    const float yz = qy * qz;
    const float wx = qw * qx;
    const float wy = qw * qy;
    const float wz = qw * qz;
    mat->x[0] = 1.0f - (yy + zz);
    mat->x[1] = xy + wz;
    mat->x[2] = xz - wy;
    mat->x[3] = 0.0f;
    mat->y[0] = xy - wz;
    mat->y[1] = 1.0f - (xx + zz);
    mat->y[2] = yz + wx;
    mat->y[3] = 0.0f;
    mat->z[0] = xz + wy;
    mat->z[1] = yz - wx;
    mat->z[2] = 1.0f - (xx + yy);
    mat->z[3] = 0.0f;
    mat->w[0] = po.pos.v.m128_f32[0];
    mat->w[1] = po.pos.v.m128_f32[1];
    mat->w[2] = po.pos.v.m128_f32[2];
    mat->w[3] = 1.0f;
}

void nalMatrix4x4_to_Axis4(nalMatrix4x4* mat, float (*axis)[3])
{
    const float* rows = &mat->x[0];
    for (int i = 0; i < 4; ++i)
    {
        axis[i][0] = rows[i * 4 + 0];
        axis[i][1] = rows[i * 4 + 1];
        axis[i][2] = rows[i * 4 + 2];
    }
}

// ============================================================================
// AnimIK - IK state (0x7C, IDA verified; ctor/dtor only here)
// ============================================================================
class AnimIK {
public:
    static float painDurationMin;    // ?painDurationMin@AnimIK@@2MA
    static float painDurationMax;    // ?painDurationMax@AnimIK@@2MA
    float ikJoints[0x50 / 4];       // +0x00 AnimIKJointVars_t[4]
    int initialized;                // +0x50
    void* pose;                     // +0x54
    void* skeleton;                 // +0x58
    unsigned char _pad5C[0x60 - 0x5C];  // +0x5C
    float mFrametime;               // +0x60
    unsigned char _pad64[0x7C - 0x64];  // +0x64

    AnimIK();  // ea: 0x4F60A0
    ~AnimIK();  // ea: 0x4F60B0
    void GetGunAndHandMatrix(Entity* ent, nalGenericBoneHandle* gunHandle,
                             nalGenericBoneHandle* handHandle,
                             nalMatrix4x4* gunMat,
                             nalMatrix4x4* handMat);   // ea: 0x4F6270
    void GetFootMatrices(nalMatrix4x4* leftFootMat,
                         nalMatrix4x4* rightFootMat);  // ea: 0x4F62A0
    float WeaponRecoilTimeScale(float fireTime, float duration,
                                float force);          // ea: 0x4F6340
    void ApplyLadderClimb(Entity* ent, nalMatrix4x4* leftFootMat,
                          nalMatrix4x4* rightFootMat); // ea: 0x4F64C0
    void Initialize();                                 // ea: 0x4FAE50
    void UpdateGunMatrix(nalGenericBoneHandle gunHandle,
                         nalGenericBoneHandle handHandle,
                         nalMatrix4x4* gunMat,
                         nalMatrix4x4* handMat);        // ea: 0x4FB150
    void ApplyFootIK(Entity* ent, nalMatrix4x4* leftFootMat,
                     nalMatrix4x4* rightFootMat);       // ea: 0x4FB510
    void ApplyHandIK(Entity* ent, nalMatrix4x4* leftMat,
                     nalMatrix4x4* rightMat);           // ea: 0x4FBD10
    void RotateBone(int boneIndex, const math::Dir3* rotation);  // ea: 0x4FC5B0
    void ApplyPainFlinch(Entity* ent);                  // ea: 0x4FCA40
    void ApplyTorsoRotations(Entity* ent);              // ea: 0x4FDDC0
    void ApplyFire(Entity* ent);                        // ea: 0x4FFC30
    void ApplyVehicleSteering(Entity* ent);             // ea: 0x504C90
    void ApplyADS(Entity* ent);                         // ea: 0x5068F0
    void ApplyTerrainMapping(Entity* ent,
                             nalMatrix4x4* leftFootMat,
                             nalMatrix4x4* rightFootMat);  // ea: 0x507EC0
    void Update(Entity* ent, nalGeneric::nalGenericSkeleton* inSkeleton,
                nalGeneric::nalGenericPose* inPose);   // ea: 0x50BBC0
};
static_assert(sizeof(AnimIK) == 0x7C, "AnimIK size mismatch");

float AnimIK::painDurationMin = 0.0f;
float AnimIK::painDurationMax = 0.0f;

// ?AnimIKGlobal@@3VAnimIK@@A (game2.o data @ 0xF05660)
AnimIK AnimIKGlobal;

// ea: 0x4F60A0
AnimIK::AnimIK()
{
    initialized = 0;
    pose = nullptr;
    skeleton = nullptr;
}

// ea: 0x4F60B0
AnimIK::~AnimIK()
{
}

// nalGeneric surface (animation/nal.cpp)
extern void nalGenericSkeleton_GetBoneHandle(void* skeleton,
                                            nalGenericBoneHandle* handle,
                                            const tlFixedString* boneName);
extern nalPositionOrientation nalGenericPose_GetModelPositionOrientation(
    void* pose, const nalGenericBoneHandle* handle);  // ?GetModelPositionOrientation@nalGenericPose@nalGeneric@@QBE?BVnalPositionOrientation@@ABVnalGenericBoneHandle@2@@Z
extern void Axis4_to_nalMatrix4x4(const float (*axis)[3],
                                  nalMatrix4x4* mat);  // ea: 0x4F6220
tlFixedString boneName[8];               // ?boneName (game2.o @ 0xF052B8)
tlFixedString stru_F05318;               // ?stru_F05318 (game2.o bone name)
tlFixedString stru_F05378;               // ?stru_F05378 (game2.o bone name)
tlFixedString stru_F05398;               // ?stru_F05398 (game2.o bone name)
tlFixedString stru_F05238;               // ?stru_F05238 (game2.o bone name)
cvar_t* ik_ADS;                          // ?ik_ADS@@3PAUcvar_t@@A (game2.o)

// ?IKenabled@@3_NA (game2.o data @ 0xDEF463, value 1)
bool IKenabled = true;

// ============================================================================
// AnimIK::GetGunAndHandMatrix - ea: 0x4F6270
// ============================================================================
void AnimIK::GetGunAndHandMatrix(Entity* ent,
                                 nalGenericBoneHandle* gunHandle,
                                 nalGenericBoneHandle* handHandle,
                                 nalMatrix4x4* gunMat,
                                 nalMatrix4x4* handMat)
{
    nalGenericSkeleton_GetBoneHandle(skeleton, handHandle, &boneName[0]);
    Axis4_to_nalMatrix4x4(ent->sentient->mLastAnimIKGunOffset, gunMat);
}

// ============================================================================
// AnimIK::GetFootMatrices - ea: 0x4F62A0
// ============================================================================
void AnimIK::GetFootMatrices(nalMatrix4x4* leftFootMat,
                             nalMatrix4x4* rightFootMat)
{
    nalGenericBoneHandle leftFootHandle;
    nalGenericBoneHandle rightFootHandle;
    leftFootHandle.index = 0;
    leftFootHandle.skeleton = nullptr;
    rightFootHandle.index = 0;
    rightFootHandle.skeleton = nullptr;
    nalGenericSkeleton_GetBoneHandle(skeleton, &leftFootHandle,
                                     &stru_F05318);
    nalGenericSkeleton_GetBoneHandle(skeleton, &rightFootHandle,
                                     &stru_F05378);
    nalPositionOrientation po =
        nalGenericPose_GetModelPositionOrientation(pose, &leftFootHandle);
    nalMatrix4x4 leftMatrix;
    nalMatrix4x4_FromPositionOrientation(po, &leftMatrix);
    *leftFootMat = leftMatrix;
    nalPositionOrientation po2 =
        nalGenericPose_GetModelPositionOrientation(pose, &rightFootHandle);
    nalMatrix4x4 rightMatrix;
    nalMatrix4x4_FromPositionOrientation(po2, &rightMatrix);
    *rightFootMat = rightMatrix;
}

// ============================================================================
// AnimIK::WeaponRecoilTimeScale - ea: 0x4F6340
// ============================================================================
float AnimIK::WeaponRecoilTimeScale(float fireTime, float duration,
                                    float force)
{
    float recoilScale = (level.time - fireTime) / (duration * 1000.0f);
    if (recoilScale > 1.0f || recoilScale < 0.0f)
        return 0.0f;
    float v = powf(recoilScale, 1.0f / force);
    if (v > 0.5f)
        v = 0.5f - (v - 0.5f);
    return v + v;
}

// ============================================================================
// AnimIK::ApplyLadderClimb - ea: 0x4F64C0
// ============================================================================
void AnimIK::ApplyLadderClimb(Entity* ent, nalMatrix4x4* leftFootMat,
                              nalMatrix4x4* rightFootMat)
{
}

// ============================================================================
// AnimIK::Initialize - ea: 0x4FAE50
// Fetch joint bone handles + precompute IK arm lengths.
// ============================================================================
// These are the 12 entries consumed by Initialize from the reference's
// contiguous static bone-name block (F04A40..F04C00).
const tlFixedString AnimIK_InitializeBoneNames[12] = {
    tlFixedString("bip01 l forearm"),
    tlFixedString("bip01 l upperarm"),
    tlFixedString("bip01 l clavicle"),
    tlFixedString("bip01 r forearm"),
    tlFixedString("bip01 r upperarm"),
    tlFixedString("bip01 r clavicle"),
    tlFixedString("bip01 l calf"),
    tlFixedString("bip01 l thigh"),
    tlFixedString("bip01 pelvis"),
    tlFixedString("bip01 r calf"),
    tlFixedString("bip01 r thigh"),
    tlFixedString("bip01 pelvis")
};
const tlFixedString AnimIK_ParentNames[8] = {};   // ?AnimIK_ParentNames (game2.o @ 0xF05318+)

void AnimIK::Initialize()
{
    nalGenericBoneHandle handles[12];
    for (int i = 0; i < 12; ++i)
    {
        handles[i].index = 0;
        handles[i].skeleton = nullptr;
    }
    ik_ADS = Cvar_Get("ik_ADS", "0", 512);
    // Joints 0..3: for each, resolve parent/joint/child bone handles from
    // the reference's four contiguous triplets, then compute the lengths.
    for (int j = 0; j < 4; ++j)
    {
        nalGenericBoneHandle* h = &handles[j * 3];
        for (int k = 0; k < 3; ++k)
            nalGenericSkeleton_GetBoneHandle(skeleton, &h[k],
                                             &AnimIK_InitializeBoneNames[j * 3 + k]);
        nalPositionOrientation a =
            nalGenericPose_GetModelPositionOrientation(pose, &h[0]);
        nalPositionOrientation b =
            nalGenericPose_GetModelPositionOrientation(pose, &h[1]);
        nalPositionOrientation c =
            nalGenericPose_GetModelPositionOrientation(pose, &h[2]);
        float* ik = ikJoints + j * 5;
        float dx = b.pos.v.m128_f32[0] - a.pos.v.m128_f32[0];
        float dy = b.pos.v.m128_f32[1] - a.pos.v.m128_f32[1];
        float dz = b.pos.v.m128_f32[2] - a.pos.v.m128_f32[2];
        float upper = sqrtf(dx * dx + dy * dy + dz * dz);
        float dx2 = c.pos.v.m128_f32[0] - b.pos.v.m128_f32[0];
        float dy2 = c.pos.v.m128_f32[1] - b.pos.v.m128_f32[1];
        float dz2 = c.pos.v.m128_f32[2] - b.pos.v.m128_f32[2];
        float lower = sqrtf(dx2 * dx2 + dy2 * dy2 + dz2 * dz2);
        ik[0] = upper;
        ik[1] = 1.0f / (upper * 2.0f);
        ik[2] = (upper * upper - lower * lower) / (upper * 2.0f);
        ik[3] = 1.0f / (upper * 2.0f);
        ik[4] = (lower * lower - upper * upper) / (upper * 2.0f);
    }
    initialized = 1;
}

// ============================================================================
// AnimIK::Update - ea: 0x50BBC0
// Main IK entry: refresh state, then run the apply passes.
// ============================================================================
extern float AnimIK_painFlinchAngle;      // game2.o
extern float AnimIK_painLowBlowPelvisShift[2];  // game2.o
extern float AnimIK_painLowBlowPelvisPow; // game2.o
extern float AnimIK_painHighBlowPelvisShift[2]; // game2.o
extern float AnimIK_painHighBlowPelvisPow;      // game2.o
extern float AnimIK_painDurationMin;      // game2.o
extern float AnimIK_painDurationMax;      // game2.o
extern float AnimIK_painAmpMin;           // game2.o
extern float AnimIK_painAmpMax;           // game2.o

void AnimIK::Update(Entity* ent, nalGeneric::nalGenericSkeleton* inSkeleton,
                    nalGeneric::nalGenericPose* inPose)
{
    pose = inPose;
    skeleton = inSkeleton;
    if (IKenabled && ent != nullptr && ent->sentient != nullptr)
    {
        if (initialized == 0)
            Initialize();
        int mLastAnimIKUpdate = ent->sentient->mLastAnimIKUpdate;
        mFrametime = mLastAnimIKUpdate != 0
            ? (level.time - mLastAnimIKUpdate) * 0.001f
            : 0.0f;
        ent->sentient->mLastAnimIKUpdate = level.time;
    }
}

// Heavy IK apply passes (SEH-heavy, need nal matrix/pose machinery) - stubs
void AnimIK::UpdateGunMatrix(nalGenericBoneHandle gunHandle,
                             nalGenericBoneHandle handHandle,
                             nalMatrix4x4* gunMat, nalMatrix4x4* handMat)
{
}
void AnimIK::ApplyFootIK(Entity* ent, nalMatrix4x4* leftFootMat,
                         nalMatrix4x4* rightFootMat)
{
}
void AnimIK::ApplyHandIK(Entity* ent, nalMatrix4x4* leftMat,
                         nalMatrix4x4* rightMat)
{
}
void AnimIK::RotateBone(int boneIndex, const math::Dir3* rotation)
{
}
void AnimIK::ApplyPainFlinch(Entity* ent)
{
}
void AnimIK::ApplyTorsoRotations(Entity* ent)
{
}
void AnimIK::ApplyFire(Entity* ent)
{
}
void AnimIK::ApplyVehicleSteering(Entity* ent)
{
}
void AnimIK::ApplyADS(Entity* ent)
{
}
void AnimIK::ApplyTerrainMapping(Entity* ent, nalMatrix4x4* leftFootMat,
                                 nalMatrix4x4* rightFootMat)
{
}

// ============================================================================
// InitDefaultPak - load the embedded default pak archive
// ea: 0x4F6040
// ============================================================================
void InitDefaultPak()
{
    unsigned char* v0 =
        (unsigned char*)tlMemAlloc(default_apk_size, 0x1000u, 0x10000);
    memcpy(v0, default_apk, default_apk_size);
    default_pak_buf = v0;
    apk::apkLoadFileInPlace(v0, true);
    MemoryBarrier();
}

// ============================================================================
// FiniDefaultPak - free the default pak buffer
// ea: 0x4F6090
// ============================================================================
void FiniDefaultPak()
{
    tlMemFree(default_pak_buf);
}

// ============================================================================
// stat_support_Shutdown - clear mission data init flag
// ea: 0x4F64D0
// ============================================================================
void stat_support_Shutdown()
{
    gMissionDataInitialized = false;
}

// ============================================================================
// stat_CommitStatsToLevel - merge temp mission stats into the mission data
// ea: 0x4F64E0
// ============================================================================
void stat_CommitStatsToLevel()
{
    if (gMissionDataInitialized)
    {
        _xmission_data* v1 = gMissionData;
        if (gMissionData != nullptr)
        {
            int* missionStat = gTempMissionData.missionStat;
            int v3 = 52;
            for (;;)
            {
                *(int*)((char*)v1->name + v3) += *missionStat;
                *missionStat = 0;
                v3 += 4;
                ++missionStat;
                if (v3 >= 120)
                    break;
                v1 = gMissionData;
            }
            gMissionData->weaponsUsed[0] |= gTempMissionData.weaponsUsed[0];
            gMissionData->weaponsUsed[1] |= gTempMissionData.weaponsUsed[1];
            gMissionData->weaponsUsed[2] |= gTempMissionData.weaponsUsed[2];
        }
    }
}

// ============================================================================
// stat_ResetMissionStats - clear temp mission stats (optionally the totals)
// ea: 0x4F6560
// ============================================================================
void stat_ResetMissionStats(bool total)
{
    memset(&gTempMissionData, 0, sizeof(gTempMissionData));
    if (total && gMissionData != nullptr)
    {
        gMissionData->missionTime = 0;
        gMissionData->missionLastTick = 0;
        memset(gMissionData->missionStat, 0, sizeof(gMissionData->missionStat));
        gMissionData->weaponsUsed[0] = 0;
        gMissionData->weaponsUsed[1] = 0;
        gMissionData->weaponsUsed[2] = 0;
    }
}

// ============================================================================
// stat_LoadMissionStatsFrom - load mission stats from an xmission data array
// ea: 0x4F65C0
// ============================================================================
void stat_LoadMissionStatsFrom(_xmission_data* data, int num_missions)
{
    _xmission_data* table = gXMissionData;
    while (num_missions > 0)
    {
        if (_stricmp(table->name, data->name) == 0)
        {
            table->missionTime = data->missionTime;
            table->missionLastTick = data->missionLastTick;
            memcpy(table->missionStat, data->missionStat, 0x44u);
            table->weaponsUsed[0] = data->weaponsUsed[0];
            table->weaponsUsed[1] = data->weaponsUsed[1];
            table->weaponsUsed[2] = data->weaponsUsed[2];
        }
        ++table;
        ++data;
        --num_missions;
    }
}

// ea: 0x4F6640
void stat_ClearMissionStats()
{
    gTotalResetOfLevel = true;
}

// ea: 0x4F6650
_xmission_data* stat_GetLevelName()
{
    if (gMissionData == nullptr)
        return (_xmission_data*)notSet;
    return gMissionData;
}

// ea: 0x4F6660
int stat_GetStat(eMissionStats which, bool allLevels, bool andCommited)
{
    STAT_ASSERT(which);
    if (!gMissionDataInitialized)
        return 0;
    int result = 0;
    if (gMissionData != nullptr)
    {
        if (allLevels)
        {
            _xmission_data* v4 = gXMissionData;
            for (;;)
            {
                result += v4->missionStat[which];
                if (v4 == gMissionData)
                    break;
                ++v4;
                if (v4 == nullptr)
                    return result;
            }
            result += gTempMissionData.missionStat[which];
        }
        else
        {
            result = gTempMissionData.missionStat[which];
            if (andCommited)
                result += gMissionData->missionStat[which];
        }
    }
    return result;
}

// ea: 0x4F6710
void stat_SetStat(eMissionStats which, int stat, _xmission_data* xd)
{
    STAT_ASSERT(which);
    if (xd != nullptr)
        xd->missionStat[which] = stat;
    else
        gTempMissionData.missionStat[which] = stat;
}

// ea: 0x4F6790
void stat_IncStat(eMissionStats which, _xmission_data* xd)
{
    STAT_ASSERT(which);
    if (xd != nullptr)
        ++xd->missionStat[which];
    else
        ++gTempMissionData.missionStat[which];
}

// ea: 0x4F6800
void stat_DecStat(eMissionStats which, _xmission_data* xd)
{
    STAT_ASSERT(which);
    if (xd != nullptr)
    {
        --xd->missionStat[which];
        if (xd->missionStat[which] < 0)
            xd->missionStat[which] = 0;
    }
    else
    {
        --gTempMissionData.missionStat[which];
        if (gTempMissionData.missionStat[which] < 0)
            gTempMissionData.missionStat[which] = 0;
    }
}

// ea: 0x4F6890
float stat_GetAvgStat(unsigned int which)
{
    if (which > 0x10u)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\stat_support.cpp";
        AeAssert::gCurrentLine = 366;
        AeAssert::gCurrentExpr = "which > eInvalid && which < eNumStats";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("invalid range access detected!"))
        {
            __debugbreak();
        }
    }
    if (!gMissionDataInitialized || gMissionData == nullptr)
        return 0.0;
    int v2 = 0;
    _xmission_data* v3 = gXMissionData;
    int v4 = 0;
    for (;;)
    {
        ++v4;
        v2 += gMissionData->missionStat[which];
        if (v3 == gMissionData)
            break;
        ++v3;
        if (v3 == nullptr)
            return 0.0;
    }
    return (float)v2 / (float)v4;
}

// ea: 0x4F6950
void stat_GetWeaponMasks(int* m1, int* m2, int* m3)
{
    if (m1 != nullptr && m2 != nullptr && m3 != nullptr)
    {
        *m3 = 0;
        *m2 = 0;
        *m1 = 0;
        if (gMissionDataInitialized && gMissionData != nullptr)
        {
            *m1 = gTempMissionData.weaponsUsed[0] | gMissionData->weaponsUsed[0];
            *m2 = gTempMissionData.weaponsUsed[1] | gMissionData->weaponsUsed[1];
            *m3 = gTempMissionData.weaponsUsed[2] | gMissionData->weaponsUsed[2];
        }
    }
}

// ea: 0x4F69D0
int stat_GetMissionCompletionTime()
{
    if (gMissionDataInitialized && gMissionData != nullptr)
        return gMissionData->missionTime;
    return 0;
}

// ea: 0x4F69F0
void stat_UpdateMissionCompletionTime()
{
    if (gMissionDataInitialized && gMissionData != nullptr)
    {
        unsigned long long v1 = __rdtsc();
        int v2 = (int)(v1 - gMissionData->missionLastTick);
        gMissionData->missionLastTick = (int)__rdtsc();
        gMissionData->missionTime += v2;
        gMissionData->missionTime = gMissionData->missionTime / 1000;
    }
}

// ea: 0x4F6A70
bool stat_WasPlayerWeaponUsed(int weaponHash, int* bitSetArray)
{
    if (!gMissionDataInitialized || gMissionData == nullptr)
        return 0;
    int v3 = 0;
    if (bg_iNumWeapons <= 0)
        return 0;
    for (;;)
    {
        weaponFileInfo_t* info = BG_GetInfoForWeapon(v3);
        if (info != nullptr && info->internalNameHash == (unsigned int)weaponHash)
            break;
        ++v3;
        if (v3 >= bg_iNumWeapons)
            return 0;
    }
    if (v3 == -1)
        return 0;
    int v5 = v3 >> 5;
    int v6 = 1 << (v3 % 32);
    if ((v6 & gMissionData->weaponsUsed[v3 >> 5]) == 0
        && (v6 & gTempMissionData.weaponsUsed[v5]) == 0)
        return 0;
    if (bitSetArray != nullptr)
        bitSetArray[v5] |= v6;
    return 1;
}

// ea: 0x4F6B10
void stat_FillMask(eWeaponCategory category, int* i1, int* i2, int* i3)
{
    *i1 = 0;
    *i2 = 0;
    *i3 = 0;
    int bitSet = 0;
    int v10 = 0;
    int v11 = 0;
    if (category <= eNonGerman)
    {
        _weapon_name* weapons = gWeaponCategories[category].weapons;
        if (weapons->name[0] != 0)
        {
            do
            {
                stat_WasPlayerWeaponUsed((int)HashString::CalcHash(weapons->name),
                                         &bitSet);
                char v6 = weapons[1].name[0];
                ++weapons;
                if (v6 == 0)
                    break;
            } while (true);
        }
        *i1 = bitSet;
        *i2 = v10;
        *i3 = v11;
    }
}

// ea: 0x4F6B90 / 0x4F6C10
bool stat_WasPlayerWeaponCategoryUsed(eWeaponCategory category, bool exclusive)
{
    if (!gMissionDataInitialized || gMissionData == nullptr)
        return false;
    int mask1, mask2, mask3;
    stat_FillMask(category, &mask1, &mask2, &mask3);
    if ((mask3 | mask2 | mask1) == 0)
        return false;
    if (!exclusive)
        return true;
    if (category < 4)
    {
        for (int v3 = eRiffle; v3 != 4; ++v3)
        {
            if (v3 != (int)category)
            {
                stat_FillMask((eWeaponCategory)v3, &mask1, &mask2, &mask3);
                if ((mask3 | mask2 | mask1) != 0)
                    return false;
            }
        }
        return true;
    }
    if (category == 4)
    {
        stat_FillMask(eNonGerman, &mask3, &mask2, &mask1);
        return (mask1 | mask2 | mask3) == 0;
    }
    if (category == eNonGerman)
    {
        stat_FillMask((eWeaponCategory)4, &mask3, &mask2, &mask1);
        return (mask1 | mask2 | mask3) == 0;
    }
    return false;
}

// ea: 0x4F6C80
void stat_SetPlayerWeaponUsed(int weapon)
{
    if (gMissionDataInitialized && gMissionData != nullptr)
        gTempMissionData.weaponsUsed[weapon >> 5] |= 1 << (weapon % 32);
}

// ea: 0x4FE870
void stat_support_Initialize()
{
    if (gXMissionData[0].name[0] != 0)
    {
        _xmission_data* table = gXMissionData;
        do
        {
            table->missionTime = 0;
            table->missionLastTick = 0;
            memset(table->missionStat, 0, 0x44u);
            table->weaponsUsed[0] = 0;
            table->weaponsUsed[1] = 0;
            table->weaponsUsed[2] = 0;
            ++table;
        } while (table->name[0] != 0);
    }
    memset(&gTempMissionData, 0, sizeof(gTempMissionData));
    gTotalResetOfLevel = false;
    gMissionDataInitialized = true;
}

// ea: 0x4FE8D0
bool stat_SetMissionToTrack(const char* mission_name)
{
    gMissionData = nullptr;
    _xmission_data* v1 = gXMissionData;
    if (mission_name == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\stat_support.cpp";
        AeAssert::gCurrentLine = 256;
        AeAssert::gCurrentExpr = "mission_name";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("mission name must be set"))
            __debugbreak();
    }
    if (!gMissionDataInitialized || gXMissionData[0].name[0] == 0)
        return 0;
    while (_stricmp(v1->name, mission_name) != 0)
    {
        ++v1;
        if (v1->name[0] == 0)
            return 0;
    }
    gMissionData = v1;
    stat_ResetMissionStats(gTotalResetOfLevel);
    gTotalResetOfLevel = false;
    if (v1 != nullptr)
        ++v1->missionStat[0];
    else
        ++gTempMissionData.missionStat[0];
    return 1;
}

// ============================================================================
// ButtonEntry - key binding entry (0xC, IDA verified)
// ============================================================================
class BaseCmdFuncInfo;

struct ButtonEntry {
    unsigned char mKeyInfoIndex;         // +0x00
    unsigned char _pad1[3];
    const BaseCmdFuncInfo* mBoundCmdPress;   // +0x04
    const BaseCmdFuncInfo* mBoundCmdRelease; // +0x08

    void SetCmdBinding();                    // ea: 0x4F6CC0
    void Init();                              // game2.o 0x004EB440
    unsigned char GetKeyInfoIndex() const;    // game2.o 0x004EB450
    const BaseCmdFuncInfo* GetBoundCmdPress();  // ea: 0x4F6D50
    const BaseCmdFuncInfo* GetBoundCmdRelease();  // ea: 0x4F6D80
    void SetKeyBinding(unsigned char keyInfoIndex);  // ea: 0x4FEAA0
    void Press(bool doCommands);             // ea: 0x4FEAD0
    void Release(bool doCommands);           // ea: 0x4FEB60
};
static_assert(sizeof(ButtonEntry) == 0xC, "ButtonEntry size mismatch");

void ButtonEntry::Init()
{
    mKeyInfoIndex = (unsigned char)-1;
    mBoundCmdPress = nullptr;
    mBoundCmdRelease = nullptr;
}

unsigned char ButtonEntry::GetKeyInfoIndex() const
{
    return mKeyInfoIndex;
}

struct KeyInfoEntry {
    int mState;              // +0x00 (bitfields mDown/mRepeats)
    char* mBoundCmdName;     // +0x04

    char* GetBoundCmdName(); // game2.o 0x004EA9C0
};
static_assert(sizeof(KeyInfoEntry) == 8, "KeyInfoEntry size mismatch");

// ea: 0x004EA9C0
char* KeyInfoEntry::GetBoundCmdName()
{
    return mBoundCmdName;
}

extern const BaseCmdFuncInfo* GetCmd(const char* cmdName);  // ?GetCmd (core.o)
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);

// KeyInfo::mKeys - release uses the sized-array object, not the old null
// gKeyInfoMKeys placeholder.  The matching definition is in cl_field.cpp.
struct KeyInfoEntry2 {
    int mState;
    char* mBoundCmdName;
};
template <typename T, int N>
struct ae_array_fixed {
    T m_elements[N];
};
struct KeyInfo {
    static ae_array_fixed<ae_array_fixed<KeyInfoEntry2, 256>, 1> mKeys;
};

// ea: 0x4F6CC0
void ButtonEntry::SetCmdBinding()
{
    int mKeyInfoIndex = this->mKeyInfoIndex;
    mBoundCmdPress = nullptr;
    mBoundCmdRelease = nullptr;
    KeyInfoEntry2* v4 =
        &KeyInfo::mKeys.m_elements[currCl].m_elements[mKeyInfoIndex];
    char* mBoundCmdName = v4->mBoundCmdName;
    if (mBoundCmdName != nullptr)
    {
        mBoundCmdPress = GetCmd(v4->mBoundCmdName);
        if (*mBoundCmdName == '+')
        {
            char releaseName[512];
            Com_sprintf(releaseName, 512, "-%s", mBoundCmdName + 1);
            mBoundCmdRelease = GetCmd(releaseName);
        }
    }
}

// ea: 0x4F6D50
const BaseCmdFuncInfo* ButtonEntry::GetBoundCmdPress()
{
    if (mBoundCmdPress == nullptr)
    {
        if (mKeyInfoIndex == 0xFF)
            return mBoundCmdPress;
    }
    else if (mKeyInfoIndex != 0xFF)
    {
        return mBoundCmdPress;
    }
    SetCmdBinding();
    return mBoundCmdPress;
}

// ea: 0x4F6D80
const BaseCmdFuncInfo* ButtonEntry::GetBoundCmdRelease()
{
    if (mBoundCmdRelease == nullptr)
    {
        if (mKeyInfoIndex == 0xFF)
            return mBoundCmdRelease;
    }
    else if (mKeyInfoIndex != 0xFF)
    {
        return mBoundCmdRelease;
    }
    SetCmdBinding();
    return mBoundCmdRelease;
}

// ============================================================================
// ButtonEntry::SetKeyBinding - ea: 0x4FEAA0
// ============================================================================
// InteractionController_Press - ea: 0x0053C080
int InteractionController_Press(void* self, int buttonIndex)
{
    return static_cast<InteractionController*>(self)->Press(buttonIndex);
}

// InteractionController_Release - ea: 0x0053C0A0
int InteractionController_Release(void* self, int buttonIndex)
{
    return static_cast<InteractionController*>(self)->Release(buttonIndex);
}
extern void Cmd_CallCmdFunction(const BaseCmdFuncInfo* cmd, int key, int time);
extern void CL_KeyEvent(int key, int down, unsigned int time);

void ButtonEntry::SetKeyBinding(unsigned char keyInfoIndex)
{
    mKeyInfoIndex = keyInfoIndex;
    mBoundCmdPress = nullptr;
    mBoundCmdRelease = nullptr;
    if (keyInfoIndex != 0xFF)
        SetCmdBinding();
}

// ea: 0x4FEAD0
void ButtonEntry::Press(bool doCommands)
{
    int mKeyInfoIndex = this->mKeyInfoIndex;
    void* v3 = InteractionController::Inst(currCl);
    if (InteractionController_Press(v3, mKeyInfoIndex) != 0)
        return;
    if (mBoundCmdPress == nullptr)
    {
        if (mKeyInfoIndex == -1)
            goto bind_done;
    }
    else if (mKeyInfoIndex != -1)
    {
        goto bind_done;
    }
    SetCmdBinding();
bind_done:
    const BaseCmdFuncInfo* mBoundCmdPress = this->mBoundCmdPress;
    if (mBoundCmdPress != nullptr && doCommands)
    {
        int v5 = Sys_Milliseconds();
        Cmd_CallCmdFunction(mBoundCmdPress, this->mKeyInfoIndex, v5);
    }
    else
    {
        unsigned int v6 = (unsigned int)Sys_Milliseconds();
        CL_KeyEvent(this->mKeyInfoIndex, true, v6);
    }
}

// ea: 0x4FEB60
void ButtonEntry::Release(bool doCommands)
{
    int mKeyInfoIndex = this->mKeyInfoIndex;
    void* v3 = InteractionController::Inst(currCl);
    if (InteractionController_Release(v3, mKeyInfoIndex) != 0)
        return;
    if (mBoundCmdRelease == nullptr)
    {
        if (mKeyInfoIndex == -1)
            goto release_bind_done;
    }
    else if (mKeyInfoIndex != -1)
    {
        goto release_bind_done;
    }
    SetCmdBinding();
release_bind_done:
    const BaseCmdFuncInfo* mBoundCmdRelease = this->mBoundCmdRelease;
    if (mBoundCmdRelease != nullptr && doCommands)
    {
        int v5 = Sys_Milliseconds();
        Cmd_CallCmdFunction(mBoundCmdRelease, this->mKeyInfoIndex, v5);
        return;
    }
    if (mBoundCmdPress == nullptr)
    {
        if (mKeyInfoIndex == -1)
            goto press_bind_done;
    }
    else if (mKeyInfoIndex != -1)
    {
        goto press_bind_done;
    }
    SetCmdBinding();
press_bind_done:
    if (this->mBoundCmdPress == nullptr || !doCommands)
    {
        unsigned int v6 = (unsigned int)Sys_Milliseconds();
        CL_KeyEvent(this->mKeyInfoIndex, false, v6);
    }
}

// ============================================================================
// ButtonMgr::UpdateBinding - ea: 0x4FE9A0
// ============================================================================
struct ButtonMgr {
    static ButtonEntry mButtons[1][16];  // ?mButtons@ButtonMgr
    static void UpdateBinding(unsigned char keyInfoIndex, int clnt);
    static void InitKeyBindings(int c);
    static void InitKeyBindings();
    static void ClearBinding(const BaseCmdFuncInfo* boundCmd, int clnt);
    static void ClearAllBindings();
};

ButtonEntry ButtonMgr::mButtons[1][16];

void ButtonMgr::UpdateBinding(unsigned char keyInfoIndex, int clnt)
{
    int v4 = 0;
    for (;;)
    {
        if (mButtons[clnt][v4].mKeyInfoIndex == keyInfoIndex)
            break;
        ++v4;
        if (v4 > 15)
            return;
    }
    mButtons[clnt][v4].SetCmdBinding();
}

// ============================================================================
// ButtonMgr::InitKeyBindings - ea: 0x501590
// ============================================================================
void ButtonMgr::InitKeyBindings(int c)
{
    for (int v2 = 0; v2 <= 15; ++v2)
    {
        unsigned char v3;
        switch (v2)
        {
        case 0:  v3 = 0x9C; break;
        case 1:  v3 = 0x9B; break;
        case 2:  v3 = 0x9D; break;
        case 3:  v3 = 0x9A; break;
        case 5:  v3 = 0xD4; break;
        case 14: v3 = 27; break;
        case 15: v3 = 9; break;
        default: v3 = (unsigned char)(v2 - 49); break;
        }
        ButtonEntry* v4 = &mButtons[c][v2];
        v4->mBoundCmdPress = nullptr;
        v4->mBoundCmdRelease = nullptr;
        v4->mKeyInfoIndex = v3;
        if (v3 != 0xFF)
            v4->SetCmdBinding();
    }
}

// ea: 0x509610
void ButtonMgr::InitKeyBindings()
{
    InitKeyBindings(0);
}

// ============================================================================
// IN_Init - ea: 0x50BDF0
// ============================================================================
extern cvar_t* in_stickSouthPaw;
extern cvar_t* in_stickLegacy;
extern cvar_t* in_mouse;
extern cvar_t* in_joystick;
extern cvar_t* in_joyBallScale;
extern cvar_t* in_debugJoystick;
extern cvar_t* joy_threshold;
bool g_waitingForPress;
bool g_controllerConnectedGamePaused;
extern bool g_controllerConnectedErrorShown[];
extern bool g_controllerConnected[4];
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value, int flags);

// ============================================================================
// IN_Frame - ea: 0x501000
// Controller poll + button dispatch (game2.o IN_Init/controller glue).
// ============================================================================
class controller { public:
public:
    enum ButtonIndex {
        LEFTBUTTON = 0, DOWNBUTTON = 1, RIGHTBUTTON = 2, UPBUTTON = 3,
        SQUARE = 4, X = 5, CIRCLE = 6, TRIANGLE = 7,
        R1 = 8, L1 = 9, R2 = 10, L2 = 11,
        R3 = 12, L3 = 13, START = 14, SELECT = 15,
    };
    enum StickIndex {
        LEFTSTICK = 0,
        RIGHTSTICK = 1,
    };

    static controller* inst();
    void poll();
    bool controller_is_connected(int index);
    bool button_pressed_clear(int index, ButtonIndex btn);
    void button_pressed_clear_all(int index);
    int  button_value(int index, ButtonIndex btn);
    bool button_pressed(int index, ButtonIndex btn);
    bool button_released(int index, ButtonIndex btn);
    void stick_value(int index, StickIndex stick, int& outX, int& outY);

    void (*button_value_fn)(int*);
    void (*button_released_fn)(int*);
    void (*button_released_clear_fn)(int*);
    void (*button_pressed_fn)(int*);
    void (*button_pressed_clear_fn)(int*);
    void (*stick_value_fn)(int*, int*);
    int  locked_port;
    bool is_locked;
};
extern int dword_F6A28C[4 * 802];  // client -> controller port
extern int dword_F6A290[4 * 802];  // client -> device flags
extern int currCl;
extern int Sys_Milliseconds();
extern void CL_RecallKeys();     // cl.o
extern void CL_BackUpKeys();     // cl.o
extern int CL_ClearKeysForAll(); // cl.o
extern void CL_GamepadEvent(unsigned int physicalAxis, int value);  // cl.o

void IN_Frame()
{
    if (in_joystick->integer == 0)
        return;
    controller* ctl = controller::inst();
    if (ctl == nullptr)
        return;
    ctl->poll();
    if (ctl->is_locked)
    {
        int locked_port = ctl->locked_port;
        if (!ctl->controller_is_connected(locked_port))
        {
            g_controllerConnected[ctl->locked_port] = false;
            ctl->button_pressed_clear_all(ctl->locked_port);
            CL_ClearKeysForAll();
            currCl = NS_CLIENT;
            return;
        }
        g_controllerConnected[ctl->locked_port] = true;
    }
    else
    {
        for (int j = 0; j < 4; ++j)
            g_controllerConnected[j] = ctl->controller_is_connected(j);
    }
    for (int k = 0; k < 4; ++k)
    {
        if (!g_controllerConnected[k])
        {
            for (int m = controller::LEFTBUTTON; m < 16; ++m)
                ctl->button_pressed_clear(k, (controller::ButtonIndex)m);
        }
    }
    int controller_port = 0;
    if (ctl->is_locked)
        controller_port = ctl->locked_port;
    if (g_controllerConnectedErrorShown[dword_F6A28C[0]])
    {
        currCl = NS_CLIENT;
        return;
    }
    currCl = NS_CLIENT;
    if (!ctl->is_locked)
        controller_port = dword_F6A28C[802 * currCl];
    CL_RecallKeys();
    bool gamePaused = false;
    InGameMenuSystem* v9 = g_femanager.mIGMS[currCl];
    if (v9 != nullptr)
    {
        gamePaused = v9->IsSystemActive();
        if (!gamePaused)
            goto stick_input;
    }
    else
    {
    stick_input:
        {
            int x, y;
            ctl->stick_value(controller_port, controller::RIGHTSTICK, x, y);
            int value = joy_threshold->value;
            if (abs(x) >= value)
                x = (int)((2 * (x >= 0) - 1)
                          * (((float)(abs(x) - value) / (128 - value)) * 128.0f));
            else
                x = 0;
            if (abs(y) >= value)
                y = (int)((2 * (y >= 0) - 1)
                          * (((float)(abs(y) - value) / (128 - value)) * 128.0f));
            else
                y = 0;
            CL_GamepadEvent(0u, x);
            CL_GamepadEvent(1u, y);
            ctl->stick_value(controller_port, controller::LEFTSTICK, x, y);
            if (abs(x) >= value)
                x = (int)((2 * (x >= 0) - 1)
                          * (((float)(abs(x) - value) / (128 - value)) * 128.0f));
            else
                x = 0;
            if (abs(y) >= value)
                y = (int)((2 * (y >= 0) - 1)
                          * (((float)(abs(y) - value) / (128 - value)) * 128.0f));
            else
                y = 0;
            CL_GamepadEvent(2u, x);
            CL_GamepadEvent(3u, y);
        }
    }
    if (currCl == NS_CLIENT)
    {
        g_inspectorManager.ReadKeys();
        if (g_inspectorManager.m_KEY_INSPECTOR_ONOFF != 0)
            g_inspectorManager.m_data.active ^= 1u;
    }
    bool selectPressed =
        ctl->button_value(controller_port, controller::SELECT) != 0;
    for (int v28 = 0; v28 <= 15; ++v28)
    {
        if (!selectPressed || v28 == 15)
        {
            bool pressed = ctl->button_pressed(controller_port,
                                               (controller::ButtonIndex)v28);
            if (g_inspectorManager.m_data.active == 0 && pressed
                && !gamePaused && g_femanager.inGame)
            {
                bool doCommands = dword_F6A290[802 * currCl] == 2;
                ButtonMgr::mButtons[currCl][v28].Press(doCommands);
            }
        }
        if (ctl->button_released(controller_port,
                                 (controller::ButtonIndex)v28)
            && g_femanager.inGame)
        {
            bool doCommands = dword_F6A290[802 * currCl] == 2;
            ButtonMgr::mButtons[currCl][v28].Release(doCommands);
        }
    }
    CL_BackUpKeys();
    currCl = NS_CLIENT;
}

void IN_Init()
{
    in_stickSouthPaw = Cvar_Get("in_stickSouthPaw", "0", 1);
    in_stickLegacy = Cvar_Get("in_stickLegacy", "0", 1);
    in_mouse = Cvar_Get("in_mouse", "0", 33);
    in_joystick = Cvar_Get("in_joystick", "1", 33);
    in_joyBallScale = Cvar_Get("in_joyBallScale", "0.02", 1);
    in_debugJoystick = Cvar_Get("in_debugjoystick", "0", 256);
    joy_threshold = Cvar_Get("joy_threshold", "27", 1);
    ButtonMgr::InitKeyBindings(0);
    g_waitingForPress = false;
    g_controllerConnectedGamePaused = false;
    g_controllerConnectedErrorShown[0] = false;
    for (int i = 0; i < 4; ++i)
        g_controllerConnected[i] = true;
}

// ============================================================================
// ButtonMgr::ClearBinding - ea: 0x5016F0
// ============================================================================
void ButtonMgr::ClearBinding(const BaseCmdFuncInfo* boundCmd, int clnt)
{
    if (boundCmd != nullptr)
    {
        for (int v2 = 0; v2 <= 15; ++v2)
        {
            ButtonEntry* v4 = &mButtons[clnt][v2];
            if (v4->mBoundCmdPress == nullptr && v4->mKeyInfoIndex != 0xFF)
                v4->SetCmdBinding();
            if (v4->mBoundCmdPress == boundCmd)
                goto found;
            ButtonEntry* v6 = &mButtons[clnt][v2];
            if (v6->mBoundCmdRelease == nullptr && v6->mKeyInfoIndex != 0xFF)
                v6->SetCmdBinding();
            if (v6->mBoundCmdRelease == boundCmd)
            {
            found:
                ButtonEntry* v8 = &mButtons[clnt][v2];
                v8->mBoundCmdPress = nullptr;
                v8->mBoundCmdRelease = nullptr;
                v8->mKeyInfoIndex = 0xFF;
                v8->SetCmdBinding();
            }
        }
    }
}

// ============================================================================
// ButtonMgr::ClearAllBindings - ea: 0x5017C0
// ============================================================================
void ButtonMgr::ClearAllBindings()
{
    for (int i = 0; i <= 196; i += 196)
    {
        for (int j = 0; j <= 180; j += 12)
        {
            ButtonEntry* e = reinterpret_cast<ButtonEntry*>(
                reinterpret_cast<unsigned char*>(&mButtons) + i + j);
            e->mKeyInfoIndex = 0xFF;
            e->mBoundCmdPress = nullptr;
            e->mBoundCmdRelease = nullptr;
            e->SetCmdBinding();
        }
    }
}

// ============================================================================
// ScriptEventHandler ctor / AddEvent (extend the existing struct)
// ============================================================================
extern void* ScriptEventHandler_sAllocator;  // ?sAllocator@ScriptEventHandler
extern void* PoolAllocator_Allocate(void* self, unsigned int s,
                                    bool forceHeapAlloc);

// ea: 0x004EAC70
void* ScriptEventHandler::operator new(size_t size, bool forceHeapAlloc,
                                        const char* file, int line)
{
    (void)file;
    (void)line;
    return PoolAllocator_Allocate(ScriptEventHandler_sAllocator,
                                  static_cast<unsigned int>(size),
                                  forceHeapAlloc);
}

// ea: 0x004EAC90
void ScriptEventHandler::operator delete(void* ptr, bool forceHeapAlloc,
                                         const char* file, int line)
{
    (void)forceHeapAlloc;
    (void)file;
    (void)line;
    reinterpret_cast<PoolAllocator*>(ScriptEventHandler_sAllocator)
        ->Release(ptr);
}

// ea: 0x004EACB0
void ScriptEventHandler::operator delete(void* ptr)
{
    reinterpret_cast<PoolAllocator*>(ScriptEventHandler_sAllocator)
        ->Release(ptr);
}

// core.o 0x4B5570
void ScriptEventHandler::SetAllocator(PoolAllocator* allocator)
{
    ScriptEventHandler_sAllocator = allocator;
}

// ea: 0x4F9860
ScriptEventHandler::ScriptEventHandler()
{
    memset(m_dlist_node, 0, sizeof(m_dlist_node));
    for (int i = 0; i < 7; ++i)
    {
        mEvents[i].notify.mHash = 0;
        mEvents[i].callback.mHash = 0;
    }
    mNext = nullptr;
}

// ea: 0x4F98D0
bool ScriptEventHandler::AddEvent(HashString h, HashString callback)
{
    ScriptEventHandler* cur = this;
    for (;;)
    {
        int v4 = 0;
        do
        {
            if (cur->mEvents[v4].callback.mHash == 0)
            {
                cur->mEvents[v4].notify = h;
                cur->mEvents[v4].callback = callback;
                return true;
            }
            ++v4;
        } while (v4 < 7);
        if (cur->mNext != nullptr)
        {
            cur = cur->mNext;
            continue;
        }
        void* v6 = PoolAllocator_Allocate(ScriptEventHandler_sAllocator, 0x44u,
                                          false);
        ScriptEventHandler* v7 = nullptr;
        if (v6 != nullptr)
            v7 = new (v6) ScriptEventHandler;
        cur->mNext = v7;
        if (v7 == nullptr)
            break;
        cur = v7;
    }
    return false;
}

// ea: 0x005E9AF0
bool Entity::AddScriptEvent(HashString h, HashString callback)
{
    if (mScriptEventHandler == nullptr)
    {
        void* v4 = PoolAllocator_Allocate(ScriptEventHandler_sAllocator,
                                           0x44u, false);
        mScriptEventHandler = v4 != nullptr
            ? new (v4) ScriptEventHandler
            : nullptr;
    }
    return mScriptEventHandler->AddEvent(h, callback);
}

// ea: 0x005E9B90
bool Entity::RemoveScriptEvent(HashString h, HashString callback)
{
    ScriptEventHandler* handler = mScriptEventHandler;
    return handler != nullptr && handler->RemoveEvent(h, callback);
}

// ============================================================================
// FnReverseOptions - flip all effect-sound toggles
// ea: 0x4F4510
// ============================================================================
class SoundOptions {
public:
    int mFxDontPlayFootSteps;     // +0x00
    int mFxDontPlayGearRattle;    // +0x04
    int mFxDontPlayLanding;       // +0x08
    int mFxDontPlayScriptCall;    // +0x0C
    int mFxDontPlayScriptCall_Dir;// +0x10
    int mFxDontPlayWeapon;        // +0x14
    int mFxDontPlayBulletHit;     // +0x18
    int mFxDontPlayGrenadeBounce; // +0x1C
    int mFxDontPlayProjExplode;   // +0x20
    int mFxDontPlayVehicle;       // +0x24
    int mFxDontPlayTurret;        // +0x28
    int mFxDontPlayVehicleWheel;  // +0x2C
    int mFxDontPlayLightFlash;    // +0x30
    int mFxDontPlayMusic;         // +0x34
};
static_assert(sizeof(SoundOptions) == 0x38, "SoundOptions size mismatch");

extern SoundOptions gSoundOptions;  // ?gSoundOptions (game2.o)

int FnReverseOptions()
{
    gSoundOptions.mFxDontPlayFootSteps =
        gSoundOptions.mFxDontPlayFootSteps == 0;
    gSoundOptions.mFxDontPlayGearRattle =
        gSoundOptions.mFxDontPlayGearRattle == 0;
    gSoundOptions.mFxDontPlayLanding =
        gSoundOptions.mFxDontPlayLanding == 0;
    gSoundOptions.mFxDontPlayScriptCall =
        gSoundOptions.mFxDontPlayScriptCall == 0;
    gSoundOptions.mFxDontPlayScriptCall_Dir =
        gSoundOptions.mFxDontPlayScriptCall_Dir == 0;
    gSoundOptions.mFxDontPlayWeapon =
        gSoundOptions.mFxDontPlayWeapon == 0;
    gSoundOptions.mFxDontPlayBulletHit =
        gSoundOptions.mFxDontPlayBulletHit == 0;
    gSoundOptions.mFxDontPlayGrenadeBounce =
        gSoundOptions.mFxDontPlayGrenadeBounce == 0;
    gSoundOptions.mFxDontPlayProjExplode =
        gSoundOptions.mFxDontPlayProjExplode == 0;
    gSoundOptions.mFxDontPlayVehicle =
        gSoundOptions.mFxDontPlayVehicle == 0;
    gSoundOptions.mFxDontPlayTurret =
        gSoundOptions.mFxDontPlayTurret == 0;
    gSoundOptions.mFxDontPlayVehicleWheel =
        gSoundOptions.mFxDontPlayVehicleWheel == 0;
    gSoundOptions.mFxDontPlayLightFlash =
        gSoundOptions.mFxDontPlayLightFlash == 0;
    int result = gSoundOptions.mFxDontPlayMusic;
    gSoundOptions.mFxDontPlayMusic =
        gSoundOptions.mFxDontPlayMusic == 0;
    return result;
}

// ============================================================================
// TestFPS::GetFilename - ea: 0x4F6F70
// ============================================================================
extern cvar_t* sv_mapname;  // ?sv_mapname@@3PAUcvar_t@@A

void TestFPS::GetFilename(char* filename)
{
    char path[256];
    sprintf(path, "c:\\cod\\assets\\levels\\%s\\stats\\", sv_mapname->string);
    sprintf(filename, "%sC%i.csv", path, mCellIndex);
}

// ============================================================================
// SplineMgr - spline asset bank (0x184, IDA verified)
// ============================================================================
struct SplineEntry {
    int pakId;   // +0x00 (PAK_ID_INVALID = -1)
    void* file;  // +0x04
    int pad8;    // +0x08

    SplineEntry();                         // ea: 0x5186C0
    void Unload();                         // ea: 0x5186E0
    bool IsUsed() const;                   // ea: 0x518700
};

struct SplineGroupFile {
    void* mTree;      // +0x00 InplaceTree<uint,uint>
    void** mPtrs;     // +0x04 InplaceVector<SplineGroup*>
};

struct SplineGroup;
struct SplinePathData {
    float* mSpline;  // +0x00 InplaceVector<float>
};
struct SplinePath {
    void* mEventIndices;  // +0x00
    void* mEventHashes;   // +0x04
    float* mSpline;       // +0x08
};

// HashGroupFile - spline group file (InplaceVector-of-vectors view)
struct HashGroupFileLocal {
    void* mTree;                                        // +0x00
    InplaceVector<InplaceVector<unsigned char>*> mPtrs; // +0x04
};

class SplineMgr : public AssetBankSet {
public:
    SplineEntry mList[32];             // +0x04

    SplineEntry* GetUnusedEntry();     // ea: 0x4F9700
    void UnloadBank(int pakId);        // ea: 0x4F97C0
    static bool EndOfSpline(const float* p);  // ea: 0x4F59A0
    void AddSplineGroupFile(unsigned char* data, int pakId);  // ea: 0x4FF5B0
    SplinePathData* GetSplinePathData(unsigned int name, int* pakId);  // ea: 0x4FF5D0
    SplineGroup* GetSplinePathGroup(unsigned int name, int* pakId);    // ea: 0x4FF660
    void GetSpline(unsigned int name, SplinePath* splinePath);         // ea: 0x4FF6F0
    void GetSpline(const char* name, SplinePath* splinePath);          // ea: 0x5045C0
    SplineMgr();                                                       // ea: 0x504580
    virtual ~SplineMgr();                                             // ea: 0x51D670
    static SplineMgr* Inst();                                         // ea: 0x5186B0
    static void* operator new(size_t size, void* p);
    void ReverseEndianSplinePath(SplinePath* spline);                   // ea: 0x5045F0
    void ReverseEndianSplineGroupFile(HashGroupFileLocal* splineGroupFile);  // ea: 0x5046C0
    static SplineMgr* sInst;  // ?sInst@SplineMgr@@2PAV1@A
    static SplineMgr* CreateInst();  // ?CreateInst@SplineMgr@@SAPAV1@XZ
    static void DeleteInst();  // ?DeleteInst@SplineMgr@@SAXXZ
};

extern void InplaceAssetBank_Fixup(void* data);  // inplace_xboxr (spline bank)
SplineMgr* SplineMgr::sInst;  // ?sInst@SplineMgr@@2PAV1@A (game2.o @ 0x12F3EA0)

// ea: 0x005186B0
SplineMgr* SplineMgr::Inst()
{
    return sInst;
}

// ea: 0x005186C0
SplineEntry::SplineEntry()
    : pakId(-1), file(nullptr)
{
}

// ea: 0x005186E0
void SplineEntry::Unload()
{
    pakId = -1;
    file = nullptr;
}

// ea: 0x00518700
bool SplineEntry::IsUsed() const
{
    return pakId != -1;
}

// ea: 0x004DDEE0
void* SplineMgr::operator new(size_t size, void* p)
{
    (void)size;
    return p;
}

// ea: 0x004DDEF0
SplineMgr* SplineMgr::CreateInst()
{
    SplineMgr* result = nullptr;
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SplineMgr.h";
        AeAssert::gCurrentLine = 39;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    result = static_cast<SplineMgr*>(
        mem_heap_malloc_ctx(0x184u, 4, "core",
                            "c:\\cod\\code\\game\\SplineMgr.h", 39));
    if (result != nullptr)
    {
        result = new (result) SplineMgr();
        sInst = result;
    }
    else
    {
        sInst = nullptr;
    }
    return result;
}

// ea: 0x004DDFF0
void SplineMgr::DeleteInst()
{
    if (sInst == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SplineMgr.h";
        AeAssert::gCurrentLine = 39;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }
    if (sInst != nullptr)
        delete sInst;
    sInst = nullptr;
}

// ea: 0x504560
void DecodeSplineGroup(const char* name, unsigned char* data, int size,
                       TPakId pakId)
{
    SplineEntry* UnusedEntry = SplineMgr::sInst->GetUnusedEntry();
    UnusedEntry->pakId = pakId;
    UnusedEntry->file = data;
    InplaceAssetBank_Fixup(data);
}

// ea: 0x5046C0
void SplineMgr::ReverseEndianSplineGroupFile(
    HashGroupFileLocal* splineGroupFile)
{
    unsigned int mSize = splineGroupFile->mPtrs.mSize;
    for (unsigned int v4 = 0; v4 < mSize; ++v4)
    {
        InplaceVector<unsigned char>* v6 = splineGroupFile->mPtrs.mList[v4];
        if (v6 == nullptr)
            continue;
        if (v6->mSize == 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
            AeAssert::gCurrentLine = 91;
            AeAssert::gCurrentExpr = "index < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                __debugbreak();
        }
        unsigned char* mList = v6->mList;
        // Byte-swap the three header words (tree count + two offsets)
        for (int w = 0; w < 3; ++w)
        {
            unsigned char* p = mList + w * 4;
            unsigned char t0 = p[0];
            p[0] = p[3];
            p[3] = t0;
            unsigned char t1 = p[1];
            p[1] = p[2];
            p[2] = t1;
        }
        int v13 = *(int*)(mList + 4);
        if (v13 == 0)
            return;
        unsigned char* v17 = mList + 12;
        unsigned char* v18 = &v17[12 * v13];
        for (int j = v13; j != 0; --j)
        {
            unsigned char* e = v17;
            for (int w = 0; w < 3; ++w)
            {
                unsigned char* p = e + w * 4;
                unsigned char t0 = p[0];
                p[0] = p[3];
                p[3] = t0;
                unsigned char t1 = p[1];
                p[1] = p[2];
                p[2] = t1;
            }
            SplinePath splineData;
            splineData.mSpline = (float*)(v18 + *(int*)(e + 0));
            splineData.mEventIndices = v18 + *(int*)(e + 4);
            splineData.mEventHashes = v18 + *(int*)(e + 8);
            ReverseEndianSplinePath(&splineData);
            v17 = e + 12;
        }
    }
}

// ea: 0x4F9700
SplineEntry* SplineMgr::GetUnusedEntry()
{
    for (int i = 0; i < 32; ++i)
    {
        if (mList[i].pakId == -1)
            return &mList[i];
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\splinemgr.cpp";
    AeAssert::gCurrentLine = 38;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored() && AeAssert::Error("Too many spline files loaded"))
        __debugbreak();
    return nullptr;
}

// ea: 0x4F97C0
void SplineMgr::UnloadBank(int pakId)
{
    int v4 = 0;
    for (;;)
    {
        if (v4 >= 32)
            return;
        if (mList[v4].pakId == pakId)
            break;
        ++v4;
    }
    mList[v4].pakId = -1;
    mList[v4].file = nullptr;
}

// ea: 0x004E9410
SmokeGrenadeMgr* SmokeGrenadeMgr::CreateInst()
{
    SmokeGrenadeMgr* result = nullptr;
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SmokeGrenadeMgr.h";
        AeAssert::gCurrentLine = 31;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }

    result = static_cast<SmokeGrenadeMgr*>(
        mem_heap_malloc_ctx(0x0Cu, 4, "core",
                            "c:\\cod\\code\\game\\SmokeGrenadeMgr.h", 31));
    if (result != nullptr)
    {
        result->mSmokeGrenadeInfoList.mElements = nullptr;
        result->mSmokeGrenadeInfoList.mCapacity = 0;
        result->mSmokeGrenadeInfoList.mSize = 0;
        sInst = result;
    }
    else
    {
        sInst = nullptr;
    }
    return result;
}

// ============================================================================
// SmokeGrenadeMgr visibility helpers
// ============================================================================
float sTime0, sTime1, sTime2, sTime3, sTime4;
float sOpacity2, sOpacity3;

// ea: 0x4FA0E0
float SmokeGrenadeMgr::CalcOpacity(const SmokeGrenadeInfo& info) const
{
    float v2 = info.mTime;
    if (sTime0 > v2)
        return 0.0f;
    if (sTime1 > v2)
    {
        float in = (v2 - sTime0) / (sTime1 - sTime0);
        float beg = 0.0f;
        float end = 1.0f;
        return ClampRange(in, beg, end);
    }
    if (sTime2 > v2)
        return 1.0f;
    if (sTime3 > v2)
    {
        float in = (v2 - sTime2) / (sTime3 - sTime2);
        float beg = 0.0f;
        float end = 1.0f;
        float v4 = ClampRange(in, beg, end);
        return (1.0f - v4) * sOpacity2 + sOpacity3 * v4;
    }
    float beg = (v2 - sTime3) / (sTime4 - sTime3);
    float in = 0.0f;
    float end = 1.0f;
    return (1.0f - ClampRange(beg, in, end)) * sOpacity3;
}

// ============================================================================
// SmokeGrenadeMgr visibility helpers (uses apsEffect bounds + opacity)
// ============================================================================
#include "aeps/apsEffect.h"
#include "aeps/apsGroup.h"

// ea: 0x4FA220
bool SmokeGrenadeMgr::PointCanSeePoint(const float* startPoint,
                                       const float* endPoint,
                                       float visThreshold)
{
    SmokeGrenadeInfo* mElements = mSmokeGrenadeInfoList.mElements;
    SmokeGrenadeInfo* end = &mElements[mSmokeGrenadeInfoList.mSize];
    if (mElements == end)
        return true;
    for (; mElements != end; ++mElements)
    {
        apsEffect* mEffect = (apsEffect*)mElements->mEffect;
        apsBounds bounds;
        mEffect->GetBounds(bounds);
        apsSphere sph = bounds.Sphere();
        bool hit = SegmentSphereIntersection(
            startPoint, endPoint, &sph.mSphere.v.m128_f32[0],
            sph.mSphere.v.m128_f32[3] * 0.33333334f);
        mElements->bHit[0] = hit;
        if (!hit)
            continue;
        int mSize = mEffect->mGroups.mSize;
        for (int i = 0; i < mSize; ++i)
        {
            apsGroup* group = mEffect->mGroups.mElements[i];
            if (group == nullptr)
                continue;
            apsBounds gb = group->mBounds;
            apsSphere gs = gb.Sphere();
            bool ghit = SegmentSphereIntersection(
                startPoint, endPoint, &gs.mSphere.v.m128_f32[0],
                gs.mSphere.v.m128_f32[3] * 0.25f);
            mElements->bHit[i + 1] = ghit;
            if (ghit)
            {
                if (CalcOpacity(*mElements) <= visThreshold)
                    break;
                return false;
            }
        }
    }
    return true;
}

// ea: 0x4FA400
bool SmokeGrenadeMgr::EntityCanSeePoint(const Entity* ent,
                                        const float* endPoint,
                                        float visThreshold)
{
    float startPoint[3];
    startPoint[0] = ent->r.currentOrigin.v.m128_f32[0];
    startPoint[1] = ent->r.currentOrigin.v.m128_f32[1];
    startPoint[2] = (ent->r.maxs.v.m128_f32[2] * 0.5f)
        + ent->r.currentOrigin.v.m128_f32[2];
    return PointCanSeePoint(startPoint, endPoint, visThreshold);
}

// ea: 0x4FA460
bool SmokeGrenadeMgr::EntityCanSeeEntity(const Entity* ent,
                                         const Entity* targEnt,
                                         float visThreshold)
{
    float startPoint[3];
    float endPoint[3];
    float v4 = ent->r.maxs.v.m128_f32[2];
    startPoint[0] = ent->r.currentOrigin.v.m128_f32[0];
    startPoint[1] = ent->r.currentOrigin.v.m128_f32[1];
    endPoint[0] = targEnt->r.currentOrigin.v.m128_f32[0];
    endPoint[1] = targEnt->r.currentOrigin.v.m128_f32[1];
    startPoint[2] = (v4 * 0.5f) + ent->r.currentOrigin.v.m128_f32[2];
    endPoint[2] = (targEnt->r.maxs.v.m128_f32[2] * 0.5f)
        + targEnt->r.currentOrigin.v.m128_f32[2];
    return PointCanSeePoint(startPoint, endPoint, visThreshold);
}

// ============================================================================
// SmokeGrenadeMgr::Update - ea: 0x4F9CC0
// ============================================================================
extern void ae_vector_erase(void* self, int idx);  // ?erase@?$ae_vector@USmokeGrenadeInfo
void ae_vector_erase(void* self, int idx)
{
    (void)self; (void)idx;
}

void SmokeGrenadeMgr::Update(float deltaT)
{
    if (deltaT == 0.0f)
        return;
    if (g_drawSmokeGren.integer != 0)
    {
        int count = mSmokeGrenadeInfoList.mSize;
        for (int i = 0; i < count; ++i)
        {
            SmokeGrenadeInfo& info = mSmokeGrenadeInfoList.mElements[i];
            apsEffect* mEffect = (apsEffect*)info.mEffect;
            apsBounds bounds;
            mEffect->GetBounds(bounds);
            apsSphere sph = bounds.Sphere();
            float color[4] = { 0.6f, 0.5f, 0.5f, 1.0f };
            math::Position3 center;
            center.v = sph.mSphere.v;
            DebugRender::RenderSphere(
                center, sph.mSphere.v.m128_f32[3] * 0.33333334f,
                Color(color[0], color[1], color[2], color[3]));
        }
    }
    int v16 = mSmokeGrenadeInfoList.mSize;
    int v17 = 0;
    if (v16 > 0)
    {
        int v18 = 0;
        do
        {
            mSmokeGrenadeInfoList.mElements[v18].mTime =
                mSmokeGrenadeInfoList.mElements[v18].mTime + deltaT;
            if (((apsEffect*)mSmokeGrenadeInfoList.mElements[v18].mEffect)->IsDone() != 0)
            {
                ae_vector_erase(&mSmokeGrenadeInfoList, v17--);
                --v18;
            }
            v16 = mSmokeGrenadeInfoList.mSize;
            ++v17;
            ++v18;
        } while (v17 < v16);
    }
}

// ============================================================================
// DisplayPoolTotals - render pool report lines (game2.o)
// ea: 0x4FEFB0
// ============================================================================
#include "core/PoolAllocator.h"
#include "core/ae_fixed_string.h"

struct InspectorManager;
extern InspectorManager g_inspectorManager;  // ?g_inspectorManager (game2.o)
extern float scaleScalar;         // render.o
extern void RE_Text_Paint(float x, float y, int font, float scale,
                          const float* color, const char* text, float a7,
                          int a8, int a9);

void DisplayPoolTotals(PoolAllocator* pool)
{
    ae_sized_array<ae_fixed_string<64, unsigned char>, 8> strList;
    for (int i = 0; i < 8; ++i)
    {
        strList.m_elements[i].mLength = 0;
        strList.m_elements[i].mBuff[0] = 0;
    }
    strList.m_size = 0;
    pool->ReportTotals(&strList);
    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    int y = 64;
    for (int i = 0; i < strList.m_size; ++i)
    {
        const char* text = (const char*)strList.m_elements[i].mBuff;
        RE_Text_Paint(340.0f, (float)y + 2.0f, 5, scaleScalar * 0.75f,
                      black, text, 0, 0, 0);
        RE_Text_Paint(330.0f, (float)y, 5, scaleScalar * 0.75f,
                      (const float*)&g_inspectorManager, text, 0, 0, 0);
        y += 18;
    }
}


// ============================================================================
// FN_Multiplayer_MapRestart - ea: 0x4F46D0
// ============================================================================
void FN_Multiplayer_MapRestart()
{
    MultiplayerMgr::sInst->MapRestart();
}

// ============================================================================
// FN_Multiplayer_Rank1 - reset persistent stats to rank 0
// ea: 0x4F46E0
// ============================================================================
Entity* FN_Multiplayer_Rank1()
{
    clientPersistent_t* p_pers =
        &EntityManager::sInst->GetPlayer(currCl)->client->pers;
    memset(p_pers, 0, 0x194u);
    p_pers->mStats[6][28] = 0;
    p_pers->mBaseScore = 0;
    Entity* result = EntityManager::sInst->GetPlayer(currCl);
    result->client->pers.rank = 0;
    return result;
}

// ============================================================================
// FN_Multiplayer_Rank2 - ea: 0x4F4740
// ============================================================================
Entity* FN_Multiplayer_Rank2()
{
    clientPersistent_t* p_pers =
        &EntityManager::sInst->GetPlayer(currCl)->client->pers;
    memset(p_pers, 0, 0x194u);
    p_pers->mStats[6][28] = 0;
    p_pers->mBaseScore = 0;
    EntityManager::sInst->GetPlayer(currCl)->client->pers.mStats[0][3] = 15;
    Entity* result = EntityManager::sInst->GetPlayer(currCl);
    result->client->pers.rank = 1;
    return result;
}

// ============================================================================
// FN_Multiplayer_Rank3 - ea: 0x4F47C0
// ============================================================================
Client* FN_Multiplayer_Rank3()
{
    EntityManager::sInst->GetPlayer(currCl)->client->pers.mStats[0][3] = 40;
    Client* result = EntityManager::sInst->GetPlayer(currCl)->client;
    result->pers.rank = 2;
    return result;
}

// ============================================================================
// SplineMgr::EndOfSpline - ea: 0x4F59A0
// ============================================================================
bool SplineMgr::EndOfSpline(const float* p)
{
    return *p == -1.0f && *(p + 1) == -1.0f && *(p + 2) == -1.0f;
}

// ============================================================================
// SplineMgr path lookup helpers
// ============================================================================
extern unsigned int InplaceTree_Find(void* tree, const unsigned int* key);
extern void InplaceAssetBank_Fixup(void* data);
extern SplineGroup* SplineGroup_GetPath(void* self);

// ea: 0x4FF5B0
void SplineMgr::AddSplineGroupFile(unsigned char* data, int pakId)
{
    SplineEntry* entry = GetUnusedEntry();
    entry->pakId = pakId;
    entry->file = data;
    InplaceAssetBank_Fixup(data);
}

// ea: 0x4FF5D0
SplinePathData* SplineMgr::GetSplinePathData(unsigned int name, int* pakId)
{
    *pakId = -1;
    if (name == 0)
        return nullptr;
    for (int v4 = 0; v4 < 32; ++v4)
    {
        if (mList[v4].pakId != -1)
        {
            void* file = mList[v4].file;
                unsigned int* v7 =
                (unsigned int*)InplaceTree_Find(&((SplineGroupFile*)file)->mTree,
                                                &name);
            if (v7 != nullptr)
            {
                SplineGroup* v8 = (SplineGroup*)
                    ((SplineGroupFile*)file)->mPtrs[*v7];
                if (v8 != nullptr)
                {
                    *pakId = mList[v4].pakId;
                    return (SplinePathData*)SplineGroup_GetPath(v8);
                }
            }
        }
    }
    return nullptr;
}

// ea: 0x4FF660
SplineGroup* SplineMgr::GetSplinePathGroup(unsigned int name, int* pakId)
{
    *pakId = -1;
    if (name == 0)
        return nullptr;
    for (int v4 = 0; v4 < 32; ++v4)
    {
        if (mList[v4].pakId != -1)
        {
            void* file = mList[v4].file;
            unsigned int* v7 =
                (unsigned int*)InplaceTree_Find(&((SplineGroupFile*)file)->mTree,
                                                &name);
            if (v7 != nullptr)
            {
                SplineGroup* v8 = (SplineGroup*)
                    ((SplineGroupFile*)file)->mPtrs[*v7];
                if (v8 != nullptr)
                {
                    *pakId = mList[v4].pakId;
                    return v8;
                }
            }
        }
    }
    return nullptr;
}

// ea: 0x4FF6F0
void SplineMgr::GetSpline(unsigned int name, SplinePath* splinePath)
{
    splinePath->mEventIndices = nullptr;
    splinePath->mEventHashes = nullptr;
    splinePath->mSpline = nullptr;
    if (name == 0)
        return;
    for (int v3 = 0; v3 < 32; ++v3)
    {
        if (mList[v3].pakId != -1)
        {
            void* file = mList[v3].file;
            unsigned int* v8 =
                (unsigned int*)InplaceTree_Find(&((SplineGroupFile*)file)->mTree,
                                                &name);
            if (v8 != nullptr)
            {
                SplineGroup* v9 = (SplineGroup*)
                    ((SplineGroupFile*)file)->mPtrs[*v8];
                if (v9 != nullptr)
                {
                    SplinePathData* path =
                        (SplinePathData*)SplineGroup_GetPath(v9);
                    splinePath->mEventHashes = nullptr;
                    splinePath->mEventIndices = nullptr;
                    splinePath->mSpline = (float*)&path->mSpline[0];
                    return;
                }
            }
        }
    }
}

// ============================================================================
// SplineMgr::GetSpline(const char*) - ea: 0x5045C0
// ============================================================================
void SplineMgr::GetSpline(const char* name, SplinePath* splinePath)
{
    unsigned int v4 = HashString::CalcHash(name);
    GetSpline(v4, splinePath);
}

// ============================================================================
// SplineMgr::SplineMgr - ea: 0x504580
// ============================================================================
float* (*GetSplineGroup)(unsigned int) = nullptr;  // ?GetSplineGroup (game2.o)
#include "aeps/apsCommon.h"

SplineMgr::SplineMgr()
    : AssetBankSet()
{
    for (int i = 0; i < 32; ++i)
    {
        mList[i].pakId = -1;
        mList[i].file = nullptr;
    }
    apsCommon::SetSplineCallback(GetSplineGroup);
}

// ea: 0x0051D670
SplineMgr::~SplineMgr() = default;

// ============================================================================
// SplineMgr::ReverseEndianSplinePath - ea: 0x5045F0
// Byte-swap spline path data (floats + event arrays, -1 terminated).
// ============================================================================
void SplineMgr::ReverseEndianSplinePath(SplinePath* spline)
{
    float* mSpline = spline->mSpline;
    do
    {
        // 12-byte float triple byte-swap
        unsigned char* p = (unsigned char*)mSpline;
        unsigned char t0 = p[0]; p[0] = p[3]; p[3] = t0;
        unsigned char t1 = p[1]; p[1] = p[2]; p[2] = t1;
        unsigned char t2 = p[4]; p[4] = p[7]; p[7] = t2;
        unsigned char t3 = p[5]; p[5] = p[6]; p[6] = t3;
        unsigned char t4 = p[8]; p[8] = p[11]; p[11] = t4;
        unsigned char t5 = p[9]; p[9] = p[10]; p[10] = t5;
        bool end = mSpline[0] == -1.0f && mSpline[1] == -1.0f
            && mSpline[2] == -1.0f;
        mSpline += 3;
        if (end)
            break;
    } while (true);
    unsigned int* mEventIndices = (unsigned int*)spline->mEventIndices;
    do
    {
        unsigned char* p = (unsigned char*)mEventIndices;
        unsigned char t0 = p[0]; p[0] = p[3]; p[3] = t0;
        unsigned char t1 = p[1]; p[1] = p[2]; p[2] = t1;
        int v = *mEventIndices;
        ++mEventIndices;
        if (v == -1)
            break;
    } while (true);
    unsigned int* mEventHashes = (unsigned int*)spline->mEventHashes;
    do
    {
        unsigned char* p = (unsigned char*)mEventHashes;
        unsigned char t0 = p[0]; p[0] = p[3]; p[3] = t0;
        unsigned char t1 = p[1]; p[1] = p[2]; p[2] = t1;
        int v = *mEventHashes;
        ++mEventHashes;
        if (v == -1)
            break;
    } while (true);
}

// ============================================================================
// ScriptEventHandler::AddEvent(HashString, const char*)
// ea: 0x4FF780
// ============================================================================
bool ScriptEventHandler::AddEvent(HashString h, const char* callback)
{
    if (callback == nullptr)
    {
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Adding a script event with no callback."))
            __debugbreak();
    }
    HashString v4;
    v4.mHash = HashString::CalcHash(callback);
    return AddEvent(h, v4);
}

// ============================================================================
// TaskSys::LookupHandler - ea: 0x4FF990
// ============================================================================
struct TaskSysImpl {
    TaskHandler* mTaskHandlers[32];  // +0x00 ae_sized_array<TaskHandler*,32>
    int m_size;                      // +0x80

    TaskHandler* LookupHandler(unsigned int id);
};

// ea: 0x4FF990
TaskHandler* TaskSysImpl::LookupHandler(unsigned int id)
{
    int count = m_size;
    for (int i = 0; i < count; ++i)
    {
        if (mTaskHandlers[i]->mTaskId.mVal == id)
            return mTaskHandlers[i];
    }
    return nullptr;
}

// ============================================================================
// Drone animation entity map helpers
// ============================================================================
template <typename A, typename B>
struct ae_pair {
    A first;   // +0x00
    B second;  // +0x04
};

// ae_vector<DbLinkedHandle<EntityHandleDb,Entity>> - 12 bytes
struct DroneHandleVec {
    DbLinkedHandle<EntityHandleDb, Entity>* mElements;  // +0x00
    int mCapacity;   // +0x04
    int mSize;       // +0x08
};

// gDroneAEMap: ae_sized_array<ae_pair<uint, DroneHandleVec*>*, 8>
struct DroneAEMap {
    ae_pair<unsigned int, DroneHandleVec*>* m_elements[8];  // +0x00
    int m_size;   // +0x20
};

DroneAEMap gDroneAEMap;  // ?gDroneAEMap@@3V?$ae_sized_array@PAV?$ae_pair@IPAV?$ae_vector@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@@@@$07@@A (game2.o @ 0x12F45E0)
extern void ae_vector_push_back_handle(DroneHandleVec* self,
    const DbLinkedHandle<EntityHandleDb, Entity>* elem);
void ae_vector_push_back_handle(DroneHandleVec* self,
                                const DbLinkedHandle<EntityHandleDb, Entity>* elem)
{
    (void)self; (void)elem;
}
extern void ae_sized_array_push_back_pair(DroneAEMap* self,
    ae_pair<unsigned int, DroneHandleVec*>* const* elt);
extern void* mem_heap_malloc_sz(unsigned int size);

// ea: 0x4FF800
int InsertDroneSlave(Entity* e, unsigned int animIndex)
{
    e->mFlags |= 8u;
    rand();
    for (int i = 0; i < gDroneAEMap.m_size; ++i)
    {
        ae_pair<unsigned int, DroneHandleVec*>* entry = gDroneAEMap.m_elements[i];
        if (entry->first == animIndex)
        {
            DbLinkedHandle<EntityHandleDb, Entity> h = e->mHandle;
            ae_vector_push_back_handle(entry->second, &h);
            return 0;
        }
    }
    return 0;
}

// ea: 0x4FF880
int InsertDroneMaster(Entity* e, unsigned int animIndex)
{
    e->mFlags |= 8u;
    DroneHandleVec* v3 = (DroneHandleVec*)mem_heap_malloc_sz(0xC);
    DroneHandleVec* v4 = nullptr;
    if (v3 != nullptr)
    {
        v3->mElements = nullptr;
        v3->mCapacity = 0;
        v3->mSize = 0;
        v4 = v3;
    }
    DbLinkedHandle<EntityHandleDb, Entity> h = e->mHandle;
    ae_vector_push_back_handle(v4, &h);
    ae_pair<unsigned int, DroneHandleVec*>* v5 =
        (ae_pair<unsigned int, DroneHandleVec*>*)mem_heap_malloc_sz(8);
    if (v5 != nullptr)
    {
        v5->first = animIndex;
        v5->second = v4;
    }
    ae_pair<unsigned int, DroneHandleVec*>* p = v5;
    ae_sized_array_push_back_pair(&gDroneAEMap, &p);
    return 0;
}

// ea: 0x4FF910
int RemoveDrone(Entity* e, unsigned int animIndex)
{
    unsigned int mVal = e->mHandle.mHandle.mVal;
    for (int i = 0; i < gDroneAEMap.m_size; ++i)
    {
        ae_pair<unsigned int, DroneHandleVec*>* entry = gDroneAEMap.m_elements[i];
        DroneHandleVec* vec = entry->second;
        for (int j = 0; j < vec->mSize; ++j)
        {
            if (vec->mElements[j].mHandle.mVal == mVal)
            {
                // erase by shifting
                for (int k = j; k < vec->mSize - 1; ++k)
                    vec->mElements[k] = vec->mElements[k + 1];
                --vec->mSize;
                break;
            }
        }
    }
    e->mFlags &= ~8u;
    return 0;
}

// ============================================================================
// UpdateDroneAEMap - ea: 0x5048C0
// ============================================================================
extern int RemoveDrone(Entity* e, unsigned int animIndex);
extern int InsertDroneMaster(Entity* e, unsigned int animIndex);
extern int InsertDroneSlave(Entity* e, unsigned int animIndex);

int UpdateDroneAEMap(Entity* e, unsigned int animIndex)
{
    unsigned int mMask = e->mFlags;
    if (((mMask & 8) != 0 || (mMask & 4) != 0) && (e->flags & 0x2000000) != 0)
    {
        RemoveDrone(e, animIndex);
        return 0;
    }
    if ((0x400000 & e->flags) == 0 && animIndex != 0)
    {
        int v6 = 0;
        for (int i = 0; i < gDroneAEMap.m_size; ++i)
        {
            if (gDroneAEMap.m_elements[i]->first == animIndex)
                ++v6;
        }
        if (v6 < 1)
        {
            InsertDroneMaster(e, animIndex);
            return 0;
        }
        InsertDroneSlave(e, animIndex);
    }
    return 0;
}

// ============================================================================
// CheckAEMapValidation - ea: 0x50B730
// Remove dead drone entries and clean flags on surviving ones.
// ============================================================================
extern void ae_vector_erase_handle(DroneHandleVec* self,
    DbLinkedHandle<EntityHandleDb, Entity>* elem);

void CheckAEMapValidation()
{
    for (int i = 0; i < gDroneAEMap.m_size; ++i)
    {
        ae_pair<unsigned int, DroneHandleVec*>* entry = gDroneAEMap.m_elements[i];
        DroneHandleVec* vec = entry->second;
        if (vec != nullptr && vec->mElements != nullptr && vec->mSize > 0)
        {
            for (int j = 0; j < vec->mSize;)
            {
                unsigned int v4 = vec->mElements[j].mHandle.mVal & 0xFFF;
                Entity* mObject = nullptr;
                if (v4 < 0x540
                    && vec->mElements[j].mHandle.mVal >> 12
                        == EntityHandleDb::sInst.mElements[v4].mKey)
                    mObject = EntityHandleDb::sInst.mElements[v4].mObject;
                if (mObject != nullptr)
                {
                    if ((0x400000 & mObject->flags) == 0)
                    {
                        mObject->mFlags &= ~8u;
                        for (int k = j; k < vec->mSize - 1; ++k)
                            vec->mElements[k] = vec->mElements[k + 1];
                        --vec->mSize;
                        continue;
                    }
                    mObject->mFlags = (mObject->mFlags & ~8u) | 4u;
                }
                ++j;
            }
        }
        else if (gDroneAEMap.m_size > 1)
        {
            gDroneAEMap.m_elements[i] =
                gDroneAEMap.m_elements[gDroneAEMap.m_size - 1];
            --gDroneAEMap.m_size;
            --i;
        }
        else
        {
            --gDroneAEMap.m_size;
            break;
        }
    }
}

// ============================================================================
// stat_StatDamageEvent - ea: 0x509460
// ============================================================================
void stat_StatDamageEvent(Entity* pSelf, Entity* pInflictor, Entity* pAttacker,
                          int iMod, hitLocation_t hitLoc)
{
    Entity* v5 = pAttacker;
    if (pAttacker->scr_vehicle != nullptr)
    {
        unsigned int v6 = pAttacker->r.mOwner.mHandle.mVal & 0xFFF;
        v5 = nullptr;
        if (v6 < 0x540
            && pAttacker->r.mOwner.mHandle.mVal >> 12
                == EntityHandleDb::sInst.mElements[v6].mKey)
            v5 = EntityHandleDb::sInst.mElements[v6].mObject;
    }
    Entity* v8 = pInflictor;
    if (pInflictor->scr_vehicle != nullptr)
    {
        unsigned int v9 = pInflictor->r.mOwner.mHandle.mVal & 0xFFF;
        v8 = nullptr;
        if (v9 < 0x540
            && pInflictor->r.mOwner.mHandle.mVal >> 12
                == EntityHandleDb::sInst.mElements[v9].mKey)
            v8 = EntityHandleDb::sInst.mElements[v9].mObject;
    }
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    if (v5 == player || v8 == player)
    {
        sentient_s* sentient = pSelf->sentient;
        if (sentient != nullptr)
        {
            if (sentient->eTeam == TEAM_ALLIES)
                return;
        }
        else
        {
            const char* team = pSelf->team.mBlock != nullptr
                ? (const char*)(pSelf->team.mBlock + 1) : defaultFileName;
            if (_stricmp(team, "axis") != 0)
                return;
        }
        ++gTempMissionData.missionStat[15];
        ++gTempMissionData.missionStat[6];
        if (pSelf->health <= 0)
        {
            switch (iMod)
            {
            case 3:
            case 4:
                ++gTempMissionData.missionStat[9];
                ++gTempMissionData.missionStat[16];
                break;
            case 10:
            case 27:
                ++gTempMissionData.missionStat[11];
                break;
            case 11:
                ++gTempMissionData.missionStat[7];
                break;
            case 32:
                ++gTempMissionData.missionStat[12];
                break;
            default:
                break;
            }
            if (hitLoc == HITLOC_HEAD)
                ++gTempMissionData.missionStat[5];
        }
    }
}

// ============================================================================
// FN_DebugAnims_Select_Target - ea: 0x50B6F0
// ============================================================================
extern vmCvar_t g_dumpAnims;  // ?g_dumpAnims (g.o vmCvar)
extern Entity* _Return_MF_UnderCrossHair();  // game2.o 0x503E30

void FN_DebugAnims_Select_Target()
{
    Entity* v0 = _Return_MF_UnderCrossHair();
    if (v0 != nullptr)
    {
        g_dumpAnims.integer = v0->mHandle.mHandle.mVal;
        G_Printf("^5Entity found\n");
    }
    else
    {
        g_dumpAnims.integer = 0;
        G_Printf("^5Cant find Entity\n");
    }
}

// ============================================================================
// _Return_MF_UnderCrossHair - ea: 0x503E30
// Trace from the player's weapon muzzle to find the target entity.
// ============================================================================
extern void CalcMuzzlePoints(Entity* ent, weaponParms* wp);  // ?CalcMuzzlePoints (game.o)
extern void g_LocationalTrace(trace_t* results,
                              const math::Position3& start,
                              const math::Position3& end,
                              const collision_context_t& context,
                              unsigned char* priorityMap,
                              float coneAngleTangent);
// ?bulletPriorityMap@@3PAEA / ?riflePriorityMap@@3PAEA (g.o @ 0xDD55D0)
unsigned char bulletPriorityMap[20] = {
    1, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 0, 0,
};
unsigned char riflePriorityMap[20] = {
    1, 9, 9, 9, 8, 7, 6, 6, 6, 6,
    5, 5, 4, 4, 4, 4, 3, 3, 0, 0,
};
void* collision_context_vftable = nullptr;  // ??_7collision_context_t@@6B@

Entity* _Return_MF_UnderCrossHair()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    weaponParms wp;
    memset(&wp, 0, sizeof(wp));
    CalcMuzzlePoints(player, &wp);
    Client* client = player->client;
    unsigned char* v4;
    if (client->ps.weapon == 0
        || BG_GetInfoForWeapon(client->ps.weapon)->bRifleBullet == 0)
        v4 = bulletPriorityMap;
    else
        v4 = riflePriorityMap;
    unsigned int mVal = player->mHandle.mHandle.mVal;

    math::Position3 start;
    start.v.m128_f32[0] = wp.muzzleTrace[0];
    start.v.m128_f32[1] = wp.muzzleTrace[1];
    start.v.m128_f32[2] = wp.muzzleTrace[2];
    math::Position3 end;
    end.v.m128_f32[0] = wp.muzzleTrace[0] + wp.forward[0] * 8192.0f;
    end.v.m128_f32[1] = wp.muzzleTrace[1] + wp.forward[1] * 8192.0f;
    end.v.m128_f32[2] = wp.muzzleTrace[2] + wp.forward[2] * 8192.0f;

    // collision_context_t with vtable + skip entity handle
    void* ctx[3];
    ctx[0] = collision_context_vftable;
    ctx[1] = (void*)mVal;
    ctx[2] = nullptr;
    int result;
    g_LocationalTrace((trace_t*)&result, start, end,
                      *((collision_context_t*)ctx), v4, 0.0f);

    unsigned int hit = result;
    if (hit == EntityManager::sInst->mWorld->mHandle.mHandle.mVal || hit == 0)
        return nullptr;
    unsigned int v7 = hit & 0xFFF;
    g_debugThread.m_entityHandle.mHandle.mVal = hit;
    if (v7 < 0x540
        && hit >> 12 == EntityHandleDb::sInst.mElements[v7].mKey)
        return EntityHandleDb::sInst.mElements[v7].mObject;
    return nullptr;
}

// ============================================================================
// SmokeGrenadeMgr::AddSmokeGrenade - ea: 0x4FFBD0
// ============================================================================
void SmokeGrenadeMgr::AddSmokeGrenade(const SmokeGrenadeInfo* smokeGrenInfo)
{
    if (smokeGrenInfo->mEffect != nullptr)
    {
        reinterpret_cast<ae_vector<SmokeGrenadeInfo>*>(
            &mSmokeGrenadeInfoList)->push_back(*smokeGrenInfo);
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SmokeGrenadeMgr.cpp";
        AeAssert::gCurrentLine = 18;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Smoke Grenade Info with null effect"))
            __debugbreak();
    }
}

// ea: 0x504C50
void SmokeGrenadeMgr::ReInitialize()
{
    mSmokeGrenadeInfoList.mSize = 0;
}

// ============================================================================
// TaskSys queue helpers (local dlist view)
// ============================================================================
struct DListNode {
    DListNode* m_next;  // +0x00
    DListNode* m_prev;  // +0x04
};

struct DList {
    DListNode m_end;     // +0x00 (sentinel: next/prev)
    DListNode* m_head;   // +0x08
    DListNode* m_tail;   // +0x0C
    int m_size;          // +0x10
};

// TaskHandler view for queue ops (mTaskId +8, mFlags +0xC, mTaskList +0x10)
struct TaskHandlerView {
    void* m_dlist[2];    // +0x00
    unsigned int mTaskId;  // +0x08
    unsigned int mFlags;   // +0x0C
    DList mTaskList;       // +0x10
};

// ea: 0x4FFA80
void TaskSys_PostTask(DList* mPostQueue, Task* t,
                      TaskHandlerView* (*lookup)(unsigned int id))
{
    if (t != nullptr)
    {
        TaskHandlerView* v3 = lookup(t->mTaskId);
        if (v3 != nullptr && (v3->mFlags & 8) != 0)
            t->mFlags |= 4u;
        DListNode* node = (DListNode*)&t->_dlist[0];
        node->m_next = &mPostQueue->m_end;
        node->m_prev = mPostQueue->m_tail;
        mPostQueue->m_tail->m_next = node;
        mPostQueue->m_tail = node;
        ++mPostQueue->m_size;
    }
}

// ea: 0x4FFAE0
void TaskSys_SendTask(TaskHandlerView* handler, Task* t)
{
    if (t != nullptr)
    {
        DListNode* sentinel = &handler->mTaskList.m_end;
        DListNode* node = (DListNode*)&t->_dlist[0];
        node->m_next = sentinel + 1;
        DListNode* m_prev = handler->mTaskList.m_tail;
        node->m_prev = m_prev;
        m_prev->m_next = node;
        handler->mTaskList.m_tail = node;
        ++handler->mTaskList.m_size;
    }
}
