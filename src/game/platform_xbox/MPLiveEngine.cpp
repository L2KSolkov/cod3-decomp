// ============================================================================
// MPLiveEngine.cpp - multiplayer Xbox Live engine (game_xbox.o MPLiveEngine.cpp)
// 26 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// bd message/refcount juggling uses the ported bd_types.h API.
// ============================================================================

#include "MPLiveEngine.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <new>

// MPUIInterface class statics (mp.o owns the originals; these satisfy the
// class-static manglings for the local build).
bool MPUIInterface::mLiveQueryActive = false;   // ?mLiveQueryActive@MPUIInterface@@1_NA
bool MPUIInterface::mQueryFromID = false;       // ?mQueryFromID@MPUIInterface@@1_NA
bool MPUIInterface::mIsViewableOnline = false;  // ?mIsViewableOnline@MPUIInterface@@1_NA
MPUIInterface::EGameConnectionType MPUIInterface::mGameConnectionType =
    MPUIInterface::kGameConnectionTypeLan;  // ?mGameConnectionType@MPUIInterface@@1W4EGameConnectionType@@A
bool MPUIInterface::mInSession = false;    // ?mInSession@MPUIInterface@@1_NA

// ea: 0x0072F480 (mp.o)
bool MPUIInterface::IsOnlineGame()  // ?IsOnlineGame@MPUIInterface@@SA_NXZ
{
    return mGameConnectionType == kGameConnectionTypeOnline;
}

// ea: 0x0072FF40 (mp.o)
bool MPUIInterface::InSession()  // ?InSession@MPUIInterface@@SA?B_NXZ
{
    if (mGameConnectionType == kGameConnectionTypeOnline)
        return LiveWrapper::theWrapper->sessionState == kInSession;
    return mInSession;
}

// ============================================================================
// Assertion system externs (core_xboxr:AeAssert.o)
// ============================================================================
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

// ============================================================================
// MPPlayerSet inline methods (MPPlayerSet.h; COMDATs in game_xbox.o)
// ============================================================================

// ea: 0x72A480
void MPPlayerSet::set(unsigned int index)
{
    if (index >= 0x10)
    {
        ASSERT("index < 16", "c:\\cod\\code\\game\\mp\\MPPlayerSet.h", 75);
    }
    mBitPlayers = (unsigned short)(1 << index);
}

// ea: 0x72A580
bool MPPlayerSet::containsPlayer(unsigned int index) const
{
    if (index >= 0x10)
    {
        ASSERT("index < 16", "c:\\cod\\code\\game\\mp\\MPPlayerSet.h", 153);
    }
    return (mBitPlayers & (1 << index)) != 0;
}

// ============================================================================
// game_xbox.o globals
// ============================================================================
bool g_IgnoreUIXInput;

// MPLiveEngine vtable (base LiveWrapper slots + derived overrides; the real
// table lives in rdata at 0xD18A04 - this static only satisfies the ctor/dtor
// vtable-pointer stores until the data is emitted with the full UI port).
static void* MPLiveEngineVftable[32];
struct MPLiveEngineVftableInit {
    MPLiveEngineVftableInit()
    {
        for (int i = 0; i < 32; ++i)
            MPLiveEngineVftable[i] = nullptr;
    }
};
static MPLiveEngineVftableInit s_mpLiveEngineVftableInit;

MultiplayerMgr* MultiplayerMgr::sInst;

