// ============================================================================
// mp_basic.cpp - multiplayer game modes (mp.o) batch 1: small helpers
// Reconstructed from IDA decompiles/disasm of the release XBE.
// ============================================================================

#include "game/logic/g_local.h"
#include "game/mp/mp_types.h"

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

// ============================================================================
// MPUIInterface statics (mp.o)
// ============================================================================
int MPUIInterface::mReturnMenu;
bool MPUIInterface::mKicked;

int MPUIInterface::GetTimeLimitCount()
{
    return 6;
}

int MPUIInterface::GetRoundLimitCount()
{
    return 1;
}

int MPUIInterface::GetMaxPlayersCount()
{
    return 4;
}

int MPUIInterface::GetRespawnTimeCount()
{
    return 3;
}

int MPUIInterface::GetReturnMenu()
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

// ============================================================================
// MPPlayerManager (mp.o)
// ============================================================================
bool MPPlayerManager::IsGuest(int) const
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
bdSession::bdSessionStatus MPPeer::GetSessionStatus() const
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
    unsigned int fromPlayerIndex, unsigned char*, unsigned int)
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
                                         short id)
{
    if (mPeer != nullptr)
        ((MPPlayerManager*)((char*)mPeer + 0x74E0))
            ->RegisterDroppedItem(itemType, item, owner, id);
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
