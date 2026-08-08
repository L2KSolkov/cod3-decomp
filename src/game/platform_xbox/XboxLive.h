// ============================================================================
// XboxLive.h - Xbox Live wrapper / session / live-player classes
// Source: game_xbox.o (LiveWrapper.cpp + xboxMatch.cpp + livePlayer.h)
// Reconstructed from IDA local types. All sizes verified.
// ============================================================================

#pragma once

#include "xlive.h"
#include "bd/bdGameInfo.h"

// ============================================================================
// CBlob - 8 bytes (verified against IDA)
// ============================================================================
#pragma pack(push, 4)

struct CBlob {
    unsigned short m_wLength;  // +0x00
    unsigned short pad;        // +0x02
    void* m_pvData;            // +0x04
};
static_assert(sizeof(CBlob) == 0x8, "CBlob size mismatch");

// ============================================================================
// LivePlayer - base live player record, 0x40 (verified)
// ============================================================================
struct ITitlePlayersListItem {
    void** vftable;  // +0x00 (interface base, vtable only)
};

struct LivePlayer : ITitlePlayersListItem {
    XUID xuid;                        // +0x04
    unsigned short gamertag[16];      // +0x10
    unsigned char consoleID;          // +0x30
    UIX_VOICE_STATUS_TYPE voiceStatus; // +0x34
    unsigned int bitflags;            // +0x38
    unsigned int notificationFlags;   // +0x3C

    void Reset();
    // NOTE: IsTalking is virtual in the binary (dispatched through the
    // ITitlePlayersListItem vtable); declared non-virtual here so the 0x40
    // layout stays exact. The vtable is reconstructed with MPLiveEngine.
    bool __stdcall IsTalking();
};
static_assert(sizeof(LivePlayer) == 0x40, "LivePlayer size mismatch");

// ============================================================================
// LiveRemote - remote player, 0x48 (verified)
// ============================================================================
struct LiveRemote : LivePlayer {
    bool muting[4];    // +0x40
    bool mutedBy[4];   // +0x44

    void CopyLivePlayer(const LivePlayer* toCopy);
    void Reset();
    inline bool IsMuting(unsigned int portNum);
    inline bool IsMutedBy(unsigned int portNum);
};
static_assert(sizeof(LiveRemote) == 0x48, "LiveRemote size mismatch");

// ============================================================================
// LiveLocal - local player, 0xFE8 (verified)
// ============================================================================
struct LiveLocal : LivePlayer {
    bool hasHeadset;                // +0x40
    unsigned int muteListSize;      // +0x44
    XONLINE_MUTELISTUSER muteList[250];  // +0x48

    bool OnMutelist(const XUID* remoteID);
    void Reset();
};
static_assert(sizeof(LiveLocal) == 0xFE8, "LiveLocal size mismatch");

// ============================================================================
// LiveTask - async mute-list task state, 0xC (verified)
// ============================================================================
struct LiveTask {
    bool needsWork;               // +0x00
    XONLINETASK_HANDLE__* taskHandle;  // +0x04
    HRESULT lastResult;           // +0x08
};
static_assert(sizeof(LiveTask) == 0xC, "LiveTask size mismatch");

// ============================================================================
// LiveWrapper - Xbox Live wrapper, 0x44CC (verified)
// ============================================================================
struct ITitleXHV {
    void** vftable;  // +0x00 (interface base, vtable only)
};

enum ELiveState {
    ELIVE_STATE_UNINITIALIZED = 0,
};

class LiveWrapper : public ITitleXHV {
public:
    int internalState;              // +0x04
    int internalMode;               // +0x08
    int sessionState;               // +0x0C
    int lastNotification;           // +0x10
    LiveEngine* uixEngine;          // +0x14
    void* uiPlugin;                 // +0x18 (ITitleUIPlugin*)
    void* audioPlugin;              // +0x1C (ITitleAudioPlugin*)
    void* uixFont;                  // +0x20 (ITitleFontRenderer*)
    void* uixPlayersList;           // +0x24 (ILivePlayersList*)
    void* backBuffer;               // +0x28 (D3DSurface*)
    void* screen;                   // +0x2C (D3DDevice*)
    LiveLocal localPlayers[4];      // +0x30
    LiveRemote remotePlayers[15];   // +0x3FD0
    short numRemotePlayers;         // +0x4408
    void* loggedInUsers;            // +0x440C (XONLINE_USER*)
    XNKID sessionID;                // +0x4410
    XNKEY commKey;                  // +0x4418
    bool playersListActive;         // +0x4428
    unsigned int uixFlags;          // +0x442C
    unsigned int activeController;  // +0x4430
    XHVEngine* voiceEngine;         // +0x4434
    bool anyoneBanned;              // +0x4438
    LiveTask muteListTask[4];       // +0x443C
    HRESULT lastLoginCode;          // +0x446C
    UIX_LOGON_TYPE logonMethod;     // +0x4470
    bool needConfirmation;          // +0x4474
    unsigned char friendToJoin[0x56];  // +0x4475 (XONLINE_FRIEND)
    bool renderingEnabled;          // +0x44CB

