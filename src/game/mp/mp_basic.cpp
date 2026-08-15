// ============================================================================
// mp_basic.cpp - multiplayer game modes (mp.o) batch 1: small helpers
// Reconstructed from IDA decompiles/disasm of the release XBE.
// ============================================================================

#include "game/logic/g_local.h"
#include "game/mp/mp_types.h"
#include "game/platform_xbox/XboxLive.h"
#include "bd/bdNet.h"
#include "bd/bdQoSProbe.h"
#include "bd/bdTiming/bdShortTimer.h"

#include <stdio.h>
#include <math.h>

#pragma comment(lib, "ws2_32.lib")

// EDroppedItemTypes definition (avoid g_local.h anonymous-enum collisions)
enum EDroppedItemTypes : int {
    kItemTypeSupport = 2,
};

int XNetCleanup();  // Xbox shim; Win32 no-op
int XNetCleanup()
{
    return 0;
}

extern void nslUpdate();   // nsl_xboxr (nsl.cpp)
extern char byte_1869F;    // unnamed byte global referenced by GetQosPing
extern bool gSkipMovies;   // GameXbox.cpp (?gSkipMovies@@3_NA)

extern int XNetGetEthernetLinkStatus();   // Xbox XNet; Win32 shim (unresolved)
extern void VEH_RespawnVehicle(Entity* ent);  // g.o (?VEH_RespawnVehicle@@YAXPAVEntity@@@Z)
extern void PlayerDead(Entity* self, Entity* inflictor, Entity* attacker,
                       int damage, int meansOfDeath, int weapon,
                       const float* position, const float* dir,
                       EHitLocation hitLoc);  // g.o (g_combat.cpp)
extern const float VectorNormalize(float* const v);  // math lib (?VectorNormalize@@YA?BMQAM@Z)
extern const float vectoyaw(const float* const v);   // math lib (?vectoyaw@@YA?BMQBM@Z)
extern const float AngleMod(const float a);          // math lib (?AngleMod@@YA?BMM@Z)
extern const unsigned char DirToByte(const float* const dir);  // ?DirToByte@@YA?BEQBM@Z
extern vmCvar_t cg_widescreen;   // ?cg_widescreen@@3UvmCvar_t@@A (cg.o)
extern kuju::knet::sTime gStartupAverageUpdateInterval;  // @ 0xE370D4
extern void j_nullsub_96();      // ?nullsub_96 (shell.o)
extern int dword_F32F40;         // 0xF32F40
extern int dword_F34B34;         // 0xF34B34
extern int dword_F36728;         // 0xF36728

extern bool MI_IsAvailableMap(char mapIndex);   // ?MI_IsAvailableMap@@YA_ND@Z (mp_shell.o)
extern const char* MI_GetMapDisplayName(char mapIndex);  // ?MI_GetMapDisplayName@@YAPADD@Z (mp_shell.o)
extern void ByteToDir(int b, float* dir);       // ?ByteToDir@@YAXHQAM@Z (core.o)
extern void ClientSpawn(Entity* ent, const float* origin, const float* angles,
                        bool stopPhysics, bool isRevive);  // ?ClientSpawn@@YAXPAVEntity@@QBM1_N2@Z (g.o)
extern void Axis4ToAngles(const float (*const axis)[4], float* const angles);  // core.o
extern void tlPrintf(const char* format, ...);  // ?tlPrintf@@YAXPBDZZ
extern void bdCore_quit();  // bdCore::quit
extern void* gDWHeap;       // ?gDWHeap@@3PAVae_heap@@A

struct scr_vehicle_t;

// MPLiveEngine (platform_xbox) - cross-object extern view; derives from
// LiveWrapper in the binary so sessionState/SetNotificationFlag resolve there
class MPLiveEngine : public LiveWrapper {
public:
    static MPLiveEngine* GetHandle();  // ?GetHandle@MPLiveEngine@@SAPAV1@XZ
    void LeaveLiveSession();           // ?LeaveLiveSession@MPLiveEngine@@QAEXXZ
    int  RegisterKey(const XNKID* sessionID, const XNKEY* securityKey);  // ?RegisterKey@MPLiveEngine@@QAEHPBUXNKID@@PBUXNKEY@@@Z
    void SubmitVoiceData(const XUID& talker, void* buffer,
                         unsigned long bufferLength);  // ?SubmitVoiceData@MPLiveEngine@@UAEXABU_XUID@@PAXK@Z
    unsigned int actualPort;           // +0x45D4
    bool invited;                      // +0x45D8
};

// AnimationPlayer (anim.o) - minimal view used by PlayAnimFlagAnim
namespace nalGeneric { class nalGenericAnim; }
class AnimationPlayer {
public:
    enum AnimationPlayerModifierType {
        nalAdditiveModifier = 0,
        nalPartialModifier = 1,
        nalFullModifier = 2,
    };
    void PlayModifier(nalGeneric::nalGenericAnim* anim, float priority,
                      unsigned int mask);  // ?PlayModifier@AnimationPlayer@@QAEXPAVnalGenericAnim@nalGeneric@@MI@Z
    bool SetModifierAlpha(unsigned int mask, float priority,
                          nalGeneric::nalGenericAnim* anim,
                          float _maxAlpha);  // ?SetModifierAlpha@AnimationPlayer@@QAE_NIMPAVnalGenericAnim@nalGeneric@@M@Z
    bool SetModifierSpeed(unsigned int mask, float priority,
                          nalGeneric::nalGenericAnim* anim,
                          float _speed);  // ?SetModifierSpeed@AnimationPlayer@@QAE_NIMPAVnalGenericAnim@nalGeneric@@M@Z
    void SetModifierType(unsigned int mask,
                         AnimationPlayerModifierType type);  // ?SetModifierType@AnimationPlayer@@QAEXIW4AnimationPlayerModifierType@1@@Z
    void StopModifiers(unsigned int mask);  // ?StopModifiers@AnimationPlayer@@QAEXI@Z
};

// MP_ANIM_INDEX / MP_ANIM_LOOKUP - mp.o animation tables (IDA types)
struct MP_ANIM_INDEX {
    char* name;                        // +0x00
    unsigned int ID;                   // +0x04
    nalGeneric::nalGenericAnim* anim;  // +0x08
    unsigned short flags;              // +0x0C
    unsigned short padding;            // +0x0E
};
struct MP_ANIM_CELL_FIELDS {
    unsigned short numAnims;   // +0x00
    unsigned short animIndex;  // +0x02
};
struct MP_ANIM_LOOKUP {
    MP_ANIM_CELL_FIELDS anims[4];
};
extern MP_ANIM_INDEX base_anim_names[];     // ?base_anim_names@@3PAUMP_ANIM_INDEX@@A (mp.o @ 0x1381388)
extern MP_ANIM_LOOKUP base_anim_indices[];  // ?base_anim_indices@@3PAUMP_ANIM_LOOKUP@@A (mp.o @ 0x138345C)

// OverlayMenu (mp_shell.o) - session join feedback overlay
class OverlayMenu {
public:
    static OverlayMenu* Me(int version);  // ?Me@OverlayMenu@@SAPAV1@H@Z
    void SetState(int state);             // ?SetState@OverlayMenu@@QAEXW4eState@1@@Z
    void* GetActiveMenu();                // vtable slot 0xE0
    int   GetAcceptMenu();                // vtable slot 0x60
    int   GetBackMenu();                  // vtable slot 0x64
};


// Xbox XNetStartup shim (Win32 no-op; XNetCleanup below)
extern "C" int __stdcall XNetStartup(void* pxnsp);
extern "C" int __stdcall XNetStartup(void* pxnsp)
{
    (void)pxnsp;
    return 0;
}

// ============================================================================
// MPPlayerSet (mp.o)
// ============================================================================
// ea: 0x00730490
unsigned long MPPlayerSet::numberOfPlayers() const
{
    unsigned int mBitPlayers = this->mBitPlayers;
    unsigned int v2 = 0;
    if (mBitPlayers != 0)
    {
        do
        {
            if ((mBitPlayers & 1) != 0)
                ++v2;
            mBitPlayers >>= 1;
        } while (mBitPlayers != 0);
        if (v2 > 0x10)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerSet.cpp";
            AeAssert::gCurrentLine = 178;
            AeAssert::gCurrentExpr = "count <= 16";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(defaultFileName))
                __debugbreak();
        }
    }
    return v2;
}

// ============================================================================
// MPUtility (mp.o) - bit-buffer serialization writers
// ============================================================================
// ea: 0x0073BDB0
void MPUtility::WritePlayerId(bdReference<bdBitBuffer> buffer,
                              unsigned char id)
{
    buffer.m_ptr->writeRangedUInt32(id, 0, 0x10u, true);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073BE90
void MPUtility::WriteVehicleId(bdReference<bdBitBuffer> buffer,
                               unsigned char id)
{
    buffer.m_ptr->writeRangedUInt32(id, 0, 0xAu, true);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073BF70
void MPUtility::WriteSeatIndex(bdReference<bdBitBuffer> buffer, int seatIdx)
{
    buffer.m_ptr->writeRangedUInt32(seatIdx, 0, 0xBu, true);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073C050
void MPUtility::WriteEntryPoint(bdReference<bdBitBuffer> buffer,
                                int entryIdx)
{
    buffer.m_ptr->writeRangedUInt32(entryIdx, 0, 6u, true);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073C130
void MPUtility::WritePlayerClass(bdReference<bdBitBuffer> buffer,
                                 int playerclass)
{
    buffer.m_ptr->writeRangedInt32(playerclass + 1, 0, 7);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ============================================================================
// MPLevelLoader (mp.o)
// ============================================================================
// ea: 0x00765DB0
unsigned int MPLevelLoader::run(void* args)
{
    (void)args;
    bdPlatformTiming::sleep(0xC8u);
    while (!m_stop)
    {
        MultiplayerMgr::sInst->Step(0, true, true);
        bdPlatformTiming::sleep(0xC8u);
    }
    return 0;
}

// ea: 0x0072E8A0 (playerState values 4/5/1 from the release binary)
bool IsACorpse(const Entity* player)
{
    int playerState = player->client->pers.playerState;
    return playerState == 4 || playerState == 5 || playerState == 1;
}

// ============================================================================
// cThreadSleep (mp.o 0x7305B0) - release no-ops (disasm: retn)
// ============================================================================
void cThreadSleep::sleepSeconds(unsigned long) {}
void cThreadSleep::sleepMilliseconds(unsigned long) {}

// ============================================================================
// MPPlayer (mp.o)
// ============================================================================
unsigned char MPPlayer::GetNullId()
{
    return 16;
}

unsigned char MPPlayer::GetId() const
{
    return mId;
}

// ea: 0x0072CE90
void MPPlayer::SetId(unsigned char id)
{
    mId = id;
}

// ea: 0x0072CE70
bool MPPlayer::IsValid(unsigned char id)
{
    return id < 0x10u;
}

// ea: 0x00735FF0
bool MPPlayer::IsValid() const
{
    return mId < 0x10u && mConnection.m_ptr != nullptr;
}

// ea: 0x00736010
bool MPPlayer::IsConnected() const
{
    return mConnection.m_ptr != nullptr
           && mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED;
}

// ea: 0x0072DDE0
int MPPlayer::FootstepEvent(int surfaceFlags)
{
    int v3 = GroundSurfaceType(surfaceFlags);
    if (v3 == 0 || bProne || bCrouching)
        return 0;
    if (bSprinting)
        return v3 + 70;
    if (bWalking)
        return v3 + 24;
    return v3 + 1;
}

// ea: 0x007360E0 (refcount at m_ptr+0x04; virtual dtor release)
void MPPlayer::SetInvalid()
{
    bdConnection* m_ptr = mConnection.m_ptr;
    if (m_ptr != nullptr && --*(int*)((char*)m_ptr + 0x04) == 0)
    {
        bdConnection* v4 = mConnection.m_ptr;
        if (v4 != nullptr)
            delete v4;
    }
    mConnection.m_ptr = nullptr;
    mId = 16;
    mClientIndex = -1;
}

// ea: 0x0072DFD0 (mInterpolatedHeading.mAngle at +0x218)
void MPPlayer::GetAngles(float (&angles)[3]) const
{
    angles[2] = 0.0f;
    angles[0] = 0.0f;
    angles[1] = *(float*)((char*)this + 0x218);
}

// ea: 0x0072DFA0 (mInterpolatedPosition at +0x1F0)
void MPPlayer::GetPosition(float (&position)[3]) const
{
    position[0] = *(float*)((char*)this + 0x1F0);
    position[1] = *(float*)((char*)this + 0x1F4);
    position[2] = *(float*)((char*)this + 0x1F8);
}

// ============================================================================
// MPVehicle (mp.o)
// ============================================================================
unsigned char MPVehicle::GetNullId()
{
    return 10;
}

unsigned char MPVehicle::GetId() const
{
    return mId;
}

// ea: 0x0072E450
void MPVehicle::SetId(unsigned char id)
{
    mId = id;
}

// ea: 0x0072E470
bool MPVehicle::IsOccupied() const
{
    return mNumOccupants != 0;
}

// ea: 0x0072E430
bool MPVehicle::IsValid(unsigned char id)
{
    return id < 0x0Au;
}

// ea: 0x0072E1A0 (seats at +0x08, 12 bytes, 0x10 = MPPlayerSet empty slot)
void MPVehicle::ClearOccupants()
{
    mNumOccupants = 0;
    memset((char*)this + 0x08, 0x10, 0x0C);
}

// ea: 0x00736EC0
void MPVehicle::SetInvalid()
{
    mEntity = nullptr;
    mNumOccupants = 0;
    mId = 10;
    memset((char*)this + 0x08, 0x10, 0x0C);
}

// ea: 0x007482E0 (mInterpolator at +0x80; mInitialDate/mTimeInterval floats at
// +0x114/+0x118)
MPVehicle::MPVehicle()
{
    mId = 10;
    mEntity = nullptr;
    mNumOccupants = 0;
    mLastReceivedTime.mTime = 0;
    mInterpolator.mInitialDate = 0.0f;
    mInterpolator.mTimeInterval = 0.0f;
    mLastInterpolatedTime.mTime = 0;
    Reset(true);
}

// ea: 0x0072E480
bool MPVehicle::IsFullyOccupied() const
{
    return mNumOccupants == G_GetVehicleSeatCount((Entity*)mEntity);
}

// ea: 0x0072E4A0 (seats at +0x08, 1-byte occupant; empty slot = 16)
bool MPVehicle::IsSeatOccupied(int vehSeatIdx,
                               bool ConsiderEachPositionUnique) const
{
    bool result = *(unsigned char*)((char*)this + 0x08 + vehSeatIdx) != 16;
    if (!ConsiderEachPositionUnique && !result && vehSeatIdx == 1)
        result = *(unsigned char*)((char*)this + 0x0E) != 16;
    return result;
}

// ea: 0x0072E190
MPVehicle::~MPVehicle()
{
}

// ============================================================================
// MPPlayerItems (mp.o)
// ============================================================================
// ea: 0x0072E020
ae_vector<MPPlayerItems::sDroppedItem>&
MPPlayerItems::GetItemList(EDroppedItemTypes item)
{
    switch (item)
    {
    case (EDroppedItemTypes)2:  // kItemTypeSupport
        return mDroppedSupport;
    case kItemTypeMines:
        return mDroppedMines;
    case (EDroppedItemTypes)3:  // kItemTypeMax
        return mDroppedKits;
    default:
        return mDroppedWeapons;
    }
}

// ea: 0x00754BA0
void MPPlayerItems::sDroppedItem::Destroy()
{
    unsigned int v1 = handle.mVal & 0xFFF;
    if (v1 < 0x540
        && handle.mVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && EntityHandleDb::sInst.mElements[v1].mObject != nullptr)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        mObject->think = THINK__G_FreeEntity;
        mObject->nextthink = level.time + 1;
    }
    handle.mVal = 0;
    time = 0;
}

// ea: 0x00755140
Entity* MPPlayerItems::FindItem(EDroppedItemTypes item, short id)
{
    ae_vector<sDroppedItem>* list;
    switch (item)
    {
    case (EDroppedItemTypes)2:  // kItemTypeSupport
        list = &mDroppedSupport;
        break;
    case kItemTypeMines:
        list = &mDroppedMines;
        break;
    case (EDroppedItemTypes)3:  // kItemTypeMax
        list = &mDroppedKits;
        break;
    default:
        list = &mDroppedWeapons;
        break;
    }
    if (list->mSize <= id)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerItems.cpp";
        AeAssert::gCurrentLine = 118;
        AeAssert::gCurrentExpr = "size > id";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("FindItem: Invalid ID"))
            __debugbreak();
    }
    unsigned int mVal = (*list)[id].handle.mVal;
    unsigned int v5 = mVal & 0xFFF;
    if (v5 < 0x540
        && mVal >> 12 == EntityHandleDb::sInst.mElements[v5].mKey)
        return EntityHandleDb::sInst.mElements[v5].mObject;
    return nullptr;
}

// ea: 0x007551F0
void MPPlayerItems::SetItem(EDroppedItemTypes item, short id, Entity* ent)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerItems.cpp";
        AeAssert::gCurrentLine = 149;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "MPPlayerItems::AddItem invalid entity passed in"))
            __debugbreak();
    }
    ae_vector<sDroppedItem>* list = &mDroppedWeapons;
    switch (item)
    {
    case (EDroppedItemTypes)2:  // kItemTypeSupport
        list = &mDroppedSupport;
        break;
    case kItemTypeMines:
        list = &mDroppedMines;
        break;
    case (EDroppedItemTypes)3:  // kItemTypeMax
        list = &mDroppedKits;
        break;
    default:
        break;
    }
    int mSize = list->mSize;
    if (id < 0 || id >= mSize)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerItems.cpp";
        AeAssert::gCurrentLine = 153;
        AeAssert::gCurrentExpr = "id >= 0 && id < size";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "MPPlayerItems::AddItem invalid id for item type %i", item))
            __debugbreak();
    }
    if (id >= 0 && id < mSize)
    {
        (*list)[id].Destroy();
        (*list)[id].time = level.time;
        (*list)[id].handle.mVal = ent->mHandle.mHandle.mVal;
    }
}

// ea: 0x00754C90
void MPPlayerItems::RemoveAll()
{
    if (mDroppedWeapons.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    mDroppedWeapons[0].Destroy();
    for (int i = 0; i < 3; ++i)
    {
        if (i < 0 || i >= mDroppedSupport.mSize)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 167;
            AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        mDroppedSupport[i].Destroy();
    }
    for (int j = 0; j < 3; ++j)
    {
        if (j < 0 || j >= mDroppedMines.mSize)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 167;
            AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        mDroppedMines[j].Destroy();
    }
    if (mDroppedKits.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    mDroppedKits[0].Destroy();
}

// MPGameInfo (mp.o 0x730510)
void MPGameInfo::getSlots(unsigned char& publicOpen,
                          unsigned char& privateOpen,
                          unsigned char& publicFilled,
                          unsigned char& privateFilled) const
{
    publicOpen = m_publicOpen;
    privateOpen = m_privateOpen;
    publicFilled = m_publicFilled;
    privateFilled = m_privateFilled;
}

// ============================================================================
// MPLanDiscovery (mp.o 0x72CB90)
// ============================================================================
unsigned int MPLanDiscovery::GetNumResults() const
{
    return mNumResults;
}

// ea: 0x0072CB60
bool MPLanDiscovery::IsDone()
{
    bdDiscoveryStatus Status = mDiscoveryClient.getStatus();
    if (Status == BD_DISCOVERY_IDLE || Status == BD_DISCOVERY_ERROR)
        return true;
    mDiscoveryClient.update();
    return false;
}

// ea: 0x00746050 (mResults at +0x4C)
void MPLanDiscovery::GetResult(unsigned int index,
                               bdReference<bdGameInfo>& result) const
{
    if (index < mNumResults)
    {
        bdGameInfo* m_ptr = mResults[index].m_ptr;
        if (m_ptr != nullptr)
        {
            int m_refCount = m_ptr->m_refCount;
            m_ptr->m_refCount = m_refCount + 1;
            m_ptr->m_refCount = m_refCount;
            if (m_refCount == 0)
                delete m_ptr;
        }
        if (result.m_ptr != nullptr && result.m_ptr->m_refCount-- == 1
            && result.m_ptr != nullptr)
            delete result.m_ptr;
        result.m_ptr = m_ptr;
        if (m_ptr != nullptr)
            ++m_ptr->m_refCount;
    }
}

// ============================================================================
// MPUIInterface statics (mp.o)
// ============================================================================
int MPUIInterface::mReturnMenu;
bool MPUIInterface::mKicked;
bool MPUIInterface::mHostMigrated;
bool MPUIInterface::mHostDisconnected;
unsigned long MPUIInterface::mGameListingNumGames;
unsigned char MPUIInterface::mGameListings[1] = { 0 };

const int MPUIInterface::GetTimeLimitCount()
{
    return 6;
}

const int MPUIInterface::GetRoundLimitCount()
{
    return 1;
}

const int MPUIInterface::GetMaxPlayersCount()
{
    return 4;
}

const int MPUIInterface::GetRespawnTimeCount()
{
    return 3;
}

const int MPUIInterface::GetReturnMenu()
{
    return mReturnMenu;
}

void MPUIInterface::GameListingEnd()
{
}

void MPUIInterface::StartDevice()
{
}

// ea: 0x00730080
void MPUIInterface::PlatformStop()
{
    WSACleanup();
    XNetCleanup();
}

// ea: 0x0072F380
void MPUIInterface::Reboot()
{
    MusicMgr::sInst->Stop(0.0f);
    nslUpdate();
    gSkipMovies = true;
    gReturnToMenu = true;
    Cmd_ExecuteServerString("reboot");
}

// ea: 0x0072F770
const char* MPUIInterface::GetGameTypeString(unsigned long gameType)
{
    if (gameType >= 6)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 1323;
        AeAssert::gCurrentExpr = "gameType < GAME_TYPE_LIMIT";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return mGameTypeStrings[gameType];
}

// ea: 0x0072F7D0
const char* MPUIInterface::GetGameTypeShortString(unsigned long gameType)
{
    if (gameType >= 6)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 1329;
        AeAssert::gCurrentExpr = "gameType < GAME_TYPE_LIMIT";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return mGameTypeShortStrings[gameType];
}

// ea: 0x0072F8A0
const char* MPUIInterface::GetMapRotationString(unsigned long mapRotation)
{
    if (mapRotation >= 4)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 1342;
        AeAssert::gCurrentExpr = "mapRotation < MAP_ROTATION_LIMIT";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return mMapRotationStrings[mapRotation];
}

// ea: 0x0072F910
const int MPUIInterface::GetTimeLimit(unsigned long index)
{
    if (index >= 6)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 1353;
        AeAssert::gCurrentExpr = "index < GetTimeLimitCount()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return mTimeLimitList[index];
}

// ea: 0x0072FB20
const int MPUIInterface::GetRoundLimit(unsigned long index)
{
    if (index != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 1413;
        AeAssert::gCurrentExpr = "index < GetRoundLimitCount()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return mRoundLimitList[index];
}

// ea: 0x0072FB90
const int MPUIInterface::GetMaxPlayers(unsigned long index)
{
    if (index >= 4)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 1424;
        AeAssert::gCurrentExpr = "index < GetMaxPlayersCount()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return mMaxPlayerList[index];
}

// ea: 0x0072FC00
const int MPUIInterface::GetRespawnTime(unsigned long index)
{
    if (index >= 3)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 1435;
        AeAssert::gCurrentExpr = "index < GetRespawnTimeCount()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return mRespawnTimeList[index];
}

// ea: 0x00766000
void MPUIInterface::bdNetStop()
{
    bdNetImpl* Instance = bdSingleton<bdNetImpl>::getInstance();
    if (Instance->getStatus() != BD_NET_STOPPED)
    {
        bdSingleton<bdNetImpl>::getInstance()->stop();
        bdNetImpl* v3 = bdSingleton<bdNetImpl>::getInstance();
        while (v3->getStatus() != BD_NET_STOPPED)
        {
            MultiplayerMgr::sInst->Step(0, false, true);
            v3 = bdSingleton<bdNetImpl>::getInstance();
        }
    }
}

void MPUIInterface::getLocalAddresses(bdArray<bdInetAddr>&)
{
}

void MPUIInterface::Logging(bool)
{
}

// ea: 0x00730090
bool MPUIInterface::NextRoundMapChanges()
{
    return mNextServerParams.mMapID != mServerParams.mMapID;
}

// ea: 0x007300A0
bool MPUIInterface::NextRoundMapRestart()
{
    return mNextServerParams.mMapID == mServerParams.mMapID
           && mNextServerParams.mGameType != mServerParams.mGameType;
}

// ea: 0x0072F730
const bool MPUIInterface::IsGameListingComplete()
{
    if (mGameConnectionType == kGameConnectionTypeLocal)
        return true;
    bool v1 = mLanDiscoveryActive;
    if (mGameConnectionType != kGameConnectionTypeLan)
        v1 = mLiveQueryActive;
    return !v1;
}

// ea: 0x00730240
void MPUIInterface::SetServerParams(const sServerCreateParams& a_ServerParams)
{
    mServerParams = a_ServerParams;
}

// ea: 0x0072F520
void MPUIInterface::SetQueryParams(sServerQueryParams& params)
{
    mQueryParams = params;
}

struct sServerCreateParams MPUIInterface::mServerParams;
struct sServerCreateParams MPUIInterface::mNextServerParams;
bool MPUIInterface::mLanDiscoveryActive;
struct sServerQueryParams MPUIInterface::mQueryParams;

const char* MPUIInterface::mGameTypeStrings[6] = {
    "MPFRONTEND_WARLONG", "MPFRONTEND_CAPTURE_THE_FLAG",
    "MPFRONTEND_SINGLE_CAPTURE_THE_FLAG", "MPFRONTEND_HEADQUARTERS",
    "MPFRONTEND_TEAM_BATTLE", "MPFRONTEND_BATTLE",
};
const char* MPUIInterface::mGameTypeShortStrings[6] = {
    "MPFRONTEND_WAR", "MPFRONTEND_CTF", "MPFRONTEND_SCF",
    "MPFRONTEND_HQ", "MPFRONTEND_TBT", "MPFRONTEND_BAT",
};
const char* MPUIInterface::mMapRotationStrings[4] = {
    "MPFRONTEND_NONE", "MPFRONTEND_FORWARDS", "MPFRONTEND_BACKWARDS",
    "MPFRONTEND_RANDOM",
};
const int MPUIInterface::mTimeLimitList[6] = { 1, 5, 10, 20, 30, 60 };
const int MPUIInterface::mRoundLimitList[1] = { 1 };
const int MPUIInterface::mMaxPlayerList[4] = { 4, 8, 12, 16 };
const int MPUIInterface::mRespawnTimeList[3] = { 0, 5, 10 };

// ============================================================================
// MPPlayerManager (mp.o)
// ============================================================================
bool MPPlayerManager::IsGuest(int)
{
    return false;
}

void MPPlayerManager::SendConsistencyUpdates()
{
}

// ea: 0x0072EC30
void MPPlayerManager::HandleKickPlayer(const bdReceivedMessage&)
{
    MPUIInterface::mKicked = true;
}

// ea: 0x0072ED90 (mLocalPlayerIndex at +0x4111)
unsigned char MPPlayerManager::getPlayerIndex(int localPlayer)
{
    return *(unsigned char*)((char*)this + 0x4111 + localPlayer);
}

// ea: 0x0072EA10 (mLocalPlayerIndex at +0x4111, size 1)
bool MPPlayerManager::AnyLocalPlayers()
{
    for (int i = 0; i < 1; ++i)
    {
        if (*(unsigned char*)((char*)this + 0x4111 + i) < 0x10u)
            return true;
    }
    return false;
}

// ea: 0x0072EB80
int MPPlayerManager::GetLocalId(const MPPlayer* player)
{
    if (player == nullptr)
        return -1;
    for (int i = 0; i < 1; ++i)
    {
        if (*(unsigned char*)((char*)this + 0x4111 + i) == player->mId)
            return i;
    }
    return -1;
}

// ea: 0x0072E9E0
MPPlayer* MPPlayerManager::GetLocalPlayer(int nLocalPlayer)
{
    unsigned char v2 = *(unsigned char*)((char*)this + 0x4111 + nLocalPlayer);
    if (v2 < 0x10u)
        return (MPPlayer*)((char*)this + 0x1010 + 0x310 * v2);
    return nullptr;
}

// ea: 0x00760690
Entity* MPPlayerManager::FindDroppedItem(EDroppedItemTypes itemType, short id,
                                         int ownerID)
{
    if (ownerID >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 8283;
        AeAssert::gCurrentExpr = "ownerID >= 0 && ownerID < 16";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("FindDroppedItem: Invalid Player ID"))
            __debugbreak();
    }
    MPPlayer* Player = GetPlayer((unsigned char)ownerID);
    if (Player != nullptr)
        return ((MPPlayerItems*)((char*)Player + 0x0C))
            ->FindItem(itemType, id);
    return nullptr;
}

// ea: 0x00760480
void MPPlayerManager::RegisterDroppedItem(EDroppedItemTypes itemType,
                                          Entity* item, Entity* owner,
                                          short id)
{
    if (item == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 8218;
        AeAssert::gCurrentExpr = "item";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("RegisterDroppedItem: Invalid item entity."))
            __debugbreak();
    }
    if (owner == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 8219;
        AeAssert::gCurrentExpr = "owner";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("RegisterDroppedItem: Invalid owner entity."))
            __debugbreak();
    }
    MPPlayer* Player = GetPlayer(owner);
    if (owner == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 8222;
        AeAssert::gCurrentExpr = "owner";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "RegisterDroppedItem: Could not get MPPlayer from owner entity"))
            __debugbreak();
    }
    if (Player != nullptr)
        ((MPPlayerItems*)((char*)Player + 0x0C))
            ->SetItem(itemType, id, item);
}

// ea: 0x007376B0
MPPlayer* MPPlayerManager::GetPlayer(const Entity* const entity)
{
    for (int i = 0; i < 16; ++i)
    {
        MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * i);
        int v4 = *(int*)((char*)p + 0x08);
        Entity* v5 = nullptr;
        if (v4 >= 0)
        {
            if (v4 >= 16)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\EntityManager.h";
                AeAssert::gCurrentLine = 19;
                AeAssert::gCurrentExpr = "idx<16";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Bounds check"))
                    __debugbreak();
            }
            v5 = EntityManager::sInst->mPlayers[v4];
        }
        if (p->mId < 0x10u && p->mConnection.m_ptr != nullptr
            && v5 == entity)
            return p;
    }
    return nullptr;
}

// ea: 0x007760450
void MPPlayerManager::RemoveDroppedItems()
{
    for (int i = 0; i < 16; ++i)
    {
        MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * i);
        ((MPPlayerItems*)((char*)p + 0x0C))->RemoveAll();
    }
}

// ea: 0x0072ED50 (mLocalPlayerIndex at +0x4111; vtable slot 7 = MakeActive)
void MPPlayerManager::DropHotJoiningPlayers()
{
    if (*(unsigned char*)((char*)this + 0x4111) != 17
        && dword_F6A290[0] == 1)
    {
        dword_F6A290[0] = 0;
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(0);
        IGMS->gap1C(IGMS, -1);
    }
    View::UpdateNumViewports();
}

// ea: 0x0072EA30 (mLocalPlayerIndex at +0x4111)
bool MPPlayerManager::IsLocalId(int Id)
{
    unsigned char idx = *(unsigned char*)((char*)this + 0x4111);
    if (idx < 0x10u)
    {
        MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * idx);
        if (p != nullptr && p->mId == Id)
            return true;
    }
    return false;
}

