// ============================================================================
// MPLiveEngine.h - multiplayer Xbox Live engine (game_xbox.o MPLiveEngine.cpp)
// Reconstructed from IDA local types (PDB symbol data). All sizes verified.
// ============================================================================

#pragma once

#include "XboxLive.h"
#include "xlive.h"
#include "bd/bd_types.h"
#include "core/mem_heap.h"

// ============================================================================
// SaveGameData - minimal view (shell.o owns the real symbol; array of 4)
// Only the StubData fields used by game_xbox.o are declared; size 0x1BF4.
// ============================================================================
struct SaveGameData {
    unsigned char _pad0[0x310];
    int liveState;                  // +0x310 (StubData.liveState)
    unsigned char _pad1[0x3B4 - 0x314];
    bool savedStateIsValid;         // +0x3B4
    unsigned char _pad2[0x3BD - 0x3B5];
    bool appearOnline;              // +0x3BD
    unsigned char _pad3[0x3BF - 0x3BE];
    bool mDisableSave;              // +0x3BF
    unsigned char _pad4[0x1BF4 - 0x3C0];
};
static_assert(sizeof(SaveGameData) == 0x1BF4, "SaveGameData size mismatch");
extern SaveGameData gSaveGameData[4];

// ============================================================================
// MPPlayerSet - 16-player bitmask (2 bytes, verified)
// ============================================================================
struct MPPlayerSet {
    unsigned short mBitPlayers;     // +0x00

    MPPlayerSet() : mBitPlayers(0) {}
    MPPlayerSet(unsigned int index) { set(index); }
    MPPlayerSet(const MPPlayerSet& set) : mBitPlayers(set.mBitPlayers) {}

    void set(unsigned int index);               // inline, ea 0x72A480
    bool containsPlayer(unsigned int index) const;  // inline, ea 0x72A580
    unsigned int lowestPlayerIndex() const;     // extern mp.o (ea 0x730350)
    unsigned int highestPlayerIndex() const;    // extern mp.o (ea 0x7302D0)
    unsigned short getBitfield() const { return mBitPlayers; }
    void setBitfield(unsigned short v) { mBitPlayers = v; }
};
static_assert(sizeof(MPPlayerSet) == 0x2, "MPPlayerSet size mismatch");

// ============================================================================
// QueryInterface - base for session queries (4 bytes, vtable only)
// Slots verified: 0=Process, 1=Cancel, 3=Done, 5=IsRunning, 6=IsProbing,
// 7=Probe. CFromIDQuery/CDefaultQuery derive from this (mp.o).
// ============================================================================
struct QueryInterface {
    void** vftable;  // +0x00

    HRESULT Process() { return ((HRESULT(__thiscall*)(QueryInterface*))vftable[0])(this); }
    void Cancel() { ((void(__thiscall*)(QueryInterface*))vftable[1])(this); }
    bool Done() { return ((bool(__thiscall*)(QueryInterface*))vftable[3])(this) != 0; }
    bool IsRunning() { return ((bool(__thiscall*)(QueryInterface*))vftable[5])(this) != 0; }
    bool IsProbing() { return ((bool(__thiscall*)(QueryInterface*))vftable[6])(this) != 0; }
    HRESULT Probe() { return ((HRESULT(__thiscall*)(QueryInterface*))vftable[7])(this); }
};

// ============================================================================
// MP network classes (mp.o / shell.o - extern views for MPLiveEngine)
// ============================================================================
struct MPPlayer {
    unsigned char _pad0[0x88];
    XUID xuid;                     // +0x88
};

struct MPPlayerManager;

struct MPPeer {
    MPPlayerManager* GetPlayerManager();  // extern mp.o
};

struct MultiplayerMgr {
    MPPeer* mPeer;                 // +0x00
    static MultiplayerMgr* sInst;  // mp.o data
};

struct MPPlayerManager {
    MPPlayer* GetLocalPlayer(int nLocalPlayer);  // extern mp.o
    MPPlayer* GetPlayer(unsigned char id);       // extern mp.o
    MPPlayerSet allPlayers();                    // extern mp.o
    void Send(bdReference<bdMessage> message, MPPlayerSet players,
              bool reliable);                    // extern mp.o
    void SendOthers(bdReference<bdMessage> message, bool reliable);  // extern mp.o
};

namespace MPUIInterface {
bool IsOnlineGame();            // extern mp.o
bool InSession();               // extern mp.o
void ExitGame();                // extern mp.o
void QueryFromID(XNKID* sessionID);  // extern mp.o
}

// game_xbox.o / game2.o globals used by MPLiveEngine
extern bool g_controllerConnectedErrorShown[];
extern bool g_IgnoreUIXInput;

// ============================================================================
// MPLiveEngine - LiveWrapper + matchmaking session + voice (0x4600, verified)
// ============================================================================
class MPLiveEngine : public LiveWrapper {
public:
    CSession liveSession;             // +0x44D0
    QueryInterface* currentQuery;     // +0x45D0
    unsigned int actualPort;          // +0x45D4
    bool invited;                     // +0x45D8
    MPPlayerSet* mListeners;          // +0x45DC
    XNKID keySet[3];                  // +0x45E0
    int keyIndex;                     // +0x45F8
    unsigned char _pad_end[4];        // +0x45FC (binary size 0x4600)

    MPLiveEngine();
    ~MPLiveEngine();

    bool CanHear(const XUID* talker, const XUID* listener);
    void ClearKeys();
    void DoWork();
    void FreeSlot(bool isPrivate);
    static MPLiveEngine* GetHandle();
    unsigned int GetPortToLock();
    unsigned int GetVoiceData(unsigned int consoleID, unsigned int buffer);
    unsigned int GetVoiceDataSize(unsigned int consoleID);
    bool HandleInput(unsigned int port, const XINPUT_STATE* controllerInput);
    void HandleOutgoingVoice(unsigned int dwLocalPort, unsigned int dwSize,
                             const unsigned char* pData);
    void HandleSessionCreation();
    void JoinGame(XONLINE_FRIEND* joinee);
    void LeaveLiveSession();
    void LogoffCallBack();
    void LogonCallBack();
    bool OccupyPrivateSlot();
    bool OccupyPublicSlot();
    int RegisterKey(const XNKID* sessionID, const XNKEY* securityKey);
    void RunQuery(QueryInterface* newQuery);
    void SendCommunicatorStatus(UIX_VOICE_STATUS_TYPE commStatus);
    void SendMuteUpdate(XUID* muter, XUID* mutee, bool isMuted);
    void SetRemoteListeners(const MPPlayerSet* listeners);
    void StartLiveSession(sServerCreateParams* sessionParams,
                          unsigned char publicOccupied,
                          unsigned char privateOccupied);
    void SubmitVoiceData(const XUID* talker, void* buffer,
                         unsigned int bufferLength);
};
static_assert(sizeof(MPLiveEngine) == 0x4600, "MPLiveEngine size mismatch");