// ?IsHost@MultiplayerMgr@@QAE_NXZ (mp.o 0xB24D80; stub)
bool MultiplayerMgr::IsHost()
{
    return true;
}
// ?GetDroppedItemType@MultiplayerMgr@@QAE?AW4EDroppedItemTypes@@W4itemType_t@@@Z (mp.o; stub)
int MultiplayerMgr::GetDroppedItemType(int itemType)
{
    (void)itemType;
    return 0;
}
// Dropped-item stubs (mp.o; MPEntityHandle surface not ported)
MultiplayerMgr::MPEntityHandle MultiplayerMgr::FindDroppedItemID(
    int itemType, Entity* item, Entity* owner)
{
    (void)itemType; (void)item; (void)owner;
    MultiplayerMgr::MPEntityHandle h;
    h.mVal = 0;
    return h;
}
MultiplayerMgr::MPEntityHandle MultiplayerMgr::RegisterDroppedItem(
    int itemType, Entity* item, Entity* owner)
{
    (void)itemType; (void)item; (void)owner;
    MultiplayerMgr::MPEntityHandle h;
    h.mVal = 0;
    return h;
}
void MultiplayerMgr::RegisterDroppedItem(int itemType, Entity* item,
                                         Entity* owner, int extra)
{
    (void)itemType; (void)item; (void)owner; (void)extra;
}
void MultiplayerMgr::GetNextDroppedItemID(void* result, int itemType,
                                         Entity* owner)
{
    (void)result; (void)itemType; (void)owner;
}

// ============================================================================
// MPLiveEngine
// ============================================================================

// ea: 0x726DB0
MPLiveEngine::MPLiveEngine()
{
    *(void***)this = MPLiveEngineVftable;
    liveSession.m_SessionQosQ.m_pTail = nullptr;
    liveSession.m_SessionQosQ.m_pHead = nullptr;
    liveSession.m_QosResponse.m_wLength = 0;
    liveSession.m_QosResponse.m_pvData = nullptr;
    liveSession.m_State = CSession::STATE_IDLE;
    liveSession.m_hSessionTask = nullptr;
    liveSession.m_bUpdate = 0;
    liveSession.m_bListening = 0;
    liveSession.m_bKeyRegistered = 0;
    liveSession.SetupAttributes();
    currentQuery = nullptr;
    actualPort = 0;
    invited = false;
    mListeners = nullptr;
    keyIndex = 0;
    memset(keySet[0].ab, 0, sizeof(keySet[0].ab));
    memset(keySet[1].ab, 0, sizeof(keySet[1].ab));
    memset(keySet[2].ab, 0, sizeof(keySet[2].ab));
}

// ea: 0x726E40
MPLiveEngine::~MPLiveEngine()
{
    *(void***)this = MPLiveEngineVftable;
    MPPlayerSet* mListeners = this->mListeners;
    if (mListeners != nullptr)
        mem_heap_free(mListeners);
    this->mListeners = nullptr;
    liveSession.Reset();
}

// ea: 0x721870
bool MPLiveEngine::CanHear(const XUID* talker, const XUID* listener)
{
    (void)talker;
    (void)listener;
    return false;
}

// ea: 0x721AA0
void MPLiveEngine::ClearKeys()
{
    XNKID blankKey;
    memset(&blankKey, 0, sizeof(blankKey));
    XNKID* keySet = this->keySet;
    for (int i = 3; i != 0; --i)
    {
        if (memcmp(&blankKey, keySet, 8) != 0)
        {
            XNetUnregisterKey(keySet);
            *keySet = blankKey;
        }
        ++keySet;
    }
    keyIndex = 0;
}

// ea: 0x726760
void MPLiveEngine::DoWork()
{
    if (g_controllerConnectedErrorShown[actualPort])
        g_IgnoreUIXInput = true;
    LiveWrapper::DoWork();
    HRESULT v2 = liveSession.Process();
    if (LiveWrapper::HandleError(v2) == 0x1500F0
        && sessionState == kEnteringSession)
    {
        HandleSessionCreation();
    }
    QueryInterface* currentQuery = this->currentQuery;
    if (currentQuery != nullptr
        && (currentQuery->IsRunning() != 0
            || this->currentQuery->IsProbing() != 0))
    {
        bool v4 = this->currentQuery->IsRunning() != 0;
        HRESULT v5 = this->currentQuery->Process();
        LiveWrapper::HandleError(v5);
        if (v4 == (this->currentQuery->IsRunning() == 0)
            && this->currentQuery->Done() != 0)
        {
            HRESULT v6 = this->currentQuery->Probe();
            LiveWrapper::HandleError(v6);
        }
    }
}

