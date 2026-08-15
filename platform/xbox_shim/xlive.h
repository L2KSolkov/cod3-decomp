// ============================================================================
// xlive.h - Xbox Live / XNet / XHV API surface used by game_xbox.o
// Game code calls these verbatim (Win32 target); implementations are stubbed
// per the plan (recorded in NEEDED.md). The executable links via /FORCE.
// Types reconstructed from IDA local types (PDB symbol data).
// ============================================================================

#pragma once

#include <windows.h>
#include "bd/bdGameInfo.h"

// The Xbox XDK compiles its Live headers at 4-byte packing (verified: XUID
// members sit at +0/+4 in LivePlayer, not +8).
#pragma pack(push, 4)

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// XUID - 12 bytes (verified against IDA)
// ============================================================================
#ifndef XUID_TYPE_DEFINED
#define XUID_TYPE_DEFINED
typedef struct _XUID {
    union {
        ULONGLONG qwValue;
        struct {
            DWORD dwUserID;   // +0x00
            DWORD dwTeamID;   // +0x04
        };
    };
    DWORD dwUserFlags;        // +0x08
} XUID;
#else
typedef struct _XUID XUID;
#endif

// ============================================================================
// XONLINE_FRIEND - 0x56 bytes (verified against IDA)
// ============================================================================
typedef struct _XONLINE_FRIEND {
    XUID xuid;                 // +0x00
    char szGamertag[16];       // +0x0C
    DWORD dwFriendState;       // +0x1C
    FILETIME gameinviteTime;   // +0x20
    XNKID sessionID;           // +0x28
    DWORD dwTitleID;           // +0x30
    unsigned char StateDataSize;  // +0x34
    unsigned char StateData[32];  // +0x35
    unsigned char bReserved;      // +0x55
} XONLINE_FRIEND;

// ============================================================================
// sServerCreateParams - 0x69 bytes (verified against IDA)
// ============================================================================
typedef struct sServerCreateParams {
    char mRandomMapList[64];   // +0x00
    char mName[24];            // +0x40
    unsigned char mMapID;      // +0x58
    unsigned char mGameType;   // +0x59
    unsigned char mGameSubType;// +0x5A
    unsigned char mMaxPlayers; // +0x5B
    unsigned char mTeamBalancing; // +0x5C
    unsigned char mFriendlyFire;  // +0x5D
    unsigned char mPrivateSlots;  // +0x5E
    unsigned char mTimeLimit;     // +0x5F
    unsigned char mScoreLimit;    // +0x60
    unsigned char mRoundLimit;    // +0x61
    unsigned char mSwapEnds;      // +0x62
    unsigned char mRespawnTime;   // +0x63
    bool mDontRotate;             // +0x64
    bool mDoChangeMap;            // +0x65
    unsigned char mEnableAARVote; // +0x66
    unsigned char mEnablePenaltyVote; // +0x67
    unsigned char mMapRotation;       // +0x68
    void SetMapRotation(unsigned char MapRotation);  // mp.o 0x72F3B0
    void Deserialize(bdReference<bdBitBuffer> buffer);  // ?Deserialize@sServerCreateParams@@QAEXV?$bdReference@VbdBitBuffer@@@@@Z (mp.o)
} sServerCreateParams;

// ============================================================================
// XINPUT_STATE - controller state (0x10, matches the XDK layout)
// ============================================================================
typedef struct _XINPUT_GAMEPAD {
    unsigned short wButtons;
    unsigned char bLeftTrigger;
    unsigned char bRightTrigger;
    short sThumbLX;
    short sThumbLY;
    short sThumbRX;
    short sThumbRY;
} XINPUT_GAMEPAD;