// ea: 0x0075A890 (mPlayers at +0x1010, mVehicles at +0x4120)
void MPPlayerManager::DebugRender()
{
    MPPlayer* mPlayers = (MPPlayer*)((char*)this + 0x1010);
    for (int i = 16; i != 0; --i)
    {
        if (mPlayers->mId < 0x10u && mPlayers->mConnection.m_ptr != nullptr)
            mPlayers->DebugRender();
        ++mPlayers;
    }
    MPVehicle* mVehicles = (MPVehicle*)((char*)this + 0x4120);
    for (int j = 10; j != 0; --j)
    {
        if (mVehicles->mId < 0x0Au)
            mVehicles->DebugRender();
        ++mVehicles;
    }
}

// ea: 0x007378D0
unsigned int MPPlayerManager::GetPlayerInfo(char* buf)
{
    unsigned int v2 = 0;
    MPPlayer* mPlayers = (MPPlayer*)((char*)this + 0x1010);
    for (int i = 16; i != 0; --i)
    {
        if (mPlayers->mId < 0x10u && mPlayers->mConnection.m_ptr != nullptr)
            v2 += bdSnprintf(&buf[v2], 0x14u, "[%u] ", mPlayers->mId);
        ++mPlayers;
    }
    return v2;
}

// ea: 0x0073A980
int MPPlayerManager::GetCurrentPlayerCount()
{
    int result = 0;
    for (int i = 0; i < 16; ++i)
    {
        MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * i);
        if (p->mId < 0x10u && p->mConnection.m_ptr != nullptr)
            ++result;
    }
    return result;
}

// ea: 0x00762AE0 (mLocalPlayerInGame at +0x4112)
void MPPlayerManager::HandleGameEnter(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = this->GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED
            && *(bool*)((char*)this + 0x4112)
            && !IsLocalId(Player->mId))
        {
            SendDroppedItems(Player);
        }
    }
}

// ea: 0x0072EA80 (mPlayers at +0x1010, stride 0x310, mClientIndex +0x08)
bool MPPlayerManager::IsLocalPlayer(const Entity* const entity)
{
    if (entity == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 465;
        AeAssert::gCurrentExpr = "entity";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid entity"))
            __debugbreak();
    }
    if (entity->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 466;
        AeAssert::gCurrentExpr = "entity->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Entity is not player"))
            __debugbreak();
    }
    for (int i = 0; i < 1; ++i)
    {
        unsigned char idx = *(unsigned char*)((char*)this + 0x4111 + i);
        if (idx < 0x10u)
        {
            MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * idx);
            if (p != nullptr)
            {
                int mClientIndex = *(int*)((char*)p + 0x08);
                Entity* v5 = mClientIndex >= 0
                                 ? EntityManager::sInst->GetPlayer(mClientIndex)
                                 : nullptr;
                if (v5 == entity)
                    return true;
            }
        }
    }
    return false;
}

// ============================================================================
// MPPeer (mp.o)
// ============================================================================
// ea: 0x0072C840
void MPPeer::operator delete(void* p)
{
    tlMemFree(p);
}

void MPPeer::DebugPrintTTYSessionInfo()
{
}

// ea: 0x0072C820
void* MPPeer::operator new(unsigned int s)
{
    return tlMemAlloc(s, 0x10u, 0);
}

// ea: 0x0072C890 (mSession at +0x7448)
bdSession::bdSessionStatus MPPeer::GetSessionStatus()
{
    return ((bdSession*)((char*)this + 0x7448))->getStatus();
}

// ea: 0x0072C8B0
bool MPPeer::IsHost()
{
    return ((bdSession*)((char*)this + 0x7448))->getRole()
           == bdSession::BD_SESSION_HOST;
}

// ea: 0x0072CA00 (m_QosIsComplete at +0x5AB0)
bool MPPeer::IsQosComplete(int qos_handle)
{
    return *(bool*)((char*)this + 0x5AB0 + qos_handle);
}

// ea: 0x0072CA20 (m_QosIsComplete +0x5AB0, m_QosIsSuccessful +0x5DD0)
bool MPPeer::IsQosSuccessful(int qos_handle)
{
    return *(bool*)((char*)this + 0x5AB0 + qos_handle)
           && *(bool*)((char*)this + 0x5DD0 + qos_handle);
}

// ea: 0x0072C850
void MPPeer::LeaveSession()
{
    ((bdSession*)((char*)this + 0x7448))->leave();
    *(bool*)((char*)this + 0xD278) = false;
}

// ea: 0x0072C870
void MPPeer::CancelJoin()
{
    ((bdSession*)((char*)this + 0x7448))->leave();
    *(bool*)((char*)this + 0xD278) = false;
}

// ea: 0x00742D80
void MPPeer::ExitLevel()
{
    if (((bdSession*)((char*)this + 0x7448))->getStatus()
        != bdSession::BD_SESSION_NOT_CONNECTED)
        ((MPPlayerManager*)((char*)this + 0x74E0))->LocalPlayerExitGame();
}

// ea: 0x0075BDE0
void MPPeer::EnterLevel()
{
    if (((bdSession*)((char*)this + 0x7448))->getStatus()
        != bdSession::BD_SESSION_NOT_CONNECTED)
        ((MPPlayerManager*)((char*)this + 0x74E0))->LocalPlayerEnterGame();
}

// ea: 0x0072CAC0 (m_QosIsAvailable at +0x60F0, 800 slots)
int MPPeer::FindAvailableQosSlot()
{
    for (int i = 0; i < 800; ++i)
    {
        if (*(bool*)((char*)this + 0x60F0 + i))
            return i;
    }
    return -1;
}

// ea: 0x00735AA0 (mPlayerManager at +0x74E0)
bool MPPeer::IsLocalPlayer(Entity* player)
{
    return ((MPPlayerManager*)((char*)this + 0x74E0))->IsLocalPlayer(player);
}

// ea: 0x0072CA50 (m_QosIsComplete +0x5AB0, m_QosIsSuccessful +0x5DD0,
// m_QosLatency +0x6730; returns int per ?GetQosPing@MPPeer@@QAEHH@Z)
int MPPeer::GetQosPing(int qos_handle)
{
    if (*(bool*)((char*)this + 0x5AB0 + qos_handle)
        && *(bool*)((char*)this + 0x5DD0 + qos_handle))
        return (int)*(float*)((char*)this + 0x6730 + qos_handle * 4);
    return (int)(intptr_t)&byte_1869F;
}

// ea: 0x00735A70 (mSession at +0x7448)
bdReference<bdConnection> MPPeer::GetConnectionByIndex(unsigned int peerID)
{
    return ((bdSession*)((char*)this + 0x7448))->getConnection(peerID);
}

// ea: 0x00740730 (mPlayerManager at +0x74E0)
bool MPPeer::IsPlayerTalking(Entity* player, int local_controller)
{
    MPPlayer* v4 = ((MPPlayerManager*)((char*)this + 0x74E0))
                       ->GetPlayer(player);
    return v4 != nullptr && IsPlayerTalking(v4, local_controller);
}

// ea: 0x00735BF0
void MPPeer::onQoSProbeSuccess(const bdQoSProbeInfo& info)
{
    printf("****** onQoSProbeSuccess ****** (%s) %5.4f seconds\n",
           info.m_data, info.m_latency);
    float m_latency = info.m_latency;
    bdCommonAddr* m_ptr = info.m_addr.m_ptr;
    if (m_ptr != nullptr)
        ++m_ptr->m_refCount;
    UpdateQosProbe(info.m_addr, true, m_latency);
}

// ea: 0x007616C0 (mCurrentGameInfo at +0x73B0)
void MPPeer::ConnectToPeersFinalize(
    const bdReference<MPGameInfo>& gameInfo)
{
    ((MPPlayerManager*)((char*)this + 0x74E0))->Reset();
    InitializeVehicles();
    bdReference<MPGameInfo>* p_mCurrentGameInfo =
        (bdReference<MPGameInfo>*)((char*)this + 0x73B0);
    if (&gameInfo != p_mCurrentGameInfo)
    {
        MPGameInfo* m_ptr = p_mCurrentGameInfo->m_ptr;
        if (m_ptr != nullptr && m_ptr->m_refCount-- == 1
            && p_mCurrentGameInfo->m_ptr != nullptr)
            delete p_mCurrentGameInfo->m_ptr;
        MPGameInfo* v6 = gameInfo.m_ptr;
        bool v5 = gameInfo.m_ptr == nullptr;
        p_mCurrentGameInfo->m_ptr = gameInfo.m_ptr;
        if (!v5)
            ++v6->m_refCount;
    }
}

// ea: 0x00761710 (mSession at +0x7448, mDiscoveryServer at +0x73B4,
// mCurrentlyInSession at +0xD278)
void MPPeer::Disconnect()
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    if (p_mSession->getStatus() != bdSession::BD_SESSION_NOT_CONNECTED)
        ((MPPlayerManager*)((char*)this + 0x74E0))->LocalPlayerExitGame();
    p_mSession->leave();
    *(bool*)((char*)this + 0xD278) = false;
    bdDiscoveryServer* pDiscovery =
        (bdDiscoveryServer*)((char*)this + 0x73B4);
    if (pDiscovery->getStatus() != BD_DISCOVERY_IDLE)
        pDiscovery->stop();
    ((MPPlayerManager*)((char*)this + 0x74E0))->Reset();
}

// ea: 0x007457A0 (mNetPosition at +0xA0, mInterpolatedPosition at +0x1F0)
void MPPeer::SetPlayerPos(const Entity* p, float* const pos)
{
    MPPlayer* Player = ((MPPlayerManager*)((char*)this + 0x74E0))
                           ->GetPlayer(p);
    if (Player != nullptr)
    {
        float* m128_f32 = (float*)((char*)Player + 0xA0);
        m128_f32[0] = pos[0];
        m128_f32[1] = pos[1];
        m128_f32[2] = pos[2];
        float* v6 = (float*)((char*)Player + 0x1F0);
        v6[0] = m128_f32[0];
        v6[1] = m128_f32[1];
        v6[2] = m128_f32[2];
        v6[3] = m128_f32[3];
    }
}

// ea: 0x00745E80 (mpVoiceManager at +0xD270)
void MPPeer::shutdownVoiceSubsystem()
{
    kuju::kvoicemanager::cVoiceManager* mpVoiceManager =
        *(kuju::kvoicemanager::cVoiceManager**)((char*)this + 0xD270);
    if (mpVoiceManager->mInitialised != 0)
    {
        mpVoiceManager->mInitialised = 0;
        mpVoiceManager->mVoiceNetworkManager.mVoiceHandlerInterface = nullptr;
    }
}

// ============================================================================
// MP options menus (mp.o) - virtual overrides
// ============================================================================
void MPOptionsScreenMenu::Select(int)
{
}

void MPOptionsScreenMenu::OnCross(int)
{
}

// ea: 0x007305D0 (menu slot 21)
MPOptionsScreenMenu* MPOptionsScreenMenu::Me()
{
    return (MPOptionsScreenMenu*)g_femanager.fems->menus[21];
}

MPOptionsSoundMenu* MPOptionsSoundMenu::Me()
{
    return (MPOptionsSoundMenu*)g_femanager.fems->menus[22];
}

MPOptionsControlsMenu* MPOptionsControlsMenu::Me()
{
    return (MPOptionsControlsMenu*)g_femanager.fems->menus[20];
}

MPOptionsGameplayMenu* MPOptionsGameplayMenu::Me()
{
    return (MPOptionsGameplayMenu*)g_femanager.fems->menus[19];
}

MPOptionsPreferencesMenu* MPOptionsPreferencesMenu::Me()
{
    return (MPOptionsPreferencesMenu*)g_femanager.fems->menus[23];
}

MPProfileEditMenu* MPProfileEditMenu::Me()
{
    return (MPProfileEditMenu*)g_femanager.fems->menus[29];
}

MPProfileMainMenu* MPProfileMainMenu::Me()
{
    return (MPProfileMainMenu*)g_femanager.fems->menus[27];
}

// ============================================================================
// MP profile menus (mp.o)
// ============================================================================
bool MPProfileEditMenu::DialogResponseOk(int)
{
    return true;
}

bool MPProfileMainMenu::DialogResponseDeleteCancel(int)
{
    return true;
}

// ea: 0x00734270
bool MPProfileMainMenu::DialogResponseProfileEdit(int)
{
    g_femanager.fems->gap1C(g_femanager.fems, 29);
    return true;
}

// ea: 0x007342D0
bool MPProfileMainMenu::DialogResponseNoMemCard(int)
{
    g_femanager.fems->gap1C(g_femanager.fems, 8);
    return true;
}

// ea: 0x00733D80 (highlighted at +0x22; Select is virtual slot +0x68)
void MPProfileMainMenu::OnCross(int)
{
    Select(*(short*)((char*)this + 0x22));
}

// ea: 0x00731560 (mWidescreen at +0x6C; FEMenu::UpdateWidescreen + panel)
void MPOptionsSoundMenu::UpdateWidescreen(bool widescreen)
{
    if (mWidescreen != widescreen)
    {
        FEMenu::UpdateWidescreen(widescreen);
        FEMultiLineText* mInstructionsText = this->mInstructionsText;
        if (mInstructionsText != nullptr)
            mInstructionsText->UpdateForWidescreen(widescreen);
        if (this->panel != nullptr)
            mWidescreen = widescreen;
    }
}

// ea: 0x007320F0 (mWidescreen at +0x60)
void MPOptionsControlsMenu::UpdateWidescreen(bool widescreen)
{
    if (mWidescreen != widescreen)
    {
        FEMenu::UpdateWidescreen(widescreen);
        FEMultiLineText* mInstructionsText = this->mInstructionsText;
        if (mInstructionsText != nullptr)
            mInstructionsText->UpdateForWidescreen(widescreen);
        if (this->panel != nullptr)
            mWidescreen = widescreen;
    }
}

// ea: 0x00733AF0 (menus[27] mSaveSlots at +104)
void MPProfileMainMenu::LoadProfileData()
{
    ProfileManager* v0 = ProfileManager::Me();
    v0->Reset();
    SaveGameData** v2 =
        (SaveGameData**)((char*)g_femanager.fems->menus[27] + 104);
    ProfileManager* v1 = ProfileManager::Me();
    v1->EnumProfiles(v2);
}

// ea: 0x00733B20 (mPanel at +0x84, mHelpBar at +0x88)
void MPProfileMainMenu::UpdateWidescreen(bool widescreen)
{
    PanelFile* mPanel = this->mPanel;
    if (mPanel != nullptr)
    {
        mPanel->UpdateWidescreen(widescreen, 320.0f);
        this->mHelpBar->UpdateForWidescreen(widescreen);
    }
}

// ea: 0x00733640 (mInstructionsText at +0x60, mListBox at +0x64)
void MPProfileEditMenu::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    Cleanup();
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    this->mInstructionsText = nullptr;
    mListBox.RemoveAllItems();
}

// ea: 0x00733770 (vtable slot 7 = MakeActive; menu 27/8)
void MPProfileEditMenu::OnTriangle(int c)
{
    (void)c;
    ProfileManager* v3 = ProfileManager::Me();
    const char* LoadedProfile = v3->GetLoadedProfile();
    FEMenuSystem* system = this->system;
    if (LoadedProfile != nullptr)
    {
        this->mNeedWrite = false;
        system->gap1C(system, 27);
    }
    else
    {
        system->gap1C(system, 8);
    }
}

// ea: 0x00733890 (button_held_down at +0x32; OnUp/OnDown via controller)
void MPProfileEditMenu::ButtonHeldAction()
{
    if (button_held_down == 4)
    {
        controller* v3 = controller::inst();
        this->OnUp(v3->locked_port);
    }
    else if (button_held_down == 8)
    {
        controller* v4 = controller::inst();
        this->OnDown(v4->locked_port);
    }
}

// ea: 0x0077337B0 (Select dispatches to sibling option menus)
void MPProfileEditMenu::Select(int entry_num)
{
    switch (entry_num)
    {
    case 0:
        this->system->gap1C(this->system, 19);
        break;
    case 1:
        this->system->gap1C(this->system, 20);
        break;
    case 2:
        this->system->gap1C(this->system, 23);
        break;
    case 3:
        this->system->gap1C(this->system, 22);
        break;
    case 4:
        this->system->gap1C(this->system, 21);
        break;
    default:
        break;
    }
}

// ea: 0x00733820 (mListBox.mTopLine/mSelectedLine)
void MPProfileEditMenu::OnCross(int c)
{
    (void)c;
    switch (mListBox.mTopLine + mListBox.mSelectedLine)
    {
    case 0:
        this->system->gap1C(this->system, 19);
        break;
    case 1:
        this->system->gap1C(this->system, 20);
        break;
    case 2:
        this->system->gap1C(this->system, 23);
        break;
    case 3:
        this->system->gap1C(this->system, 22);
        break;
    case 4:
        this->system->gap1C(this->system, 21);
        break;
    default:
        break;
    }
}

// ea: 0x00760F90 (mMenuStatus[highlighted] == 1 -> load profile)
void MPProfileMainMenu::OnTriangle(int c)
{
    (void)c;
    if (mMenuStatus[highlighted] == 1)
    {
        DialogDisplayProfileLoading(2);
        SaveGameData* v5 = mSaveSlots[highlighted];
        ProfileManager* v4 = ProfileManager::Me();
        v4->SetProfile(v5);
    }
    else
    {
        ProfileManager* v3 = ProfileManager::Me();
        v3->SetProfile(nullptr);
        g_femanager.fems->gap1C(g_femanager.fems, 8);
    }
}

// ea: 0x0075A980
void MPProfileMainMenu::LoadSelectedProfile(bool displayDialog)
{
    if (mMenuStatus[highlighted] == 1)
    {
        if (displayDialog)
            DialogDisplayProfileLoading(2);
        SaveGameData* v5 = mSaveSlots[highlighted];
        ProfileManager* v4 = ProfileManager::Me();
        v4->SetProfile(v5);
    }
    else
    {
        ProfileManager* v3 = ProfileManager::Me();
        v3->SetProfile(nullptr);
        g_femanager.fems->gap1C(g_femanager.fems, 8);
    }
}

// ea: 0x00733A90 (FEMenuEntry vtable slot 12 = SetText(const char*))
void MPProfileMainMenu::ClearEntries()
{
    for (int v2 = 0; v2 < 6; ++v2)
    {
        void* entry = this->entries[v2];
        const char* STBString =
            STBManager::sInst->GetSTBString("FEMENU_PROFILE_EMPTY");
        typedef void (__thiscall *SetTextFn)(void*, const char*);
        SetTextFn setText = (SetTextFn)((void**)entry)[12];
        setText(entry, STBString);
        mMenuStatus[v2] = 2;
    }
}

// ============================================================================
// MP option menu ctors/dtors (mp.o)
// ============================================================================
// ea: 0x007305E0
MPOptionsScreenMenu::MPOptionsScreenMenu(FEMenuSystem* s)
    : FEMenu(s, 2, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x82);
    mInstructionsText = nullptr;
    mWidescreen = false;
    mScreenText[0] = nullptr;
    mScreenText[1] = nullptr;
    mScreenText[2] = nullptr;
    mScreenText[3] = nullptr;
}

// ea: 0x00730630
MPOptionsScreenMenu::~MPOptionsScreenMenu()
{
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    this->mInstructionsText = nullptr;
}

// ea: 0x007309B0
void MPOptionsScreenMenu::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    Cleanup();
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    this->mInstructionsText = nullptr;
}

// ea: 0x00730D30
void MPOptionsScreenMenu::UpdateWidescreen(bool widescreen)
{
    if (mWidescreen != widescreen)
    {
        FEMultiLineText* mInstructionsText = this->mInstructionsText;
        if (mInstructionsText != nullptr)
            mInstructionsText->UpdateForWidescreen(widescreen);
        PanelFile* panel = this->panel;
        if (panel != nullptr)
        {
            this->mWidescreen = widescreen;
            panel->UpdateWidescreen(widescreen, 320.0f);
        }
    }
}

// ea: 0x00730D80
MPOptionsSoundMenu::MPOptionsSoundMenu(FEMenuSystem* s)
    : FEMenu(s, 1, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x82);
    mOutputVal = 0;
    mMusicVal = 0;
    mEffectsVal = 0;
    mInstructionsText = nullptr;
    mWidescreen = false;
    mSoundText[0] = nullptr;
    mSoundText[1] = nullptr;
    mSoundText[2] = nullptr;
    mSoundText[3] = nullptr;
}

// ea: 0x00730DE0
MPOptionsSoundMenu::~MPOptionsSoundMenu()
{
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    this->mInstructionsText = nullptr;
}

// ea: 0x007315B0
MPOptionsControlsMenu::MPOptionsControlsMenu(FEMenuSystem* s)
    : FEMenu(s, 8, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x82);
    mInstructionsText = nullptr;
    mWidescreen = false;
    mControlsText[0] = nullptr;
    mControlsText[1] = nullptr;
    mControlsText[2] = nullptr;
    mControlsText[3] = nullptr;
}

// ea: 0x00731600
MPOptionsControlsMenu::~MPOptionsControlsMenu()
{
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    this->mInstructionsText = nullptr;
}

// ea: 0x00732140
MPOptionsGameplayMenu::MPOptionsGameplayMenu(FEMenuSystem* s)
    : FEMenu(s, 4, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x82);
    mInstructionsText = nullptr;
    mWidescreen = false;
    mGameplayText[0] = nullptr;
    mGameplayText[1] = nullptr;
    mGameplayText[2] = nullptr;
    mGameplayText[3] = nullptr;
}

// ea: 0x00732190
MPOptionsGameplayMenu::~MPOptionsGameplayMenu()
{
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    this->mInstructionsText = nullptr;
}

// ea: 0x00732540
void MPOptionsGameplayMenu::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    Cleanup();
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    this->mInstructionsText = nullptr;
}

// ea: 0x00732890
void MPOptionsGameplayMenu::UpdateWidescreen(bool widescreen)
{
    if (mWidescreen != widescreen)
    {
        FEMultiLineText* mInstructionsText = this->mInstructionsText;
        if (mInstructionsText != nullptr)
            mInstructionsText->UpdateForWidescreen(widescreen);
        PanelFile* panel = this->panel;
        if (panel != nullptr)
        {
            this->mWidescreen = widescreen;
            panel->UpdateWidescreen(widescreen, 320.0f);
        }
    }
}

// ea: 0x007328E0
MPOptionsPreferencesMenu::MPOptionsPreferencesMenu(FEMenuSystem* s)
    : FEMenu(s, 5, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x82);
    mInstructionsText = nullptr;
    mMapCombo = nullptr;
    mWidescreen = false;
    mScreenText[0] = nullptr;
    mScreenText[1] = nullptr;
    mScreenText[2] = nullptr;
    mScreenText[3] = nullptr;
}

// ea: 0x00732940
MPOptionsPreferencesMenu::~MPOptionsPreferencesMenu()
{
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    this->mInstructionsText = nullptr;
}

// ============================================================================
// kuju (mp.o)
// ============================================================================
kuju::cBezier::cBezier()
{
}

void kuju::knetuser::cVoiceNetworkManager::deinitialise()
{
}

// ea: 0x0073F290
void kuju::kvoicemanager::cVoiceManager::deinitialise()
{
    mInitialised = 0;
    mVoiceNetworkManager.mVoiceHandlerInterface = nullptr;
}

void kuju::kvoicemanager::cVoiceManager::loadIRXModules()
{
}

// ea: 0x007348D0 (mRemoteListeners at +0x266C)
void kuju::kvoicemanager::cVoiceManager::setRemoteListeners(MPPlayerSet& players)
{
    mRemoteListeners.mBitPlayers = players.mBitPlayers;
}

void kuju::kvoicemanager::cVoiceManager::stopSystem()
{
}

void kuju::kvoicemanager::cVoiceManager::startLoopback()
{
}

void kuju::kvoicemanager::cVoiceManager::stopLoopback()
{
}

void kuju::kvoicemanager::cVoiceManager::updateLoopback()
{
}

// ea: 0x007348B0 (virtual dtor; compiler emits the vtable assignments)
kuju::kvoicemanager::cVoiceManager::~cVoiceManager()
{
}

// ea: 0x007348F0
void kuju::kvoicemanager::cVoiceManager::receiveVoiceData(
    unsigned long fromPlayerIndex, unsigned char*, unsigned long)
{
    mRemoteListeners.containsPlayer(fromPlayerIndex);
}

// ea: 0x00734910
void kuju::kvoicemanager::cVoiceManager::startSystem()
{
    memset(mEncodeBuffer, 0, sizeof(mEncodeBuffer));
    mEncodeDstOffset = 0;
    mNetworkDispatchOffset = 0;
    mLastNetworkDispatchTime.mTime = 0;
    mRealLastNetworkDispatchTime.mTime = 0;
    mRemoteListeners.mBitPlayers = 0;
}

// ea: 0x0073F240
void kuju::kvoicemanager::cVoiceManager::initialise()
{
    mVoiceNetworkManager.mVoiceHandlerInterface = this;
    mVoiceNetworkManager.initialise();
    memset(mEncodeBuffer, 0, sizeof(mEncodeBuffer));
    mEncodeDstOffset = 0;
    mNetworkDispatchOffset = 0;
    mLastNetworkDispatchTime.mTime = 0;
    mRealLastNetworkDispatchTime.mTime = 0;
    mRemoteListeners.mBitPlayers = 0;
    mInitialised = 1;
}

// ea: 0x007349F0
void kuju::knetuser::cVoiceNetworkManager::initialise()
{
    mLastTime.mTime = 0;
    mVoicePendingDispatchPacketList = nullptr;
    mActivePlayerVoices.mBitPlayers = 0;
    mLastDispatchTime.mTime = 0;
    mNextDispatchedSeqID = 0;
    mNumActivePlayerVoices = 0;
    mRecentlyReceivedPacketIndex = 0;
    mRecentlyDispatchedPacketIndex = 0;
    mMissedPackets = 0;
    memset(mPendingVoicePacketList, 0, sizeof(mPendingVoicePacketList));
    memset(mTimeVoiceActive, 0, sizeof(mTimeVoiceActive));
    memset(mRecentlyReceivedPacketRoutes, 0x10,
           sizeof(mRecentlyReceivedPacketRoutes));
    memset(mRecentlyReceivedPacketSources, 0x10,
           sizeof(mRecentlyReceivedPacketSources));
    memset(mRecentlyDispatchedPacketRoutes, 0x10,
           sizeof(mRecentlyDispatchedPacketRoutes));
    memset(mRecentlyDispatchedPacketSources, 0x10,
           sizeof(mRecentlyDispatchedPacketSources));
    mFreeVoicePacketList = mVoicePackets;
    for (int i = 0; i < 24; ++i)
    {
        mVoicePackets[i].mPrev =
            (i > 0) ? &mVoicePackets[i - 1] : nullptr;
        mVoicePackets[i].mNext =
            (i < 23) ? &mVoicePackets[i + 1] : nullptr;
    }
    for (int i = 0; i < 16; ++i)
        mLastReceivedSeqID[i] = (unsigned int)-1;
    mFreeVoicePendingDispatchPacketList = mVoicePendingDispatchPackets;
    for (int i = 0; i < 5; ++i)
    {
        mVoicePendingDispatchPackets[i].mPrev =
            (i > 0) ? &mVoicePendingDispatchPackets[i - 1] : nullptr;
        mVoicePendingDispatchPackets[i].mNext =
            (i < 4) ? &mVoicePendingDispatchPackets[i + 1] : nullptr;
    }
    for (int i = 0; i < 16; ++i)
        mLastDispatchedSeqID[i] = 0;
    for (int i = 0; i < 0x100; ++i)
        ((unsigned int*)mPlayerDistances)[i] = 0xBFC00000u;
}

// ea: 0x007642B0 (MultiplayerMgr::mUpdateTime at +0x38)
void kuju::kvoicemanager::cVoiceManager::update()
{
    if (mInitialised != 0)
    {
        MultiplayerMgr* v2 = MultiplayerMgr::sInst;
        if (v2 != nullptr && v2->mPeer != nullptr)
        {
            MPPlayerManager* pm =
                (MPPlayerManager*)((char*)v2->mPeer + 0x74E0);
            if (pm->GetLocalPlayer(0) != nullptr)
            {
                kuju::knet::sTime currentTime;
                currentTime.mTime =
                    ((kuju::knet::sTime*)((char*)v2 + 0x38))->mTime;
                evaluatePlayers();
                dispatchVoiceData();
                mVoiceNetworkManager.update(currentTime);
            }
        }
    }
}

// ea: 0x00734990
void kuju::kvoicemanager::cVoiceManager::getDiagnostics(
    sDiagnostics* diagnostics, const kuju::knet::sTime& time)
{
    (void)time;
    if (diagnostics == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/knet/audio/voicemanager/cvoicemanager.cpp";
        AeAssert::gCurrentLine = 1303;
        AeAssert::gCurrentExpr = "diagnostics";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    memset(diagnostics, 0, sizeof(sDiagnostics));
}

// ea: 0x007500E0
void kuju::knetuser::cVoiceNetworkManager::update(
    const kuju::knet::sTime& time)
{
    checkForPendingPacketsAwaitingHandling(time);
    checkForPendingPacketsAwaitingDispatch(time);
}

// ea: 0x007500C0
void kuju::knetuser::cVoiceNetworkManager::updateVoiceNetwork(
    const kuju::knet::sTime& time)
{
    checkForPendingPacketsAwaitingHandling(time);
    checkForPendingPacketsAwaitingDispatch(time);
}

kuju::cBezierTrajectoryInterpolator::cBezierTrajectoryInterpolator()
{
    mInitialDate = 0.0f;
    mTimeInterval = 0.0f;
}

// ============================================================================
// MultiplayerMgr (mp.o) - member bodies (class decl lives in sv_stubs.h)
// ============================================================================
// ea: 0x0072C4A0
void MultiplayerMgr::HandleDiscError()
{
}

// ea: 0x0072C4E0
int MultiplayerMgr::AddTestClient()
{
    return 0;
}

// ea: 0x0072C6F0
bool MultiplayerMgr::oneOffCheckLinkStatus()
{
    return true;
}

// ea: 0x0072C4D0 (mSendInterval at +0x2C)
float MultiplayerMgr::getSendInterval() const
{
    return *(float*)((char*)this + 0x2C);
}

// ea: 0x0072C7D0 (m_bFromGame at +0x04)
bool MultiplayerMgr::FromLobby()
{
    return *(char*)((char*)this + 0x04) == 0;
}

// ea: 0x0072C780 (mLinkStatus at +0x4D)
bool MultiplayerMgr::getLinkStatus()
{
    return !mLinkCheckEnabled || *(bool*)((char*)this + 0x4D);
}

// ea: 0x007613D0
void MultiplayerMgr::EnterLevel()
{
    if (mPeer != nullptr
        && ((bdSession*)((char*)mPeer + 0x7448))->getStatus()
               != bdSession::BD_SESSION_NOT_CONNECTED)
        ((MPPlayerManager*)((char*)mPeer + 0x74E0))->LocalPlayerEnterGame();
}

// ea: 0x007612F0
void MultiplayerMgr::FireMissile(int weapon, const math::Position3& position,
                                 const math::Dir3& dir,
                                 ::MPEntityHandle handle)
{
    if (mPeer != nullptr)
        mPeer->FireMissile(weapon, position, dir, handle);
}

// ea: 0x007613A0 (mSession at +0x7448)
void MultiplayerMgr::EnterGame()
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
    {
        mPeer->EnterGame();
        if (((bdSession*)((char*)mPeer + 0x7448))->getStatus()
            != bdSession::BD_SESSION_NOT_CONNECTED)
            ((MPPlayerManager*)((char*)mPeer + 0x74E0))
                ->LocalPlayerEnterGame();
    }
}

