// ============================================================================
// cg_local.h - client game (cg.o) shared types and renderer exports
// ============================================================================

#pragma once

#include <stdint.h>

#include "core/math_types.h"
#include "engine/broc_types.h"
#include "game/game_types.h"

struct Entity;
class DObj;
namespace nalGeneric { class nalGenericAnim; }

// Minimal view of AnimationPlayer (full class in anim.o; cg.o members below)
class AnimationPlayer {
public:
    float GetAnimTime(nalGeneric::nalGenericAnim* anim);  // ea: 0x006BBD00
    bool IsPartialIdle(bool checkLooping);                 // ea: 0x006BBD60
};

// Renderer export table (refexport_t; matches the layout used by core.o
// common.cpp's re_export_view and cl_parse.cpp's refexport_t2)
struct refexport_t {
    void (*Shutdown)(int);
    void (*BeginRegistration)(void*);
    void* (*RegisterModel)(void* result, const char*, int, int);
    int (*RegisterShader)(const char*, int);
    int (*RegisterShaderNoMip)(const char*, int);
    void (*LoadWorld)(const char*, int*);
    void (*SetFXImageMemory)(int);
    int (*GetFXImageMemory)();
    int (*GetImageMemory)();
    float (*GetFarPlaneDist)();
    void (*EndRegistration)();
    void (*ClearScene)();
    void (*AddPolyToScene)(void*, int, const void*);
    void (*AddLightToScene)(const float*, float, float, float, float);
    void (*SetCullDist)(float);
    void (*SetFog)(int, int, int, float, float, float, float);
    void (*RenderScene)(const void*);
    void (*ClearFlares)();
    void (*SetColor)(const float*);
    void (*DrawStretchPic)(float, float, float, float, float, float, float,
                           float, void*);
    void (*DrawStretchPicGradient)(float, float, float, float, float, float,
                                   float, float, void*, const float*, int);
    void (*DrawStretchPicRotate)(float, float, float, float, float, float,
                                 float, float, float, void*);
    void (*DrawQuadPic)(const float (*)[2], const float (*)[2], void*);
    void (*DrawStretchRaw)(int, int, int, int, int, int,
                           const unsigned char*, int, int);
    void (*UploadCinematic)(int, int, int, int, const unsigned char*, int, int);
    void (*BeginFrame)();
    void (*EndFrame)(int*, int*);
    void (*SaveScreen)();
    void (*TrackStatistics)(void*);
    int (*PickShader)(const float*, const float*, char*, char*, char*, int);
    void (*ResetImageAllocations)();
    void (*FreeImageAllocations)();
    void (*CubemapShot)(const char*, int, int, float, float);
    void (*CubemapWaterShot)(const char*, int, int, float*, float*);
    void (*LocateDebugStrings)(void*, int);
    void (*LocateDebugLines)(void*, int);
    int (*Text_Width)(const char*, int, float, float, int);
    int (*Text_Height)(int, float);
    void (*Text_Paint)(float, float, int, float, const float*, const char*,
                       float, int, int);
    int (*Text_ConsoleWidth)(const short*, int, float, float, int);
    void (*Text_ConsolePaint)(float, float, int, float, const float*,
                              const short*, float, int, int);
    void (*Text_PaintWithCursor)(float, float, int, float, const float*,
                                 const char*, int, char, float, int, int);
};

extern refexport_t re;

extern "C" void* _Z_MallocInternal(int size);
extern "C" void  _Z_FreeInternal(void* ptr);