// ea: 0x724820
void MPLiveEngine::FreeSlot(bool isPrivate)
{
    if (isPrivate)
    {
        if (liveSession.PrivateFilled == 0)
        {
            ASSERT("liveSession.PrivateFilled",
                   "c:\\cod\\code\\game\\MPLiveEngine.cpp", 235);
        }
        unsigned int v4 = liveSession.PrivateOpen + 1;
        --liveSession.PrivateFilled;
        liveSession.PrivateOpen = v4;
        liveSession.m_Attributes[6].info.integer.qwValue -= 1;
        liveSession.m_Attributes[6].fChanged = 1;
        liveSession.Update();
    }
    else
    {
        if (liveSession.PublicFilled == 0)
        {
            ASSERT("liveSession.PublicFilled",
                   "c:\\cod\\code\\game\\MPLiveEngine.cpp", 243);
        }
        unsigned int PublicFilled = liveSession.PublicFilled;
        unsigned int dwLength = liveSession.m_Attributes[6].info.blob.dwLength;
        ++liveSession.PublicOpen;
        unsigned short* lpValue = liveSession.m_Attributes[6].info.string.lpValue;
        liveSession.PublicFilled = PublicFilled - 1;
        liveSession.m_Attributes[6].info.blob.dwLength =
            (lpValue != nullptr) + dwLength - 1;
        liveSession.m_Attributes[6].info.string.lpValue = lpValue - 1;
        liveSession.m_Attributes[6].fChanged = 1;
        liveSession.Update();
    }
}

// ea: 0x721DA0
MPLiveEngine* MPLiveEngine::GetHandle()
{
    return (MPLiveEngine*)LiveWrapper::theWrapper;
}

// ea: 0x721B20
unsigned int MPLiveEngine::GetPortToLock()
{
    if (internalState != kSignedIn)
    {
        ASSERT("internalState == kSignedIn",
               "c:\\cod\\code\\game\\MPLiveEngine.cpp", 323);
    }
    unsigned int result = 0;
    XUID* i;
    for (i = &localPlayers[0].xuid; i->qwValue == 0; i = (XUID*)((char*)i + 0xFE8))
    {
        if (++result >= 4)
            return (unsigned int)-1;
    }
    return result;
}

// ea: 0x7218D0
unsigned int MPLiveEngine::GetVoiceData(unsigned int consoleID,
                                        unsigned int buffer)
{
    (void)consoleID;
    (void)buffer;
    return 0;
}

// ea: 0x7218C0
unsigned int MPLiveEngine::GetVoiceDataSize(unsigned int consoleID)
{
    (void)consoleID;
    return 0;
}

// ea: 0x724970
bool MPLiveEngine::HandleInput(unsigned int port,
                               const XINPUT_STATE* controllerInput)
{
    bool result = internalMode != kNotSetup
        && ((unsigned int)actualPort == port || actualPort == (unsigned int)-1)
        && LiveWrapper::HandleInput(port, controllerInput);
    return result;
}

// ea: 0x724370 - voice packet -> bdMessage type 0x26 to remote players
void MPLiveEngine::HandleOutgoingVoice(unsigned int dwLocalPort,
                                       unsigned int dwSize,
                                       const unsigned char* pData)
{
    if (dwLocalPort == actualPort
        && MPUIInterface::IsOnlineGame()
        && MultiplayerMgr::sInst != nullptr
        && MultiplayerMgr::sInst->mPeer != nullptr
        && MultiplayerMgr::sInst->mPeer->GetPlayerManager() != nullptr
        && mListeners != nullptr)
    {
        MPPlayerManager* PlayerManager =
            MultiplayerMgr::sInst->mPeer->GetPlayerManager();
        if (PlayerManager->GetLocalPlayer(0) != nullptr)
        {
            bdMessage* v6 = (bdMessage*)bdMemory::allocate(0x18);
            bdMessage* v8;
            if (v6 != nullptr)
            {
                v8 = new (v6) bdMessage(0x26, nullptr, 0, false,
                                        pData, dwSize);
            }
            else
            {
                v8 = nullptr;
            }
            if (v8 != nullptr)
                v8->addRef();
            MPPlayerSet set(*mListeners);
            MultiplayerMgr::sInst->mPeer->GetPlayerManager()->Send(
                bdReference<bdMessage>(v8), set, false);
        }
    }
}

