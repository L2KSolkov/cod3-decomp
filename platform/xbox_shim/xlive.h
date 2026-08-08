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

// ============================================================================
// XONLINE_ATTRIBUTE - session/search attribute (16 bytes, verified)
// ============================================================================
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
HRESULT __stdcall XOnlineMutelistGet(
    DWORD dwUserIndex, DWORD dwMutelistUserBufferCount, HANDLE hWorkEvent,
    XONLINETASK_HANDLE* phTask, XONLINE_MUTELISTUSER* pMutelistUserBuffer,
    DWORD* pdwNumMutelistUsers);

// ============================================================================
// XNet QoS / key registry
// ============================================================================
int __stdcall XNetQosListen(const XNKID* pxnkid, BYTE* pb, DWORD cb,
                            DWORD dwBitsPerSec, DWORD dwFlags);
int __stdcall XNetRegisterKey(const XNKID* pxnkid, const XNKEY* pxnkey);
int __stdcall XNetUnregisterKey(const XNKID* pxnkid);

// ============================================================================
// LiveEngine (UIX) helpers
// ============================================================================
void __stdcall LiveEngine_Release(LiveEngine* pThis);
void __stdcall LiveEngine_NotificationSetState(
    LiveEngine* pThis, DWORD Port, DWORD StateFlags, XNKID SessionID,
    DWORD StateDataSize, void* pStateData);
void __stdcall LiveEngine_SetProperty(LiveEngine* pThis, DWORD Property,
                                      DWORD Value);

// ============================================================================
// XHV voice engine
// ============================================================================
int __stdcall XHVEngine_IsTalking(XHVEngine* pThis, XUID xuidRemoteTalker);
void __stdcall XHVEngine_SetPlaybackPriority(
    XHVEngine* pThis, XUID xuidRemoteTalker, DWORD dwLocalPort,
    DWORD playbackPriority);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)