typedef struct _XINPUT_STATE {
    unsigned long dwPacketNumber;
    XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE;

// ============================================================================
// XONLINE_ATTRIBUTE - session/search attribute (16 bytes, verified)
// The info union carries an __int64 so the XDK header is 8-byte aligned.
// ============================================================================
#pragma pack(push, 8)
typedef union _XONLINE_ATTRIBUTE_INFO {
    struct {
        ULONGLONG qwValue;         // +0x00
    } integer;
    struct {
        unsigned short* lpValue;   // +0x00
    } string;
    struct {
        void* pvValue;             // +0x00
        DWORD dwLength;            // +0x04
    } blob;
} XONLINE_ATTRIBUTE_INFO;

typedef struct _XONLINE_ATTRIBUTE {
    DWORD dwAttributeID;           // +0x00
    int fChanged;                  // +0x04
    XONLINE_ATTRIBUTE_INFO info;   // +0x08
} XONLINE_ATTRIBUTE;
#pragma pack(pop)

// ============================================================================
// XONLINE_MUTELISTUSER - 16 bytes (verified)
// ============================================================================
typedef struct _XONLINE_MUTELISTUSER {
    XUID xuid;                     // +0x00
    DWORD dwReserved;              // +0x0C
} XONLINE_MUTELISTUSER;

// ============================================================================
// XONLINETASK_HANDLE - opaque task handle (4 bytes, verified)
// ============================================================================
typedef struct XONLINETASK_HANDLE__ {
    int unused;
} XONLINETASK_HANDLE__;
typedef XONLINETASK_HANDLE__* XONLINETASK_HANDLE;

// ============================================================================
// UIX voice / logon enums (values verified against IDA)
// ============================================================================
typedef enum _UIX_VOICE_STATUS_TYPE {
    UIX_VOICE_STATUS_COMMUNICATOR = 0,
    UIX_VOICE_STATUS_SPEAKERS = 1,
    UIX_VOICE_STATUS_NONE = 2,
    UIX_VOICE_STATUS_FORCE_DWORD = 0x7FFFFFFF,
} UIX_VOICE_STATUS_TYPE;

typedef enum _UIX_LOGON_TYPE {
    UIX_LOGON_TYPE_NORMAL = 0,
    UIX_LOGON_TYPE_SILENT = 1,
    UIX_LOGON_TYPE_RETRIEVED_STATE = 2,
    UIX_LOGON_TYPE_RETRIEVED_GAME_INVITE = 3,
    UIX_LOGON_FORCE_DWORD = 0x7FFFFFFF,
} UIX_LOGON_TYPE;

// LiveEngine property ids (XOnline UI engine).
enum {
    UIX_PROPERTY_ALLOW_GAME_INVITES = 3,
};

// ============================================================================
// XOnline matchmaking / task APIs (called verbatim; stub in NEEDED.md)
// ============================================================================
struct LiveEngine;
struct XHVEngine;

// ============================================================================
// XNet QoS / matchmaking result types (verified against IDA)
// ============================================================================
typedef struct _XNQOSINFO {
    unsigned char bFlags;             // +0x00
    unsigned char bReserved;          // +0x01
    unsigned short cProbesXmit;       // +0x02
    unsigned short cProbesRecv;       // +0x04
    unsigned short cbData;            // +0x06
    unsigned char* pbData;            // +0x08
    unsigned short wRttMinInMsecs;    // +0x0C
    unsigned short wRttMedInMsecs;    // +0x0E
    unsigned long dwUpBitsPerSec;     // +0x10
    unsigned long dwDnBitsPerSec;     // +0x14
} XNQOSINFO;

typedef struct _XNQOS {
    unsigned int cxnqos;              // +0x00
    unsigned int cxnqosPending;       // +0x04
    XNQOSINFO axnqosinfo[1];          // +0x08
} XNQOS;

typedef struct _XONLINE_MATCH_SEARCHRESULT {
    unsigned int dwReserved;       // +0x00
    XNKID SessionID;               // +0x04
    XNADDR HostAddress;            // +0x0C
    XNKEY KeyExchangeKey;          // +0x30
    unsigned int dwPublicOpen;     // +0x40
    unsigned int dwPrivateOpen;    // +0x44
    unsigned int dwPublicFilled;   // +0x48
    unsigned int dwPrivateFilled;  // +0x4C
    unsigned int dwNumAttributes;  // +0x50
} _XONLINE_MATCH_SEARCHRESULT;

typedef struct _XONLINE_ATTRIBUTE_SPEC {
    unsigned int dwType;   // +0x00
    unsigned int dwLength; // +0x04
} _XONLINE_ATTRIBUTE_SPEC;

// Matchmaking attribute specs (g.o data; used by CFromIDQuery/CDefaultQuery)
extern _XONLINE_ATTRIBUTE_SPEC FromIDAttributeSpec[7];  // 0xD170E0
extern _XONLINE_ATTRIBUTE_SPEC DefaultAttributeSpec[8]; // 0xD17118

HRESULT __stdcall XOnlineMatchSessionCreate(
    DWORD dwPublicCurrent, DWORD dwPublicAvailable,
    DWORD dwPrivateCurrent, DWORD dwPrivateAvailable,
    DWORD dwNumAttributes, const XONLINE_ATTRIBUTE* pAttributes,
    HANDLE hWorkEvent, XONLINETASK_HANDLE* phTask);
HRESULT __stdcall XOnlineMatchSessionUpdate(
    XNKID sessionID,
    DWORD dwPublicCurrent, DWORD dwPublicAvailable,
    DWORD dwPrivateCurrent, DWORD dwPrivateAvailable,
    DWORD dwNumAttributes, const XONLINE_ATTRIBUTE* pAttributes,
    HANDLE hWorkEvent, XONLINETASK_HANDLE* phTask);
HRESULT __stdcall XOnlineMatchSessionDelete(
    XNKID sessionID, HANDLE hWorkEvent, XONLINETASK_HANDLE* phTask);
HRESULT __stdcall XOnlineMatchSessionGetInfo(
    XONLINETASK_HANDLE hTask, XNKID* pSessionID, XNKEY* pKeyExchangeKey);
DWORD __stdcall XOnlineTaskContinue(XONLINETASK_HANDLE hTask);
HRESULT __stdcall XOnlineTaskClose(XONLINETASK_HANDLE hTask);
HRESULT __stdcall XOnlineMatchSearch(
    DWORD dwProcedureIndex, DWORD dwNumResults, DWORD dwNumAttributes,
    const XONLINE_ATTRIBUTE* pAttributes, DWORD dwResultsLen,
    HANDLE hWorkEvent, XONLINETASK_HANDLE* phTask);
HRESULT __stdcall XOnlineMatchSearchGetResults(
    XONLINETASK_HANDLE hTask,
    _XONLINE_MATCH_SEARCHRESULT** prgpSearchResults,
    DWORD* pdwReturnedResults);
HRESULT __stdcall XOnlineMatchSearchParse(
    _XONLINE_MATCH_SEARCHRESULT* pSearchResult,
    DWORD dwNumSessionAttributes, const void* pSessionAttributeSpec,
    void* pQuerySession);
HRESULT __stdcall XOnlineMatchSearchResultsLen(
    DWORD dwNumResults, DWORD dwNumSessionAttributes,
    const void* pSessionAttributeSpec);
HRESULT __stdcall XOnlineMutelistGet(
    DWORD dwUserIndex, DWORD dwMutelistUserBufferCount, HANDLE hWorkEvent,
    XONLINETASK_HANDLE* phTask, XONLINE_MUTELISTUSER* pMutelistUserBuffer,
    DWORD* pdwNumMutelistUsers);

// ============================================================================
// XNet QoS / key registry
// ============================================================================
int __stdcall XNetQosListen(const XNKID* pxnkid, BYTE* pb, DWORD cb,
                            DWORD dwBitsPerSec, DWORD dwFlags);
int __stdcall XNetQosLookup(
    unsigned int cxna, const XNADDR** apxna, const XNKID** apxnkid,
    const XNKEY** apxnkey, unsigned int cina, const void* aina,
    const DWORD* adwServiceId, unsigned int cProbes, DWORD dwBitsPerSec,
    DWORD dwFlags, HANDLE hEvent, XNQOS** ppxnqos);
int __stdcall XNetQosRelease(XNQOS* pxnqos);
int __stdcall XNetRegisterKey(const XNKID* pxnkid, const XNKEY* pxnkey);
int __stdcall XNetUnregisterKey(const XNKID* pxnkid);

// ============================================================================
// LiveEngine (UIX) helpers
// ============================================================================
void __stdcall LiveEngine_Release(LiveEngine* pThis);
void __stdcall LiveEngine_NotificationSetState(
    LiveEngine* pThis, DWORD Port, DWORD StateFlags, XNKID SessionID,
    DWORD StateDataSize, void* pStateData);
HRESULT __stdcall LiveEngine_SetProperty(LiveEngine* pThis, DWORD Property,
                                         DWORD Value);

// ============================================================================
// XHV voice engine
// ============================================================================
int __stdcall XHVEngine_IsTalking(XHVEngine* pThis, XUID xuidRemoteTalker);
void __stdcall XHVEngine_SetPlaybackPriority(
    XHVEngine* pThis, XUID xuidRemoteTalker, DWORD dwLocalPort,
    DWORD playbackPriority);
HRESULT __stdcall XHVEngine_DoWork(XHVEngine* pThis);
HRESULT __stdcall XHVEngine_RegisterLocalTalker(XHVEngine* pThis,
                                                DWORD dwLocalPort);
void __stdcall XHVEngine_UnregisterRemoteTalker(XHVEngine* pThis,
                                                XUID xuidRemoteTalker);
void __stdcall XHVEngine_SubmitIncomingVoicePacket(
    XHVEngine* pThis, XUID xuidRemoteTalker, void* pvData, DWORD dwSize);
void __stdcall XHVEngine_Release(XHVEngine* pThis);
void __stdcall XHVEngine_SetCallbackInterface(XHVEngine* pThis,
                                              void* pITitleXHV);
void __stdcall XHVEngine_EnableProcessingMode(XHVEngine* pThis,
                                              void* processingMode);
HRESULT __stdcall XHVEngine_RegisterRemoteTalker(XHVEngine* pThis,
                                                 XUID xuidRemoteTalker);
void __stdcall XHVEngine_SetMixBinMapping(XHVEngine* pThis,
                                          XUID xuidRemoteTalker,
                                          DWORD dwLocalPort,
                                          void* pMixBins);
void __stdcall XHVEngine_SetProcessingMode(XHVEngine* pThis,
                                           DWORD dwLocalPort,
                                           void* processingMode);

// DirectSound mix-bin types (AddRemotePlayer)
typedef struct _DSMIXBINVOLUMEPAIR {
    unsigned int dwMixBin;  // +0x00
    int lVolume;            // +0x04
} _DSMIXBINVOLUMEPAIR;
typedef struct _DSMIXBINS {
    unsigned int dwMixBinCount;         // +0x00
    const _DSMIXBINVOLUMEPAIR* lpMixBinVolumePairs;  // +0x04
} _DSMIXBINS;

// ============================================================================
// XHV runtime params (0x24, verified against IDA)
// ============================================================================
typedef struct _XHV_RUNTIME_PARAMS {
    DWORD dwMaxRemoteTalkers;      // +0x00
    DWORD dwMaxLocalTalkers;       // +0x04
    DWORD dwMaxCompressedBuffers;  // +0x08
    DWORD dwFlags;                 // +0x0C
    void* pEffectImageDesc;        // +0x10 (_DSEFFECTIMAGEDESC*)
    DWORD dwEffectsStartIndex;     // +0x14
    DWORD dwOutOfSyncThreshold;    // +0x18
    int bCustomVADProvided;        // +0x1C
    int bHeadphoneAlwaysOn;        // +0x20
} XHV_RUNTIME_PARAMS;

// XHV voice engine creation
HRESULT __stdcall XHVEngineCreate(XHV_RUNTIME_PARAMS* pParams,
                                  XHVEngine** ppEngine);

// ============================================================================
// XOnline startup / logon state
// ============================================================================
typedef struct _XONLINE_LOGON_STATE {
    unsigned char bType;    // +0x00
    unsigned char bVersion; // +0x01
    unsigned short cbSize;  // +0x02
    unsigned char Data[512]; // +0x04
} XONLINE_LOGON_STATE;

typedef struct _XONLINE_USER {
    XUID xuid;                 // +0x00
    char szGamertag[16];       // +0x0C
    unsigned int dwUserOptions; // +0x1C
    unsigned char passcode[4]; // +0x20
    unsigned char reserved[72]; // +0x24
    HRESULT hr;                // +0x6C
} XONLINE_USER;

HRESULT __stdcall XOnlineStartup(void* pxosp);
HRESULT __stdcall XOnlineSaveLogonState(XONLINE_LOGON_STATE* pLogonState);
const XONLINE_USER* __stdcall XOnlineGetLogonUsers(void);
unsigned int __stdcall XGetLanguage(void);

// ============================================================================
// LiveEngine (UIX) extended surface
// ============================================================================
typedef struct LiveFeature {
    int unused;
} LiveFeature;

typedef struct ILivePlayersList {
    void** vftable;
    HRESULT(__stdcall* Refresh)(ILivePlayersList* pThis);
    HRESULT(__stdcall* RegisterPlayer)(ILivePlayersList* pThis, void* player);
    HRESULT(__stdcall* UnregisterPlayer)(ILivePlayersList* pThis,
                                         void* player);
} ILivePlayersList;

HRESULT __stdcall UIXCreateLiveEngine(const char* pSkinFileName,
                                      unsigned int LanguageID,
                                      LiveEngine** ppEngine);
HRESULT __stdcall UIXCreateUIPlugin(void* pFont, void** ppUIPlugin);
HRESULT __stdcall LiveEngine_DoWork(LiveEngine* pThis, DWORD* pDoWorkFlags);
HRESULT __stdcall LiveEngine_Reboot(LiveEngine* pThis, DWORD Context);
HRESULT __stdcall LiveEngine_LogOff(LiveEngine* pThis);
HRESULT __stdcall LiveEngine_UseVoiceMail(LiveEngine* pThis,
                                          LiveFeature* VoiceMailEntryPoint);
HRESULT __stdcall LiveEngine_EndFeature(LiveEngine* pThis);
HRESULT __stdcall LiveEngine_EnableFeature(LiveEngine* pThis,
                                           LiveFeature* FeatureID);
HRESULT __stdcall LiveEngine_GetFeatureInterface(
    LiveEngine* pThis, LiveFeature* FeatureID, void* pParam,
    void** ppFeatureInterface);
HRESULT __stdcall LiveEngine_StartFeature(LiveEngine* pThis,
                                          LiveFeature* FeatureID,
                                          DWORD* pFeatureParams);
HRESULT __stdcall LiveEngine_SetInput(LiveEngine* pThis, DWORD Port,
                                      const XINPUT_STATE* pInputState);
HRESULT __stdcall LiveEngine_Render(LiveEngine* pThis, void* pSurface);
HRESULT __stdcall LiveEngine_GetNotifications(LiveEngine* pThis, DWORD Port,
                                              DWORD Purpose, DWORD* pNotifications);
HRESULT __stdcall LiveEngine_GetExitInfo(LiveEngine* pThis,
                                         void* pExitInfo);
HRESULT __stdcall LiveEngine_SetUIPlugin(LiveEngine* pThis, void* pUIPlugin);

// UIX notification purposes (GetIcon uses the MENU icon)
enum {
    UIX_NOTIFICATION_MENU = 1,
};

// UIX property ids (values verified against disassembly)
enum {
    UIX_PROPERTY_DISPLAY_CONNECTION_ERRORS = 2,
    UIX_PROPERTY_VOICE_MAIL_ENGINE = 4,
    UIX_PROPERTY_VOICE_MAIL_TO_SPEAKERS = 5,
};

#ifdef __cplusplus
}
#endif

#pragma pack(pop)