// ea: 0x007401E0 (mName at +0x68)
const char* MultiplayerMgr::GetPlayerName(const Entity* player) const
{
    if (mPeer != nullptr)
    {
        MPPlayer* v2 = ((MPPlayerManager*)((char*)mPeer + 0x74E0))
                           ->GetPlayer(player);
        if (v2 != nullptr)
            return (const char*)((char*)v2 + 0x68);
    }
    return (const char*)&defaultFileName;
}

// ea: 0x00740450 (mName at +0x68; &defaultFileName fallback)
const char* MultiplayerMgr::getVoiceConnection(const Entity* player)
{
    MPPeer* mPeer = this->mPeer;
    if (player != nullptr && mPeer != nullptr)
    {
        MPPlayerManager* pm = (MPPlayerManager*)((char*)mPeer + 0x74E0);
        if (pm != nullptr)
        {
            MPPlayer* v3 = pm->GetPlayer(player);
            if (v3 != nullptr)
                return (const char*)((char*)v3 + 0x68);
        }
    }
    return (const char*)&defaultFileName;
}

// ea: 0x007358A0 (mSession at +0x7448; getRole == BD_SESSION_HOST)
bool MultiplayerMgr::IsLocalClientHost(int client)
{
    return mPeer == nullptr
        || ((bdSession*)((char*)mPeer + 0x7448))->getRole()
               == bdSession::BD_SESSION_HOST
           && (MPUIInterface::mGameConnectionType == kGameConnectionTypeOnline
               || client == LocalClient::FirstLocalClientIndex());
}

// ea: 0x00735990 (currVote at mPlayerManager + 0x5914; kNoVote == 0)
bool MultiplayerMgr::IsVoteOngoing()
{
    MPPeer* mPeer = this->mPeer;
    bool result = false;
    if (mPeer != nullptr)
    {
        MPPlayerManager* pm = (MPPlayerManager*)((char*)mPeer + 0x74E0);
        if (pm != nullptr)
        {
            MPVote* p_currVote = (MPVote*)((char*)pm + 0x5914);
            if (p_currVote != nullptr)
            {
                if (p_currVote->mVoteType != kNoVote
                    && !p_currVote->localVoted)
                    return true;
            }
        }
    }
    return result;
}

// ea: 0x00750600 (currVote.voteStartTime at mPlayerManager + 0x5914)
void MultiplayerMgr::RoundOver(int condition, int team)
{
    if (mPeer != nullptr)
    {
        MPVote* pVote = (MPVote*)((char*)((char*)mPeer + 0x74E0) + 0x5914);
        if (pVote->voteStartTime.mTime != 0)
            MPUIInterface::ResolveVote();
        mPeer->RoundOver(condition, team);
    }
}

// ea: 0x007643B0
::MPEntityHandle MultiplayerMgr::GetNextDroppedItemID(
    EDroppedItemTypes itemType, Entity* owner)
{
    ::MPEntityHandle result;
    if (mPeer != nullptr)
        return ((MPPlayerManager*)((char*)mPeer + 0x74E0))
            ->GetNextDroppedItemID(itemType, owner);
    result.mValue = 0;
    return result;
}

// ea: 0x00751190 (mLocalPlayerIndex at +0x4111)
void MultiplayerMgr::DropSplitScreenPlayer(int localIndex)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
    {
        MPPlayerManager* pm = (MPPlayerManager*)((char*)mPeer + 0x74E0);
        unsigned char v4 =
            *(unsigned char*)((char*)pm + 0x4111 + localIndex);
        if (v4 < 0x10u)
        {
            MPPlayer* p = (MPPlayer*)((char*)pm + 0x1010 + 0x310 * v4);
            if (p != nullptr)
            {
                Entity* Player =
                    EntityManager::sInst->GetPlayer(*(int*)((char*)p + 0x08));
                mPeer->DropSplitScreenPlayer(Player);
            }
        }
    }
}

// ea: 0x00761650
void MultiplayerMgr::SendServerParams()
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1886;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendServerParams();
}

// ea: 0x00764310
void MultiplayerMgr::Disconnect()
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 958;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->Disconnect();
}

// ea: 0x00750690
void MultiplayerMgr::WeaponChange(int weapon)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1342;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->WeaponChange(weapon);
}

// ea: 0x00751090
void MultiplayerMgr::SetPlayerPos(const Entity* player, float* const pos)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1868;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SetPlayerPos(player, pos);
}

// ea: 0x00765D40
bool MultiplayerMgr::CreateGame(bdReference<MPGameInfo>& gameInfo,
                                EGameConnectionType gameState)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 948;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    return mPeer->CreateGame(gameInfo, gameState);
}

// ea: 0x00761550
Entity* MultiplayerMgr::FindDroppedItem(EDroppedItemTypes item, int id,
                                        int ownerID)
{
    if (mPeer != nullptr)
        return ((MPPlayerManager*)((char*)mPeer + 0x74E0))
            ->FindDroppedItem(item, id, ownerID);
    return nullptr;
}

// ea: 0x00761590
Entity* MultiplayerMgr::FindDroppedItem(EDroppedItemTypes item, int id,
                                        Entity* owner)
{
    if (mPeer != nullptr)
    {
        MPPlayer* Player =
            ((MPPlayerManager*)((char*)mPeer + 0x74E0))->GetPlayer(owner);
        if (Player != nullptr)
            return ((MPPlayerManager*)((char*)mPeer + 0x74E0))
                ->FindDroppedItem(item, id, Player->mId);
    }
    return nullptr;
}

// ea: 0x007614F0
void MultiplayerMgr::RegisterDroppedItem(EDroppedItemTypes itemType,
                                         Entity* item, Entity* owner,
                                         int id)
{
    if (mPeer != nullptr)
        ((MPPlayerManager*)((char*)mPeer + 0x74E0))
            ->RegisterDroppedItem(itemType, item, owner, (short)id);
}

// ea: 0x00751030
void MultiplayerMgr::SendBombExplosion(const Entity* player)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1859;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendBombExplosion(player);
}


// ea: 0x0072C4B0 (mSendInterval at +0x2C)
void MultiplayerMgr::setSendInterval(int ms)
{
    *(float*)((char*)this + 0x2C) = ms * 1000.0f;
}

// ea: 0x007511E0
void MultiplayerMgr::shutdownVoiceSubsystem()
{
    if (mPeer != nullptr)
    {
        kuju::kvoicemanager::cVoiceManager* v =
            *(kuju::kvoicemanager::cVoiceManager**)((char*)mPeer + 0xD270);
        if (v->mInitialised != 0)
        {
            v->mInitialised = 0;
            v->mVoiceNetworkManager.mVoiceHandlerInterface = nullptr;
        }
    }
}

// ea: 0x0072C4F0
EDroppedItemTypes MultiplayerMgr::GetDroppedItemType(itemType_t item)
{
    switch (item)
    {
    case IT_HEALTH:
    case IT_WEAPON_AMMO:
    case IT_WEAPON_HEALTH:
        return (EDroppedItemTypes)2;  // kItemTypeSupport
    default:
        return (EDroppedItemTypes)1;  // kItemTypeWeapons
    }
}

// ============================================================================
// mp.o globals
// ============================================================================
MPPeer* gMPPeer;  // ?gMPPeer@@3PAVMPPeer@@A

// ea: 0x007616B0
void MPDebugRenderer()
{
    if (gMPPeer != nullptr)
        gMPPeer->DebugRender();
}

// ============================================================================
// Batch 9: 100-170 byte tier (mp.o)
// ============================================================================

// ea: 0x0072C530
kuju::knet::sTime MultiplayerMgr::getLocalTime()
{
    static double sOOFreq = 0.0;            // 0xF99190
    static unsigned __int64 timeStarted = 0;  // 0xF99180
    if (sOOFreq == 0.0)
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        unsigned __int64 v2 = freq.QuadPart;
        if (v2 == 0)
            v2 = 1;
        sOOFreq = 1.0 / (double)v2;
        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);
        timeStarted = start.QuadPart;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    unsigned __int64 elapsed = counter.QuadPart - timeStarted;
    kuju::knet::sTime result;
    result.mTime = (int)((double)elapsed * sOOFreq * 1000.0);
    return result;
}

// ea: 0x0072C700 (mLastLinkStatusCheckTime +0x48, mTimeLinkWentDown +0x44,
// mOldLinkStatus +0x4C, mLinkStatus +0x4D)
void MultiplayerMgr::updateLinkStatus()
{
    kuju::knet::sTime LocalTime = getLocalTime();
    int mTime = LocalTime.mTime;
    if (LocalTime.mTime - ((kuju::knet::sTime*)((char*)this + 0x48))->mTime > 1000)
    {
        ((kuju::knet::sTime*)((char*)this + 0x48))->mTime = mTime;
        BOOL v4 = XNetGetEthernetLinkStatus() != 0;
        if (v4)
        {
            *(bool*)((char*)this + 0x4D) = true;
            *(bool*)((char*)this + 0x4C) = v4 != 0;
        }
        else if (*(bool*)((char*)this + 0x4C))
        {
            ((kuju::knet::sTime*)((char*)this + 0x44))->mTime = mTime;
            *(bool*)((char*)this + 0x4C) = false;
        }
        else
        {
            if (mTime - ((kuju::knet::sTime*)((char*)this + 0x44))->mTime > 3000)
                *(bool*)((char*)this + 0x4D) = false;
            *(bool*)((char*)this + 0x4C) = false;
        }
    }
}

// ea: 0x0072C610
void MultiplayerMgr::MPLogSubscriber::publish(
    const char* fullChannelName, const char* file, const char* function,
    unsigned int line, const char* msg)
{
    const char* v6 = strrchr(file, '\\');
    int v7 = 0;
    if (v6 != nullptr)
        v7 = (int)(v6 - file) + 1;
    if (strstr(fullChannelName, "info") == fullChannelName)
        tlPrintf("DW:%s(%u): %s\n", &file[v7], line, msg);
    else if (strstr(fullChannelName, "warn") == fullChannelName)
        tlPrintf("DW:%s(%u): %s\n\tWARNING: %s\n", &file[v7], line,
                 function, msg);
    else if (strstr(fullChannelName, "err") == fullChannelName)
        tlPrintf("DW:%s(%u): %s\n\tERROR: %s\n", &file[v7], line,
                 function, msg);
    else
    {
        tlPrintf("DW:bdLogSubscriber::publish: invalid channel name!\n");
        DebugBreak();
    }
}

// ea: 0x00750100
void MultiplayerMgr::LoadLevel(int map, bool restart, bool rotate)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 868;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->LoadLevel(map, restart, rotate);
}

// ea: 0x00750520
void MultiplayerMgr::PlayerRevive(Entity* player, Entity* medic,
                                  const math::Position3& position,
                                  const math::Dir3& angles)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1303;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->PlayerRevive(player, medic, position, angles);
}

// ea: 0x0075AE30
void MultiplayerMgr::DropItem(int itemType, const math::Position3& position,
                              const math::Dir3& angles,
                              const math::Dir3& velocity, int netIndex,
                              bool scriptFrom, int typeIndex)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1680;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->DropItem(itemType, position, angles, velocity, netIndex,
                    scriptFrom, typeIndex);
}

// ============================================================================
// MPUIInterface (mp.o)
// ============================================================================
// ea: 0x0072F970
const int MPUIInterface::GetScoreLimitCount(eGameType gameType)
{
    switch (gameType)
    {
    case GAME_TYPE_WAR:
    case GAME_TYPE_CTF:
    case GAME_TYPE_SCF:
    case GAME_TYPE_HQ:
    case GAME_TYPE_DM:
    case GAME_TYPE_SND:
    case (eGameType)8:
        return 5;
    case GAME_TYPE_TDM:
        return 6;
    default:
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 1374;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return -1;
    }
}

// ea: 0x0073D430
const int MPUIInterface::GetMaxPlayersOptionFromMap(char mapID)
{
    if (mapID == 5 || mapID == 8)
        return 2;
    for (int i = 0; i < 4; ++i)
    {
        if (GetMaxPlayers(i) == 16)
            return i;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
    AeAssert::gCurrentLine = 1639;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("old cod assert"))
        __debugbreak();
    return 0;
}

// ea: 0x0072F4A0
void MPUIInterface::CancelJoin()
{
    MPPeer* mPeer = MultiplayerMgr::sInst->mPeer;
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 979;
        AeAssert::gCurrentExpr = "peer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no peer"))
            __debugbreak();
    }
    ((bdSession*)((char*)mPeer + 0x7448))->leave();
    *(bool*)((char*)mPeer + 0xD278) = false;
    if (mGameConnectionType == kGameConnectionTypeOnline)
    {
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        Handle->LeaveLiveSession();
    }
    mInSession = false;
}

// ea: 0x00730000
void MPUIInterface::PlatformStart()
{
    WSAData wsaData;
    struct XNetStartupParams {
        unsigned int cfgSizeOfStruct;
        unsigned int cfgIpFragMaxSimultaneous;
        unsigned int cfgSockDefaultSendBufsizeInK;
    };
    XNetStartupParams xnsp;
    xnsp.cfgSizeOfStruct = 12;
    xnsp.cfgIpFragMaxSimultaneous = 0;
    xnsp.cfgSockDefaultSendBufsizeInK = 0;
    if (XNetStartup(&xnsp) != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 2038;
        AeAssert::gCurrentExpr = "err == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Error starting XBox network"))
            __debugbreak();
    }
    WSAStartup(0x202u, &wsaData);
}

// ============================================================================
// MPPlayer (mp.o)
// ============================================================================
// ea: 0x007365F0 (mNetHeading +0xC0, mInterpolatedPitch +0x210,
// mInterpolatedHeading +0x218, mLastHeadingAngle +0x260)
void MPPlayer::SetAngles(const float* angles)
{
    float v2 = angles[1];
    *(float*)((char*)this + 0xC0) = v2;
    if (v2 < -180.0f || v2 > 180.0f)
        v2 = fmodf(v2 + 180.0f, 360.0f) - 180.0f;
    *(float*)((char*)this + 0x218) = v2;
    *(float*)((char*)this + 0x260) = v2;
    *(float*)((char*)this + 0x210) = 0.0f;
}

// ea: 0x00761E30
MPPlayer::~MPPlayer()
{
    ((MPPlayerItems*)((char*)this + 0x0C))->~MPPlayerItems();
    bdConnection* m_ptr = mConnection.m_ptr;
    if (m_ptr != nullptr && --*(int*)((char*)m_ptr + 0x04) == 0)
    {
        bdConnection* v4 = mConnection.m_ptr;
        if (v4 != nullptr)
            delete v4;
        mConnection.m_ptr = nullptr;
    }
}

// ============================================================================
// MPPlayerSet (mp.o)
// ============================================================================
// ea: 0x007302D0
unsigned long MPPlayerSet::highestPlayerIndex() const
{
    if (mBitPlayers == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerSet.cpp";
        AeAssert::gCurrentLine = 51;
        AeAssert::gCurrentExpr = "mBitPlayers";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    unsigned short test = 0x8000;
    unsigned int result = 15;
    while ((test & mBitPlayers) == 0)
    {
        test >>= 1;
        --result;
    }
    return result;
}

// ============================================================================
// MPGameInfo (mp.o)
// ============================================================================
// ea: 0x0073DA50
MPGameInfo::MPGameInfo(unsigned int titleID, const XNKID& securityID,
                       const XNKEY& securityKey,
                       bdReference<bdCommonAddr> hostAddr)
    : bdGameInfo(titleID, securityID, securityKey, hostAddr)
{
}

// ea: 0x00730540
void MPGameInfo::updateSlots(char publicOpenDelta, char privateOpenDelta,
                             char publicFilledDelta, char privateFilledDelta)
{
    unsigned char m_publicOpen = m_publicOpen;
    if (publicOpenDelta + m_publicOpen > 0)
        m_publicOpen = (unsigned char)(publicOpenDelta + m_publicOpen);
    unsigned char m_privateOpen = m_privateOpen;
    if (privateOpenDelta + m_privateOpen > 0)
        m_privateOpen = (unsigned char)(privateOpenDelta + m_privateOpen);
    unsigned char m_publicFilled = m_publicFilled;
    if (publicFilledDelta + m_publicFilled > 0)
        m_publicFilled = (unsigned char)(publicFilledDelta + m_publicFilled);
    unsigned char m_privateFilled = m_privateFilled;
    if (privateFilledDelta + m_privateFilled > 0)
        m_privateFilled = (unsigned char)(privateFilledDelta + m_privateFilled);
}

// ============================================================================
// MPLanDiscovery (mp.o)
// ============================================================================
// ea: 0x0072CAF0
void MPLanDiscovery::Start()
{
    mNumResults = 0;
    bdInetAddr v3 = bdInetAddr::Broadcast();
    mDiscoveryClient.discover(0x2C0DC0Du, 2.0f, v3);
}

// ea: 0x00735EE0
MPLanDiscovery::~MPLanDiscovery()
{
    mDiscoveryClient.unregisterListener(this);
    for (int i = 0; i < 10; ++i)
    {
        bdGameInfo* p = mResults[i].m_ptr;
        if (p != nullptr && p->m_refCount-- == 1)
            delete p;
    }
    mDiscoveryClient.~bdDiscoveryClient();
}

// ============================================================================
// MPPlayerManager (mp.o)
// ============================================================================
// ea: 0x0072E970 (mPlayers at +0x1010, mName at +0x68)
const char* MPPlayerManager::GetPlayerName(unsigned char id)
{
    if (id > 0x10u)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 414;
        AeAssert::gCurrentExpr = "id >= 0 && id <= 16";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid player id"))
            __debugbreak();
    }
    return (const char*)((char*)this + 0x1010 + 0x310 * id + 0x68);
}

// ea: 0x0072EF50 (mVehicles at +0x4120, mVehicleCount at +0x55C0)
MPVehicle* MPPlayerManager::GetVehicle(unsigned char vehId)
{
    int mVehicleCount = *(int*)((char*)this + 0x55C0);
    int v3 = 0;
    if (mVehicleCount > 0)
    {
        MPVehicle* mVehicles = (MPVehicle*)((char*)this + 0x4120);
        while (mVehicles->mId != vehId)
        {
            ++v3;
            ++mVehicles;
            if (v3 >= mVehicleCount)
                break;
        }
        if (v3 < mVehicleCount)
            return (MPVehicle*)((char*)this + 0x4120 + 0x210 * v3);
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
    AeAssert::gCurrentLine = 7814;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("Could not locate vehicle"))
        __debugbreak();
    return nullptr;
}

// ea: 0x0072EFE0
MPVehicle* MPPlayerManager::GetVehicleFromOccupant(const MPPlayer* player)
{
    if (player == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 7823;
        AeAssert::gCurrentExpr = "player";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid player."))
            __debugbreak();
    }
    if (*(bool*)((char*)player + 0x44))
        return GetVehicle(*(unsigned char*)((char*)player + 0x3C));
    return nullptr;
}

// ea: 0x007394A0 (mPlayers at +0x1010, mVehicleEventSequence at +0x94)
void MPPlayerManager::ResetVehicleEventSequenceIds()
{
    for (int i = 0; i < 16; ++i)
    {
        MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * i);
        if (p->mId < 0x10u
            && *(int*)((char*)p + 0x08) != 0)
            *(unsigned int*)((char*)p + 0x94) = 0;
    }
}

// ea: 0x0072EBB0 (MP_MessageCallbacks at +0x55C8; general member pointer)
void MPPlayerManager::AddAcceptCallback(
    int MESSAGE_ID,
    void (MPPlayerManager::*callback)(const bdReceivedMessage&))
{
    unsigned int v4 = MESSAGE_ID - 29;
    if (MESSAGE_ID - 29 < 0 || v4 >= 0x54)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 1118;
        AeAssert::gCurrentExpr =
            "index >= 0 && index < (MESSAGE_TYPE_NUM - ((28u)+1))";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("BOLLOCKS"))
            __debugbreak();
    }
    void* slots[2];
    memcpy(slots, &callback, 8);
    typedef void (__thiscall *CallbackFn)(MPPlayerManager*,
                                          const bdReceivedMessage*);
    CallbackFn* table =
        (CallbackFn*)((char*)this + 0x55C8);
    table[2 * v4] = (CallbackFn)slots[0];
    table[2 * v4 + 1] = (CallbackFn)slots[1];
}

// ea: 0x0075ED70 (mLocalPlayerInGame at +0x4112, mItems at +0x0C)
void MPPlayerManager::HandleMapRestart(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = this->GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED
            && *(bool*)((char*)this + 0x4112))
        {
            for (int i = 0; i < 16; ++i)
            {
                MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * i);
                ((MPPlayerItems*)((char*)p + 0x0C))->RemoveAll();
            }
            void (*mCallbackRestartMap)() =
                gpBrocAPI->mBrocExports.mCallbackRestartMap;
            if (mCallbackRestartMap != nullptr)
                mCallbackRestartMap();
        }
    }
}

// ============================================================================
// MPPeer (mp.o)
// ============================================================================
// ea: 0x00735A00 (mSession at +0x7448)
void MPPeer::ConnectToSession(bdReference<bdCommonAddr> hostAddr,
                              const XNKID& secID, const XNKEY& secKey)
{
    if (hostAddr.m_ptr != nullptr)
        ++hostAddr.m_ptr->m_refCount;
    ((bdSession*)((char*)this + 0x7448))->join(hostAddr, secID, secKey,
                                               nullptr);
    if (hostAddr.m_ptr != nullptr && hostAddr.m_ptr->m_refCount-- == 1)
        delete hostAddr.m_ptr;
}

// ea: 0x00735C40
void MPPeer::onQoSProbeFail(bdReference<bdCommonAddr> addr)
{
    printf("****** onQoSProbeFail ******\n");
    float v6 = 0.0f;
    bool v5 = false;
    bdReference<bdCommonAddr> v4;
    v4.m_ptr = addr.m_ptr;
    if (addr.m_ptr != nullptr)
        ++addr.m_ptr->m_refCount;
    UpdateQosProbe(v4, v5, v6);
    if (addr.m_ptr != nullptr && addr.m_ptr->m_refCount-- == 1)
        delete addr.m_ptr;
}

// ============================================================================
// MPVehicle (mp.o)
// ============================================================================
// ea: 0x00737210
void MPVehicle::RespawnVehicle()
{
    if (mEntity == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPVehicle.cpp";
        AeAssert::gCurrentLine = 971;
        AeAssert::gCurrentExpr = "GetEntity()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid vehicle entity during respawn"))
            __debugbreak();
    }
    if (mEntity != nullptr)
        VEH_RespawnVehicle((Entity*)mEntity);
    mNumOccupants = 0;
    memset((char*)this + 0x08, 0x10, 0x0C);
    UpdateFromLocalVehicle();
}

// ea: 0x0072E1C0 (610 bytes)
void MPVehicle::Reset(bool clearOccupant)
{
    if (clearOccupant)
    {
        mNumOccupants = 0;
        memset((char*)this + 0x08, 0x10, 0x0C);
    }
    memset(&mNetPosition, 0, sizeof(mNetPosition));
    memset(&mNetSpeed, 0, sizeof(mNetSpeed));
    memset(&mNetAngularVelocity, 0, sizeof(mNetAngularVelocity));
    memset(&mInterpolatedPosition, 0, sizeof(mInterpolatedPosition));
    memset(&mInterpolatedSpeed, 0, sizeof(mInterpolatedSpeed));
    memset(&mInterpolatedAngularVelocity, 0,
           sizeof(mInterpolatedAngularVelocity));
    mReceived = false;
    mLastLocalNetworkTime = 0;
    mLastRemoteNetworkTime = 0;
    mLastDeltaDifference = 0;
    mNetHeading = 0.0f;
    mNetPitch = 0.0f;
    mNetRoll = 0.0f;
    mInterpolationState = 0;
    mInterpolatedHeading = 0.0f;
    mInterpolatedPitch = 0.0f;
    mInterpolatedRoll = 0.0f;
    mInterpolatedSteering = 0.0f;
    memset(&mLastRemoteVelocity, 0, sizeof(mLastRemoteVelocity));
    int mTime = MultiplayerMgr::sInst->getLocalTime().mTime;
    mLastReceivedTime.mTime = mTime;
    mLastInterpolatedTime.mTime = mTime;
    mAverageUpdateInterval = gStartupAverageUpdateInterval.mTime * 0.001f;
    mNbReceivedMessages = 0;
    if (mEntity != nullptr)
    {
        Entity* ent = (Entity*)mEntity;
        mNetPosition = ent->r.currentOrigin;
        mNetHeading = ent->r.currentAngles.v.m128_f32[1];
        mNetPitch = ent->r.currentAngles.v.m128_f32[0];
        mNetRoll = ent->r.currentAngles.v.m128_f32[2];
        mInterpolatedPosition = mNetPosition;
        mInterpolatedPitch = mNetPitch;
        mInterpolatedHeading = mNetHeading;
        mInterpolatedRoll = mNetRoll;
        mInterpolatedSpeed = mNetSpeed;
        mNetSteering = 0.0f;
    }
}

// ============================================================================
// MP options/profile menu helpers (mp.o)
// ============================================================================
const char* const MPOptionsSoundMenu::kSoundOptionStrings[] = {
    "FEMENU_COP_VOLUME", "FEMENU_SOUND_INST_VOLUME",
};
const char* const MPOptionsSoundMenu::kSoundInstructionStrings[] = {
    "FEMENU_SOUND_INST_VOLUME",
};

const char* const MPProfileEditMenu::kProfileTextOptionStrings[] = {
    "FEMENU_OP_GAMEPLAY", "FEMENU_OP_CONTROLS", "FEMENU_OP_PREFERENCES",
    "FEMENU_OP_SOUND", "FEMENU_OP_SCREEN",
};

const char* const MPProfileEditMenu::kProfileTextInstructionStrings[] = {
    "FEMENU_OP_INST_GAMEPLAY", "FEMENU_OP_INST_CONTROLS",
    "FEMENU_OP_INST_PREFERENCES", "FEMENU_OP_INST_SOUND",
    "FEMENU_OP_INST_SCREEN",
};

// ea: 0x00731440
void MPOptionsSoundMenu::ButtonHeldAction()
{
    flags = (int16_t)(flags & ~0x100);
    char button_held_down = this->button_held_down;
    __int16 flags = this->flags;
    if (button_held_down == 4)
    {
        this->OnUp(0);
    }
    else if (button_held_down == 8)
    {
        this->OnDown(0);
    }
    else if (highlighted == 0)
    {
        if (button_held_down == 32)
        {
            this->flags = (int16_t)(flags | 0x100);
            this->OnRight(0);
        }
        else if (button_held_down == 16)
        {
            this->flags = (int16_t)(flags | 0x100);
            this->OnLeft(0);
        }
    }
}

// ea: 0x00731E80
void MPOptionsControlsMenu::ButtonHeldAction()
{
    flags = (int16_t)(flags & ~0x100);
    char button_held_down = this->button_held_down;
    __int16 flags = this->flags;
    if (button_held_down == 4)
    {
        this->OnUp(0);
    }
    else if (button_held_down == 8)
    {
        this->OnDown(0);
    }
    else
    {
        __int16 highlighted = this->highlighted;
        if (highlighted == 2 || highlighted == 3)
        {
            if (button_held_down == 32)
            {
                this->flags = (int16_t)(flags | 0x100);
                this->OnRight(0);
            }
            else if (button_held_down == 16)
            {
                this->flags = (int16_t)(flags | 0x100);
                this->OnLeft(0);
            }
        }
    }
}

// ea: 0x0073E0D0
void MPOptionsSoundMenu::OnActivate()
{
    if (mWidescreen != (cg_widescreen.integer != 0))
        this->UpdateWidescreen(cg_widescreen.integer != 0);
    int highlighted = this->highlighted;
    flags = (int16_t)(flags | 0x80);
    mSoundText[2]->SetText(kSoundOptionStrings[highlighted]);
    *(short*)((char*)this->entries[0] + 0x08) = 0;
    *(short*)((char*)this->entries[0] + 0x0A) = 0;
    this->SetHigh(0, true);
    SetOptions();
}

// ea: 0x00733670
void MPProfileEditMenu::OnUp(int c)
{
    if (!lockInput)
    {
        this->Up();
        mListBox.OnUp(c);
        int v3 = mListBox.mTopLine + mListBox.mSelectedLine;
        mProfileEditText[2]->SetText(kProfileTextOptionStrings[v3]);
        const char* STBString = STBManager::sInst->GetSTBString(
            kProfileTextInstructionStrings[v3]);
        Broc::string v6(STBString);
        mInstructionsText->SetTextBoxNoLocalize(v6, 520, -1.5f);
    }
}

// ea: 0x007336F0
void MPProfileEditMenu::OnDown(int c)
{
    if (!lockInput)
    {
        this->Down();
        mListBox.OnDown(c);
        int v3 = mListBox.mTopLine + mListBox.mSelectedLine;
        mProfileEditText[2]->SetText(kProfileTextOptionStrings[v3]);
        const char* STBString = STBManager::sInst->GetSTBString(
            kProfileTextInstructionStrings[v3]);
        Broc::string v6(STBString);
        mInstructionsText->SetTextBoxNoLocalize(v6, 520, -1.5f);
    }
}

// ea: 0x00765750
MPProfileEditMenu::~MPProfileEditMenu()
{
    mListBox.RemoveAllItems();
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    mListBox.~UIListBox();
}

// ============================================================================
// MPUtility (mp.o) - remaining bit-buffer helpers
// ============================================================================
// ea: 0x0073BCB0
void MPUtility::WriteEntityHandle(bdReference<bdBitBuffer> buffer,
                                  ::MPEntityHandle id)
{
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER16_TYPE);
    unsigned short v3 = id.mValue;
    buffer.m_ptr->writeBits(&v3, 0x10u);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073AAE0
