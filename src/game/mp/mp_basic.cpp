// ============================================================================
// mp_basic.cpp - multiplayer game modes (mp.o) batch 1: small helpers
// Reconstructed from IDA decompiles/disasm of the release XBE.
// ============================================================================

#include "game/logic/g_local.h"
#include "game/mp/mp_types.h"

#pragma comment(lib, "ws2_32.lib")

int XNetCleanup();  // Xbox shim; Win32 no-op
int XNetCleanup()
{
    return 0;
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

// ea: 0x0072E190
MPVehicle::~MPVehicle()
{
}

// ============================================================================
// MPLanDiscovery (mp.o 0x72CB90)
// ============================================================================
unsigned int MPLanDiscovery::GetNumResults() const
{
    return mNumResults;
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

void MPUIInterface::getLocalAddresses(bdArray<bdInetAddr>&)
{
}

void MPUIInterface::Logging(bool)
{
}

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

// ea: 0x0072C890 (mSession at +0x7448)
bdSession::bdSessionStatus MPPeer::GetSessionStatus() const
{
    return ((bdSession*)((char*)this + 0x7448))->getStatus();
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