// EntityHandleDb - entity handle database (full layout, verified against IDA)
struct EntityHandleDbDbElement {
    Entity* mObject;  // +0x00
    int     mKey;     // +0x04
};
class EntityHandleDb {
public:
    uint8_t  _pad[0xA8];                 // HandleDb BitSet<1344> (168 bytes)
    EntityHandleDbDbElement mElements[0x540];  // +0xA8 (1344 * 8 = 10752)
    uint8_t  mDebugCallback[4];          // +0x2AA8
    struct AeSizedEntityArray {
        Entity* m_elements[4096];        // +0x00
        int     m_size;                  // +0x4000
    } mActiveList;                       // +0x2AAC (16388 bytes)
    static EntityHandleDb sInst;         // ?sInst@EntityHandleDb@@0V1@A
    void AssignHandle(Entity& e);        // ?AssignHandle@EntityHandleDb@@QAEXAAVEntity@@@Z
    Entity* GetObject(unsigned int val) const {
        unsigned int idx = val & 0xFFF;
        if (idx < 0x540 && val >> 12 == (unsigned int)mElements[idx].mKey)
            return mElements[idx].mObject;
        return NULL;
    }
};
static_assert(offsetof(EntityHandleDb, mElements) == 0xA8, "EntityHandleDb::mElements offset mismatch");
static_assert(sizeof(EntityHandleDb) == 27312, "EntityHandleDb size mismatch");

struct weaponFileInfo_t {
    char  szKillIcon[64];      // +0x00
    char  szInternalName[64];  // +0x40
    unsigned char bWideKillIcon;  // +0x80
};

struct weaponFileInfoFull : weaponFileInfo_t {
    int  weapClass;       // +0x84
    int  slot;            // +0x88
    int  bOffHand;        // +0x8C
    int  bSlotStackable;  // +0x90
    int  bBoltAction;     // +0x94
    int  bADSPositionInfo; // +0x98
    int  bAnimateCamReload;  // +0x9C
    int  bAnimateCamMelee;   // +0xA0
    int  bAnimateCamFire;    // +0xA4
    int  ammoType;           // +0xA8
    int  iReticleCenterSize; // +0xAC
    float fOOPosAnimLength[2];  // +0x8C0 (offset in actual struct)
};

class DObj {
public:
    void* tree[8];        // +0x00 XAnimTree*[8]
    void* animPlayers[8]; // +0x20 AnimationPlayer*[8]
    void* mPose[8];       // +0x40
    unsigned char modelParents[8];  // +0x60
    unsigned char matOffset[8];     // +0x68
    void* skel;           // +0x70
    void* animToModel;    // +0x74
    unsigned int gameId;  // +0x78
    int ignoreCollision;  // +0x7C
    IVPointerRaw models[8];  // +0x80
    int  mPakId;          // +0xC0
    IVPointerRaw mPhysData;  // +0xC4
    unsigned short duplicateParts;  // +0xCC
    unsigned char numModels;        // +0xCE
    unsigned char numBones;         // +0xCF
    Entity* mEntity;      // +0xD0
    unsigned int mHandle; // +0xD4
    int mLOD;             // +0xD8
    int mLODOverride;     // +0xDC
    int mLODAnim;         // +0xE0
    unsigned int mFlags;  // +0xE4

    TPakId GetPakId() const;              // ?GetPakId@DObj@@QBE?AW4TPakId@@XZ (anim.o 0x53A890)
    int GetNonAnimLOD() const;            // ?GetNonAnimLOD@DObj@@QBEHXZ (anim.o 0x53A8A0)
    void SetLODOverride(int startLod);    // ?SetLODOverride@DObj@@QAEXH@Z (anim.o 0x5612E0)
};

struct XAnimEntry {
    unsigned int hash;        // +0x00
    unsigned short numAnims;  // +0x04
    unsigned short parent;    // +0x06
    void* anim;               // +0x08 nalGeneric::nalGenericAnim*
    void* notify;             // +0x0C
    int   lastAttempt;        // +0x10
    unsigned char ucLastChosenChild;  // +0x14
    unsigned char _pad[3];    // +0x15
    union {
        struct {
            unsigned short flags;    // +0x18
            unsigned short children; // +0x1A
        } s;
    } u;                      // +0x18
};

class AnimTree {
public:
    void* name;               // +0x00 InplaceString
    struct {
        unsigned int mSize;   // +0x04
        XAnimEntry*  mList;   // +0x08
    } entries;
};