void MPUtility::WritePositionDelta(bdReference<bdBitBuffer> buffer,
                                   float old_position, float position)
{
    buffer.m_ptr->writeRangedFloat32(position - old_position, -512.0f,
                                     512.0f, 1.0f);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073AFB0
void MPUtility::WriteNormal(bdReference<bdBitBuffer> buffer,
                            const float* normal)
{
    unsigned char v3 = DirToByte(normal);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&v3, 8u);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073B020
void MPUtility::WriteNormal(bdReference<bdBitBuffer> buffer,
                            const math::Dir3& normal)
{
    unsigned char v3 = DirToByte(normal.v.m128_f32);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&v3, 8u);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073BB90 (angle quantization: 32768/180 == 182.04445)
void MPUtility::WriteAngle(bdReference<bdBitBuffer> buffer, float angle)
{
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE);
    int v2 = (int)(angle * 182.04445f);
    buffer.m_ptr->writeBits(&v2, 0x10u);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073C200
void MPUtility::WritePlayerTeam(bdReference<bdBitBuffer> buffer, team_t team)
{
    bool v2 = team == TEAM_AXIS;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_BOOL_TYPE);
    unsigned char byte = v2 ? 0xFF : 0x00;
    buffer.m_ptr->writeBits(&byte, 1u);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073B600
void MPUtility::WriteCompressedVector(bdReference<bdBitBuffer> buffer,
                                      const float* vec)
{
    const float* v2 = vec;
    float normal[3];
    normal[0] = *vec;
    normal[1] = vec[1];
    normal[2] = 0.0f;
    float v3 = VectorNormalize(normal);
    if (v3 > 1024.0f)
        v3 = 1024.0f;
    bdBitBuffer* m_ptr = buffer.m_ptr;
    if (*v2 == 0.0f && v2[1] == 0.0f)
    {
        m_ptr->writeDataType(bdBitBuffer::BD_BB_BOOL_TYPE);
        unsigned char byte = 0;
        m_ptr->writeBits(&byte, 1u);
    }
    else
    {
        m_ptr->writeDataType(bdBitBuffer::BD_BB_BOOL_TYPE);
        unsigned char byte = 0xFF;
        m_ptr->writeBits(&byte, 1u);
        float a = vectoyaw(normal);
        unsigned char yawByte =
            (unsigned char)(AngleMod(a) * 0.0027777778f * 255.0f);
        m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
        m_ptr->writeBits(&yawByte, 8u);
        m_ptr->writeRangedFloat32(v3, -1024.0f, 1024.0f, 1.0f);
    }
    m_ptr->writeRangedFloat32(v2[2], -1024.0f, 1024.0f, 1.0f);
    if (m_ptr != nullptr && m_ptr->m_refCount-- == 1)
        delete m_ptr;
}

// ea: 0x0073BE10
bool MPUtility::ReadPlayerId(bdReference<bdBitBuffer> buffer,
                             unsigned char& id)
{
    unsigned int id32 = 16;
    bool v2 = buffer.m_ptr->readRangedUInt32(id32, 0, 0x10u, true);
    id = (unsigned char)id32;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v2;
}

// ea: 0x0073C0B0
bool MPUtility::ReadVehicleId(bdReference<bdBitBuffer> buffer,
                              unsigned char& id)
{
    unsigned int id32 = 10;
    bool v2 = buffer.m_ptr->readRangedUInt32(id32, 0, 0xAu, true);
    id = (unsigned char)id32;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v2;
}

// ea: 0x0073BEF0
bool MPUtility::ReadSeatIndex(bdReference<bdBitBuffer> buffer, int& seatIdx)
{
    unsigned int id32 = 11;
    bool v2 = buffer.m_ptr->readRangedUInt32(id32, 0, 0xBu, true);
    seatIdx = (int)id32;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v2;
}

// ea: 0x0073BFD0
bool MPUtility::ReadEntryPoint(bdReference<bdBitBuffer> buffer,
                               int& entryIdx)
{
    unsigned int id32 = 6;
    bool v2 = buffer.m_ptr->readRangedUInt32(id32, 0, 6u, true);
    entryIdx = (int)id32;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v2;
}

// ea: 0x0073C190
bool MPUtility::ReadPlayerClass(bdReference<bdBitBuffer> buffer,
                                int& playerclass)
{
    int player_class = 0;
    bool v2 = buffer.m_ptr->readRangedInt32(player_class, 0, 7);
    playerclass = player_class - 1;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v2;
}

// ============================================================================
// PlayerStats (mp.o)
// ============================================================================
TPlayerStatsInfo PlayerStats::playerStatsInfo[] = {
    { 0.0f }, { 0.0f }, { 0.0f }, { 1.0f }, { 0.0f }, { 0.25f },
    { 1.0f }, { -1.0f }, { -3.0f }, { 1.0f }, { 0.0f }, { 1.0f },
    { 0.0f }, { 0.0f }, { 0.0f }, { 0.0f }, { 1.0f }, { 4.0f },
    { 1.0f }, { 4.0f }, { 1.0f }, { 1.0f }, { 1.0f }, { 0.25f },
    { 1.0f }, { 0.5f }, { 2.0f }, { 2.0f }, { 1.0f },
};

// ea: 0x00734660
int PlayerStats::ScoreForStat(int stat, int value)
{
    if (stat > 0x1C)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ePlayerStats.cpp";
        AeAssert::gCurrentLine = 19;
        AeAssert::gCurrentExpr =
            "stat >= kPlayerStatsMin && stat <= kPlayerStatsMax";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid stat index."))
            __debugbreak();
    }
    return (int)((float)value * playerStatsInfo[stat].mContributesToScore);
}

// ea: 0x007346D0 (first 24 stats x 5 entries + final 5 x 1)
int PlayerStats::TotalScoreForStats(short* stats)
{
    float v1 = 0.0f;
    // 24 stat groups: each stat's mContributesToScore lives in a 20-byte
    // record; the binary unrolls 5 stats per iteration.
    for (int group = 0; group < 24; group += 5)
    {
        v1 += (float)stats[group + 0]
                  * playerStatsInfo[group + 0].mContributesToScore;
        v1 += (float)stats[group + 1]
                  * playerStatsInfo[group + 1].mContributesToScore;
        v1 += (float)stats[group + 2]
                  * playerStatsInfo[group + 2].mContributesToScore;
        v1 += (float)stats[group + 3]
                  * playerStatsInfo[group + 3].mContributesToScore;
        v1 += (float)stats[group + 4]
                  * playerStatsInfo[group + 4].mContributesToScore;
    }
    return (int)v1;
}

// ============================================================================
// kuju cBezier / cBezierTrajectoryInterpolator (mp.o)
// ============================================================================
// ea: 0x007343A0
math::Position3 kuju::cBezier::position(float time) const
{
    if (time < 0.0f || time > 1.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/math/Bezier.cpp";
        AeAssert::gCurrentLine = 44;
        AeAssert::gCurrentExpr = "time>=0.0f && time<=1.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid time parameter"))
            __debugbreak();
    }
    math::Position3 result;
    float t2 = time * time;
    float t3 = t2 * time;
    result.v.m128_f32[0] = mInitialPoint.v.m128_f32[0]
        + mAFactor.v.m128_f32[0] * t3
        + mBFactor.v.m128_f32[0] * t2
        + mCFactor.v.m128_f32[0] * time;
    result.v.m128_f32[1] = mInitialPoint.v.m128_f32[1]
        + mAFactor.v.m128_f32[1] * t3
        + mBFactor.v.m128_f32[1] * t2
        + mCFactor.v.m128_f32[1] * time;
    result.v.m128_f32[2] = mInitialPoint.v.m128_f32[2]
        + mAFactor.v.m128_f32[2] * t3
        + mBFactor.v.m128_f32[2] * t2
        + mCFactor.v.m128_f32[2] * time;
    result.v.m128_f32[3] = 0.0f;
    return result;
}

// ea: 0x00734480
math::Dir3 kuju::cBezier::speed(float time) const
{
    if (time < 0.0f || time > 1.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/math/Bezier.cpp";
        AeAssert::gCurrentLine = 55;
        AeAssert::gCurrentExpr = "time>=0.0f && time<=1.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid time parameter"))
            __debugbreak();
    }
    math::Dir3 result;
    float t2 = time * time;
    result.v.m128_f32[0] = mAFactor.v.m128_f32[0] * 3.0f * t2
        + mBFactor.v.m128_f32[0] * 2.0f * time
        + mCFactor.v.m128_f32[0];
    result.v.m128_f32[1] = mAFactor.v.m128_f32[1] * 3.0f * t2
        + mBFactor.v.m128_f32[1] * 2.0f * time
        + mCFactor.v.m128_f32[1];
    result.v.m128_f32[2] = mAFactor.v.m128_f32[2] * 3.0f * t2
        + mBFactor.v.m128_f32[2] * 2.0f * time
        + mCFactor.v.m128_f32[2];
    result.v.m128_f32[3] = 0.0f;
    return result;
}

// ea: 0x0073F180
math::Position3 kuju::cBezierTrajectoryInterpolator::position(
    float date) const
{
    switch (mInterpolationType)
    {
    case kInterpolationLinear:
    {
        // tLinearInterpolator<math::Position3>: initial/final value/time
        const float* lin = (const float*)mLinear;
        float span = lin[16 + 3] - lin[16 + 1];
        float t = span != 0.0f ? (date - lin[16 + 1]) / span : 0.0f;
        math::Position3 result;
        for (int i = 0; i < 3; ++i)
            result.v.m128_f32[i] = lin[i] + (lin[16 + i] - lin[i]) * t;
        result.v.m128_f32[3] = 0.0f;
        return result;
    }
    case kInterpolationBezier:
        return ((const kuju::cBezier*)mBezier)
            ->position((date - mInitialDate) / mTimeInterval);
    default:
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/math/BezierTrajectoryInterpolator.cpp";
        AeAssert::gCurrentLine = 91;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("Oops"))
            __debugbreak();
        math::Position3 result;
        result.v.m128_f32[0] = 0.0f;
        result.v.m128_f32[1] = 0.0f;
        result.v.m128_f32[2] = 0.0f;
        result.v.m128_f32[3] = 0.0f;
        return result;
    }
}

// ============================================================================
// SetSeatState (mp.o)
// ============================================================================
// ea: 0x0072E110 (scr_vehicle_t seats: byte +506 = occupied, +505 = state)
void SetSeatState(scr_vehicle_t* vehicle, int seat, int state)
{
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPVehicle.cpp";
        AeAssert::gCurrentLine = 104;
        AeAssert::gCurrentExpr = "vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid vehicle in SetSeatState"))
            __debugbreak();
    }
    char* v3 = (char*)vehicle + 0x1F9 + 28 * seat;
    v3[0] = 0;
    if (state == 1)
    {
        v3[0] = 1;
    }
    else if (state == 2)
    {
        v3[1] = 1;
    }
}

// ============================================================================
// cVoiceNetworkManager (mp.o)
// ============================================================================
// ea: 0x0074F6D0 (mLocalPlayerIndex at mPlayerManager + 0x4111)
void kuju::knetuser::cVoiceNetworkManager::handleDirectDestinations(
    MPPlayerSet& sendTo, MPPlayerSet& exclude,
    unsigned long destinationPlayer)
{
    unsigned char v4 = 0;
    unsigned char playerIndex =
        *(unsigned char*)((char*)MultiplayerMgr::sInst->mPeer + 0x74E0
                          + 0x4111);
    unsigned char i = 0;
    unsigned int v5 = 0;
    do
    {
        if (v5 != destinationPlayer
            && sendTo.containsPlayer((unsigned long)v5) != 0
            && getRoutePlayer(playerIndex, i) == v4)
        {
            sendTo.removePlayer((unsigned long)v5);
            exclude.addPlayer((unsigned long)v5);
        }
        ++v4;
        ++v5;
        i = v4;
    } while (v4 < 0x10u);
}

// ea: 0x00734EC0
void kuju::knetuser::cVoiceNetworkManager::check_for_looped()
{
    sVoicePendingDispatchPacket* head = mVoicePendingDispatchPacketList;
    if (head != nullptr)
    {
        for (sVoicePendingDispatchPacket* cur = head;
             cur->mNext != nullptr; cur = cur->mNext)
        {
            sVoicePendingDispatchPacket* mNext = cur->mNext;
            if (cur == mNext)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
                AeAssert::gCurrentLine = 670;
                AeAssert::gCurrentExpr = "a!=b";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
            }
            if (mNext == nullptr)
                break;
            sVoicePendingDispatchPacket* i = mNext->mNext;
            if (cur == i)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
                AeAssert::gCurrentLine = 673;
                AeAssert::gCurrentExpr = "a!=b";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
            }
            if (i == nullptr)
                break;
        }
    }
}

// ea: 0x00734FA0
void kuju::knetuser::cVoiceNetworkManager::addVoicePendingDispatchPacket(
    sVoicePendingDispatchPacket* packet)
{
    if (packet == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
        AeAssert::gCurrentLine = 690;
        AeAssert::gCurrentExpr = "packet";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    sVoicePendingDispatchPacket* head = mVoicePendingDispatchPacketList;
    if (head != nullptr)
        head->mPrev = packet;
    if (packet->mPrev != nullptr && packet->mPrev->mNext == packet)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
        AeAssert::gCurrentLine = 699;
        AeAssert::gCurrentExpr = "packet->mPrev->mNext != packet";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    packet->mPrev = nullptr;
    packet->mNext = mVoicePendingDispatchPacketList;
    mVoicePendingDispatchPacketList = packet;
}

// ea: 0x0074FF40
void kuju::knetuser::cVoiceNetworkManager::dispatchPendingVoicePackets(
    const kuju::knet::sTime& time, MPPlayerSet& connectionsUsed,
    unsigned long& connectionsLeft)
{
    sVoicePendingDispatchPacket* cur = mVoicePendingDispatchPacketList;
    while (cur != nullptr)
    {
        sVoicePendingDispatchPacket* mNext = cur->mNext;
        unsigned char playerIndex =
            (unsigned char)cur->mPlayersToSendTo.lowestPlayerIndex();
        dispatchPacketDirectToPlayer(time, cur, playerIndex);
        if (playerIndex >= 0x10u)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp\\MPPlayerSet.h";
            AeAssert::gCurrentLine = 110;
            AeAssert::gCurrentExpr = "index < 16";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(defaultFileName))
                __debugbreak();
        }
        connectionsUsed.mBitPlayers =
            (unsigned short)(connectionsUsed.mBitPlayers
                             | (1u << playerIndex));
        --connectionsLeft;
        cur = mNext;
    }
}

// ea: 0x0074FB00
void kuju::knetuser::cVoiceNetworkManager::dispatchPacketDirectToPlayer(
    const kuju::knet::sTime& time, sVoicePendingDispatchPacket* packet,
    unsigned char player)
{
    (void)time;
    if (packet == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
        AeAssert::gCurrentLine = 1266;
        AeAssert::gCurrentExpr = "packet";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    dispatchVoicePendingDispatchPacket(packet, player);
    packet->mPlayersToSendTo.removePlayer(player);
    if (packet->mPlayersToSendTo.mBitPlayers == 0)
        discardVoicePendingDispatchPacket(packet);
}

// ea: 0x0074F740
void kuju::knetuser::cVoiceNetworkManager::dispatchVoicePendingDispatchPacket(
    sVoicePendingDispatchPacket* packet, unsigned int destinationPlayer)
{
    sVoicePendingDispatchPacket* v3 = packet;
    if (packet == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
        AeAssert::gCurrentLine = 871;
        AeAssert::gCurrentExpr = "packet";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    bdMessage* msg = new bdMessage(0x26u, false);
    bdReference<bdMessage> message;
    message.m_ptr = msg;
    if (msg != nullptr)
        ++msg->m_refCount;
    extern int g_NumBdMessages;  // 0xF93FA4
    ++g_NumBdMessages;
    bdReference<bdBitBuffer> buffer = msg->getPayload();
    unsigned char mSourcePlayer = v3->mSourcePlayer;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&mSourcePlayer, 8u);
    MPPlayerSet sendTo;
    MPPlayerSet exclude;
    sendTo.mBitPlayers = 0;
    exclude.mBitPlayers = 0;
    sendTo.addPlayers(v3->mPlayersToSendTo);
    exclude.addPlayers(v3->mPlayersToExclude);
    handleDirectDestinations(sendTo, exclude, destinationPlayer);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER16_TYPE);
    unsigned short sendToBits = sendTo.mBitPlayers;
    buffer.m_ptr->writeBits(&sendToBits, 0x10u);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER16_TYPE);
    unsigned short excludeBits = exclude.mBitPlayers;
    buffer.m_ptr->writeBits(&excludeBits, 0x10u);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
    unsigned int seq = v3->mSeqID;
    buffer.m_ptr->writeBits(&seq, 0x20u);
    for (unsigned int i = 0; i < 16; ++i)
    {
        if ((sendTo.mBitPlayers & (1u << i)) != 0)
        {
            if (v3->mPrevSeqID[i] == (unsigned int)-1)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
                AeAssert::gCurrentLine = 902;
                AeAssert::gCurrentExpr =
                    "packet->mPrevSeqID[i] != INVALID_SEQUENCE_ID";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
            }
            buffer.m_ptr->writeDataType(
                bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
            unsigned int delta = v3->mSeqID - v3->mPrevSeqID[i];
            buffer.m_ptr->writeBits(&delta, 0x20u);
        }
    }
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
    unsigned int mSize = v3->mSize;
    buffer.m_ptr->writeBits(&mSize, 0x20u);
    buffer.m_ptr->writeBits(v3->mBuffer, 8 * v3->mSize);
    unsigned char v18 = (unsigned char)destinationPlayer;
    if (destinationPlayer >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp\\MPPlayerSet.h";
        AeAssert::gCurrentLine = 75;
        AeAssert::gCurrentExpr = "index < 16";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    MPPlayerSet players;
    players.mBitPlayers = (unsigned short)(1u << v18);
    MPPlayerManager* pm =
        (MPPlayerManager*)((char*)MultiplayerMgr::sInst->mPeer + 0x74E0);
    pm->Send(message, players, false);
    mRecentlyDispatchedPacketSources[mRecentlyDispatchedPacketIndex] =
        v3->mSourcePlayer;
    mRecentlyDispatchedPacketRoutes[mRecentlyDispatchedPacketIndex] = v18;
    unsigned char v22 = mRecentlyDispatchedPacketIndex + 1;
    mRecentlyDispatchedPacketIndex = v22 >= 0xAu ? 0 : v22;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
        delete message.m_ptr;
}

// ============================================================================
// Batch 11: 260-400 byte tier (mp.o)
// ============================================================================

// ea: 0x007663E0
const bool MPUIInterface::StartClient(sGameListing& game, bool bStartGame,
                                      int nGameIndex)
{
    mHostDisconnected = false;
    mHostMigrated = false;
    if (bStartGame)
        StartGame(true, true);
    MPPeer* mPeer = MultiplayerMgr::sInst->mPeer;
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 924;
        AeAssert::gCurrentExpr = "peer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no peer"))
            __debugbreak();
    }
    bdReference<MPGameInfo>& p_mGameInfo = game.mGameInfo;
    bool result = mPeer->ConnectToPeers(p_mGameInfo, nGameIndex,
                                        mGameConnectionType);
    if (result)
    {
        mServerParams.mFriendlyFire = p_mGameInfo.m_ptr->mFriendlyFire;
        mServerParams.mGameSubType = p_mGameInfo.m_ptr->mGameSubType;
        mServerParams.mGameType = p_mGameInfo.m_ptr->mGameType;
        mServerParams.mMapID = p_mGameInfo.m_ptr->mMapID;
        strcpy(mServerParams.mName, p_mGameInfo.m_ptr->mName);
        mServerParams.mTeamBalancing = p_mGameInfo.m_ptr->mTeamBalancing;
        NextRoundServerParams();
        SetupCvars(true);
        if (mGameConnectionType == kGameConnectionTypeOnline)
        {
            MPLiveEngine* Handle = MPLiveEngine::GetHandle();
            Handle->sessionState = kInSession;
            const XNKID& SecurityID = p_mGameInfo.m_ptr->getSecurityID();
            LiveWrapper::theWrapper->SetSessionID(SecurityID);
            ((LiveWrapper*)Handle)->SetNotificationFlag(Handle->actualPort,
                                                        0x12u, true);
        }
        mInSession = true;
        return true;
    }
    return result;
}

// ea: 0x007300D0
void MPUIInterface::NextRoundServerParams()
{
    extern int g_NumTotalMaps;  // 0xF99864
    mNextServerParams = mServerParams;
    int v0 = 0;
    switch (mServerParams.mMapRotation)
    {
    case 0:
        return;
    case 1:
        ++mNextServerParams.mMapID;
        if (mNextServerParams.mMapID == g_NumTotalMaps)
            mNextServerParams.mMapID = 0;
        break;
    case 2:
        if (mNextServerParams.mMapID != 0)
            --mNextServerParams.mMapID;
        else
            mNextServerParams.mMapID = (unsigned char)(g_NumTotalMaps - 1);
        break;
    case 3:
        if (mServerParams.mRandomMapList[0] == -1)
        {
            do
            {
                if (++v0 == g_NumTotalMaps)
                {
                    if (mServerParams.mMapRotation != 0)
                        mServerParams.mMapRotation = 0;
                    mServerParams.SetMapRotation(3u);
                    v0 = 0;
                }
            } while (mServerParams.mRandomMapList[v0] == -1);
        }
        mNextServerParams.mMapID = mServerParams.mRandomMapList[v0];
        mServerParams.mRandomMapList[v0] = -1;
        mNextServerParams = mServerParams;
        break;
    default:
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 2263;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
        break;
    }
}

// ea: 0x00731B60
void MPOptionsControlsMenu::SetOptions()
{
    controller* v2 = controller::inst();
    this->entries[0]->SetText(
        kStickLayoutStrings[gSaveGameData[v2->locked_port]
                                .mStubData.mControllerStickConfiguration]);
    controller* v3 = controller::inst();
    this->entries[1]->SetText(
        kButtonLayoutStrings[gSaveGameData[v3->locked_port]
                                 .mStubData.mControllerButtonConfiguration]);
    controller* v4 = controller::inst();
    this->entries[2]->SetValue(
        gSaveGameData[v4->locked_port].mStubData.mHorizontalSensitivity);
    controller* v5 = controller::inst();
    this->entries[3]->SetValue(
        gSaveGameData[v5->locked_port].mStubData.mVerticalSensitivity);
    controller* v6 = controller::inst();
    this->entries[4]->SetValue(
        gSaveGameData[v6->locked_port].mStubData.mInvertAim);
    controller* v7 = controller::inst();
    this->entries[5]->SetValue(
        gSaveGameData[v7->locked_port].mStubData.mAdsToggle);
    controller* v8 = controller::inst();
    this->entries[6]->SetValue(
        gSaveGameData[v8->locked_port].mStubData.mTankStyle == 0);
    controller* v9 = controller::inst();
    this->entries[7]->SetValue(
        gSaveGameData[v9->locked_port].mStubData.mVibration);
}

// ea: 0x00738EC0
void MPPlayerManager::HandleSDHostBombRequest(
    const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED
            && *(bool*)((char*)this + 0x4112))
        {
            bdReference<bdMessage> msg = receivedMsg.getMessage();
            bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
            if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
                delete msg.m_ptr;
            if (gpBrocAPI->mBrocExports.mCallbackSDHostBombRequest != nullptr)
            {
                Entity* v9 = Player->mClientIndex >= 0
                                 ? EntityManager::sInst->GetPlayer(Player->mClientIndex)
                                 : nullptr;
                unsigned int mVal = v9->mHandle.mHandle.mVal;
                bool v11 = buffer.m_ptr->testBool();
                gpBrocAPI->mBrocExports.mCallbackSDHostBombRequest(mVal, v11);
            }
            if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
                delete buffer.m_ptr;
        }
    }
}

// ea: 0x00745510
void MPPeer::SendCallForMedic(const Entity* player)
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    if (p_mSession->getStatus() != bdSession::BD_SESSION_NOT_CONNECTED)
    {
        MPPlayerManager* p_mPlayerManager =
            (MPPlayerManager*)((char*)this + 0x74E0);
        MPPlayer* v3 = p_mPlayerManager->GetPlayer(player);
        if (v3 != nullptr)
        {
            bdMessage* msg = new bdMessage(0x6Au, false);
            bdReference<bdMessage> message;
            message.m_ptr = msg;
            if (msg != nullptr)
                ++msg->m_refCount;
            extern int g_NumBdMessages;
            ++g_NumBdMessages;
            bdReference<bdBitBuffer> buffer = msg->getPayload();
            unsigned char id = v3->mId;
            if (buffer.m_ptr != nullptr)
                ++buffer.m_ptr->m_refCount;
            MPUtility::WritePlayerId(buffer, id);
            bdReference<bdMessage> msg2;
            msg2.m_ptr = msg;
            if (msg != nullptr)
                ++msg->m_refCount;
            p_mPlayerManager->SendAll(msg2, true, false);
            if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
                delete buffer.m_ptr;
            if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
                delete message.m_ptr;
        }
    }
}

// ea: 0x0075C160
void MPPeer::DropWeapon(int weapon, int netIndex,
                        const math::Position3& position,
                        const math::Dir3& angles,
                        const math::Dir3& velocity, int clipCount,
                        int ammoCount)
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    if (p_mSession->getStatus() != bdSession::BD_SESSION_NOT_CONNECTED)
    {
        bdMessage* msg = new bdMessage(0x44u, false);
        bdReference<bdMessage> message;
        message.m_ptr = msg;
        if (msg != nullptr)
            ++msg->m_refCount;
        extern int g_NumBdMessages;
        ++g_NumBdMessages;
        bdReference<bdBitBuffer> buffer = msg->getPayload();
        if (buffer.m_ptr != nullptr)
            ++buffer.m_ptr->m_refCount;
        MPPlayerItems::SerializeDropWeapon(buffer, weapon, netIndex, position,
                                           angles, velocity, clipCount,
                                           ammoCount);
        if (msg != nullptr)
            ++msg->m_refCount;
        ((MPPlayerManager*)((char*)this + 0x74E0))->SendOthers(message,
                                                               nullptr, true);
        if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
            delete buffer.m_ptr;
        if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
            delete message.m_ptr;
    }
}

// ea: 0x007442C0
void MPPeer::SwapKit(int playerClass, int netIndex)
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    if (p_mSession->getStatus() != bdSession::BD_SESSION_NOT_CONNECTED)
    {
        bdMessage* msg = new bdMessage(0x69u, false);
        bdReference<bdMessage> message;
        message.m_ptr = msg;
        if (msg != nullptr)
            ++msg->m_refCount;
        extern int g_NumBdMessages;
        ++g_NumBdMessages;
        bdReference<bdBitBuffer> buffer = msg->getPayload();
        buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
        unsigned char pc = (unsigned char)playerClass;
        buffer.m_ptr->writeBits(&pc, 8u);
        buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE);
        int ni = netIndex;
        buffer.m_ptr->writeBits(&ni, 0x20u);
        bdReference<bdMessage> msg2;
        msg2.m_ptr = msg;
        if (msg != nullptr)
            ++msg->m_refCount;
        ((MPPlayerManager*)((char*)this + 0x74E0))->SendAll(msg2, true, false);
        if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
            delete buffer.m_ptr;
        if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
            delete message.m_ptr;
    }
}

// ea: 0x0073B2C0
bool MPUtility::ReadAngles(bdReference<bdBitBuffer> buffer, float* angles)
{
    short v8 = 0;
    bool ok = buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE)
              && buffer.m_ptr->readBits(&v8, 0x10u);
    angles[0] = v8 * 0.0054931641f;
    if (ok
        && buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE)
        && buffer.m_ptr->readBits(&v8, 0x10u))
        ok = true;
    else
        ok = false;
    angles[1] = v8 * 0.0054931641f;
    if (ok
        && buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE)
        && buffer.m_ptr->readBits(&v8, 0x10u))
        ok = true;
    else
        ok = false;
    angles[2] = v8 * 0.0054931641f;
    bool v5 = ok;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v5;
}

// ea: 0x0073A370
void MPPlayerManager::ClientDisconnect(MPPlayer* player, bool noScript)
{
    int mClientIndex = player->mClientIndex;
    if (mClientIndex >= 0)
    {
        const Entity* v5 = EntityManager::sInst->GetPlayer(mClientIndex);
        if (!IsLocalPlayer(v5) && player->mInVehicle)
        {
            MPVehicle* VehicleFromOccupant = GetVehicleFromOccupant(player);
            if (VehicleFromOccupant != nullptr)
                VehicleFromOccupant->GetOutOfVehicle(
                    player, ((Entity*)VehicleFromOccupant->mEntity)->health,
                    true);
        }
        int v7 = mClientIndex;
        if (svs.clients[v7].state != 1)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
            AeAssert::gCurrentLine = 7310;
            AeAssert::gCurrentExpr = "svs.clients[clientIndex].state == CS_ACTIVE";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Having trouble disconnecting client."))
                __debugbreak();
        }
        svs.clients[v7].state = (clientState_t)0;
        if (!noScript && gpBrocAPI->mBrocExports.mCallbackPlayerLeave != nullptr)
        {
            Entity* v8 = player->mClientIndex >= 0
                             ? EntityManager::sInst->GetPlayer(player->mClientIndex)
                             : nullptr;
            gpBrocAPI->mBrocExports.mCallbackPlayerLeave(
                v8->mHandle.mHandle.mVal);
        }
        Entity* v9 = player->mClientIndex >= 0
                         ? EntityManager::sInst->GetPlayer(player->mClientIndex)
                         : nullptr;
        ::ClientDisconnect(v9->mHandle);
        player->Reset(true);
        player->mClientIndex = -1;
    }
}

// ea: 0x0074ED90
void MPProfileMainMenu::Update(float time_inc)
{
    ProfileManager* v3 = ProfileManager::Me();
    v3->Update();
    int mCurrentStatus = ProfileManager::Me()->mCurrentStatus;
    switch (mCurrentStatus)
    {
    case -4:
        DialogDisplayDataCorrupt();
        ProfileManager::Me()->Reset();
        FEMenu::Update(time_inc);
        return;
    case -3:
    case 2:
    case 4:
        ProfileManager::Me()->Reset();
        LoadProfilesDone();
        OnSelectionChange();
        FEMenu::Update(time_inc);
        return;
    case -2:
        DialogDisplayNoFreeSpace();
        ProfileManager::Me()->Reset();
        FEMenu::Update(time_inc);
        return;
    case -1:
        ProfileManager::Me()->Reset();
        FEMenu::Update(time_inc);
        return;
    case 0:
    case 1:
        break;
    case 5:
        ProfileManager::Me()->Reset();
        DialogDisplaySaveSuccess();
        FEMenu::Update(time_inc);
        return;
    case 7:
        ProfileManager::Me()->Reset();
        DialogDisplayDeleteSuccess();
        FEMenu::Update(time_inc);
        return;
    default:
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/MPProfileMainMenu.cpp";
        AeAssert::gCurrentLine = 176;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Illegal state (%d) from Profile Manager",
                                mCurrentStatus))
            __debugbreak();
        break;
    }
    FEMenu::Update(time_inc);
}

// ea: 0x00745090
void MPPeer::SendBombOperation(const Entity* player, bool defusing)
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    if (p_mSession->getStatus() != bdSession::BD_SESSION_NOT_CONNECTED
        && p_mSession->getRole() == bdSession::BD_SESSION_HOST)
    {
        MPPlayerManager* p_mPlayerManager =
            (MPPlayerManager*)((char*)this + 0x74E0);
        MPPlayer* v6 = p_mPlayerManager->GetPlayer(player);
        bdMessage* msg = new bdMessage(0x56u, false);
        bdReference<bdMessage> message;
        message.m_ptr = msg;
        if (msg != nullptr)
            ++msg->m_refCount;
        extern int g_NumBdMessages;
        ++g_NumBdMessages;
        bdReference<bdBitBuffer> buffer = msg->getPayload();
        unsigned char playera = v6->mId;
        if (buffer.m_ptr != nullptr)
            ++buffer.m_ptr->m_refCount;
        MPUtility::WritePlayerId(buffer, playera);
        buffer.m_ptr->writeBool(defusing);
        if (msg != nullptr)
            ++msg->m_refCount;
        p_mPlayerManager->SendAll(message, true, false);
        if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
            delete buffer.m_ptr;
        if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
            delete message.m_ptr;
    }
}

// ea: 0x007577C0
void MPPlayerManager::SendPlayerEnter(MPPlayer* player)
{
    bdMessage* msg = new bdMessage(0x31u, false);
    bdReference<bdMessage> message;
    message.m_ptr = msg;
    if (msg != nullptr)
        ++msg->m_refCount;
    extern int g_NumBdMessages;  // 0xF93FA4
    ++g_NumBdMessages;
    bdReference<bdBitBuffer> buffer = msg->getPayload();
    unsigned char playera = player->mId;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    MPUtility::WritePlayerId(buffer, playera);
    if (msg != nullptr)
        ++msg->m_refCount;
    SendOthers(message, nullptr, true);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
        delete message.m_ptr;
}

// ea: 0x007625E0
void MPPlayerManager::onSessionDisconnect(
    bdReference<bdConnection> connection)
{
    if (*(char*)((char*)this + 0x2CCD) == 0)  // mPlayers[15].mYaw[2].mLift
    {
        bdReference<bdConnection> conn = connection;
        if (connection.m_ptr != nullptr)
            ++connection.m_ptr->m_refCount;
        FlushBufferedMessages(conn);
    }
    for (int i = 0; i < 16; ++i)
    {
        MPPlayer* Player = GetPlayer(i);
        bool v7 = false;
        if (Player != nullptr)
        {
            bdReference<bdConnection> result = Player->GetConnection();
            v7 = result.m_ptr == connection.m_ptr;
            if (result.m_ptr != nullptr && result.m_ptr->m_refCount-- == 1)
                delete result.m_ptr;
        }
        if (v7)
            DisconnectPlayer(Player);
    }
    if (connection.m_ptr != nullptr && connection.m_ptr->m_refCount-- == 1)
        delete connection.m_ptr;
}