    static LiveWrapper* theWrapper; // ?theWrapper@LiveWrapper@@1PAV1@A

    ~LiveWrapper();
    LiveLocal* GetLocalPlayer(unsigned int portNumber);
    void SetNotificationFlag(unsigned int portNumber, unsigned int flagID,
                             bool flagState);
    bool GetNotificationFlag(unsigned int portNumber, unsigned int flagID);
    void ToggleNotificationFlag(unsigned int portNumber, unsigned int flagID);
    void SetSessionID(const XNKID* newSessionID);
    bool IsTalking(XUID talker, unsigned int controllerIndex);
    HRESULT __stdcall LocalChatDataReady(unsigned int dwLocalPort,
                                         unsigned int dwSize, void* pData);
    HRESULT __stdcall VoiceMailDataReady(unsigned int dwLocalPort,
                                         unsigned int dwDuration,
                                         unsigned int dwSize);
    HRESULT __stdcall VoiceMailStopped(unsigned int dwLocalPort);
    void RefreshMuteList(unsigned int controllerIndex);
};
static_assert(sizeof(LiveWrapper) == 0x44CC, "LiveWrapper size mismatch");

#pragma pack(pop)

// ============================================================================
// CSession - matchmaking session state machine, 0x100 (verified)
// ============================================================================
class CSession {
public:
    enum STATE {
        STATE_IDLE = 0,
        STATE_CREATING = 1,
        STATE_UPDATING = 2,
        STATE_DELETING = 3,
        STATE_ACTIVE = 4,
    };

    struct QosQEntry {
        XNKID SessionID;          // +0x00
        unsigned int dwStartTick; // +0x08
        QosQEntry* pNext;         // +0x0C
    };
    static_assert(sizeof(QosQEntry) == 0x10, "QosQEntry size mismatch");

    struct CSessionQosQ {
        QosQEntry* m_pHead;  // +0x00
        QosQEntry* m_pTail;  // +0x04

        CSessionQosQ();
        void Add(XNKID* SessionID, unsigned int dwStartTick);
        void Remove(XNKID* SessionID);
        void Dequeue();
    };
    static_assert(sizeof(CSessionQosQ) == 0x8, "CSessionQosQ size mismatch");

    unsigned int PublicFilled;      // +0x00
    unsigned int PublicOpen;        // +0x04
    unsigned int PrivateFilled;     // +0x08
    unsigned int PrivateOpen;       // +0x0C
    XNKEY KeyExchangeKey;           // +0x10
    XNKID SessionID;                // +0x20
    XONLINE_ATTRIBUTE m_Attributes[9];  // +0x28
    unsigned short m_strhost_name[17];  // +0xB8
    CSessionQosQ m_SessionQosQ;     // +0xDC
    int m_bListening;               // +0xE4
    CBlob m_QosResponse;            // +0xE8
    XONLINETASK_HANDLE__* m_hSessionTask;  // +0xF0
    STATE m_State;                  // +0xF4
    int m_bKeyRegistered;           // +0xF8
    int m_bUpdate;                  // +0xFC

    CSession();
    ~CSession();
    HRESULT Create();
    HRESULT Delete();
    HRESULT Update();
    HRESULT Process();
    void Reset();
    void Close();
    void Listen(int bEnable, unsigned int dwBitsPerSec);
    void SetQosResponse(CBlob Value);
    CBlob GetQosResponse();

    // Attribute accessors
    unsigned __int64 Getgame_type();
    void Setgame_type(unsigned __int64 Value);
    unsigned __int64 Getgame_map();
    void Setgame_map(unsigned __int64 Value);
    unsigned __int64 Getgame_version();
    void Setgame_version(unsigned __int64 Value);
    unsigned __int64 Getfriendly_fire();
    void Setfriendly_fire(unsigned __int64 Value);
    unsigned __int64 Getteam_balancing();
    void Setteam_balancing(unsigned __int64 Value);
    unsigned __int64 Getsub_type();
    void Setsub_type(unsigned __int64 Value);
    unsigned __int64 Getnum_players();
    void Setnum_players(unsigned __int64 Value);
    const unsigned short* Gethost_name();
    void Sethost_name(const unsigned short* Value);
    unsigned __int64 Getmax_players();
    void Setmax_players(unsigned __int64 Value);

private:
    void SetupAttributes();
    HRESULT ProcessStateUpdateSession();
    HRESULT ProcessStateDeleteSession();
    HRESULT ProcessStateActiveSession();
    HRESULT ProcessStateCreateSession();
    void PurgeSessionQHead();
    void PurgeSessionQ(int fRemoveAll);
};
static_assert(sizeof(CSession) == 0x100, "CSession size mismatch");