// ea: 0x721CD0
void MPLiveEngine::HandleSessionCreation()
{
    HRESULT v2 = LiveEngine_SetProperty(uixEngine,
                                        UIX_PROPERTY_ALLOW_GAME_INVITES, 1);
    LiveWrapper::HandleError(v2);
    sessionID = liveSession.SessionID;
    unsigned int actualPort = this->actualPort;
    sessionState = kInSession;
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        actualPort = 0;
    unsigned int notificationFlags =
        localPlayers[actualPort].notificationFlags;
    if ((localPlayers[actualPort].notificationFlags & 0x12) != 0x12)
    {
        localPlayers[actualPort].notificationFlags = notificationFlags | 0x12;
        LiveEngine_NotificationSetState(uixEngine, actualPort,
                                        notificationFlags | 0x12,
                                        sessionID, 0, nullptr);
        LiveEngine_SetProperty(uixEngine,
                               UIX_PROPERTY_ALLOW_GAME_INVITES, 1);
    }
}

// ea: 0x7218E0
void MPLiveEngine::JoinGame(XONLINE_FRIEND* joinee)
{
    DoWork();
    MPUIInterface::QueryFromID(&joinee->sessionID);
    DoWork();
    invited = true;
    if (MPUIInterface::InSession())
        MPUIInterface::ExitGame();
}

// ea: 0x7260F0
void MPLiveEngine::LeaveLiveSession()
{
    LiveEngine_EndFeature(uixEngine);
    memset(sessionID.ab, 0, sizeof(sessionID.ab));
    ClearKeys();
    if (internalState == kSignedIn)
    {
        HRESULT v2 = liveSession.Delete();
        LiveWrapper::HandleError(v2);
        unsigned int actualPort = this->actualPort;
        XNKID nullSession;
        memset(&nullSession, 0, sizeof(nullSession));
        sessionState = kNotInSession;
        if (logonMethod == UIX_LOGON_TYPE_SILENT)
            actualPort = 0;
        unsigned int notificationFlags =
            localPlayers[actualPort].notificationFlags;
        if ((notificationFlags & 0x12) != 0)
        {
            unsigned int v6 = notificationFlags & 0xFFFFFFED;
            localPlayers[actualPort].notificationFlags = v6;
            LiveEngine_NotificationSetState(uixEngine, actualPort, v6,
                                            sessionID, 0, nullptr);
            LiveEngine_SetProperty(uixEngine,
                                   UIX_PROPERTY_ALLOW_GAME_INVITES, 0);
        }
        LiveWrapper::SetSessionID(&nullSession);
    }
}

// ea: 0x721D70
void MPLiveEngine::LogoffCallBack()
{
    gSaveGameData[0].savedStateIsValid = false;
    if (MPUIInterface::IsOnlineGame() && sessionState == kInSession)
        MPUIInterface::ExitGame();
}

// ea: 0x7249A0
void MPLiveEngine::LogonCallBack()
{
    if (!gSaveGameData[0].appearOnline)
    {
        unsigned int actualPort = this->actualPort;
        if (logonMethod == UIX_LOGON_TYPE_SILENT)
            actualPort = 0;
        LiveWrapper::ToggleNotificationFlag(actualPort, 1);
        gSaveGameData[0].appearOnline = true;
    }
    gSaveGameData[0].savedStateIsValid = false;
}

// ea: 0x7247C0
bool MPLiveEngine::OccupyPrivateSlot()
{
    unsigned int PrivateOpen = liveSession.PrivateOpen;
    if (PrivateOpen == 0)
        return 0;
    liveSession.PrivateOpen = PrivateOpen - 1;
    ++liveSession.PrivateFilled;
    liveSession.m_Attributes[6].info.integer.qwValue += 1;
    liveSession.m_Attributes[6].fChanged = 1;
    liveSession.Update();
    return 1;
}