// ea: 0x00739E90
void MPPlayerManager::HandleMute(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdMessage> msg = receivedMsg.getMessage();
    bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
    bool shouldMute = false;
    if (buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_FULL_TYPE))
    {
        XUID muterID;
        if (buffer.m_ptr->readBits(&muterID, 0x60u))
        {
            if (buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_BOOL_TYPE))
            {
                unsigned char byte = 0;
                if (buffer.m_ptr->readBits(&byte, 1u))
                    shouldMute = byte != 0;
            }
            MPLiveEngine* Handle = MPLiveEngine::GetHandle();
            LiveWrapper::theWrapper->RemoteMute(Handle->actualPort, &muterID,
                                                shouldMute);
        }
    }
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
        delete msg.m_ptr;
}

// ea: 0x0072DE40
void MPPlayer::UpdateInGamePlayerInfo(bool autoBalance, bool clear_stats)
{
    int mClientIndex = this->mClientIndex;
    if (mClientIndex >= 0 && *(bool*)((char*)this + 0x254))
    {
        Entity* Player = EntityManager::sInst->GetPlayer(mClientIndex);
        Player->sentient->eTeam = (team_t)*(short*)((char*)this + 0x25C);
        Player->client->pers.rank = *(short*)((char*)this + 0x25E);
        if (clear_stats)
        {
            memset(&Player->client->pers, 0, 0x194u);
            *(int*)((char*)&Player->client->pers + 0x194) = 0;
            *(int*)((char*)&Player->client->pers + 0x198) = 0;
            Player->client->pers.mStats[0][4] = *(short*)((char*)this + 0x25A);
            Player->client->pers.mStats[0][3] = *(short*)((char*)this + 0x258);
            Player->client->pers.mBaseScore = *(short*)((char*)this + 0x256);
        }
        if (gpBrocAPI->mBrocExports.mCallbackPlayerTeamChange != nullptr)
        {
            Entity* v8 = this->mClientIndex >= 0
                             ? EntityManager::sInst->GetPlayer(this->mClientIndex)
                             : nullptr;
            gpBrocAPI->mBrocExports.mCallbackPlayerTeamChange(
                v8->mHandle.mHandle.mVal, autoBalance, 0);
        }
    }
}

// ea: 0x00735CC0 (m_QosAddr +0x10, m_QosIsAvailable +0x60F0,
// m_QosIsComplete +0x5AB0, m_QosDeleteProbe +0x6410)
int MPPeer::FindActiveQosProbe(bdReference<bdCommonAddr> address,
                               const XNKID& SecurityID,
                               const XNKEY& SecurityKey)
{
    int v5 = 0;
    while (*(bool*)((char*)this + 0x60F0 + v5)
           || ((bdQoSRemoteAddr*)((char*)this + 0x10 + 0x1C * v5))
                  ->m_addr.m_ptr != address.m_ptr)
    {
        ++v5;
        if (v5 >= 800)
        {
            if (address.m_ptr != nullptr && address.m_ptr->m_refCount-- == 1)
                delete address.m_ptr;
            return -1;
        }
    }
    if (*(bool*)((char*)this + 0x5AB0 + v5))
    {
        *(bool*)((char*)this + 0x60F0 + v5) = true;
        if (address.m_ptr != nullptr && address.m_ptr->m_refCount-- == 1)
            delete address.m_ptr;
        return -1;
    }
    char* addressa = (char*)this + 28 * v5;
    if (memcmp(addressa + 20, &SecurityID, 8u) != 0
        || memcmp(addressa + 28, &SecurityKey, 0x10u) != 0)
    {
        *(bool*)((char*)this + 0x6410 + v5) = true;
        if (address.m_ptr != nullptr && address.m_ptr->m_refCount-- == 1)
            delete address.m_ptr;
        return -1;
    }
    if (address.m_ptr != nullptr && address.m_ptr->m_refCount-- == 1)
        delete address.m_ptr;
    return v5;
}

// ea: 0x00748A20
void MPPlayerManager::SendOthers(bdReference<bdMessage> message,
                                 const MPPlayer* excludePlayer,
                                 bool reliable)
{
    if (reliable)
    {
        unsigned char Type = message.m_ptr->getType();
        tlPrintf("Sending reliable message %d to OTHERS\n", Type);
    }
    unsigned char localIdx = *(unsigned char*)((char*)this + 0x4111);
    if (localIdx >= 0x10u)
        return;
    for (int i = 0; i < 16; ++i)
    {
        MPPlayer* mPlayers = (MPPlayer*)((char*)this + 0x1010 + 0x310 * i);
        bdConnection* m_ptr = mPlayers->mConnection.m_ptr;
        if (m_ptr != nullptr
            && !IsLocalId(mPlayers->mId)
            && mPlayers != excludePlayer
            && *(bool*)((char*)mPlayers + 0x46))  // mMasterClient
        {
            bdReference<bdMessage> msg2;
            msg2.m_ptr = message.m_ptr;
            if (message.m_ptr != nullptr)
                ++message.m_ptr->m_refCount;
            SendPlayer(mPlayers, msg2, reliable);
        }
        if (m_ptr != nullptr && m_ptr->m_refCount-- == 1)
            delete m_ptr;
    }
    if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
        delete message.m_ptr;
}

// ea: 0x00763830
::MPEntityHandle MPPlayerManager::RegisterDroppedItem(
    EDroppedItemTypes itemType, Entity* item, Entity* owner)
{
    ::MPEntityHandle result;
    if (item == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 8201;
        AeAssert::gCurrentExpr = "item";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("RegisterDroppedItem: Invalid item entity."))
            __debugbreak();
    }
    if (owner == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 8202;
        AeAssert::gCurrentExpr = "owner";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("RegisterDroppedItem: Invalid owner entity."))
            __debugbreak();
    }
    MPPlayer* Player = GetPlayer(owner);
    if (owner == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 8205;
        AeAssert::gCurrentExpr = "owner";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "RegisterDroppedItem: Could not get MPPlayer from owner entity"))
            __debugbreak();
    }
    if (Player != nullptr)
    {
        short v7 =
            ((MPPlayerItems*)((char*)Player + 0x0C))->AddItem(itemType, item);
        result.mValue =
            (unsigned short)((Player->mId << 12) | (v7 & 0xFFF));
    }
    else
    {
        result.mValue = 0;
    }
    return result;
}

// ea: 0x0072F060
void MPPlayerManager::ClearStateVariables()
{
    *(bool*)((char*)this + 0x5868) = false;   // mGameParamsSet
    *(int*)((char*)this + 0x586C) = 0;        // mGameType
    *(float*)((char*)this + 0x5870) = 0.0f;   // mCurrentTime
    *(int*)((char*)this + 0x5874) = 0;        // mTimeLimit
    *(int*)((char*)this + 0x5878) = 0;        // mScoreLimit
    *(int*)((char*)this + 0x587C) = 0;        // mRoundLimit
    *(int*)((char*)this + 0x5880) = 0;        // mAlliesScore
    *(int*)((char*)this + 0x5884) = 0;        // mAxisScore
    *(bool*)((char*)this + 0x5888) = false;   // mFriendlyFire
    *(bool*)((char*)this + 0x5889) = false;   // mLastManStanding
    *(bool*)((char*)this + 0x588A) = false;   // mTeamBalance
    *(int*)((char*)this + 0x588C) = 0;        // mRespawnTime
    *(unsigned char*)((char*)this + 0x5890) = 16;  // mAlliedFlagStatus
    *(unsigned char*)((char*)this + 0x5891) = 16;  // mAxisFlagStatus
    *(float*)((char*)this + 0x5894) = 0.0f;
    *(float*)((char*)this + 0x5898) = 0.0f;
    *(float*)((char*)this + 0x589C) = 0.0f;
    *(float*)((char*)this + 0x58A0) = 0.0f;
    *(float*)((char*)this + 0x58A4) = 0.0f;
    *(float*)((char*)this + 0x58A8) = 0.0f;
    memset((char*)this + 0x58AC, 0, 20);  // mFlagStatus
    *(unsigned char*)((char*)this + 0x58C0) = 16;  // mSDPlanter
    *(unsigned char*)((char*)this + 0x58C1) = 16;  // mSDDefuser
    *(bool*)((char*)this + 0x58C2) = false;        // mSDPlanted
    memset((char*)this + 0x58C4, 0, 24);   // mSDBombPosition + mSDBombAngles
    *(int*)((char*)this + 0x58DC) = 0;     // mSDTimeLeft
    *(bool*)((char*)this + 0x58E0) = false;  // mRoundStarted
    *(bool*)((char*)this + 0x58E1) = false;  // mRoundOver
    *(int*)((char*)this + 0x58E4) = 0;       // mRoundCount
    *(int*)((char*)this + 0x58EC) = 0;       // mHQStage
    memset((char*)this + 0x58F0, 0, 24);     // mHQvA + mHQvB
}

// ea: 0x00734D90
void kuju::knetuser::cVoiceNetworkManager::flushFirstPacketInList(
    unsigned long listIndex, unsigned int discardData)
{
    if (listIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
        AeAssert::gCurrentLine = 519;
        AeAssert::gCurrentExpr = "listIndex < 16";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    if (mPendingVoicePacketList[listIndex] == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
        AeAssert::gCurrentLine = 520;
        AeAssert::gCurrentExpr = "mPendingVoicePacketList[listIndex]";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    do
    {
        sVoicePacket* v4 = mPendingVoicePacketList[listIndex];
        mLastReceivedSeqID[listIndex] = v4->mSeqID;
        mPendingVoicePacketList[listIndex] = v4->mNext;
        if (discardData == 0)
            ((iVoiceHandlerInterface*)mVoiceHandlerInterface)
                ->receiveVoiceData(listIndex, v4->mBuffer, v4->mSize);
        if (mFreeVoicePacketList != nullptr)
            mFreeVoicePacketList->mPrev = v4;
        v4->mPrev = nullptr;
        v4->mNext = mFreeVoicePacketList;
        mFreeVoicePacketList = v4;
    } while (mPendingVoicePacketList[listIndex] != nullptr
             && mPendingVoicePacketList[listIndex]->mPrevSeqID
                    == mLastReceivedSeqID[listIndex]);
}

// ea: 0x007352C0
void kuju::knetuser::cVoiceNetworkManager::determineClosestDestination(
    sVoicePendingDispatchPacket* packet,
    unsigned char& closestPlayerForThisPacket,
    float& closestDistanceForThisPacket)
{
    unsigned int v4 = 0;
    if (packet == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/knetuser/cvoicenetworkmanager.cpp";
        AeAssert::gCurrentLine = 1152;
        AeAssert::gCurrentExpr = "packet";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    closestPlayerForThisPacket = 16;
    closestDistanceForThisPacket = 1000000.0f;
    for (unsigned char i = 0; i < 0x10u; ++i)
    {
        if (v4 >= 0x10)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp\\MPPlayerSet.h";
            AeAssert::gCurrentLine = 153;
            AeAssert::gCurrentExpr = "index < 16";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(defaultFileName))
                __debugbreak();
        }
        if (((1u << v4) & packet->mPlayersToSendTo.mBitPlayers) != 0)
        {
            float* v6 = &mPlayerDistances[packet->mSourcePlayer][v4];
            if (*v6 < 0.0f || closestDistanceForThisPacket <= *v6)
            {
                if (closestPlayerForThisPacket == 16)
                    closestPlayerForThisPacket = i;
            }
            else
            {
                closestPlayerForThisPacket = i;
                closestDistanceForThisPacket =
                    mPlayerDistances[packet->mSourcePlayer][v4];
            }
        }
        ++v4;
    }
}

// ea: 0x0073F2A0
kuju::knetuser::cVoiceNetworkManager::cVoiceNetworkManager()
{
    mLastTime.mTime = 0;
    mVoiceHandlerInterface = nullptr;
    for (int i = 0; i < 24; ++i)
        mVoicePackets[i].mTimeReceived.mTime = 0;
    mFreeVoicePacketList = nullptr;
    for (int i = 0; i < 5; ++i)
    {
        mVoicePendingDispatchPackets[i].mPlayersToExclude.mBitPlayers = 0;
        mVoicePendingDispatchPackets[i].mPlayersToSendTo.mBitPlayers = 0;
        mVoicePendingDispatchPackets[i].mTimeReceived.mTime = 0;
    }
    mFreeVoicePendingDispatchPacketList = nullptr;
    mVoicePendingDispatchPacketList = nullptr;
    mActivePlayerVoices.mBitPlayers = 0;
    for (int i = 0; i < 16; ++i)
        mTimeVoiceActive[i].mTime = 0;
    mLastDispatchTime.mTime = 0;
    mNextDispatchedSeqID = 0;
    mNumActivePlayerVoices = 0;
    mRecentlyReceivedPacketIndex = 0;
    mRecentlyDispatchedPacketIndex = 0;
    mMissedPackets = 0;
}

// ea: 0x00742DA0 (host-only map-restart broadcast)
void MPPeer::MapRestart()
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    if (p_mSession->getStatus() != bdSession::BD_SESSION_NOT_CONNECTED
        && p_mSession->getRole() == bdSession::BD_SESSION_HOST)
    {
        bdMessage* msg = new bdMessage(0x5Au, false);
        bdReference<bdMessage> message;
        message.m_ptr = msg;
        if (msg != nullptr)
            ++msg->m_refCount;
        extern int g_NumBdMessages;
        ++g_NumBdMessages;
        bdReference<bdBitBuffer> buffer = msg->getPayload();
        bdReference<bdMessage> msg2;
        msg2.m_ptr = msg;
        if (msg != nullptr)
            ++msg->m_refCount;
        ((MPPlayerManager*)((char*)this + 0x74E0))->SendAll(msg2, true, false);
        if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
            delete buffer.m_ptr;
        if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
            delete message.m_ptr;
    }
}

// ea: 0x00747E80
void MPPlayerItems::SerializeDropWeapon(
    bdReference<bdBitBuffer> buffer, int weapon, int netIndex,
    const math::Position3& position, const math::Dir3& angles,
    const math::Dir3& velocity, int clipCount, int ammoCount)
{
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
    unsigned char w = (unsigned char)weapon;
    buffer.m_ptr->writeBits(&w, 8u);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE);
    int ni = netIndex;
    buffer.m_ptr->writeBits(&ni, 0x20u);
    bdReference<bdBitBuffer> sub;
    sub.m_ptr = buffer.m_ptr;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    MPUtility::WritePosition(sub, position.v.m128_f32);
    sub.m_ptr = buffer.m_ptr;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    MPUtility::WriteAngles(sub, angles);
    sub.m_ptr = buffer.m_ptr;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    MPUtility::WriteVector(sub, velocity.v.m128_f32);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE);
    short cc = (short)clipCount;
    buffer.m_ptr->writeBits(&cc, 0x10u);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE);
    short ac = (short)ammoCount;
    buffer.m_ptr->writeBits(&ac, 0x10u);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x007481D0
void MPPlayerItems::SerializeFireMissile(
    bdReference<bdBitBuffer> buffer, int weapon,
    const math::Position3& position, const math::Dir3& dir,
    ::MPEntityHandle handle)
{
    bdReference<bdBitBuffer> sub;
    sub.m_ptr = buffer.m_ptr;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    MPUtility::WritePosition(sub, position.v.m128_f32);
    sub.m_ptr = buffer.m_ptr;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    MPUtility::WriteVector(sub, dir.v.m128_f32);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
    unsigned char w = (unsigned char)weapon;
    buffer.m_ptr->writeBits(&w, 8u);
    MPPlayerManager* pm = (MPPlayerManager*)((char*)MultiplayerMgr::sInst->mPeer + 0x74E0);
    unsigned char localIdx = *(unsigned char*)((char*)pm + 0x4111);
    unsigned char id = 16;
    if (localIdx < 0x10u)
        id = *(unsigned char*)((char*)pm + 0x1010 + 0x310 * localIdx);
    sub.m_ptr = buffer.m_ptr;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    MPUtility::WritePlayerId(sub, id);
    sub.m_ptr = buffer.m_ptr;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    MPUtility::WriteEntityHandle(sub, handle);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x007629D0
void MPPlayerManager::HandleFireMissile(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED
            && *(bool*)((char*)this + 0x4112))
        {
            bdReference<bdMessage> msg = receivedMsg.getMessage();
            bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
            if (buffer.m_ptr != nullptr)
                ++buffer.m_ptr->m_refCount;
            MPPlayerItems::DeserializeFireMissile(buffer, true);
            if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
                delete buffer.m_ptr;
            if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
                delete msg.m_ptr;
        }
    }
}

// ============================================================================
// Batch 12: smallest remaining mp.o functions (menu virtuals, wrappers)
// ============================================================================

// ea: 0x0072C8A0 (mPlayerManager at +0x74E0)
MPPlayerManager* MPPeer::GetPlayerManager()
{
    return (MPPlayerManager*)((char*)this + 0x74E0);
}

// ea: 0x0072E8D0 (mLocalPlayerIndex at +0x4111)
bool MPPlayerManager::AddingLocalPlayer() const
{
    return *(unsigned char*)((char*)this + 0x4111) == 17;
}

// ea: 0x0072F470
const bool MPUIInterface::IsLANGame()
{
    return mGameConnectionType == kGameConnectionTypeLan;
}

// ea: 0x0072F490
const bool MPUIInterface::IsLocalGame()
{
    return mGameConnectionType == kGameConnectionTypeLocal;
}

// ea: 0x0074EFD0 (profile dialog: confirm delete -> deleting screen)
bool MPProfileMainMenu::DialogResponseDeleteConfirm(int index)
{
    (void)index;
    ((MPProfileMainMenu*)g_femanager.fems->menus[27])
        ->DialogDisplayDeleting();
    return false;
}

// ea: 0x007615C0
void MultiplayerMgr::DropWeapon(int weapon, int netIndex,
                                const math::Position3& position,
                                const math::Dir3& angles,
                                const math::Dir3& velocity, int clipCount,
                                int ammoCount)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
        mPeer->DropWeapon(weapon, netIndex, position, angles, velocity,
                          clipCount, ammoCount);
}

// ea: 0x00750370
void MultiplayerMgr::FireArtillery(Entity* attacker, int weapon,
                                   const math::Position3& position, int seed,
                                   bool fire)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
        mPeer->FireArtillery(attacker, weapon, position, seed, fire);
}

// ea: 0x007503D0
void MultiplayerMgr::PlayerDamage(Entity* hitEntity, Entity* attacker,
                                  const math::Position3& position,
                                  const math::Dir3& normal, int weapon,
                                  short damage, unsigned char mod,
                                  short dflags, int hitLocation)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
        mPeer->PlayerDamage(hitEntity, attacker, position, normal, weapon,
                            damage, mod, dflags, hitLocation);
}

// ea: 0x00761290
void MultiplayerMgr::SpreadFire(Entity* player, float gunPitch, float gunYaw,
                                const math::Position3& weaponPosition,
                                int weapon, float spread,
                                float coneAngleTangent, int seed)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
        mPeer->SpreadFire(player, gunPitch, gunYaw, weaponPosition, weapon,
                          spread, coneAngleTangent, seed);
}

// ea: 0x007610D0
void kuju::kvoicemanager::cVoiceManager::updateSystem(
    kuju::knet::sTime& currentTime)
{
    (void)currentTime;
    evaluatePlayers();
    dispatchVoiceData();
}

// ea: 0x0075AC20
void MultiplayerMgr::VehicleDamage(Entity* hitEntity, Entity* attacker,
                                   const math::Position3& position,
                                   const math::Dir3& normal, short damage,
                                   int weapon, unsigned char mod,
                                   short dflags)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
        mPeer->VehicleDamage(hitEntity, attacker, position, normal, damage,
                             weapon, mod, dflags);
}

// ea: 0x00761320
void MultiplayerMgr::VehicleFireMissile(Entity* vehEnt, int weapon,
                                        const math::Position3& position,
                                        const math::Dir3& dir)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
        mPeer->VehicleFireMissile(vehEnt, weapon, position, dir);
}

// ea: 0x00748A00 (cls.state == CA_ACTIVE == 2)
void MPPlayerManager::AttemptHotJoin(int clientIndex)
{
    if (cls.state == 2)
        AddLocalPlayer(clientIndex);
}

// ea: 0x0075AF40 (mPlayerManager at +0x74E0)
void MultiplayerMgr::ApplyLocalPhysicsToVehicle(
    Entity* vehicle, const math::Position3& position,
    const math::Dir3& angles, const math::Dir3& velocity)
{
    if (this->mPeer != nullptr)
        ((MPPlayerManager*)((char*)this->mPeer + 0x74E0))
            ->ApplyLocalPhysicsToVehicle(vehicle, position, angles, velocity);
}

// ea: 0x0073A8A0 (empty stub in the release build)
void MPPlayerManager::DeserializeGameState(bdReference<bdBitBuffer> buffer)
{
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073A880 (empty stub in the release build)
void MPPlayerManager::SerializeGameState(bdReference<bdBitBuffer> buffer)
{
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0072C480
void MultiplayerMgr::Stop()
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
        delete mPeer;
    this->mPeer = nullptr;
    this->mInitialized = false;
}

// ea: 0x007347C0
int PlayerStats::TotalScoreForSingleStat(int stat, int value)
{
    return (int)((float)value
                 * playerStatsInfo[stat].mContributesToScore);
}

// ea: 0x00733D90 (VKMenu::mSlotNum at +0x1BC)
void MPProfileMainMenu::CreateProfile()
{
    int highlighted = this->highlighted;
    *(int*)((char*)VKMenu::Me() + 0x1BC) = highlighted;
    this->system->gap1C(this->system, 18);
}

// ea: 0x00736190
bdReference<bdConnection> MPPlayer::GetConnection() const
{
    bdReference<bdConnection> result;
    result.m_ptr = mConnection.m_ptr;
    if (mConnection.m_ptr != nullptr)
        ++mConnection.m_ptr->m_refCount;
    return result;
}

// ea: 0x00731B00
void MPOptionsControlsMenu::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    FEMenu::Cleanup();
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    mInstructionsText = nullptr;
}

// ea: 0x00732F30
void MPOptionsPreferencesMenu::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    FEMenu::Cleanup();
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    mInstructionsText = nullptr;
}

// ea: 0x007311F0
void MPOptionsSoundMenu::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    FEMenu::Cleanup();
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    mInstructionsText = nullptr;
}

// ea: 0x00734220
void MPProfileMainMenu::ButtonHeldAction()
{
    char button_held_down = this->button_held_down;
    if (button_held_down == 4)
        this->OnUp(0);
    else if (button_held_down == 8)
        this->OnDown(0);
}

// ea: 0x0073E7C0
void MPProfileMainMenu::OnActivate(int previous)
{
    (void)previous;
    FEMenu::OnActivate();
    this->SetHigh(0, false);
    OnSelectionChange();
}

// ea: 0x00751160 (cls.state == CA_ACTIVE == 2)
void MultiplayerMgr::AttemptHotJoin(int clientIndex)
{
    if (this->mPeer != nullptr && cls.state == 2)
        ((MPPlayerManager*)((char*)this->mPeer + 0x74E0))
            ->AddLocalPlayer(clientIndex);
}

// ea: 0x007333D0
void MPProfileEditMenu::Update(float time_inc)
{
    mListBox.Update(time_inc);
    FEMenu::Update(time_inc);
}

// ea: 0x00733A60 (mHelpBar vtable slot 19 = Draw(bool))
void MPProfileMainMenu::Draw()
{
    mPanel->Draw();
    typedef void (__thiscall* DrawFn)(FEMultiLineText*, bool);
    ((DrawFn)((void**)mHelpBar)[19])(mHelpBar, false);
    FEMenu::Draw();
}

// ea: 0x00736EE0 (rb_vehicle::m_flags.mMask bit 0)
bool MPVehicle::IsPhysicsPaused() const
{
    Entity* mEntity = (Entity*)this->mEntity;
    if (mEntity != nullptr && mEntity->scr_vehicle->mRBVeh != nullptr)
        return ((rb_vehicle*)mEntity->scr_vehicle->mRBVeh)->m_flags & 1;
    return false;
}

// ea: 0x0073E340
void MPOptionsControlsMenu::OnTriangle(int c)
{
    (void)c;
    if (SaveOptions())
        ProfileEditMenu::Me()->mNeedWrite = true;
    this->system->gap1C(this->system, 29);
}

// ea: 0x0073E4B0
void MPOptionsGameplayMenu::OnTriangle(int c)
{
    (void)c;
    if (SaveOptions())
        ProfileEditMenu::Me()->mNeedWrite = true;
    this->system->gap1C(this->system, 29);
}

// ea: 0x00731B50
void MPOptionsControlsMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00732720
void MPOptionsGameplayMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00732F80
void MPOptionsPreferencesMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00730AF0
void MPOptionsScreenMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00731240
void MPOptionsSoundMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x0072C7E0
EVoipGroup GetVoipGroup(int state)
{
    switch (state)
    {
    case 3:
    case 4:
        return kVoipGroupPlaying;
    case 5:
        return kVoipGroupDead;
    default:
        return kVoipGroupSpectating;
    }
}

// ea: 0x0073EEE0
kuju::cBezier::cBezier(const math::Position3& initialPoint,
                       const math::Dir3& initialInflexion,
                       const math::Position3& finalPoint,
                       const math::Dir3& finalInflexion)
{
    reset(initialPoint, initialInflexion, finalPoint, finalInflexion);
}

// ============================================================================
// Batch 13: menu draw/triangle/widescreen virtuals, game-state forwarders
// ============================================================================

// ea: 0x00733320
void MPOptionsPreferencesMenu::OnTriangle(int c)
{
    (void)c;
    if (SaveOptions())
        ProfileEditMenu::Me()->mNeedWrite = true;
    this->system->gap1C(this->system, 29);
}

// ea: 0x00730CE0
void MPOptionsScreenMenu::OnTriangle(int c)
{
    (void)c;
    if (SaveOptions())
        ProfileEditMenu::Me()->mNeedWrite = true;
    this->system->gap1C(this->system, 29);
}

// ea: 0x0073E200
void MPOptionsSoundMenu::OnTriangle(int c)
{
    (void)c;
    if (SaveOptions())
        ProfileEditMenu::Me()->mNeedWrite = true;
    this->system->gap1C(this->system, 29);
}

// ea: 0x0072CA90 (m_QosIsAvailable +0x60F0, m_QosIsComplete +0x5AB0)
void MPPeer::DeleteQosProbes()
{
    for (int i = 0; i < 800; ++i)
    {
        if (!*(bool*)((char*)this + 0x60F0 + i)
            && *(bool*)((char*)this + 0x5AB0 + i))
            *(bool*)((char*)this + 0x60F0 + i) = true;
    }
}

// ea: 0x007614C0 (mPlayers at +0x1010, stride 0x310, mItems at +0x0C)
void MultiplayerMgr::RemoveDroppedItems()
{
    if (mPeer != nullptr)
    {
        char* mgr = (char*)mPeer + 0x74E0;
        for (int i = 0; i < 16; ++i)
            ((MPPlayerItems*)(mgr + 0x1010 + 0x310 * i + 0x0C))->RemoveAll();
    }
}

// ea: 0x0075D410
void MPVehicle::Step()
{
    kuju::knet::sTime time = MultiplayerMgr::sInst->getLocalTime();
    UpdateInterpolation(time);
}

// ea: 0x00731B20 (mInstructionsText vtable slot 19 = Draw(bool))
void MPOptionsControlsMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    if (mInstructionsText != nullptr)
    {
        typedef void (__thiscall* DrawFn)(FEMultiLineText*, bool);
        ((DrawFn)((void**)mInstructionsText)[19])(mInstructionsText, false);
    }
    FEMenu::Draw();
}

// ea: 0x007321F0
void MPOptionsGameplayMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    if (mInstructionsText != nullptr)
    {
        typedef void (__thiscall* DrawFn)(FEMultiLineText*, bool);
        ((DrawFn)((void**)mInstructionsText)[19])(mInstructionsText, false);
    }
    FEMenu::Draw();
}

// ea: 0x00732F50
void MPOptionsPreferencesMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    if (mInstructionsText != nullptr)
    {
        typedef void (__thiscall* DrawFn)(FEMultiLineText*, bool);
        ((DrawFn)((void**)mInstructionsText)[19])(mInstructionsText, false);
    }
    FEMenu::Draw();
}

// ea: 0x00730AC0
void MPOptionsScreenMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    if (mInstructionsText != nullptr)
    {
        typedef void (__thiscall* DrawFn)(FEMultiLineText*, bool);
        ((DrawFn)((void**)mInstructionsText)[19])(mInstructionsText, false);
    }
    FEMenu::Draw();
}

// ea: 0x00731210
void MPOptionsSoundMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    if (mInstructionsText != nullptr)
    {
        typedef void (__thiscall* DrawFn)(FEMultiLineText*, bool);
        ((DrawFn)((void**)mInstructionsText)[19])(mInstructionsText, false);
    }
    FEMenu::Draw();
}

// ea: 0x007333A0
void MPProfileEditMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    if (mInstructionsText != nullptr)
    {
        typedef void (__thiscall* DrawFn)(FEMultiLineText*, bool);
        ((DrawFn)((void**)mInstructionsText)[19])(mInstructionsText, false);
    }
    FEMenu::Draw();
}

// ea: 0x007383C0 (mNumPlayers at +0x4110)
bool MPPlayerManager::onSessionConnectRequest(
    bdReference<bdBitBuffer> requestUserData, bdBitBuffer* replyUserData)
{
    (void)replyUserData;
    bool v3 = *(unsigned char*)((char*)this + 0x4110)
              < MPUIInterface::mServerParams.mMaxPlayers;
    if (requestUserData.m_ptr != nullptr
        && requestUserData.m_ptr->m_refCount-- == 1)
        delete requestUserData.m_ptr;
    return v3;
}

// ea: 0x0073A9F0
void MPPlayerManager::HandleRequestPlayerState(
    const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr)
            Player->mConnection.m_ptr->getStatus();
    }
}

// ea: 0x00761510
::MPEntityHandle MultiplayerMgr::FindDroppedItemID(
    EDroppedItemTypes itemType, Entity* item, Entity* owner)
{
    ::MPEntityHandle result;
    if (mPeer != nullptr)
    {
        result = ((MPPlayerManager*)((char*)mPeer + 0x74E0))
                     ->FindDroppedItemID(itemType, item, owner);
        return result;
    }
    else
    {
        result.mValue = 0;
        return result;
    }
}

// ea: 0x00764370
::MPEntityHandle MultiplayerMgr::RegisterDroppedItem(
    EDroppedItemTypes itemType, Entity* item, Entity* owner)
{
    ::MPEntityHandle result;
    if (mPeer != nullptr)
    {
        result = ((MPPlayerManager*)((char*)mPeer + 0x74E0))
                     ->RegisterDroppedItem(itemType, item, owner);
        return result;
    }
    else
    {
        result.mValue = 0;
        return result;
    }
}

// ea: 0x00734D50
unsigned int kuju::knetuser::cVoiceNetworkManager::seqIDInList(
    unsigned long seqID, unsigned long listIndex)
{
    for (sVoicePacket* i = mPendingVoicePacketList[listIndex];
         i != nullptr; i = i->mNext)
    {
        unsigned int mSeqID = i->mSeqID;
        if (seqID < mSeqID)
            break;
        if (seqID == mSeqID)
            return 1;
    }
    return 0;
}