struct weaponInfo_s {
    float viewModelAnimRates[25];  // +0x00
    char  handModel[24];           // +0x64
    unsigned char registered;      // +0x7C
    const void* item;              // +0x80
    const char* pszTranslatedDisplayName;  // +0x84
    const char* pszTranslatedModename;     // +0x88
    const char* pszTranslatedAIOverlayDescription;  // +0x8C
    IVPointerRaw iWorldSurfIndex;   // +0x90
    IVPointerRaw iPickupSurfIndex;  // +0x98
    void* weaponIcon[2];         // +0xA0
    void* ammoIcon;              // +0xA8
    void* hHudIcon;              // +0xAC
    void* hAmmoIcon;             // +0xB0
    int   missileRenderfx;       // +0xB4
    void* hReticleCenter;        // +0xB8
    void* hReticleSide;          // +0xBC
    void* hADSOverlay;           // +0xC0
    IVPointerRaw iMissileSurfIndex; // +0xC4
};

struct refEntity_t {
    int   reType;          // +0x00
    int   renderfx;        // +0x04
    float lightingOrigin[3]; // +0x08
    float axis[3][3];      // +0x14
    float scale;           // +0x38
    float origin[3];       // +0x3C
    float oldorigin[3];    // +0x48
    void* obj;             // +0x54
    Entity* entity;        // +0x58
    void* pStaticModel;    // +0x5C
};

struct scr_vehicle_t {
    void* gunnerWeapon;  // +0x00
    void* altWeapon;     // +0x04
    int   shooter;       // +0x08
    uint8_t _pad0C[0x178 - 0x0C];
    int16_t infoIdx;     // +0x178
};

// PlayerState - subset of the fields cg.o touches (full size 0x5D0)
class PlayerState {
public:
    math::Position3 origin;            // +0x00
    math::Position3 velocity;          // +0x10
    unsigned char bobCycle;            // +0x28
    int pm_flags;                      // +0x2C
    int delta_angles[3];               // +0x54
    int jumpTime;                      // +0x70
    unsigned int mTarget;              // +0x94
    int mTargetTime;                   // +0x98
    int weaponstate;                   // +0xA8
    float fWeaponPosFrac;              // +0xAC
    unsigned char queuedReloadSoundPlayStarted;  // +0xB8
    int lastWeapon;                    // +0xC4
    int eFlags;                        // +0xF4
    int pm_type;                       // +0x24
    int weaponTime;                    // +0x34
    int weapAnim;                      // +0x530
    unsigned int mViewLockedEntity;    // +0x4A0
    float leanf;                       // +0x4C
    int proneViewHeight;               // +0x458
    int crouchViewHeight;              // +0x45C
    int standViewHeight;               // +0x460
    float viewHeightCurrent;           // +0xE0
    int viewHeightTarget;              // +0xDC
    float mHoldBreathScale;            // +0x5C0
    float viewangles[3];               // +0xD0
    int ammoclip[92];                  // +0x2B4
    int weapons[2];                    // +0x424
    unsigned char weaponslots[10];     // +0x42C
    int weapon;                        // +0xA4
    int vehPos;                        // +0x524
    int vehType;                       // +0x528
    int vehSubType;                    // +0x52C
    unsigned int mFlags;               // +0x5C8
    unsigned char _pad[0x5D0 - 0x5CC];
};

struct AnimIKFireEvent {
    int fireTime;    // +0x00
    int fireWeapon;  // +0x04
};

struct Client {
    PlayerState ps;  // +0x00
    AnimIKFireEvent AnimIKFireEvents[15];  // +0x5D0
    int mNoDrawTime;  // +0x654
    float mLastTorsoIKLegsYaw;  // +0x658
    struct {
        int playerState;  // +0x00
    } pers;              // +0x65C
};

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

#define CG_ASSERT(expr, file, line)                                       \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)