// ea: 0x724760
bool MPLiveEngine::OccupyPublicSlot()
{
    unsigned int PublicOpen = liveSession.PublicOpen;
    if (PublicOpen == 0)
        return 0;
    unsigned int PublicFilled = liveSession.PublicFilled;
    liveSession.PublicOpen = PublicOpen - 1;
    liveSession.PublicFilled = PublicFilled + 1;
    ++liveSession.m_Attributes[6].info.integer.qwValue;
    liveSession.m_Attributes[6].fChanged = 1;
    liveSession.Update();
    return 1;
}

// ea: 0x7219D0
int MPLiveEngine::RegisterKey(const XNKID* sessionID, const XNKEY* securityKey)
{
    int control = 0;
    XNKID* keySet = this->keySet;
    do
    {
        if (memcmp(sessionID, keySet, 8) == 0)
            return 0;
        ++keySet;
        ++control;
    }
    while (control < 3);
    int keyIndex = this->keyIndex;
    XNKID blankKey;
    memset(&blankKey, 0, sizeof(blankKey));
    XNKID* v6 = &this->keySet[keyIndex];
    if (memcmp(&blankKey, v6, 8) != 0)
        XNetUnregisterKey(v6);
    this->keySet[this->keyIndex] = *sessionID;
    int v7 = this->keyIndex + 1;
    bool v8 = this->keyIndex - 2 < 0;
    this->keyIndex = v7;
    // __OFSUB__(v7, 3) is always false here (v7 in [1,4) cannot overflow)
    if (!v8)
        this->keyIndex = 0;
    return XNetRegisterKey(sessionID, securityKey);
}

// ea: 0x721920
void MPLiveEngine::RunQuery(QueryInterface* newQuery)
{
    QueryInterface* currentQuery = this->currentQuery;
    if (currentQuery != nullptr)
    {
        if (currentQuery->IsProbing() != 0
            || this->currentQuery->IsRunning() != 0)
        {
            this->currentQuery->Cancel();
        }
        mem_heap_free(this->currentQuery);
    }
    this->currentQuery = newQuery;
    newQuery->Process();
}

// ea: 0x724640 - comm status -> bdMessage type 0x6F to others
void MPLiveEngine::SendCommunicatorStatus(UIX_VOICE_STATUS_TYPE commStatus)
{
    if (MPUIInterface::InSession() && MPUIInterface::IsOnlineGame())
    {
        bdMessage* v2 = (bdMessage*)bdMemory::allocate(0x18);
        bdMessage* v3;
        if (v2 != nullptr)
            v3 = new (v2) bdMessage(0x6F, false);
        else
            v3 = nullptr;
        if (v3 != nullptr)
        {
            bdReference<bdBitBuffer> buffer = v3->getPayload();
            buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_FULL_TYPE);
            buffer.m_ptr->writeBits(&commStatus, 0x20);
            MultiplayerMgr::sInst->mPeer->GetPlayerManager()->SendOthers(
                bdReference<bdMessage>(v3), true);
        }
    }
}

// ea: 0x7244A0 - mute state -> bdMessage type 0x36 to the single target
void MPLiveEngine::SendMuteUpdate(XUID* muter, XUID* mutee, bool isMuted)
{
    if (MultiplayerMgr::sInst != nullptr
        && MultiplayerMgr::sInst->mPeer != nullptr)
    {
        MPPlayerManager* PlayerManager =
            MultiplayerMgr::sInst->mPeer->GetPlayerManager();
        MPPlayerManager* v5 = PlayerManager;
        if (PlayerManager != nullptr
            && PlayerManager->GetLocalPlayer(0) != nullptr)
        {
            MPPlayerSet allPlayers = v5->allPlayers();
            unsigned int v6 = allPlayers.lowestPlayerIndex();
            if (v6 <= allPlayers.highestPlayerIndex())
            {
                while (1)
                {
                    if (allPlayers.containsPlayer(v6) != 0)
                    {
                        XUID* p_xuid = &v5->GetPlayer((unsigned char)v6)->xuid;
                        if (p_xuid->qwValue == mutee->qwValue)
                            break;
                    }
                    if (++v6 > allPlayers.highestPlayerIndex())
                        return;
                }
                bdMessage* v11 = (bdMessage*)bdMemory::allocate(0x18);
                bdMessage* v12;
                if (v11 != nullptr)
                    v12 = new (v11) bdMessage(0x36, false);
                else
                    v12 = nullptr;
                if (v12 != nullptr)
                {
                    bdReference<bdBitBuffer> payload = v12->getPayload();
                    payload.m_ptr->writeDataType(bdBitBuffer::BD_BB_FULL_TYPE);
                    payload.m_ptr->writeBits(muter, 0x60);
                    payload.m_ptr->writeBool(isMuted);
                    MPPlayerSet target;
                    target.set(v6);
                    v5->Send(bdReference<bdMessage>(v12), target, true);
                }
            }
        }
    }
}