// ea: 0x00750170
bool MultiplayerMgr::IsPlayerTalking(Entity* player)
{
    MPPeer* mPeer = this->mPeer;
    if (this->mPeer == nullptr)
        return true;
    MPPlayer* v4 =
        ((MPPlayerManager*)((char*)mPeer + 0x74E0))->GetPlayer(player);
    return v4 != nullptr && mPeer->IsPlayerTalking(v4, 0);
}

// ea: 0x007341E0
void MPProfileMainMenu::OnUp(int c)
{
    (void)c;
    if (highlighted != 0)
    {
        Up();
    }
    else
    {
        highlighted = 5;
        SetHigh(5, true);
    }
    OnSelectionChange();
}

// ea: 0x0072E4D0
void MPVehicle::SetGunnerState(int state)
{
    Entity* mEntity = (Entity*)this->mEntity;
    this->mGunnerState = state;
    if (mEntity != nullptr)
    {
        mEntity->scr_vehicle->seats[1].firing = state == 1;
        mEntity->scr_vehicle->seats[1].overheating = state == 2;
    }
}

// ea: 0x007338D0
void MPProfileEditMenu::UpdateWidescreen(bool widescreen)
{
    if (mWidescreen != widescreen)
    {
        if (mInstructionsText != nullptr)
            mInstructionsText->UpdateForWidescreen(widescreen);
        if (panel != nullptr)
        {
            mWidescreen = widescreen;
            panel->UpdateWidescreen(widescreen, 320.0f);
        }
    }
}

// ea: 0x00733350
void MPOptionsPreferencesMenu::UpdateWidescreen(bool widescreen)
{
    if (mWidescreen != widescreen)
    {
        if (mInstructionsText != nullptr)
            mInstructionsText->UpdateForWidescreen(widescreen);
        if (panel != nullptr)
        {
            mWidescreen = widescreen;
            panel->UpdateWidescreen(widescreen, 320.0f);
        }
    }
}

// ea: 0x0072C790
bool MultiplayerMgr::getValidNetConnection(const char* address)
{
    if (address == nullptr || *address == 0)
        return 0;
    unsigned int v2 = HashString::CalcHash(address);
    unsigned int* v3 = ValidAddress;
    while (*v3 != v2)
    {
        if (++v3 > (unsigned int*)&g_networkOwner)
            return 0;
    }
    return 1;
}

// ea: 0x0074EFF0
kuju::kvoicemanager::cVoiceManager::cVoiceManager()
{
    mInitialised = 0;
    mRemoteListeners.mBitPlayers = 0;
    mConnectedPlayers.mBitPlayers = 0;
    mLastNetworkDispatchTime.mTime = 0;
    mRealLastNetworkDispatchTime.mTime = 0;
    mPlaybackDataAvailableStartTime.mTime = 0;
}

// ea: 0x00765FC0
sGameListing* MPUIInterface::GameListingGet(unsigned long& numGames)
{
    if (mGameConnectionType == kGameConnectionTypeLocal)
        return nullptr;
    if (mGameConnectionType != kGameConnectionTypeLan)
        BlockUntilNetReady();
    numGames = mGameListingNumGames;
    return (sGameListing*)mGameListings;
}

// ea: 0x0073E3C0
void MPOptionsControlsMenu::OnCross(int c)
{
    (void)c;
    if (SaveOptions())
        ProfileEditMenu::Me()->mNeedWrite = true;
    if (highlighted != 0)
    {
        if (highlighted == 1)
            this->system->gap1C(this->system, 25);
    }
    else
    {
        this->system->gap1C(this->system, 24);
    }
}

// ea: 0x007397A0 (mVehicleEvents at +0x5970)
void MPPlayerManager::VehicleEventProcessAllEvents()
{
    for (int i = 0; i < 32; ++i)
    {
        MPVehicleEvent* v = &((MPVehicleEvent*)((char*)this + 0x5970))[i];
        if (v->bUsed && VehicleEventProcessEvent(v))
            i = -1;
    }
}

// ea: 0x0073E370
void MPOptionsControlsMenu::Select(int entry_num)
{
    if (SaveOptions())
        ProfileEditMenu::Me()->mNeedWrite = true;
    if (entry_num != 0)
    {
        if (entry_num == 1)
            this->system->gap1C(this->system, 25);
    }
    else
    {
        this->system->gap1C(this->system, 24);
    }
}

// ea: 0x0075D3C0
short MPPlayerItems::FindFreeSlot(EDroppedItemTypes item)
{
    switch (item)
    {
    case (EDroppedItemTypes)2:  // kItemTypeSupport
        return FindOldestItem(mDroppedSupport);
    case kItemTypeMines:
        return FindOldestItem(mDroppedMines);
    case (EDroppedItemTypes)3:  // kItemTypeMax
        return FindOldestItem(mDroppedKits);
    default:
        return FindOldestItem(mDroppedWeapons);
    }
}

// ea: 0x00740210
const char* MultiplayerMgr::GetPlayerName(int clientIndex) const
{
    const Entity* Player = nullptr;
    if (mPeer != nullptr)
        Player = EntityManager::sInst->GetPlayer(clientIndex);
    MPPlayer* v5 = nullptr;
    if (mPeer != nullptr && Player != nullptr)
        v5 = ((MPPlayerManager*)((char*)mPeer + 0x74E0))->GetPlayer(Player);
    if (v5 != nullptr)
        return (const char*)((char*)v5 + 0x68);  // mName
    return defaultFileName;
}

// ea: 0x007383F0
void MPPlayerManager::RemovePlayerFromSession(MPPlayer* player)
{
    if (player != nullptr
        && MPUIInterface::mGameConnectionType == kGameConnectionTypeOnline)
    {
        XUID remotePlayer = *(XUID*)((char*)player + 0x88);
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        Handle->RemoveRemotePlayer(remotePlayer);
    }
}

// ea: 0x0074ED40 (accepting_input_from_controller at +0x1D)
void MPUIInterface::ExitFrontend(int returnMenu)
{
    mReturnMenu = returnMenu;
    *(int*)((char*)controller::inst() + 0x1D) = 0x01010101;
    LoadMap(mServerParams.mMapID, false, false);
    MPLiveEngine* Handle = MPLiveEngine::GetHandle();
    if (Handle != nullptr)
        Handle->invited = false;
}

// ea: 0x0072DF50 (mNetPosition +0xA0, mInterpolatedPosition +0x1F0)
void MPPlayer::SetPosition(const float* position)
{
    math::Position3* mNetPosition =
        (math::Position3*)((char*)this + 0xA0);
    math::Position3* mInterpolatedPosition =
        (math::Position3*)((char*)this + 0x1F0);
    mNetPosition->v.m128_f32[0] = position[0];
    mNetPosition->v.m128_f32[1] = position[1];
    mNetPosition->v.m128_f32[2] = position[2];
    mInterpolatedPosition->v.m128_f32[0] = mNetPosition->v.m128_f32[0];
    mInterpolatedPosition->v.m128_f32[1] = mNetPosition->v.m128_f32[1];
    mInterpolatedPosition->v.m128_f32[2] = mNetPosition->v.m128_f32[2];
    mInterpolatedPosition->v.m128_f32[3] = mNetPosition->v.m128_f32[3];
}

// ea: 0x00750B00
void MultiplayerMgr::SendRespawnRequest(unsigned int clientID)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
    {
        mPeer->SendRespawnRequest(clientID);
    }
    else if (gpBrocAPI->mBrocExports.mCallbackPlayerRespawnRequest != nullptr)
    {
        Entity* Player = EntityManager::sInst->GetPlayer(currCl);
        gpBrocAPI->mBrocExports.mCallbackPlayerRespawnRequest(
            Player->mHandle.mHandle.mVal, 1);
    }
}

// ea: 0x00761460
void MultiplayerMgr::BroadcastVehicleRespawn(Entity* vehicle)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1495;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->BroadcastVehicleRespawn(vehicle);
}

// ea: 0x00750630
void MultiplayerMgr::NextRound(bool allowChange)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1334;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->NextRound(allowChange);
}

// ea: 0x00750720
void MultiplayerMgr::SendInitialGameState(Entity* player)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1479;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendInitialGameState(player);
}

// ea: 0x00761400
void MultiplayerMgr::SendVehicleStates(Entity* player)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1487;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendVehicleStates(player);
}

// ea: 0x00734850
EPlayerClass PlayerStats::GetStatSpecificToAPlayerClass(int stat)
{
    if (stat > 0x1C)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ePlayerStats.cpp";
        AeAssert::gCurrentLine = 62;
        AeAssert::gCurrentExpr =
            "stat >= kPlayerStatsMin && stat <= kPlayerStatsMax";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid stat index."))
            __debugbreak();
    }
    return (EPlayerClass)playerStatsInfo[stat].mSpecificToPlayerClass;
}

// ea: 0x0072C8D0
bool MPPeer::IsPlayerTalking(MPPlayer* player, int local_controller)
{
    if (LiveWrapper::theWrapper != nullptr
        && LiveWrapper::theWrapper->sessionState == kInSession)
        return LiveWrapper::theWrapper->IsTalking(
            *(XUID*)((char*)player + 0x88), (unsigned int)local_controller);
    else
        return *(int*)((char*)player + 0xDC) + 250 > level.time;
}

// ea: 0x00750F50
void MultiplayerMgr::SendBombOperation(const Entity* player, bool defusing)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1841;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendBombOperation(player, defusing);
}

// ea: 0x00750820
void MultiplayerMgr::SendGameScore(int alliesScore, int axisScore)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1513;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendGameScore(alliesScore, axisScore);
}

// ea: 0x00750EE0
void MultiplayerMgr::SendHostBombRequest(const Entity* player, bool defusing)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1832;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendHostBombRequest(player, defusing);
}

// ea: 0x00730260
MPPlayerSet::MPPlayerSet(eDefaultSets e)
{
    if (e != eEveryone)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerSet.cpp";
        AeAssert::gCurrentLine = 41;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("OUCHA: UNknown enum!"))
            __debugbreak();
        return;
    }
    else
    {
        mBitPlayers = 0xFFFF;
    }
}

// ============================================================================
// Batch 14: game-state forwarders, menu up/down, item/qos/voice helpers
// ============================================================================

// ea: 0x0072F830
const char* MPUIInterface::GetMapString(unsigned long mapIndex)
{
    if (!MI_IsAvailableMap(mapIndex))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPUIInterface.cpp";
        AeAssert::gCurrentLine = 1335;
        AeAssert::gCurrentExpr = "MI_IsAvailableMap(mapIndex)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return MI_GetMapDisplayName(mapIndex);
}

// ea: 0x00735DE0 (mDisconnectStatusTimerRunning at +0x74A8 etc.)
void MPPeer::onSessionDisconnect(bdReference<bdConnection> connection)
{
    *(bool*)((char*)this + 0x74A8) = true;   // mDisconnectStatusTimerRunning
    *(float*)((char*)this + 0x74AC) = 0.0f;  // mDisconnectStatusTimer
    *(float*)((char*)this + 0x74B0) = 0.0f;  // mDisconnectNotReadyTime
    *(float*)((char*)this + 0x74B4) = 0.0f;  // mBadHashTime
    if (connection.m_ptr != nullptr
        && connection.m_ptr->m_refCount-- == 1)
        delete connection.m_ptr;
}

// ea: 0x00750C30
void MultiplayerMgr::AreaCaptured(int netIndex, int itemType, int hostOnly)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1723;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->AreaCaptured(netIndex, itemType, hostOnly);
}

// ea: 0x00750FC0
void MultiplayerMgr::SendBombOperationEvent(const Entity* player,
                                            bool defusing, bool success)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1850;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendBombOperationEvent(player, defusing, success);
}

// ea: 0x00735250
void kuju::knetuser::cVoiceNetworkManager::
    checkForPendingPacketsAwaitingHandling(const kuju::knet::sTime& time)
{
    int mTime = mVoiceLifeTime.mTime;
    unsigned int v4 = 0;
    do
    {
        sVoicePacket* v6 = mPendingVoicePacketList[v4];
        if (v6 != nullptr)
        {
            int v7 = time.mTime - v6->mTimeReceived.mTime;
            unsigned int mPrevSeqID = v6->mPrevSeqID;
            if (mPrevSeqID != 0xFFFFFFFFu
                && mPrevSeqID != (unsigned int)mPendingVoicePacketList[v4 + 16])
            {
                if (v7 > mTime)
                {
                    ++this->mMissedPackets;
                    flushFirstPacketInList(v4, 0);
                    mTime = mVoiceLifeTime.mTime;
                }
                ++v4;
                continue;
            }
            if (v7 > mTime)
                ++this->mMissedPackets;
            flushFirstPacketInList(v4, 0);
            mTime = mVoiceLifeTime.mTime;
        }
        ++v4;
    } while (v4 < 0x10);
}

// ea: 0x007504B0
void MultiplayerMgr::PlayerRespawn(Entity* player,
                                   const math::Position3& position,
                                   const math::Dir3& angles, int team)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
    {
        mPeer->PlayerRespawn(player, position, angles, team);
    }
    else
    {
        ClientSpawn(player, position.v.m128_f32, angles.v.m128_f32, true,
                    false);
        if (gpBrocAPI->mBrocExports.mCallbackPlayerSpawn != nullptr)
        {
            Entity* v6 = EntityManager::sInst->GetPlayer(currCl);
            gpBrocAPI->mBrocExports.mCallbackPlayerSpawn(
                v6->mHandle.mHandle.mVal, 0);
        }
    }
}

// ea: 0x007347E0
bool PlayerStats::IsStatSpecificToAPlayerClass(int stat)
{
    if (stat > 0x1C)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ePlayerStats.cpp";
        AeAssert::gCurrentLine = 53;
        AeAssert::gCurrentExpr =
            "stat >= kPlayerStatsMin && stat <= kPlayerStatsMax";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid stat index."))
            __debugbreak();
    }
    return playerStatsInfo[stat].mSpecificToPlayerClass != -1;
}

// ea: 0x0072DD70
int MPPlayer::GroundSurfaceType(int surfaceFlags)
{
    if ((surfaceFlags & 0x2000) != 0)
        return 0;
    if (((surfaceFlags >> 20) & 0x1Fu) >= 0x17)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayer.cpp";
        AeAssert::gCurrentLine = 1563;
        AeAssert::gCurrentExpr = "iSurfType >= 0 && iSurfType < 23";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return (surfaceFlags >> 20) & 0x1F;
}

// ea: 0x0072D070 (anonymous namespace helper; hash cannot be reproduced)
namespace {
void PlayPartialAnimationRate(DObj* dobj, MP_ANIM_INDEX* anim_index,
                              unsigned int mask, float alpha,
                              float speedScale)
{
    if (dobj != nullptr)
    {
        AnimationPlayer* v5 = (AnimationPlayer*)dobj->animPlayers[0];
        if (v5 != nullptr && anim_index != nullptr)
        {
            nalGeneric::nalGenericAnim* anim = anim_index->anim;
            if (anim != nullptr)
            {
                v5->PlayModifier(anim, 1.0f, mask);
                v5->SetModifierAlpha(mask, 1.0f, anim, alpha);
                v5->SetModifierSpeed(mask, 1.0f, anim, speedScale);
            }
        }
    }
}
}  // namespace

// ea: 0x00750990
void MultiplayerMgr::SendGameStateSCF(Entity* player, int currentFlagIndex,
                                      const math::Position3& flagPosition,
                                      const math::Dir3& flagAngles,
                                      Entity* flagHolder)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1537;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendGameStateSCF(player, currentFlagIndex, flagPosition,
                            flagAngles, flagHolder);
}

// ea: 0x00735B80 (m_QosAddr +0x10, m_QosLatency +0x2450, etc.)
void MPPeer::UpdateQosProbe(bdReference<bdCommonAddr> addr, bool bSuccess,
                            float latency)
{
    int v4 = 0;
    char* m_QosAddr = (char*)this + 0x10;
    float* m_QosLatency = (float*)((char*)this + 0x2450);
    do
    {
        if (((bdQoSRemoteAddr*)(m_QosAddr + 0x1C * v4))
                ->m_addr.m_ptr == addr.m_ptr)
        {
            *(bool*)((char*)this + 0x690 + v4) = true;   // m_QosIsComplete
            *(bool*)((char*)this + 0xA90 + v4) = bSuccess;  // m_QosIsSuccessful
            m_QosLatency[v4] = latency;
            if (*(bool*)((char*)this + 0x2490 + v4))  // m_QosDeleteProbe
                *(bool*)((char*)this + 0xE90 + v4) = true;  // m_QosIsAvailable
        }
        ++v4;
    } while (v4 < 800);
    if (addr.m_ptr != nullptr && addr.m_ptr->m_refCount-- == 1)
        delete addr.m_ptr;
}

// ea: 0x00750A00
void MultiplayerMgr::SendGameStateDOM(Entity* player, int flag0, int flag1,
                                      int flag2, int flag3, int flag4)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1545;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendGameStateDOM(player, flag0, flag1, flag2, flag3, flag4);
}

// ea: 0x00750910
void MultiplayerMgr::SendGameStateCTF(
    Entity* player, const math::Position3& allied_flag,
    const math::Dir3& alliedAngles, Entity* allied_flag_holder,
    const math::Position3& axis_flag, const math::Dir3& axisAngles,
    Entity* axis_flag_holder)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1529;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendGameStateCTF(player, allied_flag, alliedAngles,
                            allied_flag_holder, axis_flag, axisAngles,
                            axis_flag_holder);
}

// ea: 0x00750890
void MultiplayerMgr::SendGameStateHQ(Entity* player, unsigned int stage,
                                     const math::Position3& pA,
                                     const math::Position3& pB,
                                     unsigned int triggerIndex,
                                     bool alliesDefending,
                                     bool pointAIsHQ)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1521;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendGameStateHQ(player, stage, pA, pB, triggerIndex,
                           alliesDefending, pointAIsHQ);
}

// ea: 0x00750A80
void MultiplayerMgr::SendGameStateSD(Entity* player, Entity* planter,
                                     Entity* defuser, bool planting,
                                     const math::Position3& bombPosition,
                                     const math::Dir3& bombAngles,
                                     int bombTimeLeft)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1553;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    mPeer->SendGameStateSD(player, planter, defuser, planting, bombPosition,
                           bombAngles, bombTimeLeft);
}

// ea: 0x00740310
bool MultiplayerMgr::IsInVehicle(Entity* player)
{
    if (mPeer == nullptr)
        return false;
    MPPlayer* v3 =
        ((MPPlayerManager*)((char*)mPeer + 0x74E0))->GetPlayer(player);
    return v3 != nullptr && v3->mInVehicle;
}

// ea: 0x0072CBA0
MP_ANIM_INDEX* MPPlayer::getAnimIndex(int sheet, int row, int col,
                                      bool useDefault)
{
    MP_ANIM_INDEX* result = nullptr;
    if (base_anim_indices[38 * sheet + row].anims[col].animIndex != 0)
    {
        result =
            &base_anim_names[base_anim_indices[38 * sheet + row].anims[col]
                                 .animIndex];
        if (result->anim == nullptr && useDefault
            && base_anim_indices[row].anims[col].animIndex != 0)
            return &base_anim_names[base_anim_indices[row].anims[col]
                                        .animIndex];
    }
    else if (useDefault && base_anim_indices[row].anims[col].animIndex != 0)
    {
        return &base_anim_names[base_anim_indices[row].anims[col].animIndex];
    }
    return result;
}

// ea: 0x00766060
MultiplayerMgr::~MultiplayerMgr()
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
        delete mPeer;
    this->mPeer = nullptr;
    this->mInitialized = false;
    MPUIInterface::bdNetStop();
    WSACleanup();
    XNetCleanup();
    bdCore_quit();
    if (gDWHeap != nullptr)
        delete gDWHeap;
}

// ea: 0x0075A4F0
void MPPlayerManager::ApplyLocalPhysicsToVehicle(
    Entity* vehicle, const math::Position3& position,
    const math::Dir3& angles, const math::Dir3& velocity)
{
    if (vehicle == nullptr || vehicle->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 7835;
        AeAssert::gCurrentExpr = "vehicle && vehicle->scr_vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid vehicle entity"))
            __debugbreak();
    }
    MPVehicle* v6 = nullptr;
    for (int veh = 0; veh < 10; ++veh)
    {
        MPVehicle* v = (MPVehicle*)((char*)this + 0x4120 + 0x210 * veh);
        if (v->mEntity == vehicle)
        {
            v6 = v;
            break;
        }
    }
    if (v6 != nullptr)
        v6->SetPhysicsInfo(position, angles, velocity);
}

// ea: 0x00736110
void MPPlayer::SetConnection(bdReference<bdConnection> connection)
{
    bdConnection* m_ptr = connection.m_ptr;
    bdReference<bdConnection>* p_mConnection = &this->mConnection;
    if (p_mConnection->m_ptr != nullptr)
    {
        if (p_mConnection->m_ptr->m_refCount-- == 1
            && p_mConnection->m_ptr != nullptr)
            delete p_mConnection->m_ptr;
    }
    p_mConnection->m_ptr = m_ptr;
    if (m_ptr != nullptr)
        ++m_ptr->m_refCount;
    if (m_ptr != nullptr && m_ptr->m_refCount-- == 1)
        delete m_ptr;
}

// ea: 0x0072E8E0
Entity* MPPlayerManager::GetPlayerEntity(unsigned char id)
{
    if (id > 0x10u)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerMgr.cpp";
        AeAssert::gCurrentLine = 406;
        AeAssert::gCurrentExpr = "id >= 0 && id <= 16";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid player id"))
            __debugbreak();
    }
    MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * id);
    if (p->mClientIndex >= 0)
        return EntityManager::sInst->GetPlayer(p->mClientIndex);
    return nullptr;
}

// ea: 0x00740260
int MultiplayerMgr::GetPlayerId(const Entity* player) const
{
    int result = (int)mPeer;
    if (mPeer != nullptr)
    {
        if (player != nullptr)
            return ((MPPlayerManager*)((char*)mPeer + 0x74E0))
                ->GetPlayer(player)->mId;
        else
            return 16;
    }
    return result;
}

// ea: 0x00735E50
MPLanDiscovery::MPLanDiscovery()
{
    mNumResults = 0;
    for (int i = 0; i < 10; ++i)
        mResults[i].m_ptr = nullptr;
    m_lastStatus = 0;  // BD_IDLE
    mDiscoveryClient.registerListener(this);
}

// ea: 0x0073B090
bool MPUtility::ReadNormal(bdReference<bdBitBuffer> buffer, float* normal)
{
    char temp = 0;
    bool v2 = buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE)
              && buffer.m_ptr->readBits(&temp, 8u);
    ByteToDir(temp, normal);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v2;
}

// ea: 0x0075AEB0
void MultiplayerMgr::ApplyLocalPhysicsToVehicle(Entity* vehicle,
                                                const math::Mat43& mat,
                                                const math::Dir3& velocity)
{
    if (mPeer != nullptr)
    {
        float v6[3];
        memcpy(v6, &vehicle->r.currentAngles, sizeof(v6));
        Axis4ToAngles((const float(*)[4])&mat, v6);
        ((MPPlayerManager*)((char*)mPeer + 0x74E0))
            ->ApplyLocalPhysicsToVehicle(vehicle, mat.w,
                                         *(math::Dir3*)v6, velocity);
    }
}

// ea: 0x007339D0
MPProfileMainMenu::~MPProfileMainMenu()
{
    this->mPanel = nullptr;
    if (mHelpBar != nullptr)
        delete mHelpBar;
    this->mHelpBar = nullptr;
    for (int i = 0; i < 6; ++i)
    {
        mem_heap_free(mSaveSlots[i]);
        mSaveSlots[i] = nullptr;
    }
    FEMenu::~FEMenu();
}

// ea: 0x00754C00
MPPlayerItems::MPPlayerItems()
{
    mDroppedWeapons.mElements = nullptr;
    mDroppedWeapons.mCapacity = 0;
    mDroppedWeapons.mSize = 0;
    mDroppedSupport.mElements = nullptr;
    mDroppedSupport.mCapacity = 0;
    mDroppedSupport.mSize = 0;
    mDroppedMines.mElements = nullptr;
    mDroppedMines.mCapacity = 0;
    mDroppedMines.mSize = 0;
    mDroppedKits.mElements = nullptr;
    mDroppedKits.mCapacity = 0;
    mDroppedKits.mSize = 0;
    mDroppedWeapons.resize(1);
    mDroppedSupport.resize(3);
    mDroppedMines.resize(3);
    mDroppedKits.resize(1);
}

// ea: 0x007656C0
MPProfileEditMenu::MPProfileEditMenu(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0)
{
    mNeedWrite = false;
    mWidescreen = false;
    mInstructionsText = nullptr;
    flags = (int16_t)(flags | 0x82);
    mProfileEditText[0] = nullptr;
    mProfileEditText[1] = nullptr;
    mProfileEditText[2] = nullptr;
    mProfileEditText[3] = nullptr;
    default_color_scheme = 19;
}

// ea: 0x00764180
void MPUIInterface::bdNetStart()
{
    bdNetStartParams params;
    params.m_onlineGame = false;
    bdNetImpl* Instance = bdSingleton<bdNetImpl>::getInstance();
    Instance->start(params);
}

// ea: 0x0073BD20
bool MPUtility::ReadEntityHandle(bdReference<bdBitBuffer> buffer,
                                 MPEntityHandle& id)
{
    unsigned short m_ptr = 0;
    int v7 = 0;
    bool v3 = buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE)
              && buffer.m_ptr->readBits(&v7, 0x10u);
    if (v3)
        m_ptr = (unsigned short)v7;
    else
        m_ptr = (unsigned short)(uintptr_t)buffer.m_ptr;
    id.mValue = m_ptr;
    bool v4 = v3;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v4;
}

// ea: 0x0073E9F0
void MPProfileMainMenu::DialogDisplayDeleting()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_DELETE_WARNING", false, false, defaultFileName,
                 true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    int v3 = v2->GetActiveMenu();
    v2->GetLayer(v3 == 0)->triangleResponse = (void (*)(int))j_nullsub_96;
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    int v5 = v4->GetActiveMenu();
    v4->GetLayer(v5 == 0)->Reformat(true, 0);
}

// ea: 0x0073E8D0
void MPProfileMainMenu::DialogDisplayLoading()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_CHECK_WARNING", false, false, defaultFileName,
                 true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    int v3 = v2->GetActiveMenu();
    v2->GetLayer(v3 == 0)->triangleResponse = (void (*)(int))j_nullsub_96;
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    int v5 = v4->GetActiveMenu();
    v4->GetLayer(v5 == 0)->Reformat(true, 0);
}

// ea: 0x0073E960
void MPProfileMainMenu::DialogDisplaySaving()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_SAVE_WARNING", false, false, defaultFileName,
                 true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    int v3 = v2->GetActiveMenu();
    v2->GetLayer(v3 == 0)->triangleResponse = (void (*)(int))j_nullsub_96;
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    int v5 = v4->GetActiveMenu();
    v4->GetLayer(v5 == 0)->Reformat(true, 0);
}

// ea: 0x007357F0 (mLocalPlayerIndex at +0x4111)
int MultiplayerMgr::GetLocalPlayerId(int localPlayer) const
{
    int result = (int)mPeer;
    if (mPeer != nullptr)
    {
        unsigned char v4 = *(unsigned char*)((char*)mPeer + 0x74E0 + 0x4111
                                             + localPlayer);
        MPPlayer* v5 = nullptr;
        if (v4 < 0x10u)
            v5 = (MPPlayer*)((char*)mPeer + 0x74E0 + 0x1010 + 0x310 * v4);
        return v5->mId;
    }
    return result;
}

// ea: 0x00754FE0
void MPPlayerItems::RemoveItem(EDroppedItemTypes item, short id)
{
    ae_vector<sDroppedItem>* p_mDropped;
    switch (item)
    {
    case (EDroppedItemTypes)2:  // kItemTypeSupport
        p_mDropped = &mDroppedSupport;
        break;
    case kItemTypeMines:
        p_mDropped = &mDroppedMines;
        break;
    case (EDroppedItemTypes)3:  // kItemTypeMax
        p_mDropped = &mDroppedKits;
        break;
    default:
        p_mDropped = &mDroppedWeapons;
        break;
    }
    int mSize = p_mDropped->mSize;
    if (mSize <= id)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPPlayerItems.cpp";
        AeAssert::gCurrentLine = 85;
        AeAssert::gCurrentExpr = "size > id";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("RemoveItem: Invalid ID"))
            __debugbreak();
    }
    if (mSize > id)
        (*p_mDropped)[id].Destroy();
}

// ea: 0x007325F0
void MPOptionsGameplayMenu::OnDown(int c)
{
    (void)c;
    Down();
    mGameplayText[2]->SetText(kGameplayOptionStrings[highlighted]);
    const char* STBString = STBManager::sInst->GetSTBString(
        kGameplayInstructionStrings[highlighted]);
    Broc::string v5(STBString);
    mInstructionsText->SetTextBoxNoLocalize(
        v5, mWidescreen ? 390 : 520, -1.5f);
}

// ea: 0x00730C50
void MPOptionsScreenMenu::OnDown(int c)
{
    (void)c;
    Down();
    mScreenText[2]->SetText(kScreenOptionStrings[highlighted]);
    const char* STBString = STBManager::sInst->GetSTBString(
        kScreenInstructionStrings[highlighted]);
    Broc::string v5(STBString);
    mInstructionsText->SetTextBoxNoLocalize(
        v5, mWidescreen ? 390 : 520, -1.5f);
}

// ea: 0x007313B0
void MPOptionsSoundMenu::OnDown(int c)
{
    (void)c;
    Down();
    mSoundText[2]->SetText(kSoundOptionStrings[highlighted]);
    const char* STBString = STBManager::sInst->GetSTBString(
        kSoundInstructionStrings[highlighted]);
    Broc::string v5(STBString);
    mInstructionsText->SetTextBoxNoLocalize(
        v5, mWidescreen ? 390 : 520, -1.5f);
}

// ea: 0x00732560
void MPOptionsGameplayMenu::OnUp(int c)
{
    (void)c;
    Up();
    mGameplayText[2]->SetText(kGameplayOptionStrings[highlighted]);
    const char* STBString = STBManager::sInst->GetSTBString(
        kGameplayInstructionStrings[highlighted]);
    Broc::string v5(STBString);
    mInstructionsText->SetTextBoxNoLocalize(
        v5, mWidescreen ? 390 : 520, -1.5f);
}

// ea: 0x00730BC0
void MPOptionsScreenMenu::OnUp(int c)
{
    (void)c;
    Up();
    mScreenText[2]->SetText(kScreenOptionStrings[highlighted]);
    const char* STBString = STBManager::sInst->GetSTBString(
        kScreenInstructionStrings[highlighted]);
    Broc::string v5(STBString);
    mInstructionsText->SetTextBoxNoLocalize(
        v5, mWidescreen ? 390 : 520, -1.5f);
}

// ea: 0x00731320
void MPOptionsSoundMenu::OnUp(int c)
{
    (void)c;
    Up();
    mSoundText[2]->SetText(kSoundOptionStrings[highlighted]);
    const char* STBString = STBManager::sInst->GetSTBString(
        kSoundInstructionStrings[highlighted]);
    Broc::string v5(STBString);
    mInstructionsText->SetTextBoxNoLocalize(
        v5, mWidescreen ? 390 : 520, -1.5f);
}

// ea: 0x00735F60
void MPLanDiscovery::onDiscovery(bdReference<bdGameInfo> gameInfo)
{
    unsigned int mNumResults = this->mNumResults;
    bdGameInfo* m_ptr = gameInfo.m_ptr;
    if (mNumResults < 0xA)
    {
        bdReference<bdGameInfo>* v5 = &mResults[mNumResults];
        if (v5->m_ptr != nullptr)
        {
            if (v5->m_ptr->m_refCount-- == 1 && v5->m_ptr != nullptr)
                delete v5->m_ptr;
        }
        v5->m_ptr = m_ptr;
        if (m_ptr != nullptr)
            ++m_ptr->m_refCount;
        ++this->mNumResults;
    }
    if (m_ptr != nullptr && m_ptr->m_refCount-- == 1)
        delete m_ptr;
}

// ea: 0x00737B90
bool MPPlayerManager::SendHost(const bdReference<bdMessage> message,
                               bool reliable)
{
    unsigned char Type = message.m_ptr->getType();
    if (reliable)
        tlPrintf("Sending reliable message %d to HOST\n", Type);
    if (message.m_ptr)
        ++message.m_ptr->m_refCount;
    bool v4 = ((bdSession*)((char*)this + 0x4114))->sendHost(message,
                                                              reliable);
    if (message.m_ptr && message.m_ptr->m_refCount-- == 1)
        delete message.m_ptr;
    return v4;
}

// ea: 0x00764210
void MPProfileMainMenu::Select(int entry_num)
{
    int v3 = mMenuStatus[entry_num];
    if (v3 != 0)
    {
        if (v3 == 1)
        {
            DialogDisplayProfileSelected();
        }
        else if (v3 != 2)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/MPProfileMainMenu.cpp";
            AeAssert::gCurrentLine = 278;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Unknown profile menu item: %d", v3))
                __debugbreak();
        }
    }
    else
    {
        int highlighted = this->highlighted;
        *(int*)((char*)VKMenu::Me() + 0x1BC) = highlighted;
        this->system->gap1C(this->system, 18);
    }
}

// ea: 0x00762F20
void MPPlayerManager::HandleDropWeapon(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED
            && *(bool*)((char*)this + 0x4112))
        {
            if (Player->mClientIndex >= 0)
                EntityManager::sInst->GetPlayer(Player->mClientIndex);
            bdReference<bdMessage> msg = receivedMsg.getMessage();
            bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
            if (buffer.m_ptr != nullptr)
                ++buffer.m_ptr->m_refCount;
            MPPlayerItems::DeserializeDropWeapon(buffer);
            if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
                delete buffer.m_ptr;
            if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
                delete msg.m_ptr;
        }
    }
}

// ea: 0x0072E780
void MPVehicle::SeatChange(MPPlayer* player, int newSeatIdx)
{
    if (!player->mInVehicle)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPVehicle.cpp";
        AeAssert::gCurrentLine = 951;
        AeAssert::gCurrentExpr = "player->IsInVehicle()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Player not in vehicle"))
            __debugbreak();
    }
    if (newSeatIdx > 0xA)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPVehicle.cpp";
        AeAssert::gCurrentLine = 952;
        AeAssert::gCurrentExpr =
            "newSeatIdx > VEHPOS_UNKNOWN && newSeatIdx < VEHPOS_MAX";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid seat index"))
            __debugbreak();
    }
    unsigned char mId = this->mId;
    int mVehSeatIdx = player->mVehSeatIdx;
    player->mInVehicle = true;
    player->mVehSeatIdx = (int)newSeatIdx;
    player->mVehicleId = mId;
    *(unsigned char*)((char*)this + 0x08 + mVehSeatIdx) = 16;
    *(unsigned char*)((char*)this + 0x08 + newSeatIdx) = player->mId;
    if (player->mClientIndex >= 0
        && EntityManager::sInst->GetPlayer(player->mClientIndex) != nullptr)
    {
        int mClientIndex = player->mClientIndex;
        if (mClientIndex >= 0)
        {
            Entity* v7 = EntityManager::sInst->GetPlayer(mClientIndex);
            Scr_Vehicle_SeatChange(v7, (int)newSeatIdx);
        }
        else
        {
            Scr_Vehicle_SeatChange(nullptr, (int)newSeatIdx);
        }
    }
}

// ea: 0x007739D60
void MPPlayerManager::HandleVoiceData(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED)
        {
            XUID speaker = *(XUID*)((char*)Player + 0x88);
            bdReference<bdMessage> msg = receivedMsg.getMessage();
            bdReference<bdByteBuffer> unenc = msg.m_ptr->getUnencryptedPayload();
            unsigned char voicedata[10];
            unenc.m_ptr->read(voicedata, 0xAu);
            MPLiveEngine::GetHandle()->SubmitVoiceData(speaker, voicedata, 10u);
            Player->mLastVoiceReceivedTime = level.time;
            if (unenc.m_ptr != nullptr && unenc.m_ptr->m_refCount-- == 1)
                delete unenc.m_ptr;
            if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
                delete msg.m_ptr;
        }
    }
}

// ea: 0x007643F0
MPPeer::~MPPeer()
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    p_mSession->unregisterListener((bdSessionListener*)this);
    kuju::kvoicemanager::cVoiceManager* mpVoiceManager =
        *(kuju::kvoicemanager::cVoiceManager**)((char*)this + 0xD270);
    if (mpVoiceManager->mInitialised != 0)
    {
        mpVoiceManager->mInitialised = 0;
        mpVoiceManager->mVoiceNetworkManager.mVoiceHandlerInterface = nullptr;
    }
    if (mpVoiceManager != nullptr)
        delete mpVoiceManager;
    *(void**)((char*)this + 0xD270) = nullptr;
    ((MPPlayerManager*)((char*)this + 0x74E0))->~MPPlayerManager();
    p_mSession->~bdSession();
    ((MPLanDiscovery*)((char*)this + 0x73D0))->~MPLanDiscovery();
    ((bdDiscoveryServer*)((char*)this + 0x73B4))->~bdDiscoveryServer();
    bdReference<MPGameInfo>* pInfo =
        (bdReference<MPGameInfo>*)((char*)this + 0x73B0);
    if (pInfo->m_ptr != nullptr && pInfo->m_ptr->m_refCount-- == 1)
        delete pInfo->m_ptr;
    for (int i = 0; i < 800; ++i)
    {
        bdQoSRemoteAddr* qa =
            (bdQoSRemoteAddr*)((char*)this + 0x10 + 0x1C * i);
        if (qa->m_addr.m_ptr != nullptr && qa->m_addr.m_ptr->m_refCount-- == 1)
            delete qa->m_addr.m_ptr;
    }
}

// ea: 0x0074DFB0
void MPPlayerManager::HandleVehicleRespawn(
    const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED
            && *(bool*)((char*)this + 0x4112))
        {
            bdReference<bdMessage> msg = receivedMsg.getMessage();
            bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
            unsigned char vehId = 0;
            if (buffer.m_ptr != nullptr)
                ++buffer.m_ptr->m_refCount;
            if (MPUtility::ReadVehicleId(buffer, vehId))
                ((MPVehicle*)((char*)this + 0x4120 + 0x210 * vehId))
                    ->RespawnVehicle();
            if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
                delete buffer.m_ptr;
            if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
                delete msg.m_ptr;
        }
    }
}

// ea: 0x00739A90
void MPPlayerManager::HandleGameScore(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED)
        {
            bdReference<bdMessage> msg = receivedMsg.getMessage();
            bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
            int alliesScore = 0;
            int axisScore = 0;
            int v6 = 0;
            if (buffer.m_ptr->readInt32(alliesScore))
            {
                buffer.m_ptr->readInt32(axisScore);
                v6 = axisScore;
            }
            if (*(bool*)((char*)this + 0x4112))
            {
                if (gpBrocAPI->mBrocExports.mCallbackGameScore != nullptr)
                    gpBrocAPI->mBrocExports.mCallbackGameScore(alliesScore, v6);
            }
            else
            {
                *(int*)((char*)this + 0x5880) = v6;
            }
            if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
                delete buffer.m_ptr;
            if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
                delete msg.m_ptr;
        }
    }
}

// ea: 0x0073B4D0
bool MPUtility::ReadVector(bdReference<bdBitBuffer> buffer, float* vec)
{
    float temp = 0.0f;
    bool ok = buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_FLOAT32_TYPE)
              && buffer.m_ptr->readBits(&temp, 0x20u);
    vec[0] = temp;
    if (ok
        && buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_FLOAT32_TYPE)
        && buffer.m_ptr->readBits(&temp, 0x20u))
        ok = true;
    else
        ok = false;
    vec[1] = temp;
    if (ok
        && buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_FLOAT32_TYPE)
        && buffer.m_ptr->readBits(&temp, 0x20u))
        ok = true;
    else
        ok = false;
    vec[2] = temp;
    bool v4 = ok;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v4;
}

// ea: 0x0072D9A0
int MPPlayer::ChooseFootYawSide(float maxYawForceThreshold)
{
    Entity* Player = mClientIndex >= 0
                         ? EntityManager::sInst->GetPlayer(mClientIndex)
                         : nullptr;
    float leftYaw = AngleNormalize180(
        *(float*)((char*)this + 0x2C8 + 44) - Player->client->ps.legsYaw);
    float rightYaw = AngleNormalize180(
        *(float*)((char*)this + 0x2C8 + 88) - Player->client->ps.legsYaw);
    int mLegsYawing = *(int*)((char*)this + 0x2C8 + 88 + 8);
    if (mLegsYawing != 0 && fabs(rightYaw) > maxYawForceThreshold)
        return true;
    int v6 = *(int*)((char*)this + 0x2C8 + 44 + 8);
    if (v6 == 0 || fabs(leftYaw) <= maxYawForceThreshold)
    {
        float v11 = fabs(leftYaw);
        float v7 = fabs(rightYaw);
        float v12 = v7;
        if (v11 <= v7)
        {
            if (v12 > maxYawForceThreshold)
            {
                *(int*)((char*)this + 0x2C8 + 44 + 8) = 0;
                return true;
            }
            if (v11 <= maxYawForceThreshold)
                return v6 == 0
                       && (mLegsYawing != 0
                           || leftYaw >= 0.0f && rightYaw >= 0.0f);
        }
        else if (v11 <= maxYawForceThreshold)
        {
            if (v12 > maxYawForceThreshold)
            {
                *(int*)((char*)this + 0x2C8 + 44 + 8) = 0;
                return true;
            }
            return v6 == 0
                   && (mLegsYawing != 0
                       || leftYaw >= 0.0f && rightYaw >= 0.0f);
        }
        *(int*)((char*)this + 0x2C8 + 88 + 8) = 0;
        return false;
    }
    return false;
}

// ============================================================================
// SetSeatState / GetSeatState (mp.o)
// ============================================================================
// ea: 0x0072E050
int GetSeatState(scr_vehicle_t* vehicle, int seat)
{
    extern float threshhold;  // 0xE373A0
    extern vehicle_info_t* VEH_GetVehicleInfo(int iIndex);  // g.o
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/MPVehicle.cpp";
        AeAssert::gCurrentLine = 74;
        AeAssert::gCurrentExpr = "vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid vehicle in GetSeatState"))
            __debugbreak();
    }
    if (((char*)vehicle + 0x1F9)[28 * seat])
        return 2;
    if (((char*)vehicle + 0x1FA)[28 * seat])
        return 1;
    if (seat != 0)
        return 0;
    vehicle_info_t* VehicleInfo = VEH_GetVehicleInfo(vehicle->infoIdx);
    if (VehicleInfo->type != 1
        || *(float*)((char*)vehicle + 0x3D8) <= threshhold)  // hornSndLerp
        return 0;
    return 1;
}

// ============================================================================
// Batch 10: 170-260 byte tier (mp.o)
// ============================================================================

// ============================================================================
// MultiplayerMgr (mp.o)
// ============================================================================
// ea: 0x007358E0
void MultiplayerMgr::AddVehicle(Entity* vehicle)
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1731;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    if (mPeer == (MPPeer*)-29920)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 1732;
        AeAssert::gCurrentExpr = "mPeer->GetPlayerManager()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer does not yet have a PlayerManager"))
            __debugbreak();
    }
    ((MPPlayerManager*)((char*)mPeer + 0x74E0))->AddVehicle(vehicle);
}

// ea: 0x0073FFB0
int MultiplayerMgr::GetCurrentPlayerCount()
{
    if (mPeer == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 809;
        AeAssert::gCurrentExpr = "mPeer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    if (mPeer == (MPPeer*)-29920)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MultiplayerMgr.cpp";
        AeAssert::gCurrentLine = 810;
        AeAssert::gCurrentExpr = "mPeer->GetPlayerManager()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Peer has not been created yet"))
            __debugbreak();
    }
    if (mPeer == nullptr || mPeer == (MPPeer*)-29920)
        return 0;
    return ((MPPlayerManager*)((char*)mPeer + 0x74E0))
        ->GetCurrentPlayerCount();
}

// ea: 0x007503F0
void MultiplayerMgr::PlayerDead(Entity* player, Entity* inflictor,
                                Entity* attacker, int damage,
                                int meansOfDeath, int weapon,
                                const float* position, const float* dir,
                                EHitLocation hitLoc)
{
    MPPeer* mPeer = this->mPeer;
    if (mPeer != nullptr)
    {
        mPeer->PlayerDead(player, inflictor, attacker, damage, meansOfDeath,
                          weapon, position, dir, hitLoc);
    }
    else
    {
        ::PlayerDead(player, inflictor, attacker, damage, meansOfDeath,
                     weapon, position, dir, hitLoc);
        unsigned int mVal = 0;
        if (gpBrocAPI->mBrocExports.mCallbackPlayerKilled != nullptr)
        {
            unsigned int hitLoca =
                attacker != nullptr ? attacker->mHandle.mHandle.mVal : 0;
            if (inflictor != nullptr)
                mVal = inflictor->mHandle.mHandle.mVal;
            (gpBrocAPI->mBrocExports.mCallbackPlayerKilled)(
                player->mHandle.mHandle.mVal, mVal, hitLoca, weapon,
                meansOfDeath, player->health);
        }
    }
}

// ============================================================================
// MPPeer (mp.o)
// ============================================================================
// ea: 0x00764620 (timers at +0xD284..+0xD298)
bool MPPeer::ConnectToPeers(const bdReference<MPGameInfo>& gameInfo,
                            int nGameIndex, EGameConnectionType gameState)
{
    (void)nGameIndex; (void)gameState;
    *(bool*)((char*)this + 0xD284) = false;
    *(float*)((char*)this + 0xD288) = 0.0f;
    *(float*)((char*)this + 0xD28C) = 0.0f;
    *(bool*)((char*)this + 0xD290) = false;
    *(float*)((char*)this + 0xD294) = 0.0f;
    *(float*)((char*)this + 0xD298) = 0.0f;
    bool result = ((bdSession*)((char*)this + 0x7448))->getStatus()
                  == bdSession::BD_SESSION_NOT_CONNECTED;
    if (result)
    {
        MPGameInfo* m_ptr = gameInfo.m_ptr;
        const XNKEY& SecurityKey = m_ptr->getSecurityKey();
        const XNKID& SecurityID = m_ptr->getSecurityID();
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        Handle->RegisterKey(&SecurityID, &SecurityKey);
        MPGameInfo* v8 = gameInfo.m_ptr;
        const XNKEY& v9 = v8->getSecurityKey();
        MPGameInfo* v10 = gameInfo.m_ptr;
        const XNKID& v11 = v10->getSecurityID();
        bdReference<bdCommonAddr> hostAddr = v10->getHostAddr();
        ((bdSession*)((char*)this + 0x7448))->join(hostAddr, v11, v9,
                                                   nullptr);
        ConnectToPeersFinalize(gameInfo);
        return true;
    }
    return result;
}

// ea: 0x00761910 (mCurrentlyInSession +0xD278, mLastSessionReadyTime +0xD280,
// mSessionStatusTimerRunning +0xD284, mSessionStatusTimer +0xD288,
// mSessionNotReadyTime +0xD28C, mBadHashTime +0xD29C,
// mSessionNotReadyTimeout +0xD27C, mCurrentGameInfo +0x73B0)
void MPPeer::onSessionConnectSuccess()
{
    *(bool*)((char*)this + 0xD278) = 1;
    *(kuju::knet::sTime*)((char*)this + 0xD280) =
        MultiplayerMgr::sInst->getLocalTime();
    int v2 = rand();
    *(bool*)((char*)this + 0xD284) = 1;
    *(float*)((char*)this + 0xD288) = 0.0f;
    *(float*)((char*)this + 0xD28C) = 0.0f;
    *(float*)((char*)this + 0xD29C) = 0.0f;
    *(int*)((char*)this + 0xD27C) = (v2 >> 8) % 10 + 25;
    bdNetImpl* Instance = bdSingleton<bdNetImpl>::getInstance();
    bdConnectionStore* ConnectionStore = Instance->getConnectionStore();
    bdSocketRouter* SocketRouter = ConnectionStore->getSocketRouter();
    bdQoSProbe* QoSProber = SocketRouter->getQoSProber();
    unsigned int v9;
    unsigned char* v8;
    if (((bdSession*)((char*)this + 0x7448))->getRole() != 0)
    {
        v9 = 0;
        v8 = nullptr;
    }
    else
    {
        v9 = 5;
        v8 = (unsigned char*)"shit";
    }
    bdReference<MPGameInfo>* pInfo =
        (bdReference<MPGameInfo>*)((char*)this + 0x73B0);
    QoSProber->listen(pInfo->m_ptr->getSecurityID(), v8, v9);
    QoSProber->enableListener();
}

// ea: 0x0072C940
void MPPeer::onSessionConnectFail()
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    bdSession::bdSessionStatus Status = p_mSession->getStatus();
    p_mSession->leave();
    *(bool*)((char*)this + 0xD278) = false;
    int v4 = 8;   // JOIN_FAILED
    if (Status == bdSession::BD_SESSION_CONNECTING_TO_HOST_FAILED)
        v4 = 10;  // CANNOT_CONNECT_TO_PEERS
    else if (Status == bdSession::BD_SESSION_CONNECTING_TO_PEERS_FAILED)
        v4 = 11;  // JOIN_SUCCESS
    OverlayMenu* v5 = OverlayMenu::Me(0);
    if (v5 != nullptr)
    {
        v5->SetState(v4);
        void* activeMenu = v5->GetActiveMenu();
        if (*(int*)((char*)activeMenu + 0x14) == -1)
        {
            *(int*)((char*)v5 + 0x50) = (int)v5->GetAcceptMenu();
            *(int*)((char*)v5 + 0x54) = (int)v5->GetBackMenu();
        }
        else
        {
            *(int*)((char*)v5 + 0x50) = *(int*)((char*)activeMenu + 0x14);
            void* activeMenu2 = v5->GetActiveMenu();
            *(int*)((char*)v5 + 0x54) = *(int*)((char*)activeMenu2 + 0x14);
        }
    }
}

// ea: 0x00735AB0 (mSession at +0x7448, mCurrentlyInSession +0xD278)
void MPPeer::onSessionJoinRefused(bdReference<bdBitBuffer> userData)
{
    ((bdSession*)((char*)this + 0x7448))->leave();
    *(bool*)((char*)this + 0xD278) = false;
    OverlayMenu* v3 = OverlayMenu::Me(0);
    if (v3 != nullptr)
    {
        v3->SetState(9);  // JOIN_REFUSED
        void* activeMenu = v3->GetActiveMenu();
        if (*(int*)((char*)activeMenu + 0x14) == -1)
        {
            *(int*)((char*)v3 + 0x50) = (int)v3->GetAcceptMenu();
            *(int*)((char*)v3 + 0x54) = (int)v3->GetBackMenu();
        }
        else
        {
            *(int*)((char*)v3 + 0x50) = *(int*)((char*)activeMenu + 0x14);
            void* activeMenu2 = v3->GetActiveMenu();
            *(int*)((char*)v3 + 0x54) = *(int*)((char*)activeMenu2 + 0x14);
        }
    }
    if (userData.m_ptr != nullptr && userData.m_ptr->m_refCount-- == 1)
        delete userData.m_ptr;
}

// ============================================================================
// MPPlayerManager (mp.o)
// ============================================================================
// ea: 0x00737770 (compare mConnection.m_ptr against each player's)
MPPlayer* MPPlayerManager::GetPlayer(
    bdReference<bdConnection> connection)
{
    for (int i = 0; i < 16; ++i)
    {
        MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * i);
        if (p->mConnection.m_ptr == connection.m_ptr)
            return p;
    }
    return nullptr;
}

// ea: 0x00737930 (mSession at +0x4114; mPlayerUpdateQueued +0x34)
void MPPlayerManager::SendAll(bdReference<bdMessage> message,
                              bool reliable, bool forceSend)
{
    bdSession* session = *(bdSession**)((char*)this + 0x4114);
    if (session != nullptr)
    {
        if (reliable && forceSend)
            *(bool*)((char*)MultiplayerMgr::sInst + 0x34) = true;
        if (message.m_ptr != nullptr)
            ++message.m_ptr->m_refCount;
        if (!session->send(message, reliable))
            printf("SEND ALL failed\n");
        if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
            delete message.m_ptr;
    }
    else
    {
        tlPrintf("MPPlayerManager: Cannot send not have a session to send to.\n");
        if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
            delete message.m_ptr;
    }
}

// ea: 0x0073A700
void MPPlayerManager::Send(const bdReference<bdMessage>& message,
                           MPPlayerSet players, bool reliable)
{
    unsigned int peerIDs[16];
    memset(peerIDs, 0, sizeof(peerIDs));
    int v4 = 0;
    for (unsigned int v5 = 0; v5 < 16; ++v5)
    {
        MPPlayer* player = (MPPlayer*)((char*)this + 0x1010 + 0x310 * v5);
        bdConnection* m_ptr = player->mConnection.m_ptr;
        if ((players.mBitPlayers & (1u << v5)) != 0 && m_ptr != nullptr)
        {
            unsigned int peerID;
            bdReference<bdConnection> conn;
            conn.m_ptr = m_ptr;
            ++m_ptr->m_refCount;
            bdSession* session = *(bdSession**)((char*)this + 0x4114);
            if (session->getPeerIndex(conn, peerID))
            {
                int v9 = 0;
                while (v9 < v4 && peerID != peerIDs[v9])
                    ++v9;
                if (v9 >= v4)
                {
                    bdReference<bdMessage> msg2;
                    msg2.m_ptr = message.m_ptr;
                    if (message.m_ptr != nullptr)
                        ++message.m_ptr->m_refCount;
                    SendPlayer(player, msg2, reliable);
                    peerIDs[v4++] = peerID;
                }
            }
            if (m_ptr != nullptr && m_ptr->m_refCount-- == 1)
                delete m_ptr;
        }
    }
    if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
        delete message.m_ptr;
}

// ea: 0x0073A4B0
MPPlayerSet MPPlayerManager::allPlayers()
{
    MPPlayerSet result;
    result.mBitPlayers = 0;
    for (int v3 = 0; v3 < 16; ++v3)
    {
        MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * v3);
        if (p->mConnection.m_ptr != nullptr
            && p->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED)
        {
            if (v3 >= 0x10)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\mp\\MPPlayerSet.h";
                AeAssert::gCurrentLine = 110;
                AeAssert::gCurrentExpr = "index < 16";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
            }
            result.mBitPlayers =
                (unsigned short)(result.mBitPlayers | (1u << v3));
        }
    }
    if (result.mBitPlayers >= 0x10000)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp\\MPPlayerSet.h";
        AeAssert::gCurrentLine = 82;
        AeAssert::gCurrentExpr = "set.mBitPlayers < (1<<16)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return result;
}

// ea: 0x0073A590 (mLocalPlayerIndex[0] at +0x4111)
MPPlayerSet MPPlayerManager::allPlayersButMe(int localPlayer)
{
    (void)localPlayer;
    MPPlayerSet result;
    result.mBitPlayers = 0;
    unsigned char localIdx = *(unsigned char*)((char*)this + 0x4111);
    for (int v4 = 0; v4 < 16; ++v4)
    {
        MPPlayer* p = (MPPlayer*)((char*)this + 0x1010 + 0x310 * v4);
        if (v4 != localIdx && p->mConnection.m_ptr != nullptr
            && p->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED)
            result.addPlayer((unsigned long)v4);
    }
    if (result.mBitPlayers >= 0x10000)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp\\MPPlayerSet.h";
        AeAssert::gCurrentLine = 82;
        AeAssert::gCurrentExpr = "set.mBitPlayers < (1<<16)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return result;
}

// ea: 0x00738B90 (reads session ID from message payload)
void MPPlayerManager::HandleSessionID(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdMessage> msg = receivedMsg.getMessage();
    bdReference<bdBitBuffer> payload = msg.m_ptr->getPayload();
    XNKID newSessionID;
    if (payload.m_ptr->readDataType(bdBitBuffer::BD_BB_FULL_TYPE))
    {
        if (payload.m_ptr->readBits(&newSessionID, 0x40u))
            LiveWrapper::theWrapper->SetSessionID(newSessionID);
    }
    if (payload.m_ptr != nullptr && payload.m_ptr->m_refCount-- == 1)
        delete payload.m_ptr;
    if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
        delete msg.m_ptr;
}

// ea: 0x00738DD0 (reads joinable flag, sets live notification)
void MPPlayerManager::HandleJoinableFlag(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdMessage> v8 = receivedMsg.getMessage();
    bdReference<bdBitBuffer> v9 = v8.m_ptr->getPayload();
    bool sessionJoinable = false;
    if (v9.m_ptr->readDataType(bdBitBuffer::BD_BB_BOOL_TYPE))
    {
        unsigned char byte = 0;
        if (v9.m_ptr->readBits(&byte, 1u))
            sessionJoinable = byte != 0;
    }
    if (v9.m_ptr != nullptr && v9.m_ptr->m_refCount-- == 1)
        delete v9.m_ptr;
    if (v8.m_ptr != nullptr && v8.m_ptr->m_refCount-- == 1)
        delete v8.m_ptr;
    MPLiveEngine* Handle = MPLiveEngine::GetHandle();
    ((LiveWrapper*)Handle)->SetNotificationFlag(Handle->actualPort, 0x10u,
                                                sessionJoinable);
}

// ============================================================================
// MPPlayerItems (mp.o)
// ============================================================================
// ea: 0x00755300
short MPPlayerItems::FindOldestItem(const ae_vector<sDroppedItem>& list)
{
    int mSize = list.mSize;
    int v4 = 0;
    unsigned int v5 = (unsigned int)-1;
    int size = mSize;
    short oldest = 0;
    if (mSize <= 0)
        return oldest;
    while (1)
    {
        if (v4 < 0 || v4 >= list.mSize)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 161;
            AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        unsigned int time = list.mElements[v4].time;
        const sDroppedItem* v7 = &list.mElements[v4];
        if (time == 0)
            break;
        unsigned int mVal = v7->handle.mVal;
        unsigned int v9 = v7->handle.mVal & 0xFFF;
        if (v9 >= 0x540
            || mVal >> 12 != EntityHandleDb::sInst.mElements[v9].mKey
            || EntityHandleDb::sInst.mElements[v9].mObject == nullptr)
            break;
        if (time < v5)
        {
            v5 = list.mElements[v4].time;
            oldest = (short)v4;
        }
        if (++v4 >= size)
            return oldest;
    }
    return (short)v4;
}

// ea: 0x00755070
bool MPPlayerItems::FindItemID(EDroppedItemTypes item, Entity* ent,
                               short& id)
{
    ae_vector<sDroppedItem>* list = &mDroppedWeapons;
    switch (item)
    {
    case (EDroppedItemTypes)1:  // kItemTypeSupport
        list = &mDroppedSupport;
        break;
    case kItemTypeMines:
        list = &mDroppedMines;
        break;
    case (EDroppedItemTypes)3:  // kItemTypeMax
        list = &mDroppedKits;
        break;
    default:
        break;
    }
    int v5 = 0;
    int size = list->mSize;
    if (size <= 0)
        return false;
    while (1)
    {
        if (v5 < 0 || v5 >= list->mSize)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 167;
            AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        unsigned int v6 = list->mElements[v5].handle.mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v6 < 0x540
            && list->mElements[v5].handle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v6].mKey)
            mObject = EntityHandleDb::sInst.mElements[v6].mObject;
        if (mObject == ent)
            break;
        if (++v5 >= size)
            return false;
    }
    id = (short)v5;
    return true;
}

// ea: 0x00754F00
void MPPlayerItems::RemoveAll(EDroppedItemTypes item)
{
    ae_vector<sDroppedItem>* list = &mDroppedWeapons;
    switch (item)
    {
    case (EDroppedItemTypes)1:
        list = &mDroppedSupport;
        break;
    case kItemTypeMines:
        list = &mDroppedMines;
        break;
    case (EDroppedItemTypes)3:
        list = &mDroppedKits;
        break;
    default:
        break;
    }
    int v3 = 0;
    int size = list->mSize;
    if (size > 0)
    {
        do
        {
            if (v3 < 0 || v3 >= list->mSize)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
                AeAssert::gCurrentLine = 167;
                AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            sDroppedItem* v4 = &list->mElements[v3];
            unsigned int v5 = v4->handle.mVal & 0xFFF;
            if (v5 < 0x540
                && list->mElements[v3].handle.mVal >> 12
                       == EntityHandleDb::sInst.mElements[v5].mKey)
            {
                Entity* mObject = EntityHandleDb::sInst.mElements[v5].mObject;
                if (mObject != nullptr)
                {
                    mObject->think = THINK__G_FreeEntity;
                    mObject->nextthink = level.time + 1;
                }
            }
            ++v3;
            v4->handle.mVal = 0;
            v4->time = 0;
        } while (v3 < size);
    }
}

// ea: 0x0075D300
short MPPlayerItems::AddItem(EDroppedItemTypes item, Entity* ent)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/MPPlayerItems.cpp";
        AeAssert::gCurrentLine = 127;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "MPPlayerItems::AddItem invalid entity passed in"))
            __debugbreak();
    }
    ae_vector<sDroppedItem>* p_mDropped = &mDroppedWeapons;
    switch (item)
    {
    case (EDroppedItemTypes)2:  // kItemTypeSupport
        p_mDropped = &mDroppedSupport;
        break;
    case kItemTypeMines:
        p_mDropped = &mDroppedMines;
        break;
    case (EDroppedItemTypes)3:  // kItemTypeMax
        p_mDropped = &mDroppedKits;
        break;
    default:
        break;
    }
    short OldestItem = FindOldestItem(*p_mDropped);
    sDroppedItem* v6 = &(*p_mDropped)[OldestItem];
    v6->Destroy();
    (*p_mDropped)[OldestItem].time = level.time;
    (*p_mDropped)[OldestItem].handle.mVal = ent->mHandle.mHandle.mVal;
    return OldestItem;
}

// ============================================================================
// MPUtility (mp.o) - batch 10 writers/readers
// ============================================================================
// ea: 0x0073ACA0
bool MPUtility::ReadPosition(bdReference<bdBitBuffer> buffer,
                             float* position)
{
    extern float gMPFloatPositionMin;  // 0xE370E0
    extern float gMPFloatPositionMax;  // 0xE370DC
    bool ok = buffer.m_ptr->readRangedFloat32(position[0],
                                              gMPFloatPositionMin,
                                              gMPFloatPositionMax, 1.0f);
    bool v2 = false;
    if (ok && buffer.m_ptr->readRangedFloat32(
                  position[1], gMPFloatPositionMin, gMPFloatPositionMax, 1.0f))
    {
        ok = true;
        if (buffer.m_ptr->readRangedFloat32(
                position[2], gMPFloatPositionMin, gMPFloatPositionMax, 1.0f))
            v2 = true;
    }
    ok = v2;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v2;
}

// ea: 0x... (writes 3 float32s with type tags)
void MPUtility::WritePosition(bdReference<bdBitBuffer> buffer,
                              const float* position)
{
    for (int i = 0; i < 3; ++i)
    {
        buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_FLOAT32_TYPE);
        float v = position[i];
        buffer.m_ptr->writeBits(&v, 0x20u);
    }
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