// ea: 0x721980
void MPLiveEngine::SetRemoteListeners(const MPPlayerSet* listeners)
{
    if (mListeners == nullptr)
    {
        MPPlayerSet* v3 = (MPPlayerSet*)mem_heap_malloc(2);
        if (v3 != nullptr)
            v3->mBitPlayers = 0;
        mListeners = v3;
    }
    mListeners->mBitPlayers = listeners->mBitPlayers;
}

// ea: 0x721B90
void MPLiveEngine::StartLiveSession(sServerCreateParams* sessionParams,
                                    unsigned char publicOccupied,
                                    unsigned char privateOccupied)
{
    liveSession.m_Attributes[1].info.integer.qwValue =
        sessionParams->mMapID;
    liveSession.m_Attributes[1].fChanged = 1;
    liveSession.m_Attributes[3].fChanged = 1;
    liveSession.m_Attributes[3].info.integer.qwValue =
        sessionParams->mFriendlyFire;
    liveSession.m_Attributes[5].info.integer.qwValue =
        sessionParams->mGameSubType;
    liveSession.m_Attributes[5].fChanged = 1;
    liveSession.m_Attributes[0].info.integer.qwValue =
        sessionParams->mGameType;
    liveSession.m_Attributes[0].fChanged = 1;
    liveSession.m_Attributes[4].info.integer.qwValue =
        sessionParams->mTeamBalancing;
    liveSession.m_Attributes[4].fChanged = 1;
    liveSession.m_Attributes[6].info.integer.qwValue =
        privateOccupied + publicOccupied;
    liveSession.m_Attributes[6].fChanged = 1;
    __int64 mMaxPlayers = sessionParams->mMaxPlayers;
    liveSession.m_Attributes[8].info.integer.qwValue = mMaxPlayers;
    liveSession.m_Attributes[8].fChanged = 1;
    // Server name: byte-by-byte sign-extended copy into a wide buffer
    unsigned short wideName[24];
    int i = 0;
    do
    {
        wideName[i] = (unsigned short)(signed char)sessionParams->mName[i];
        ++i;
    }
    while (wideName[i - 1] != 0);
    wcscpy((wchar_t*)liveSession.m_strhost_name,
           (const wchar_t*)wideName);
    liveSession.m_Attributes[7].fChanged = 1;
    liveSession.PrivateFilled = privateOccupied;
    liveSession.PublicFilled = publicOccupied;
    liveSession.PrivateOpen =
        sessionParams->mPrivateSlots - privateOccupied;
    liveSession.PublicOpen =
        sessionParams->mMaxPlayers - sessionParams->mPrivateSlots
        - publicOccupied;
    HRESULT v11 = liveSession.Create();
    LiveWrapper::HandleError(v11);
    sessionState = kEnteringSession;
}

// ea: 0x721880
void MPLiveEngine::SubmitVoiceData(const XUID* talker, void* buffer,
                                   unsigned int bufferLength)
{
    XHVEngine* voiceEngine = this->voiceEngine;
    if (voiceEngine != nullptr)
    {
        XHVEngine_SubmitIncomingVoicePacket(voiceEngine, *talker, buffer,
                                            bufferLength);
    }
}