void MPUtility::WritePosition(bdReference<bdBitBuffer> buffer,
                              const math::Position3& position)
{
    WritePosition(buffer, position.v.m128_f32);
}

void MPUtility::WriteAnglesYawPitch(bdReference<bdBitBuffer> buffer,
                                    const float* angles)
{
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE);
    short yaw = (short)(angles[1] * 182.04445f);
    buffer.m_ptr->writeBits(&yaw, 0x10u);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE);
    short pitch = (short)(angles[0] * 182.04445f);
    buffer.m_ptr->writeBits(&pitch, 0x10u);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

void MPUtility::WriteAnglesYawPitch(bdReference<bdBitBuffer> buffer,
                                    const math::Dir3& angles)
{
    WriteAnglesYawPitch(buffer, angles.v.m128_f32);
}

bool MPUtility::ReadAnglesYawPitch(bdReference<bdBitBuffer> buffer,
                                   float* angles)
{
    short v8 = 0;
    bool ok = buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE)
              && buffer.m_ptr->readBits(&v8, 0x10u);
    angles[1] = v8 * 0.0054931641f;
    bool ok2 = buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE)
               && buffer.m_ptr->readBits(&v8, 0x10u);
    angles[0] = v8 * 0.0054931641f;
    bool result = ok && ok2;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return result;
}

// ea: 0x0073B400
void MPUtility::WriteVector(bdReference<bdBitBuffer> buffer,
                            const float* vec)
{
    for (int i = 0; i < 3; ++i)
    {
        buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_FLOAT32_TYPE);
        float v = vec[i];
        buffer.m_ptr->writeBits(&v, 0x20u);
    }
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073B120
void MPUtility::WriteAngles(bdReference<bdBitBuffer> buffer,
                            const float* angles)
{
    for (int i = 0; i < 3; ++i)
    {
        buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE);
        int v = (int)(angles[i] * 182.04445f);
        buffer.m_ptr->writeBits(&v, 0x10u);
    }
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073B1F0
void MPUtility::WriteAngles(bdReference<bdBitBuffer> buffer,
                            const math::Dir3& angles)
{
    for (int i = 0; i < 3; ++i)
    {
        buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER16_TYPE);
        int v = (int)(angles.v.m128_f32[i] * 182.04445f);
        buffer.m_ptr->writeBits(&v, 0x10u);
    }
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ============================================================================
// sServerCreateParams (mp.o)
// ============================================================================
// ea: 0x0072F3B0 (mMapRotation at +0x68 in xlive.h layout)
void sServerCreateParams::SetMapRotation(unsigned char MapRotation)
{
    extern int g_NumTotalMaps;  // 0xF99864
    if (mMapRotation != MapRotation)
    {
        mMapRotation = MapRotation;
        if (MapRotation == 3)
        {
            int* v3 = (int*)mem_heap_malloc(4, 4 * g_NumTotalMaps);
            for (int i = 0; i < g_NumTotalMaps; ++i)
                v3[i] = i;
            v3[mMapID] = -1;
            int v5 = g_NumTotalMaps;
            int v6 = g_NumTotalMaps - 1;
            for (int j = 0; j < g_NumTotalMaps - 1; v6 = g_NumTotalMaps - 1)
            {
                int k = irand(0, v6);
                while (v3[k] == -1)
                    k = (k + 1) % g_NumTotalMaps;
                v3[k] = -1;
                mRandomMapList[j] = (char)k;
                v5 = g_NumTotalMaps;
                ++j;
            }
            mRandomMapList[v5 - 1] = -1;
            mem_heap_free(v3);
        }
    }
}

// ============================================================================
// MPGameInfo (mp.o)
// ============================================================================
// ea: 0x0073DAD0
MPGameInfo::MPGameInfo(unsigned int titleID,
                       bdReference<bdCommonAddr> hostAddr,
                       unsigned char publicOpen,
                       unsigned char privateOpen,
                       unsigned char publicFilled,
                       unsigned char privateFilled)
    : bdGameInfo()
{
    m_publicOpen = publicOpen;
    m_privateOpen = privateOpen;
    m_publicFilled = publicFilled;
    m_privateFilled = privateFilled;
    memset(m_secID.ab, 0, sizeof(m_secID.ab));
    memset(m_secKey.ab, 0, sizeof(m_secKey.ab));
    setTitleID(titleID);
    if (hostAddr.m_ptr != nullptr)
        ++hostAddr.m_ptr->m_refCount;
    setHostAddr(hostAddr);
    if (hostAddr.m_ptr != nullptr && hostAddr.m_ptr->m_refCount-- == 1)
        delete hostAddr.m_ptr;
}

// ea: 0x0073DFF0
bool MPGameInfo::operator==(const MPGameInfo& other) const
{
    return m_titleId == other.m_titleId
        && m_hostAddr.m_ptr == other.m_hostAddr.m_ptr
        && m_publicOpen == other.m_publicOpen
        && m_privateOpen == other.m_privateOpen
        && m_publicFilled == other.m_publicFilled
        && m_privateFilled == other.m_privateFilled
        && strcmp(mName, other.mName) == 0
        && mMapID == other.mMapID
        && mGameSubType == other.mGameSubType
        && mTeamBalancing == other.mTeamBalancing
        && mFriendlyFire == other.mFriendlyFire
        && mEnableAARVote == other.mEnableAARVote
        && mEnablePenaltyVote == other.mEnablePenaltyVote;
}

// ============================================================================
// Menu options (mp.o)
// ============================================================================
const char* const MPOptionsScreenMenu::kScreenOptionStrings[4] = {
    "FEMENU_COP_SCREENSIZE", "FEMENU_COP_RESOLUTION", "FEMENU_COP_GAMMA",
    "FEMENU_COP_SIZE_NORMAL",
};
const char* const MPOptionsScreenMenu::kScreenInstructionStrings[4] = {
    "FEMENU_SCREEN_INST_SIZE", "FEMENU_SCREEN_INST_RESOLUTION",
    "FEMENU_SCREEN_INST_GAMMA", "text_title_main",
};
const char* const MPOptionsGameplayMenu::kGameplayOptionStrings[4] = {
    "FEMENU_COP_SUBTITLES", "FEMENU_COP_CROSSHAIR",
    "FEMENU_COP_SFRIENDLY_TAGS", "FEMENU_COP_STICKYAIM",
};
const char* const MPOptionsGameplayMenu::kGameplayInstructionStrings[4] = {
    "FEMENU_GAMEPLAY_INST_SUBTITLES", "FEMENU_GAMEPLAY_INST_CROSSHAIR",
    "FEMENU_GAMEPLAY_INST_FRIENDLYTAGS", "FEMENU_GAMEPLAY_INST_STICKYAIM",
};

// ea: 0x007309D0
void MPOptionsScreenMenu::OnActivate()
{
    FEMenu::OnActivate();
    if (mWidescreen != (cg_widescreen.integer != 0))
        UpdateWidescreen(cg_widescreen.integer != 0);
    mScreenText[2]->SetText(kScreenOptionStrings[highlighted]);
    const char* STBString = STBManager::sInst->GetSTBString(
        kScreenInstructionStrings[highlighted]);
    Broc::string v7(STBString);
    mInstructionsText->SetTextBoxNoLocalize(
        v7, mWidescreen ? 390 : 520, -1.5f);
    controller* v5 = controller::inst();
    this->entries[0]->SetValue(
        gSaveGameData[controller::inst()->locked_port].mStubData.mRatioIs4by3);
    controller* v6 = controller::inst();
    this->entries[1]->SetValue(
        gSaveGameData[v6->locked_port].mStubData.mResolutionIs480p);
}

// ea: 0x00730B00
bool MPOptionsScreenMenu::SaveOptions()
{
    bool v2 = false;
    int locked_port = controller::inst()->locked_port;
    if (gSaveGameData[locked_port].mStubData.mRatioIs4by3
        != (this->entries[0]->GetValue() != 0))
    {
        gSaveGameData[controller::inst()->locked_port]
            .mStubData.mRatioIs4by3 = this->entries[0]->GetValue() != 0;
        v2 = true;
    }
    if (gSaveGameData[controller::inst()->locked_port]
            .mStubData.mResolutionIs480p
        == (this->entries[1]->GetValue() != 0))
        return v2;
    gSaveGameData[controller::inst()->locked_port]
        .mStubData.mResolutionIs480p = this->entries[1]->GetValue() != 0;
    return true;
}

// ea: 0x0073E400
void MPOptionsGameplayMenu::OnActivate()
{
    FEMenu::OnActivate();
    if (mWidescreen != (cg_widescreen.integer != 0))
        UpdateWidescreen(cg_widescreen.integer != 0);
    mGameplayText[2]->SetText(kGameplayOptionStrings[highlighted]);
    const char* STBString = STBManager::sInst->GetSTBString(
        kGameplayInstructionStrings[highlighted]);
    Broc::string v5(STBString);
    mInstructionsText->SetTextBoxNoLocalize(
        v5, mWidescreen ? 390 : 520, -1.5f);
    SetOptions();
}

// ea: 0x007314B0
bool MPOptionsSoundMenu::SaveOptions()
{
    bool v2 = false;
    int locked_port = controller::inst()->locked_port;
    if (gSaveGameData[locked_port].mStubData.mEffectVolume
        != this->entries[0]->GetValue())
    {
        gSaveGameData[controller::inst()->locked_port].mStubData.mVolume =
            this->entries[0]->GetValue();
        gSaveGameData[controller::inst()->locked_port].mStubData.mMusicVolume =
            this->entries[0]->GetValue();
        gSaveGameData[controller::inst()->locked_port].mStubData.mEffectVolume =
            this->entries[0]->GetValue();
        v2 = true;
        gSaveGameData[controller::inst()->locked_port].mStubData
            .ApplyStubOptions();
    }
    return v2;
}

// ea: 0x0073E140
void MPOptionsSoundMenu::AdjustOptions(bool up)
{
    int* v3 = &gSaveGameData[controller::inst()->locked_port]
                   .mStubData.mVolume;
    int v4 = 2 * (int)up - 1;
    int v7 = v4 + *v3;
    if (v7 < 0)
        v7 = 0;
    if (v7 > 50)
        v7 = 50;
    *v3 = v7;
    MusicMgr::sInst->ScaleVolume(v7 * 0.02f);
    SoundDevice::sInst->ScaleVolume(*v3 * 0.02f);
    gSaveGameData[controller::inst()->locked_port].mStubData.mVolume = *v3;
    dword_F32F40 = *v3;
    dword_F34B34 = *v3;
    dword_F36728 = *v3;
    if (SaveOptions())
        ProfileEditMenu::Me()->mNeedWrite = true;
}

// ea: 0x00732F90
void MPOptionsPreferencesMenu::SetOptions()
{
    controller* v2 = controller::inst();
    this->entries[0]->SetValue(
        gSaveGameData[v2->locked_port].mStubData.mMaxPlayerCntPreference + 1);
    controller* v3 = controller::inst();
    this->entries[1]->SetValue(
        gSaveGameData[v3->locked_port].mStubData.mGameModePreference + 1);
    controller* v4 = controller::inst();
    this->entries[2]->SetValue(
        gSaveGameData[v4->locked_port].mStubData.mMapPreference + 1);
    controller* v5 = controller::inst();
    this->entries[3]->SetValue(
        gSaveGameData[v5->locked_port].mStubData.mAutoTeamBalancePreference
        + 1);
    controller* v6 = controller::inst();
    this->entries[4]->SetValue(
        gSaveGameData[v6->locked_port].mStubData.mTeamDamagePreference + 1);
}

// ============================================================================
// MPProfileMainMenu dialogs (mp.o)
// ============================================================================
// ea: 0x0073E7F0
void MPProfileMainMenu::DialogDisplayProfileLoadSuccess(int index)
{
    (void)index;
    g_femanager.GetDMS(currCl)->BringUp("MEM_LOAD_SUCCESS", false, false,
                                        defaultFileName, true);
    DialogMenuSystem* v1 = g_femanager.GetDMS(currCl);
    DialogMenu* Layer = v1->GetLayer(v1->GetActiveMenu() == 0);
    Layer->AddOption("MEM_DIALOG_OK", DialogResponseProfileLoadOk);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    v4->GetLayer(v4->GetActiveMenu() == 0)->triangleResponse = (void (*)(int))j_nullsub_96;
    g_femanager.GetDMS(currCl)->HighlightOption(0);
    DialogMenuSystem* v7 = g_femanager.GetDMS(currCl);
    DialogMenu* v9 = v7->GetLayer(v7->GetActiveMenu() == 0);
    v9->Reformat(true, 0);
}

// ea: 0x0073EA80
void MPProfileMainMenu::DialogDisplaySaveSuccess()
{
    g_femanager.GetDMS(currCl)->BringUp("MEM_SAVE_SUCCESS", false, false,
                                        defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    DialogMenu* Layer = v2->GetLayer(v2->GetActiveMenu() == 0);
    Layer->AddOption("MEM_DIALOG_OK", DialogResponseSaveSuccess);
    g_femanager.GetDMS(currCl)->HighlightOption(0);
    DialogMenuSystem* v6 = g_femanager.GetDMS(currCl);
    v6->GetLayer(v6->GetActiveMenu() == 0)->triangleResponse = (void (*)(int))j_nullsub_96;
    DialogMenuSystem* v8 = g_femanager.GetDMS(currCl);
    DialogMenu* v10 = v8->GetLayer(v8->GetActiveMenu() == 0);
    v10->Reformat(true, 0);
}

// ea: 0x0073EB60
void MPProfileMainMenu::DialogDisplayDeleteSuccess()
{
    g_femanager.GetDMS(currCl)->BringUp("MEM_DELETE_SUCCESSFUL_XBOX", false,
                                        false, defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    DialogMenu* Layer = v2->GetLayer(v2->GetActiveMenu() == 0);
    Layer->AddOption("MEM_DIALOG_OK", DialogResponseDeleteSuccess);
    g_femanager.GetDMS(currCl)->HighlightOption(0);
    DialogMenuSystem* v6 = g_femanager.GetDMS(currCl);
    v6->GetLayer(v6->GetActiveMenu() == 0)->triangleResponse = (void (*)(int))j_nullsub_96;
    DialogMenuSystem* v8 = g_femanager.GetDMS(currCl);
    DialogMenu* v10 = v8->GetLayer(v8->GetActiveMenu() == 0);
    v10->Reformat(true, 0);
}

// ea: 0x0073EC40
void MPProfileMainMenu::DialogDisplayNoMemDevice()
{
    g_femanager.GetDMS(currCl)->BringUp("MEM_ERROR_INSERT_CARD", false, false,
                                        defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    DialogMenu* Layer = v2->GetLayer(v2->GetActiveMenu() == 0);
    Layer->AddOption("MEM_DIALOG_OK", DialogResponseNoMemCard);
    g_femanager.GetDMS(currCl)->HighlightOption(0);
    DialogMenuSystem* v6 = g_femanager.GetDMS(currCl);
    v6->GetLayer(v6->GetActiveMenu() == 0)->triangleResponse = (void (*)(int))j_nullsub_96;
    DialogMenuSystem* v8 = g_femanager.GetDMS(currCl);
    DialogMenu* v10 = v8->GetLayer(v8->GetActiveMenu() == 0);
    v10->Reformat(true, 0);
}

// ea: 0x0073ED20
void MPProfileMainMenu::DialogDisplayDataCorrupt()
{
    g_femanager.GetDMS(currCl)->BringUp("MEM_XBOX_CORRUPT_DETECT", false,
                                        false, defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    DialogMenu* Layer = v2->GetLayer(v2->GetActiveMenu() == 0);
    Layer->AddOption("MEM_DIALOG_OK", DialogResponseNoMemCard);
    g_femanager.GetDMS(currCl)->HighlightOption(0);
    DialogMenuSystem* v6 = g_femanager.GetDMS(currCl);
    v6->GetLayer(v6->GetActiveMenu() == 0)->triangleResponse = (void (*)(int))j_nullsub_96;
    DialogMenuSystem* v8 = g_femanager.GetDMS(currCl);
    DialogMenu* v10 = v8->GetLayer(v8->GetActiveMenu() == 0);
    v10->Reformat(true, 0);
}

// ea: 0x0073EE00
void MPProfileMainMenu::DialogDisplayNoFreeSpace()
{
    g_femanager.GetDMS(currCl)->BringUp("MEM_ERROR_NOT_ENOUGH_BLOCKS1",
                                        false, false, defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    DialogMenu* Layer = v2->GetLayer(v2->GetActiveMenu() == 0);
    Layer->AddOption("MEM_DIALOG_OK", DialogResponseNoMemCard);
    g_femanager.GetDMS(currCl)->HighlightOption(0);
    DialogMenuSystem* v6 = g_femanager.GetDMS(currCl);
    v6->GetLayer(v6->GetActiveMenu() == 0)->triangleResponse = (void (*)(int))j_nullsub_96;
    DialogMenuSystem* v8 = g_femanager.GetDMS(currCl);
    DialogMenu* v10 = v8->GetLayer(v8->GetActiveMenu() == 0);
    v10->Reformat(true, 0);
}

// ea: 0x00734250
bool MPProfileMainMenu::DialogResponseProfileLoadOk(int index)
{
    (void)index;
    return true;
}

// ea: 0x00734290
bool MPProfileMainMenu::DialogResponseSaveSuccess(int index)
{
    (void)index;
    return true;
}

// ea: 0x007342B0
bool MPProfileMainMenu::DialogResponseDeleteSuccess(int index)
{
    (void)index;
    return true;
}

// ============================================================================
// Batch 11 remainder: anim flags, usercmd serialization, controls menu
// OnActivate, dropped-item deserialize, score/area/vote/revive messages
// ============================================================================

const char* const MPOptionsControlsMenu::kControlsOptionStrings[] = {
    "FEMENU_COP_STICKLAYOUT", "FEMENU_COP_BUTTONLAYOUT",
    "FEMENU_COP_HORIZONTALSENS", "FEMENU_COP_VERTICALSENS",
    "FEMENU_COP_INVERTAIM", "FEMENU_COP_TOGGLEADS",
    "FEMENU_COP_ALTTANKCONTROL", "FEMENU_COP_VIBRATION",
};
const char* const MPOptionsControlsMenu::kControlsInstructionStrings[] = {
    "FEMENU_CONTROLS_INST_STICKLAYOUT", "FEMENU_CONTROLS_INST_BUTTONLAYOUT",
    "FEMENU_CONTROLS_INST_HORIZONTALSENS", "FEMENU_CONTROLS_INST_VERTICALSENS",
    "FEMENU_CONTROLS_INST_INVERTAIM", "FEMENU_CONTROLS_INST_TOGGLEADS",
    "FEMENU_CONTROLS_INST_ALTTANKCONTROL", "FEMENU_CONTROLS_INST_VIBRATION",
};
const char* const MPOptionsControlsMenu::kStickLayoutStrings[] = {
    "FEMENU_COP_STICK_DEFAULT", "FEMENU_COP_STICK_SOUTHPAW",
    "FEMENU_COP_STICK_LEGACY", "FEMENU_COP_STICK_LEGACYSOUTHPAW",
};
const char* const MPOptionsControlsMenu::kButtonLayoutStrings[] = {
    "FEMENU_COP_BUTTON_DEFAULT", "FEMENU_COP_BUTTON_SOUTHPAW",
    "FEMENU_COP_BUTTON_SCI_FI", "FEMENU_COP_BUTTON_LEGACY",
};

// ea: 0x0072D100
void MPPlayer::PlayAnimFlagAnim(DObj* dobj, unsigned int packBit, int sheet,
                                int row, int col, bool doStopAnim, float alpha,
                                float speed, bool force, bool force_val)
{
    bool v11 = ((1 << packBit) & this->mAnimFlags) != 0;
    if (force)
        v11 = force_val;
    MP_ANIM_INDEX* v12 = nullptr;
    int animIndex;
    if (base_anim_indices[38 * sheet + row].anims[col].animIndex != 0)
    {
        v12 =
            &base_anim_names[base_anim_indices[38 * sheet + row].anims[col]
                                 .animIndex];
        if (v12->anim != nullptr)
            goto LABEL_9;
        animIndex = base_anim_indices[row].anims[col].animIndex;
    }
    else
    {
        animIndex = base_anim_indices[row].anims[col].animIndex;
    }
    if (animIndex != 0)
        v12 = &base_anim_names[animIndex];
LABEL_9:
    if (v11)
    {
        if (v12 != nullptr)
        {
            nalGeneric::nalGenericAnim* anim = v12->anim;
            if (anim != nullptr)
            {
                AnimationPlayer* v15 = (AnimationPlayer*)dobj->animPlayers[0];
                v15->PlayModifier(anim, 1.0f, packBit);
                v15->SetModifierAlpha(packBit, 1.0f, anim, alpha);
                v15->SetModifierSpeed(packBit, 1.0f, anim, speed);
                if ((v12->flags & 1) != 0)
                    v15->SetModifierType(
                        packBit, AnimationPlayer::nalAdditiveModifier);
            }
        }
    }
    else if (doStopAnim && dobj != nullptr)
    {
        AnimationPlayer* v15 = (AnimationPlayer*)dobj->animPlayers[0];
        if (v15 != nullptr)
            v15->StopModifiers(packBit);
    }
}

// ea: 0x0073C7C0
void MPUtility::WriteUserCmd(bdReference<bdBitBuffer> buffer,
                             const usercmd_s& cmd)
{
    int serverTime = cmd.serverTime;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
    buffer.m_ptr->writeBits(&serverTime, 0x20u);
    buffer.m_ptr->writeRangedInt32(cmd.buttons, 0, 16);
    unsigned char weapon = (unsigned char)cmd.weapon;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&weapon, 8u);
    int angle = cmd.angles[0];
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE);
    buffer.m_ptr->writeBits(&angle, 0x20u);
    angle = cmd.angles[1];
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE);
    buffer.m_ptr->writeBits(&angle, 0x20u);
    unsigned char move = (unsigned char)cmd.forwardmove;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&move, 8u);
    move = (unsigned char)cmd.rightmove;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&move, 8u);
    move = (unsigned char)cmd.upmove;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&move, 8u);
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
}

// ea: 0x0073E230
void MPOptionsControlsMenu::OnActivate(int previous)
{
    (void)previous;
    FEMenu::OnActivate();
    if (mWidescreen != (cg_widescreen.integer != 0))
        UpdateWidescreen(cg_widescreen.integer != 0);
    mControlsText[2]->SetText(kControlsOptionStrings[highlighted]);
    const char* STBString = STBManager::sInst->GetSTBString(
        kControlsInstructionStrings[highlighted]);
    Broc::string v12(STBString);
    mInstructionsText->SetTextBoxNoLocalize(
        v12, mWidescreen ? 390 : 520, -1.5f);
    float v6 = 1.0f;
    if (highlighted != 2)
        v6 = 0.5f;
    ((FESlider*)this->entries[2])->mBar->SetAlpha(v6);
    float v9 = 1.0f;
    if (highlighted != 3)
        v9 = 0.5f;
    ((FESlider*)this->entries[3])->mBar->SetAlpha(v9);
    SetOptions();
}

// ea: 0x00748080
bool MPPlayerItems::DeserializeDropItem(bdReference<bdBitBuffer> buffer,
                                        float* position, float* angles,
                                        float* velocity, int& item, int& index,
                                        int& typeIndex)
{
    int v7 = 0;
    int itemType = 0;
    bool ok =
        buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE)
        && buffer.m_ptr->readBits(&itemType, 0x20u);
    if (ok
        && buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE)
        && buffer.m_ptr->readBits(&v7, 0x20u))
        ok = true;
    else
        ok = false;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    if (ok && MPUtility::ReadPosition(buffer, position))
        ok = true;
    else
        ok = false;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    if (ok && MPUtility::ReadAnglesYawPitch(buffer, angles))
        ok = true;
    else
        ok = false;
    if (buffer.m_ptr != nullptr)
        ++buffer.m_ptr->m_refCount;
    if (ok && MPUtility::ReadVector(buffer, velocity)
        && buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE)
        && buffer.m_ptr->readBits(&typeIndex, 0x20u))
        ok = true;
    else
        ok = false;
    item = itemType;
    index = v7;
    bool v8 = ok;
    if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
        delete buffer.m_ptr;
    return v8;
}

// ea: 0x007578D0
void MPPlayerManager::HandlePlayerState(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = GetPlayer(conn);
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED)
        {
            bdReference<bdMessage> msg = receivedMsg.getMessage();
            bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
            unsigned char playerID[4];
            playerID[0] = Player->mId;
            if (buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE))
                buffer.m_ptr->readBits(playerID, 8u);
            MPPlayer* v9 = GetPlayer(playerID[0]);
            if (v9 != nullptr && !IsLocalId(playerID[0]))
            {
                if (buffer.m_ptr != nullptr)
                    ++buffer.m_ptr->m_refCount;
                v9->deserialize(buffer);
                v9->OnModified();
            }
            if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
                delete buffer.m_ptr;
            if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
                delete msg.m_ptr;
        }
    }
}

// ea: 0x00742670
void MPPeer::PlayerReviveRequest(Entity* player, Entity* medic)
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    if (p_mSession->getStatus() != bdSession::BD_SESSION_NOT_CONNECTED)
    {
        if (player != nullptr && medic != nullptr)
        {
            MPPlayerManager* p_mPlayerManager =
                (MPPlayerManager*)((char*)this + 0x74E0);
            MPPlayer* v6 = p_mPlayerManager->GetPlayer(medic);
            MPPlayer* v7 = p_mPlayerManager->GetPlayer(player);
            if (v7 != nullptr && v6 != nullptr)
            {
                bdMessage* msg = new bdMessage(0x5Du, false);
                bdReference<bdMessage> message;
                message.m_ptr = msg;
                if (msg != nullptr)
                    ++msg->m_refCount;
                extern int g_NumBdMessages;
                ++g_NumBdMessages;
                bdReference<bdBitBuffer> buffer = msg->getPayload();
                unsigned char playerId = v7->mId;
                if (buffer.m_ptr != nullptr)
                    ++buffer.m_ptr->m_refCount;
                MPUtility::WritePlayerId(buffer, playerId);
                unsigned char medicId = v6->mId;
                if (buffer.m_ptr != nullptr)
                    ++buffer.m_ptr->m_refCount;
                MPUtility::WritePlayerId(buffer, medicId);
                p_mPlayerManager->SendAll(message, true, false);
                if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
                    delete buffer.m_ptr;
                if (message.m_ptr != nullptr
                    && message.m_ptr->m_refCount-- == 1)
                    delete message.m_ptr;
            }
        }
    }
}

// ea: 0x007432F0
void MPPeer::SendGameScore(int alliesScore, int axisScore)
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    if (p_mSession->getStatus() != bdSession::BD_SESSION_NOT_CONNECTED
        && p_mSession->getRole() == bdSession::BD_SESSION_HOST)
    {
        bdMessage* msg = new bdMessage(0x40u, false);
        bdReference<bdMessage> message;
        message.m_ptr = msg;
        if (msg != nullptr)
            ++msg->m_refCount;
        extern int g_NumBdMessages;
        ++g_NumBdMessages;
        bdReference<bdBitBuffer> buffer = msg->getPayload();
        buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE);
        buffer.m_ptr->writeBits(&alliesScore, 0x20u);
        buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE);
        buffer.m_ptr->writeBits(&axisScore, 0x20u);
        if (msg != nullptr)
            ++msg->m_refCount;
        ((MPPlayerManager*)((char*)this + 0x74E0))->SendAll(message, true,
                                                            false);
        if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
            delete buffer.m_ptr;
        if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
            delete message.m_ptr;
    }
}

// ea: 0x00744400
void MPPeer::AreaCaptured(int netIndex, int team, int hostOnly)
{
    bdSession* p_mSession = (bdSession*)((char*)this + 0x7448);
    if (p_mSession->getStatus() != bdSession::BD_SESSION_NOT_CONNECTED
        && (hostOnly != 1
            || p_mSession->getRole() == bdSession::BD_SESSION_HOST))
    {
        bdMessage* msg = new bdMessage(0x46u, false);
        bdReference<bdMessage> message;
        message.m_ptr = msg;
        if (msg != nullptr)
            ++msg->m_refCount;
        extern int g_NumBdMessages;
        ++g_NumBdMessages;
        bdReference<bdBitBuffer> buffer = msg->getPayload();
        buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_SIGNED_INTEGER32_TYPE);
        buffer.m_ptr->writeBits(&netIndex, 0x20u);
        buffer.m_ptr->writeRangedUInt32(team, 0, 2u, true);
        if (msg != nullptr)
            ++msg->m_refCount;
        ((MPPlayerManager*)((char*)this + 0x74E0))->SendAll(message, true,
                                                            false);
        if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
            delete buffer.m_ptr;
        if (message.m_ptr != nullptr && message.m_ptr->m_refCount-- == 1)
            delete message.m_ptr;
    }
}

// ea: 0x00738A40
void MPPlayerManager::HandleVoteEnded(const bdReceivedMessage& receivedMsg)
{
    if (*(bool*)((char*)this + 0x4112))  // mLocalPlayerInGame
    {
        bdReference<bdMessage> msg = receivedMsg.getMessage();
        bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
        unsigned char voteSucceeded = 0;
        if (buffer.m_ptr->readDataType(bdBitBuffer::BD_BB_BOOL_TYPE))
        {
            if (buffer.m_ptr->readBits(&voteSucceeded, 1u))
                voteSucceeded = voteSucceeded != 0;
        }
        if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
            delete buffer.m_ptr;
        if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
            delete msg.m_ptr;
        const char* v9 = voteSucceeded != 0 ? "MPGAME_VOTE_SUCCEEDED"
                                            : "MPGAME_VOTE_FAILED";
        char* v7 = va("%s \"%s\"", "gm", v9);
        SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(), v7);
        unsigned int v10[21] = {};
        v10[3] = 0xFF;  // LOWORD(v10[3]) = 255
        memset(&v10[4], 255, 16);
        v10[0] = 0;
        v10[1] = 0;
        v10[2] = -16777216;
        v10[20] = 0;  // LOBYTE(v10[20]) = 0
        memcpy((char*)this + 0x5914, v10, 0x54);  // currVote
    }
}

// ea: 0x00739350
void MPPlayerManager::HandleAreaCaptured(const bdReceivedMessage& receivedMsg)
{
    bdReference<bdConnection> conn = receivedMsg.getConnection();
    MPPlayer* Player = GetPlayer(conn);
    int netIndex = 0;
    unsigned int team = 0;
    if (Player != nullptr)
    {
        if (Player->mConnection.m_ptr != nullptr
            && Player->mConnection.m_ptr->getStatus() == bdConnection::BD_CONNECTED)
        {
            bdReference<bdMessage> msg = receivedMsg.getMessage();
            bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
            if (buffer.m_ptr->readInt32(netIndex))
                buffer.m_ptr->readRangedUInt32(team, 0, 2u, true);
            if (team != 0)
                ((char*)this + 0x58AC)[netIndex] = team != 1 ? 0 : 0x7F;
            else
                ((char*)this + 0x58AC)[netIndex] = -127;
            if (*(bool*)((char*)this + 0x4112))  // mLocalPlayerInGame
            {
                if (gpBrocAPI->mBrocExports.mCallbackAreaCaptured != nullptr)
                    gpBrocAPI->mBrocExports.mCallbackAreaCaptured(netIndex,
                                                                  (int)team);
            }
            if (buffer.m_ptr != nullptr && buffer.m_ptr->m_refCount-- == 1)
                delete buffer.m_ptr;
            if (msg.m_ptr != nullptr && msg.m_ptr->m_refCount-- == 1)
                delete msg.m_ptr;
        }
    }
}
