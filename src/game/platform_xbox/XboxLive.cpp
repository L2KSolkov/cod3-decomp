// ============================================================================
// XboxLive.cpp - Xbox Live wrapper / session / live-player classes
// Source: game_xbox.o (LiveWrapper.cpp + xboxMatch.cpp + livePlayer.h)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// Xbox Live APIs are called verbatim; shims are recorded in NEEDED.md and
// the executable links with /FORCE:UNRESOLVED until they are implemented.
// ============================================================================

#include "XboxLive.h"
#include "d3d8.h"
#include "xlive.h"
#include "core/mem_heap.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

// Minimal FEManager view (shell.o owns the real class; only DrawDiscError
// is referenced here). shell.o:0x96CDA0 ?DrawDiscError@FEManager@@QAEXXZ.
class FEManager {
public:
    void DrawDiscError();
};
extern FEManager g_femanager;

// ============================================================================
// Print - debug output for the matchmaking code (game_xbox.o local helper)
// ea: 0x7203A0
// ============================================================================
static void Print(const wchar_t* strFormat, ...)
{
    wchar_t strBuffer[80];
    va_list arglist;
    va_start(arglist, strFormat);
    if (wvsprintfW(strBuffer, strFormat, arglist) >= 80)
    {
        ASSERT("iChars < MAX_OUTPUT_STR",
               "c:\\cod\\code\\game\\xboxMatch.cpp", 27);
    }
    va_end(arglist);
    OutputDebugStringW(L"\n");
    OutputDebugStringW(strBuffer);
    OutputDebugStringW(L"\n\n");
}

// ============================================================================
// LivePlayer / LiveRemote / LiveLocal
// ============================================================================

// ea: 0x7216D0
void LivePlayer::Reset()
{
    xuid.qwValue = 0;
    xuid.dwUserFlags = 0;
    memset(gamertag, 0, sizeof(gamertag));
    consoleID = 0;
    voiceStatus = UIX_VOICE_STATUS_NONE;
    notificationFlags = 0;
    bitflags = 0;
}

// ea: 0x721740
bool __stdcall LivePlayer::IsTalking()
{
    return LiveWrapper::theWrapper->IsTalking(
        xuid, LiveWrapper::theWrapper->activeController);
}

// ea: 0x721750
void LiveRemote::CopyLivePlayer(const LivePlayer* toCopy)
{
    xuid = toCopy->xuid;
    consoleID = toCopy->consoleID;
    voiceStatus = toCopy->voiceStatus;
    bitflags = toCopy->bitflags;
    notificationFlags = toCopy->notificationFlags;
    memcpy(gamertag, toCopy->gamertag, sizeof(gamertag));
}

// ea: 0x7217A0
void LiveRemote::Reset()
{
    LivePlayer::Reset();
    memset(muting, 0, sizeof(muting));
    memset(mutedBy, 0, sizeof(mutedBy));
}

// ea: inline (livePlayer.h)
void LiveRemote::SetMuted(unsigned int portNum, bool shouldMute)
{
    mutedBy[portNum] = shouldMute;
}

// ea: inline (livePlayer.h)
void LiveRemote::Mute(unsigned int portNum, bool shouldMute)
{
    muting[portNum] = shouldMute;
}

// ea: 0x729F00 (inline, livePlayer.h:66)
inline bool LiveRemote::IsMuting(unsigned int portNum)
{
    if (portNum >= 4)
    {
        ASSERT("portNum < 4", "c:\\cod\\code\\game\\livePlayer.h", 66);
    }
    return muting[portNum];
}

// ea: 0x729F80 (inline, livePlayer.h:71)
inline bool LiveRemote::IsMutedBy(unsigned int portNum)
{
    if (portNum >= 4)
    {
        ASSERT("portNum < 4", "c:\\cod\\code\\game\\livePlayer.h", 71);
    }
    return mutedBy[portNum];
}

// ea: 0x7217E0
bool LiveLocal::OnMutelist(const XUID* remoteID)
{
    unsigned int muteListSize = this->muteListSize;
    unsigned int v3 = 0;
    if (muteListSize == 0)
        return 0;
    for (XONLINE_MUTELISTUSER* i = muteList;
         (unsigned int)i->xuid.qwValue != (unsigned int)remoteID->qwValue
             || (unsigned int)(i->xuid.qwValue >> 32)
                    != (unsigned int)(remoteID->qwValue >> 32);
         ++i)
    {
        if (++v3 >= muteListSize)
            return 0;
    }
    return 1;
}

// ea: 0x721830
void LiveLocal::Reset()
{
    LivePlayer::Reset();
    hasHeadset = false;
    muteListSize = 0;
}

// ============================================================================
// LiveWrapper
// ============================================================================

LiveWrapper* LiveWrapper::theWrapper;

// LiveWrapper vtable (slots 0..19; matches the binary's ??_7LiveWrapper@@6B@
// at rdata 0xD173BC). Pure-virtual/placeholder slots are nullsub or purecall.
static void* LiveWrapperVftable[20];
struct LiveWrapperVftableInit {
    LiveWrapperVftableInit() {
        for (int i = 0; i < 20; ++i)
            LiveWrapperVftable[i] = nullptr;
        // slots 18/19 are the nullsub_302/303 callbacks in the binary
    }
};
static LiveWrapperVftableInit s_liveWrapperVftableInit;

// ea: 0x71F7A0
LiveWrapper::~LiveWrapper()
{
    *(void***)this = LiveWrapperVftable;
    if (uixFont)
    {
        void** vt = *(void***)uixFont;
        ((void(__stdcall*)(void*))vt[0])(uixFont);  // ITitleFontRenderer::Release
    }
    if (backBuffer)
        D3DResource_Release((D3DResource*)backBuffer);
    if (uiPlugin)
    {
        void** vt = *(void***)uiPlugin;
        ((void(__stdcall*)(void*))vt[0])(uiPlugin);  // ITitleUIPlugin::Release
    }
    if (audioPlugin)
    {
        void** vt = *(void***)audioPlugin;
        ((void(__stdcall*)(void*))vt[0])(audioPlugin);  // ITitleAudioPlugin::Release
    }
    if (uixEngine)
        LiveEngine_Release(uixEngine);
    if (!theWrapper)
    {
        ASSERT("theWrapper && \"If this pops, somehow the wrapper was already destroyed.\"",
               "c:\\cod\\code\\game\\LiveWrapper.cpp", 40);
    }
    theWrapper = nullptr;
}

// ea: 0x71F840
LiveLocal* LiveWrapper::GetLocalPlayer(unsigned int portNumber)
{
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        return localPlayers;
    return &localPlayers[portNumber];
}

// ea: 0x71F870
void LiveWrapper::SetNotificationFlag(unsigned int portNumber,
                                      unsigned int flagID, bool flagState)
{
    unsigned int v5;
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        v5 = 0;
    else
        v5 = portNumber;
    if (flagState)
    {
        unsigned int* p_notificationFlags =
            &localPlayers[v5].notificationFlags;
        if ((flagID & *p_notificationFlags) != flagID)
        {
            unsigned int v7 = flagID | *p_notificationFlags;
            *p_notificationFlags = v7;
            LiveEngine_NotificationSetState(uixEngine, v5, v7, sessionID,
                                            0, nullptr);
            if ((flagID & 0x10) != 0)
                LiveEngine_SetProperty(uixEngine,
                                       UIX_PROPERTY_ALLOW_GAME_INVITES, 1);
        }
    }
    else
    {
        unsigned int notificationFlags = localPlayers[v5].notificationFlags;
        if ((notificationFlags & flagID) != 0)
        {
            localPlayers[v5].notificationFlags = notificationFlags & ~flagID;
            LiveEngine_NotificationSetState(uixEngine, v5,
                                            notificationFlags & ~flagID,
                                            sessionID, 0, nullptr);
            if ((flagID & 0x10) != 0)
                LiveEngine_SetProperty(uixEngine,
                                       UIX_PROPERTY_ALLOW_GAME_INVITES, 0);
        }
    }
}

// ea: 0x71F940
bool LiveWrapper::GetNotificationFlag(unsigned int portNumber,
                                      unsigned int flagID)
{
    unsigned int v3;
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        v3 = 0;
    else
        v3 = portNumber;
    return (flagID & localPlayers[v3].notificationFlags) != 0;
}

// ea: 0x71F970
void LiveWrapper::ToggleNotificationFlag(unsigned int portNumber,
                                         unsigned int flagID)
{
    unsigned int v3;
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        v3 = 0;
    else
        v3 = portNumber;
    unsigned int* p_notificationFlags = &localPlayers[v3].notificationFlags;
    unsigned int v5 = *p_notificationFlags;
    if ((*p_notificationFlags & flagID) != 0)
        *p_notificationFlags = v5 & ~flagID;
    else
        *p_notificationFlags = flagID | v5;
    LiveEngine_NotificationSetState(uixEngine, v3, *p_notificationFlags,
                                    sessionID, 0, nullptr);
}

// ea: 0x71F9D0
void LiveWrapper::SetSessionID(const XNKID* newSessionID)
{
    sessionID = *newSessionID;
    for (int v3 = 0; v3 < 4; ++v3)
    {
        // disasm: eax=[ebx-38h]; or eax,[ebx-34h] -> (voiceStatus|bitflags)!=0
        if (localPlayers[v3].voiceStatus != 0
            || localPlayers[v3].bitflags != 0)
        {
            LiveEngine_NotificationSetState(uixEngine, v3,
                                            localPlayers[v3].notificationFlags,
                                            sessionID, 0, nullptr);
        }
    }
}

// ea: 0x71FA30
bool LiveWrapper::IsTalking(XUID talker, unsigned int controllerIndex)
{
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        controllerIndex = 0;
    if (localPlayers[controllerIndex].voiceStatus == UIX_VOICE_STATUS_NONE)
        return 0;
    if ((localPlayers[controllerIndex].xuid.dwUserFlags & 0x10003) != 0)
        return 0;
    XHVEngine* voiceEngine = this->voiceEngine;
    if (voiceEngine == nullptr
        || XHVEngine_IsTalking(voiceEngine, talker) == 0)
    {
        return 0;
    }
    int v4 = 0;
    for (LiveRemote* i = remotePlayers;
         talker.qwValue != i->xuid.qwValue
             || (((unsigned char)talker.dwUserFlags
                  ^ (unsigned char)i->xuid.dwUserFlags) & 3) != 0
             || i->IsMutedBy(controllerIndex)
             || i->IsMuting(controllerIndex);
         ++i)
    {
        if (++v4 >= 15)
            return 0;
    }
    return 1;
}

// ea: 0x71FAF0 - virtual dispatch through ITitleXHV vtable slot 0x3C
HRESULT __stdcall LiveWrapper::LocalChatDataReady(unsigned int dwLocalPort,
                                                  unsigned int dwSize,
                                                  void* pData)
{
    void** vt = *(void***)this;
    typedef void(__stdcall* HandleOutgoingVoiceFn)(void*, unsigned int,
                                                   unsigned int, void*);
    ((HandleOutgoingVoiceFn)vt[0x3C / 4])(this, dwLocalPort, dwSize, pData);
    return 0;
}

// ea: 0x71FB10
HRESULT __stdcall LiveWrapper::VoiceMailDataReady(unsigned int dwLocalPort,
                                                  unsigned int dwDuration,
                                                  unsigned int dwSize)
{
    (void)dwLocalPort;
    (void)dwDuration;
    (void)dwSize;
    return 0;
}

// ea: 0x71FB20
HRESULT __stdcall LiveWrapper::VoiceMailStopped(unsigned int dwLocalPort)
{
    (void)dwLocalPort;
    return 0;
}

// ea: 0x71FDC0
void LiveWrapper::RefreshMuteList(unsigned int controllerIndex)
{
    unsigned int v3;
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        v3 = 0;
    else
        v3 = controllerIndex;
    XOnlineMutelistGet(v3, 0xFA, nullptr,
                       &muteListTask[v3].taskHandle,
                       localPlayers[v3].muteList,
                       (DWORD*)&localPlayers[v3].muteListSize);
    muteListTask[v3].needsWork = true;
}

// ============================================================================
// LiveWrapper batch 2 - voice / logon / session lifecycle
// ============================================================================

// Feature descriptors (LiveEngine feature ids; game_xbox.o rdata)
LiveFeature g_LogonFeature;
LiveFeature g_FriendsFeature;
LiveFeature g_PlayersFeature;
LiveFeature g_VoiceMailPseudoFeature;
void* nsl_fxDesc;
void* g_voicemailMode;
void* g_voicechatMode;

// ea: 0x722530
LiveWrapper::LiveWrapper()
{
    *(void***)this = LiveWrapperVftable;
    internalState = 0;        // kNotSignedIn
    internalMode = 0;         // kNotSetup
    sessionState = 0;         // kNotInSession
    lastNotification = 0;     // kLiveOk
    uixEngine = nullptr;
    uiPlugin = nullptr;
    audioPlugin = nullptr;
    uixFont = nullptr;
    for (int i = 0; i < 4; ++i)
        localPlayers[i].Reset();
    for (int j = 0; j < 15; ++j)
        remotePlayers[j].Reset();
    numRemotePlayers = 0;
    playersListActive = false;
    uixFlags = 0;
    voiceEngine = nullptr;
    anyoneBanned = false;
    for (int k = 0; k < 4; ++k)
    {
        muteListTask[k].needsWork = false;
        muteListTask[k].taskHandle = nullptr;
    }
    lastLoginCode = 0;
    logonMethod = UIX_LOGON_FORCE_DWORD;
    needConfirmation = false;
    renderingEnabled = true;
    if (theWrapper != nullptr)
    {
        ASSERT("(!theWrapper) && \"If this pops, the singleton wrapper already exists.\"",
               "c:\\cod\\code\\game\\LiveWrapper.cpp", 19);
    }
    theWrapper = this;
    memset(sessionID.ab, 0, sizeof(sessionID.ab));
    memset(commKey.ab, 0, sizeof(commKey.ab));
}

// ea: 0x7226A0
void LiveWrapper::SetupAsAware(void* renderDevice, const char* skinPath,
                               void* font)
{
    HRESULT v5 = XOnlineStartup(nullptr);
    HandleError(v5);
    unsigned int v6 = XGetLanguage();
    if (v6 == 5)
        v6 = 4;
    if (UIXCreateLiveEngine(skinPath, v6, &uixEngine) != 0)
        g_femanager.DrawDiscError();
    HRESULT v8 = UIXCreateUIPlugin(font, &uiPlugin);
    HandleError(v8);
    uixFont = font;
    screen = renderDevice;
    backBuffer = (void*)D3DDevice_GetBackBuffer2(0);
    void* uiPlugin = this->uiPlugin;
    audioPlugin = nullptr;
    HRESULT v11 = LiveEngine_SetUIPlugin(uixEngine, uiPlugin);
    HandleError(v11);
    HRESULT v12 = LiveEngine_EnableFeature(uixEngine, &g_LogonFeature);
    HandleError(v12);
    HRESULT v13 = LiveEngine_EnableFeature(uixEngine, &g_FriendsFeature);
    HandleError(v13);
    HRESULT v14 = LiveEngine_SetProperty(uixEngine,
                                         UIX_PROPERTY_ALLOW_GAME_INVITES, 0);
    HandleError(v14);
    internalMode = kAware;
}

// ea: 0x722780
void LiveWrapper::SetupAsSession(void* renderDevice, const char* skinPath,
                                 void* font)
{
    HRESULT v5 = XOnlineStartup(nullptr);
    HandleError(v5);
    unsigned int v6 = XGetLanguage();
    if (v6 == 5)
        v6 = 4;
    HRESULT v7 = UIXCreateLiveEngine(skinPath, v6, &uixEngine);
    HandleError(v7);
    HRESULT v8 = UIXCreateUIPlugin(font, &uiPlugin);
    HandleError(v8);
    uixFont = font;
    screen = renderDevice;
    backBuffer = (void*)D3DDevice_GetBackBuffer2(0);
    void* uiPlugin = this->uiPlugin;
    audioPlugin = nullptr;
    HRESULT v11 = LiveEngine_SetUIPlugin(uixEngine, uiPlugin);
    HandleError(v11);
    HRESULT v12 = LiveEngine_EnableFeature(uixEngine, &g_LogonFeature);
    HandleError(v12);
    HRESULT v13 = LiveEngine_EnableFeature(uixEngine, &g_FriendsFeature);
    HandleError(v13);
    HRESULT v14 = LiveEngine_EnableFeature(uixEngine, &g_PlayersFeature);
    HandleError(v14);
    HRESULT v15 = LiveEngine_SetProperty(uixEngine,
                                         UIX_PROPERTY_ALLOW_GAME_INVITES, 0);
    HandleError(v15);
    internalMode = kSession;
}

// ea: 0x722B10
void LiveWrapper::SignInSilently(unsigned int serviceBitfield)
{
    DWORD params[21];
    params[1] = 1;
    params[2] = 1;
    params[0] = 84;
    params[20] = 0;
    params[19] = 0;
    int v2 = 0;
    for (unsigned int i = 0; i < 0x10; ++i)
    {
        if (((1 << i) & serviceBitfield) != 0)
            params[v2++ + 3] = i;
    }
    params[v2 + 3] = 0;
    HRESULT v5 = LiveEngine_SetProperty(uixEngine,
                                        UIX_PROPERTY_DISPLAY_CONNECTION_ERRORS,
                                        0);
    HandleError(v5);
    HRESULT started = LiveEngine_StartFeature(uixEngine, &g_LogonFeature,
                                              params);
    HandleError(started);
    internalState = kSigningIn;
}

// ea: 0x722F80
void LiveWrapper::RemoteMute(unsigned int talkerPort, const XUID* listenerID,
                             bool shouldMute)
{
    unsigned int v5;
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        v5 = 0;
    else
        v5 = talkerPort;
    int v6 = 0;
    XUID* i;
    for (i = &remotePlayers[0].xuid;
         (unsigned int)listenerID->qwValue != (unsigned int)i->qwValue
             || (unsigned int)(listenerID->qwValue >> 32)
                    != (unsigned int)(i->qwValue >> 32);
         i += 6)
    {
        if (++v6 >= 15)
            return;
    }
    remotePlayers[v6].Mute(v5, shouldMute);
    CalcVoicePriorities();
}

// ea: 0x722FF0
void LiveWrapper::SetRemoteVoiceComm(const XUID* remoteID,
                                     UIX_VOICE_STATUS_TYPE commStatus)
{
    int v4 = 0;
    XUID* i;
    for (i = &remotePlayers[0].xuid;
         (unsigned int)remoteID->qwValue != (unsigned int)i->qwValue
             || (unsigned int)(remoteID->qwValue >> 32)
                    != (unsigned int)(i->qwValue >> 32);
         i += 6)
    {
        if (++v4 >= 15)
            return;
    }
    remotePlayers[v4].voiceStatus = commStatus;
    CalcVoicePriorities();
    if (playersListActive)
    {
        HRESULT v6 = uixPlayersList->Refresh(uixPlayersList);
        HandleError(v6);
    }
}

// ea: 0x723320
void LiveWrapper::RemoveRemotePlayer(const XUID* remotePlayer)
{
    int v3 = 0;
    XUID* i;
    for (i = &remotePlayers[0].xuid;
         (unsigned int)i->qwValue != (unsigned int)remotePlayer->qwValue
             || (unsigned int)(i->qwValue >> 32)
                    != (unsigned int)(remotePlayer->qwValue >> 32);
         i += 6)
    {
        if (++v3 >= 15)
            return;
    }
    --numRemotePlayers;
    if ((remotePlayer->dwUserFlags & 0x10003) == 0)
    {
        XHVEngine* voiceEngine = this->voiceEngine;
        if (voiceEngine != nullptr)
            XHVEngine_UnregisterRemoteTalker(voiceEngine, *remotePlayer);
    }
    LiveRemote* v6 = &remotePlayers[v3];
    uixPlayersList->UnregisterPlayer(uixPlayersList, v6);
    v6->Reset();
    if (playersListActive)
    {
        HRESULT v7 = uixPlayersList->Refresh(uixPlayersList);
        HandleError(v7);
    }
}

// ea: 0x7233D0
void LiveWrapper::ClearRemotePlayers()
{
    for (int i = 0; i < 15; ++i)
    {
        if (remotePlayers[i].xuid.qwValue != 0)
            RemoveRemotePlayer(&remotePlayers[i].xuid);
    }
}

// ea: 0x723400
void LiveWrapper::SetVTS(unsigned int controllerIndex, bool vtsOn)
{
    unsigned int v4;
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        v4 = 0;
    else
        v4 = controllerIndex;
    unsigned int dwUserFlags = localPlayers[v4].xuid.dwUserFlags;
    LiveLocal* v7 = &localPlayers[v4];
    if ((dwUserFlags & 0x10003) == 0)
    {
        UIX_VOICE_STATUS_TYPE voiceStatus = v7->voiceStatus;
        if (vtsOn != (voiceStatus == UIX_VOICE_STATUS_SPEAKERS))
        {
            if (voiceStatus == UIX_VOICE_STATUS_SPEAKERS)
                v7->voiceStatus = v7->hasHeadset
                    ? UIX_VOICE_STATUS_COMMUNICATOR
                    : UIX_VOICE_STATUS_NONE;
            else
                v7->voiceStatus = UIX_VOICE_STATUS_SPEAKERS;
            SendCommunicatorStatus(v7->voiceStatus);
        }
    }
    CalcVoicePriorities();
}

// ea: 0x723480
void LiveWrapper::ToggleVTS(unsigned int controllerIndex)
{
    int v2 = 1;
    unsigned int v3;
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        v3 = 0;
    else
        v3 = controllerIndex;
    LiveLocal* v4 = &localPlayers[v3];
    if ((v4->xuid.dwUserFlags & 0x10003) == 0)
    {
        if (v4->voiceStatus == UIX_VOICE_STATUS_SPEAKERS)
            v2 = v4->hasHeadset ? 0 : 2;
        v4->voiceStatus = (UIX_VOICE_STATUS_TYPE)v2;
    }
    CalcVoicePriorities();
}

// ea: 0x7234D0
bool LiveWrapper::SaveLogonState(void* savedState)
{
    memset(savedState, 0, 0x204);  // sizeof(_XONLINE_LOGON_STATE)
    bool result = false;
    if (internalState == kSignedIn)
    {
        HRESULT v3 = XOnlineSaveLogonState((XONLINE_LOGON_STATE*)savedState);
        if (HandleError(v3) == 0)
            return true;
    }
    return result;
}

// ea: 0x723510
void LiveWrapper::RetrieveLogonState(void* state, unsigned int serviceBitfield)
{
    UIX_LOGON_TYPE logonMethod = this->logonMethod;
    this->lastLoginCode = 0;
    if (logonMethod == UIX_LOGON_FORCE_DWORD)
    {
        ASSERT("logonMethod != UIX_LOGON_FORCE_DWORD",
               "c:\\cod\\code\\game\\LiveWrapper.cpp", 722);
    }
    if (this->logonMethod == UIX_LOGON_TYPE_SILENT)
    {
        SignInSilently(serviceBitfield);
        return;
    }
    DWORD params[21];
    params[19] = (DWORD)state;
    params[1] = 2;
    params[2] = 4;
    params[0] = 84;
    params[20] = 0;
    int v5 = 0;
    for (unsigned int i = 0; i < 0x10; ++i)
    {
        if (((1 << i) & serviceBitfield) != 0)
            params[v5++ + 3] = i;
    }
    params[v5 + 3] = 0;
    HRESULT started = LiveEngine_StartFeature(uixEngine, &g_LogonFeature,
                                              params);
    HandleError(started);
    internalState = kSigningIn;
}

// ea: 0x723600
HRESULT __stdcall LiveWrapper::CommunicatorStatusUpdate(
    unsigned int dwLocalPort, int communicatorStatus)
{
    unsigned int v3;
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        v3 = 0;
    else
        v3 = dwLocalPort;
    LiveLocal* v4 = &localPlayers[v3];
    if ((v4->xuid.dwUserFlags & 0x10003) == 0)
    {
        if (communicatorStatus != XHV_VOICE_COMMUNICATOR_STATUS_INSERTED)
        {
            v4->voiceStatus = anyoneBanned
                ? (UIX_VOICE_STATUS_TYPE)2
                : (UIX_VOICE_STATUS_TYPE)1;
            v4->muteListSize = 0;
            if (logonMethod == UIX_LOGON_TYPE_SILENT)
                v3 = 0;
            unsigned int notificationFlags = localPlayers[v3].notificationFlags;
            if ((notificationFlags & 8) != 0)
            {
                unsigned int v8 = notificationFlags & 0xFFFFFFF7;
                localPlayers[v3].notificationFlags = v8;
                LiveEngine_NotificationSetState(uixEngine, v3, v8,
                                                sessionID, 0, nullptr);
            }
            LiveEngine_SetProperty(uixEngine,
                                   UIX_PROPERTY_VOICE_MAIL_TO_SPEAKERS, 1);
        }
        else
        {
            v4->voiceStatus = (UIX_VOICE_STATUS_TYPE)0;
            if (logonMethod == UIX_LOGON_TYPE_SILENT)
                v3 = 0;
            unsigned int* p_notificationFlags =
                &localPlayers[v3].notificationFlags;
            if ((*p_notificationFlags & 8) == 0)
            {
                unsigned int v6 = *p_notificationFlags | 8;
                *p_notificationFlags = v6;
                LiveEngine_NotificationSetState(uixEngine, v3, v6,
                                                sessionID, 0, nullptr);
            }
            v4->muteListSize = 1;
            LiveEngine_SetProperty(uixEngine,
                                   UIX_PROPERTY_VOICE_MAIL_TO_SPEAKERS, 0);
        }
        SendCommunicatorStatus(v4->voiceStatus);
        CalcVoicePriorities();
    }
    return 0;
}

// ea: 0x723710
void LiveWrapper::UpdateLocalPlayers()
{
    const XONLINE_USER* LogonUsers = XOnlineGetLogonUsers();
    loggedInUsers = (XONLINE_USER*)LogonUsers;
    if (LogonUsers != nullptr)
    {
        unsigned int playerIndex = 0;
        for (int v3 = 0; v3 < 4; ++v3)
        {
            if (voiceEngine != nullptr)
            {
                HRESULT v6 = XHVEngine_RegisterLocalTalker(voiceEngine,
                                                           playerIndex);
                HandleError(v6);
            }
            const XONLINE_USER* loggedInUsers = this->loggedInUsers;
            if ((unsigned int)loggedInUsers[v3].xuid.qwValue
                    != (unsigned int)localPlayers[v3].xuid.qwValue
                || (unsigned int)(loggedInUsers[v3].xuid.qwValue >> 32)
                    != (unsigned int)(localPlayers[v3].xuid.qwValue >> 32))
            {
                if (internalMode == kSession
                    && (unsigned int)(localPlayers[v3].xuid.qwValue >> 32) != 0)
                {
                    uixPlayersList->UnregisterPlayer(uixPlayersList,
                                                     &localPlayers[v3]);
                }
                localPlayers[v3].Reset();
                const XONLINE_USER* v8 = this->loggedInUsers;
                unsigned __int64 qw = v8[v3].xuid.qwValue;
                if (qw != 0)
                {
                    localPlayers[v3].xuid.qwValue = qw;
                    localPlayers[v3].xuid.dwUserFlags = v8[v3].xuid.dwUserFlags;
                    unsigned int v12 = v8[v3].xuid.dwUserFlags;
                    if ((v12 & 3) != 0)
                    {
                        localPlayers[v3].voiceStatus =
                            (UIX_VOICE_STATUS_TYPE)2;
                    }
                    else if ((v12 & 0x10000) != 0)
                    {
                        localPlayers[v3].voiceStatus =
                            (UIX_VOICE_STATUS_TYPE)2;
                        anyoneBanned = true;
                    }
                    else
                    {
                        localPlayers[v3].voiceStatus =
                            (UIX_VOICE_STATUS_TYPE)1;
                    }
                    swprintf((wchar_t*)localPlayers[v3].gamertag, L"%S",
                             loggedInUsers[v3].szGamertag);
                    unsigned int v13 = playerIndex;
                    if (logonMethod == UIX_LOGON_TYPE_SILENT)
                        v13 = 0;
                    unsigned int* p_notificationFlags =
                        &localPlayers[v13].notificationFlags;
                    if ((*p_notificationFlags & 1) == 0)
                    {
                        unsigned int v15 = *p_notificationFlags | 1;
                        *p_notificationFlags = v15;
                        LiveEngine_NotificationSetState(uixEngine, v13, v15,
                                                        sessionID, 0, nullptr);
                    }
                    if (internalMode == kSession)
                    {
                        ILivePlayersList* uixPlayersList =
                            this->uixPlayersList;
                        if (uixPlayersList != nullptr)
                        {
                            HRESULT v17 = uixPlayersList->RegisterPlayer(
                                uixPlayersList, &localPlayers[v3]);
                            HandleError(v17);
                        }
                    }
                    RefreshMuteList(playerIndex);
                }
            }
            ++playerIndex;
        }
    }
    else
    {
        internalState = kNotSignedIn;
        for (int i = 0; i < 4; ++i)
            localPlayers[i].Reset();
    }
}

// ea: 0x7238C0
void LiveWrapper::PostLogon()
{
    XHV_RUNTIME_PARAMS xhvParams;
    memset(&xhvParams, 0, sizeof(xhvParams));
    xhvParams.bCustomVADProvided = 0;
    xhvParams.bHeadphoneAlwaysOn = 0;
    xhvParams.dwMaxLocalTalkers = 4;
    xhvParams.dwMaxRemoteTalkers = 15;
    xhvParams.dwFlags = 0;
    xhvParams.pEffectImageDesc = nsl_fxDesc;
    xhvParams.dwEffectsStartIndex = 9;
    xhvParams.dwMaxCompressedBuffers = 4;
    xhvParams.dwOutOfSyncThreshold = 10;
    XHVEngineCreate(&xhvParams, &voiceEngine);
    if (voiceEngine == nullptr)
    {
        ASSERT("voiceEngine", "c:\\cod\\code\\game\\LiveWrapper.cpp", 1063);
    }
    if (voiceEngine != nullptr)
    {
        XHVEngine_SetCallbackInterface(voiceEngine, this);
        XHVEngine_EnableProcessingMode(voiceEngine, g_voicemailMode);
        XHVEngine_EnableProcessingMode(voiceEngine, g_voicechatMode);
    }
    anyoneBanned = false;
    if (voiceEngine != nullptr)
    {
        HRESULT v4 = LiveEngine_UseVoiceMail(uixEngine,
                                             &g_VoiceMailPseudoFeature);
        HandleError(v4);
        HRESULT v5 = LiveEngine_SetProperty(uixEngine,
                                            UIX_PROPERTY_VOICE_MAIL_ENGINE,
                                            (DWORD)voiceEngine);
        HandleError(v5);
        HRESULT v6 = LiveEngine_SetProperty(
            uixEngine, UIX_PROPERTY_VOICE_MAIL_TO_SPEAKERS, 1);
        HandleError(v6);
    }
    HRESULT FeatureInterface = LiveEngine_GetFeatureInterface(
        uixEngine, &g_PlayersFeature, nullptr, (void**)&uixPlayersList);
    HandleError(FeatureInterface);
    UpdateLocalPlayers();
    UIX_LOGON_TYPE logonMethod = this->logonMethod;
    internalState = kSignedIn;
    if (logonMethod == UIX_LOGON_TYPE_NORMAL
        || logonMethod == UIX_LOGON_TYPE_RETRIEVED_STATE
        || logonMethod == UIX_LOGON_TYPE_RETRIEVED_GAME_INVITE)
    {
        LiveEngine_SetProperty(uixEngine,
                               UIX_PROPERTY_DISPLAY_CONNECTION_ERRORS, 1);
        LogonCallBack();
    }
    else if (logonMethod == UIX_LOGON_TYPE_SILENT)
    {
        LiveEngine_SetProperty(uixEngine,
                               UIX_PROPERTY_DISPLAY_CONNECTION_ERRORS, 0);
        LogonCallBack();
    }
    else
    {
        LogonCallBack();
    }
}

// ea: 0x723A70
void LiveWrapper::PreLogoff()
{
    for (int i = 15; i != 0; --i)
    {
        if (remotePlayers[15 - i].xuid.qwValue != 0)
            RemoveRemotePlayer(&remotePlayers[15 - i].xuid);
    }
    if (voiceEngine != nullptr)
    {
        XHVEngine_Release(voiceEngine);
        voiceEngine = nullptr;
        LiveEngine_SetProperty(uixEngine, UIX_PROPERTY_VOICE_MAIL_ENGINE, 0);
    }
    for (int v4 = 0; v4 < 4; ++v4)
    {
        if (localPlayers[v4].xuid.qwValue != 0)
        {
            unsigned int v6;
            if (logonMethod == UIX_LOGON_TYPE_SILENT)
                v6 = 0;
            else
                v6 = v4;
            unsigned int* p_notificationFlags =
                &localPlayers[v6].notificationFlags;
            if ((*p_notificationFlags & 1) != 0)
            {
                unsigned int v8 = *p_notificationFlags & 0xFFFFFFFE;
                *p_notificationFlags = v8;
                LiveEngine_NotificationSetState(uixEngine, v6, v8,
                                                sessionID, 0, nullptr);
            }
        }
    }
    memset(sessionID.ab, 0, sizeof(sessionID.ab));
    memset(commKey.ab, 0, sizeof(commKey.ab));
}

// ea: 0x723B40
void LiveWrapper::PostLogoff()
{
    internalState = kNotSignedIn;
    LiveEngine_LogOff(uixEngine);
    UpdateLocalPlayers();
    LogoffCallBack();
    sessionState = kNotInSession;
}

// ea: 0x723B70
void LiveWrapper::CheckMutingChanges(unsigned int controllerIndex)
{
    if (logonMethod == UIX_LOGON_TYPE_SILENT)
        controllerIndex = 0;
    bool mutingChanged = false;
    for (int v11 = 0; v11 < 15; ++v11)
    {
        XUID* p_xuid = &remotePlayers[v11].xuid;
        if (p_xuid->qwValue != 0)
        {
            unsigned int muteListSize =
                localPlayers[controllerIndex].muteListSize;
            char currMuteState = 0;
            for (unsigned int v6 = 0; v6 < muteListSize; ++v6)
            {
                if (localPlayers[controllerIndex].muteList[v6].xuid.qwValue
                    == p_xuid->qwValue)
                {
                    currMuteState = 1;
                    break;
                }
            }
            if (remotePlayers[v11].mutedBy[controllerIndex] != currMuteState)
            {
                remotePlayers[v11].SetMuted(controllerIndex, currMuteState);
                SendMuteUpdate(&localPlayers[controllerIndex].xuid,
                               p_xuid, currMuteState);
                mutingChanged = true;
            }
        }
    }
    if (mutingChanged)
        CalcVoicePriorities();
}

// ea: 0x71FE20
HRESULT LiveWrapper::HandleError(HRESULT errorCode)
{
    if (errorCode > -2146086650)
    {
        if (errorCode <= 0)
        {
            if (errorCode != 0)
            {
                switch (errorCode)
                {
                case -2146086649:
                    ASSERT("!\"TThe attribute type for the create, update, or search operation is something other than player or session.\"",
                           "c:\\cod\\code\\game\\LiveWrapper.cpp", 1216);
                    return errorCode;
                case -2146086644:
                    ASSERT("!\"The attribute string for the create, update, or search operation is longer than 400 characters.\"",
                           "c:\\cod\\code\\game\\LiveWrapper.cpp", 1210);
                    return errorCode;
                case -2146086643:
                    ASSERT("!\"The BLOB for the create, update, or search operation is longer then 800 bytes.\"",
                           "c:\\cod\\code\\game\\LiveWrapper.cpp", 1213);
                    return errorCode;
                case -2146086631:
                    ASSERT("!\"The data type that was passed to the stored procedure is not valid.\"",
                           "c:\\cod\\code\\game\\LiveWrapper.cpp", 1225);
                    return errorCode;
                default:
                    goto LABEL_59;
                }
            }
            return errorCode;
        }
        if (errorCode == 1)
        {
            Print(L"A UIX feature has encountered an error. This is probably minor and to be ignored.");
            return errorCode;
        }
        if (errorCode == 1168)
        {
            ASSERT("!\"unrecognized UIX feature\\n\"",
                   "c:\\cod\\code\\game\\LiveWrapper.cpp", 1189);
            return errorCode;
        }
        if (errorCode - 1168 == (int)0x14FC60)
            return errorCode;
        goto LABEL_59;
    }
    if (errorCode == -2146086650)
    {
        ASSERT("!\"The attribute ID was OR'd with an invalid data type.\"",
               "c:\\cod\\code\\game\\LiveWrapper.cpp", 1222);
        return errorCode;
    }
    if (errorCode > -2146107387)
    {
        if (errorCode > -2146086655)
        {
            if (errorCode == -2146086651)
            {
                ASSERT("!\"The wrong number of attributes was passed.\"",
                       "c:\\cod\\code\\game\\LiveWrapper.cpp", 1207);
                return errorCode;
            }
        }
        else
        {
            switch (errorCode)
            {
            case -2146086655:
                ASSERT("!\"The title is not properly registered on the Matchmaking server. Make sure Matchsim or partnernet is properly configured\"",
                       "c:\\cod\\code\\game\\LiveWrapper.cpp", 1228);
                return errorCode;
            case -2146107384:
                goto LABEL_28;
            case -2146086656:
                ASSERT("!\"The specified session ID is not a valid session ID returned by XOnlineMatchSessionGetInfo or XOnlineMatchSessionFindFromID.\"",
                       "c:\\cod\\code\\game\\LiveWrapper.cpp", 1219);
                return errorCode;
            default:
                break;
            }
        }
        goto LABEL_59;
    }
    if (errorCode == -2146107387)
    {
        ASSERT("!\"XOnlineStartup has not been called yet. Call XOnlineStartup first.\"",
               "c:\\cod\\code\\game\\LiveWrapper.cpp", 1204);
        return errorCode;
    }
    if (errorCode > -2146107389)
    {
        ASSERT("!\"This function cannot be used with guests.\"",
               "c:\\cod\\code\\game\\LiveWrapper.cpp", 1201);
        return errorCode;
    }
    if (errorCode == -2146107389)
    {
        ASSERT("!\"This function requires a logged on user. Verify logon status before calling.\"",
               "c:\\cod\\code\\game\\LiveWrapper.cpp", 1198);
        return errorCode;
    }
    if (errorCode == -2147024882)
        goto LABEL_28;
    if (errorCode != -2146107390)
        goto LABEL_59;
    ASSERT("!\"This function requires an online session. Call logon or monitor session state prior to this.\"",
           "c:\\cod\\code\\game\\LiveWrapper.cpp", 1195);
    return errorCode;
LABEL_28:
    ASSERT("!\"Xbox live does not have enough memory to complete its internal operations\"",
           "c:\\cod\\code\\game\\LiveWrapper.cpp", 1192);
    return errorCode;
LABEL_59:
    Print(L"\n!!!!! Unrecognized error code %d encountered in Live wrapper. Handle this error properly in LiveWrapper::HandleError !!!!!\n",
          errorCode);
    return errorCode;
}

// ea: 0x71FB30
void LiveWrapper::CalcVoicePriorities()
{
    for (int v14 = 15; v14 != 0; --v14)
    {
        XUID* p_xuid = &remotePlayers[15 - v14].xuid;
        if (p_xuid->qwValue != 0)
        {
            unsigned int speakerPriority = 0xFFFFFFFF;
            if (logonMethod == UIX_LOGON_TYPE_SILENT)
            {
                UIX_VOICE_STATUS_TYPE voiceStatus =
                    localPlayers[0].voiceStatus;
                if (voiceStatus != UIX_VOICE_STATUS_COMMUNICATOR)
                {
                    if (voiceStatus == UIX_VOICE_STATUS_SPEAKERS)
                    {
                        for (int i = 0; i < 4; ++i)
                        {
                            XHVEngine* voiceEngine = this->voiceEngine;
                            if (voiceEngine != nullptr)
                            {
                                XHVEngine_SetPlaybackPriority(
                                    voiceEngine, *p_xuid, i, 0xFFFFFFFF);
                            }
                        }
                        if (!anyoneBanned
                            && (unsigned int)(p_xuid->qwValue >> 32) == 0
                            && (unsigned char)p_xuid->dwUserFlags == 0)
                        {
                            speakerPriority = 0;
                        }
                    }
                }
                else if ((unsigned int)(p_xuid->qwValue >> 32) != 0
                         || (unsigned char)p_xuid->dwUserFlags != 0)
                {
                    for (int j = 0; j < 4; ++j)
                    {
                        XHVEngine* v7 = this->voiceEngine;
                        if (v7 != nullptr)
                        {
                            XHVEngine_SetPlaybackPriority(v7, *p_xuid, j,
                                                          0xFFFFFFFF);
                        }
                    }
                }
                else
                {
                    for (int k = 0; k < 4; ++k)
                    {
                        XHVEngine* v5 = this->voiceEngine;
                        if (v5 != nullptr)
                        {
                            XHVEngine_SetPlaybackPriority(v5, *p_xuid, k, 0);
                        }
                    }
                }
            }
            else
            {
                for (int v10 = 0; v10 < 4; ++v10)
                {
                    UIX_VOICE_STATUS_TYPE* p_voiceStatus =
                        &localPlayers[v10].voiceStatus;
                    if (*p_voiceStatus != UIX_VOICE_STATUS_COMMUNICATOR)
                    {
                        if (*p_voiceStatus == UIX_VOICE_STATUS_SPEAKERS)
                        {
                            XHVEngine* v11 = this->voiceEngine;
                            if (v11 != nullptr)
                            {
                                XHVEngine_SetPlaybackPriority(
                                    v11, *p_xuid, v10, 0xFFFFFFFF);
                            }
                            if (!anyoneBanned
                                && !remotePlayers[15 - v14].IsMutedBy(v10)
                                && !remotePlayers[15 - v14].IsMuting(v10))
                            {
                                speakerPriority = 0;
                            }
                        }
                    }
                    else if (this->voiceEngine != nullptr)
                    {
                        if (remotePlayers[15 - v14].IsMutedBy(v10)
                            || remotePlayers[15 - v14].IsMuting(v10))
                        {
                            XHVEngine_SetPlaybackPriority(
                                this->voiceEngine, *p_xuid, v10, 0xFFFFFFFF);
                        }
                        else
                        {
                            XHVEngine_SetPlaybackPriority(
                                this->voiceEngine, *p_xuid, v10, 0);
                        }
                    }
                }
            }
            XHVEngine* v12 = this->voiceEngine;
            if (v12 != nullptr)
            {
                XHVEngine_SetPlaybackPriority(v12, *p_xuid, 4,
                                              speakerPriority);
            }
        }
    }
}

// ea: 0x726490
void LiveWrapper::DoWork()
{
    if (internalMode == kNotSetup)
    {
        ASSERT("internalMode != kNotSetup",
               "c:\\cod\\code\\game\\LiveWrapper.cpp", 216);
    }
    HRESULT v2 = LiveEngine_DoWork(uixEngine, (DWORD*)&uixFlags);
    HandleError(v2);
    if ((uixFlags & 8) != 0)
        HandleFeatureExit();
    if ((uixFlags & 4) != 0)
    {
        if (needConfirmation)
        {
            lastNotification = kConfirmReboot;
        }
        else
        {
            HRESULT v3 = LiveEngine_Reboot(uixEngine, 0);
            HandleError(v3);
        }
    }
    if (voiceEngine != nullptr)
    {
        HRESULT v4 = XHVEngine_DoWork(voiceEngine);
        HandleError(v4);
    }
    if (internalMode == kSession && internalState == kSignedIn)
    {
        for (int v5 = 0; v5 < 4; ++v5)
        {
            if (muteListTask[v5].needsWork)
            {
                HRESULT v7 = XOnlineTaskContinue(
                    muteListTask[v5].taskHandle);
                muteListTask[v5].lastResult = v7;
                if (HandleError(v7) != 0)
                {
                    if (muteListTask[v5].lastResult == 0x1500F0)
                        CheckMutingChanges(v5);
                    XOnlineTaskClose(muteListTask[v5].taskHandle);
                    muteListTask[v5].taskHandle = nullptr;
                    muteListTask[v5].needsWork = false;
                }
            }
        }
    }
}

// ea: 0x725C90
void LiveWrapper::LogOut()
{
    PreLogoff();
    LiveEngine_LogOff(uixEngine);
    internalState = kNotSignedIn;
    LiveEngine_LogOff(uixEngine);
    UpdateLocalPlayers();
    LogoffCallBack();
    sessionState = kNotInSession;
}

// ea: 0x725CD0
void LiveWrapper::HandleFeatureExit()
{
    UIX_EXIT_INFO exitInfo;
    LiveEngine_GetExitInfo(uixEngine, &exitInfo);
    switch (exitInfo.ExitCode)
    {
    case 1:
        PostLogon();
        break;
    case 2:
        PreLogoff();
        PostLogoff();
        lastLoginCode = exitInfo.hr;
        if (exitInfo.hr == -2146103291 && internalMode == kSession)
            lastNotification = kNeedToExitSession;
        break;
    case 3:
        if (internalState == kSigningIn)
            internalState = kNotSignedIn;
        break;
    case 5:
        if (internalState == kSigningIn)
            PostLogon();
        if (needConfirmation)
        {
            memcpy(&friendToJoin, exitInfo.pExitData, 0x54);
            lastNotification = kConfirmFriendJoin;
        }
        else
        {
            JoinGame(exitInfo.pExitData);
        }
        break;
    case 7:
        PreLogoff();
        PostLogoff();
        break;
    default:
        break;
    }
    if (playersListActive)
    {
        RefreshMuteList(activeController);
        playersListActive = false;
    }
}

// ============================================================================
// CSession - matchmaking session state machine (xboxMatch.cpp)
// ============================================================================

// ea: 0x723CB0
CSession::CSession()
{
    m_SessionQosQ.m_pTail = nullptr;
    m_SessionQosQ.m_pHead = nullptr;
    m_QosResponse.m_wLength = 0;
    m_QosResponse.m_pvData = nullptr;
    m_State = STATE_IDLE;
    m_hSessionTask = nullptr;
    m_bUpdate = 0;
    m_bListening = 0;
    m_bKeyRegistered = 0;
    SetupAttributes();
}

// ea: 0x726DA0
CSession::~CSession()
{
    Reset();
}

// ea: 0x720500
void CSession::SetupAttributes()
{
    memset(m_Attributes, 0, sizeof(m_Attributes));
    m_Attributes[0].dwAttributeID = 1;
    m_Attributes[1].dwAttributeID = 2;
    m_Attributes[2].dwAttributeID = 3;
    m_Attributes[3].dwAttributeID = 4;
    m_Attributes[4].dwAttributeID = 5;
    m_Attributes[5].dwAttributeID = 6;
    m_Attributes[6].dwAttributeID = 8;
    // disasm: mov dword ptr [edx+98h], offset 0x100009 (literal attribute id)
    m_Attributes[7].dwAttributeID = 0x100009;
    m_Attributes[7].info.string.lpValue = m_strhost_name;
    m_strhost_name[0] = 0;
    m_Attributes[8].dwAttributeID = 10;
}

// ea: 0x720430
HRESULT CSession::Create()
{
    if (m_State != STATE_IDLE)
    {
        ASSERT("m_State == STATE_IDLE", "c:\\cod\\code\\game\\xboxMatch.cpp", 95);
    }
    HRESULT result = XOnlineMatchSessionCreate(
        PublicFilled, PublicOpen, PrivateFilled, PrivateOpen,
        9, m_Attributes, nullptr, &m_hSessionTask);
    if (result != 0)
    {
        Print(L"Session Creation Failed with 0x%x", result);
        return result;
    }
    m_State = STATE_CREATING;
    return result;
}

// ea: 0x7204D0
void CSession::Close()
{
    if (m_hSessionTask != nullptr)
    {
        XOnlineTaskClose(m_hSessionTask);
        m_hSessionTask = nullptr;
        m_State = STATE_IDLE;
    }
    m_bUpdate = 0;
}

// ea: 0x720580
void CSession::Setgame_type(unsigned __int64 Value)
{
    m_Attributes[0].info.integer.qwValue = Value;
    m_Attributes[0].fChanged = 1;
}

// ea: 0x720570
unsigned __int64 CSession::Getgame_type()
{
    return m_Attributes[0].info.integer.qwValue;
}

// ea: 0x7205A0
unsigned __int64 CSession::Getgame_map()
{
    return m_Attributes[1].info.integer.qwValue;
}

// ea: 0x7205D0
unsigned __int64 CSession::Getgame_version()
{
    return m_Attributes[2].info.integer.qwValue;
}

// ea: 0x720600
unsigned __int64 CSession::Getfriendly_fire()
{
    return m_Attributes[3].info.integer.qwValue;
}

// ea: 0x720630
unsigned __int64 CSession::Getteam_balancing()
{
    return m_Attributes[4].info.integer.qwValue;
}

// ea: 0x720660
unsigned __int64 CSession::Getsub_type()
{
    return m_Attributes[5].info.integer.qwValue;
}

// ea: 0x720690
unsigned __int64 CSession::Getnum_players()
{
    return m_Attributes[6].info.integer.qwValue;
}

// ea: 0x7206D0
const unsigned short* CSession::Gethost_name()
{
    return m_strhost_name;
}

// ea: 0x720710
unsigned __int64 CSession::Getmax_players()
{
    return m_Attributes[8].info.integer.qwValue;
}

// ea: 0x7205B0
void CSession::Setgame_map(unsigned __int64 Value)
{
    m_Attributes[1].info.integer.qwValue = Value;
    m_Attributes[1].fChanged = 1;
}

// ea: 0x7205E0
void CSession::Setgame_version(unsigned __int64 Value)
{
    m_Attributes[2].info.integer.qwValue = Value;
    m_Attributes[2].fChanged = 1;
}

// ea: 0x720610
void CSession::Setfriendly_fire(unsigned __int64 Value)
{
    m_Attributes[3].info.integer.qwValue = Value;
    m_Attributes[3].fChanged = 1;
}

// ea: 0x720640
void CSession::Setteam_balancing(unsigned __int64 Value)
{
    m_Attributes[4].info.integer.qwValue = Value;
    m_Attributes[4].fChanged = 1;
}

// ea: 0x720670
void CSession::Setsub_type(unsigned __int64 Value)
{
    m_Attributes[5].info.integer.qwValue = Value;
    m_Attributes[5].fChanged = 1;
}

// ea: 0x7206A0
void CSession::Setnum_players(unsigned __int64 Value)
{
    m_Attributes[6].info.integer.qwValue = Value;
    m_Attributes[6].fChanged = 1;
}

// ea: 0x7206E0
void CSession::Sethost_name(const unsigned short* Value)
{
    wcscpy((wchar_t*)m_strhost_name, (const wchar_t*)Value);
    m_Attributes[7].fChanged = 1;
}

// ea: 0x720720
void CSession::Setmax_players(unsigned __int64 Value)
{
    m_Attributes[8].info.integer.qwValue = Value;
    m_Attributes[8].fChanged = 1;
}

// ea: 0x720750
CSession::CSessionQosQ::CSessionQosQ()
{
    m_pTail = nullptr;
    m_pHead = nullptr;
}

// ea: 0x720760
void CSession::CSessionQosQ::Add(XNKID* SessionID, unsigned int dwStartTick)
{
    if (!((m_pHead != nullptr && m_pTail != nullptr)
          || (m_pHead == nullptr && m_pTail == nullptr)))
    {
        ASSERT("m_pHead && m_pTail || !m_pHead && !m_pTail",
               "c:\\cod\\code\\game\\xboxMatch.cpp", 836);
    }
    QosQEntry* v5 = (QosQEntry*)mem_heap_malloc(0x10);
    v5->SessionID = *SessionID;
    v5->dwStartTick = dwStartTick;
    v5->pNext = nullptr;
    QosQEntry* v6 = m_pTail;
    if (v6 != nullptr)
        v6->pNext = v5;
    bool v7 = m_pHead == nullptr;
    m_pTail = v5;
    if (v7)
        m_pHead = v5;
}

// ea: 0x720800
void CSession::CSessionQosQ::Remove(XNKID* SessionID)
{
    if (!((m_pHead != nullptr && m_pTail != nullptr)
          || (m_pHead == nullptr && m_pTail == nullptr)))
    {
        ASSERT("m_pHead && m_pTail || !m_pHead && !m_pTail",
               "c:\\cod\\code\\game\\xboxMatch.cpp", 857);
    }
    QosQEntry* m_pHead = this->m_pHead;
    QosQEntry* v5 = nullptr;
    if (this->m_pHead != nullptr)
    {
        while (memcmp(m_pHead, SessionID, 8) != 0)
        {
            v5 = m_pHead;
            m_pHead = m_pHead->pNext;
            if (m_pHead == nullptr)
                return;
        }
        if (v5 != nullptr)
            v5->pNext = m_pHead->pNext;
        else
            this->m_pHead = m_pHead->pNext;
        if (m_pHead == this->m_pTail)
            this->m_pTail = v5;
        mem_heap_free(m_pHead);
    }
}

// ea: 0x7208D0
void CSession::CSessionQosQ::Dequeue()
{
    if (!((m_pHead != nullptr && m_pTail != nullptr)
          || (m_pHead == nullptr && m_pTail == nullptr)))
    {
        ASSERT("m_pHead && m_pTail || !m_pHead && !m_pTail",
               "c:\\cod\\code\\game\\xboxMatch.cpp", 892);
    }
    if (m_pHead != nullptr)
    {
        QosQEntry* m_pHead = this->m_pHead;
        this->m_pHead = this->m_pHead->pNext;
        mem_heap_free(m_pHead);
        if (this->m_pHead == nullptr)
            this->m_pTail = nullptr;
    }
}

// ea: 0x723D00
HRESULT CSession::Update()
{
    switch (m_State)
    {
    case STATE_IDLE:
    case STATE_DELETING:
        return -2147418113;  // 0x8000FFFF
    case STATE_CREATING:
    case STATE_UPDATING:
        m_bUpdate = 1;
        return 0;
    case STATE_ACTIVE:
    {
        XOnlineTaskClose(m_hSessionTask);
        unsigned int PublicFilled = this->PublicFilled;
        unsigned int PublicOpen = this->PublicOpen;
        unsigned int PrivateFilled = this->PrivateFilled;
        unsigned int PrivateOpen = this->PrivateOpen;
        XNKID SessionID = this->SessionID;
        m_hSessionTask = nullptr;
        HRESULT result = XOnlineMatchSessionUpdate(
            SessionID, PublicFilled, PublicOpen, PrivateFilled, PrivateOpen,
            9, m_Attributes, nullptr, &m_hSessionTask);
        m_bUpdate = 0;
        if (result < 0)
        {
            Print(L"Session Update Failed with 0x%x", result);
            Close();
            return result;
        }
        m_State = STATE_UPDATING;
        return result;
    }
    default:
        ASSERT("0", "c:\\cod\\code\\game\\xboxMatch.cpp", 165);
        return -2147418113;
    }
}

// ea: 0x723E00
HRESULT CSession::ProcessStateUpdateSession()
{
    int v2 = XOnlineTaskContinue(m_hSessionTask);
    if (v2 == 0)
        return v2;
    if (v2 < 0)
    {
        if (m_hSessionTask != nullptr)
        {
            XOnlineTaskClose(m_hSessionTask);
            m_hSessionTask = nullptr;
            m_State = STATE_IDLE;
        }
        m_bUpdate = 0;
        Print(L"Session Update Failed with 0x%x", v2);
        return v2;
    }
    int m_bUpdate = this->m_bUpdate;
    m_State = STATE_ACTIVE;
    if (m_bUpdate == 0)
        return v2;
    return Update();
}

// ea: 0x723E80
HRESULT CSession::ProcessStateDeleteSession()
{
    int v2 = XOnlineTaskContinue(m_hSessionTask);
    if (v2 != 0)
    {
        if (v2 < 0)
            Print(L"Session Delete Failed with 0x%x", v2);
        if (m_hSessionTask != nullptr)
        {
            XOnlineTaskClose(m_hSessionTask);
            m_hSessionTask = nullptr;
            m_State = STATE_IDLE;
        }
        m_bUpdate = 0;
    }
    return v2;
}

// ea: 0x723EE0
HRESULT CSession::ProcessStateActiveSession()
{
    int v2 = XOnlineTaskContinue(m_hSessionTask);
    if (v2 < 0)
    {
        if (m_hSessionTask != nullptr)
        {
            XOnlineTaskClose(m_hSessionTask);
            m_hSessionTask = nullptr;
            m_State = STATE_IDLE;
        }
        m_bUpdate = 0;
        Print(L"Session Task Failed with 0x%x", v2);
    }
    return v2;
}

// ea: 0x723F40
void CSession::PurgeSessionQHead()
{
    QosQEntry* m_pHead = m_SessionQosQ.m_pHead;
    if (m_pHead != nullptr)
    {
        Print(L"Qos listening rejection period expired for 0x%x", m_pHead);
        if (XNetQosListen(&m_pHead->SessionID, nullptr, 0, 0, 0x10) != 0)
        {
            ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 464);
        }
        if ((m_bKeyRegistered == 0
             || memcmp(m_pHead, &SessionID, 8) != 0)
            && XNetUnregisterKey(&m_pHead->SessionID) != 0)
        {
            ASSERT("iResult == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 470);
        }
        m_SessionQosQ.Dequeue();
    }
}

// ea: 0x724040
void CSession::Listen(int bEnable, unsigned int dwBitsPerSec)
{
    if (bEnable != 0)
    {
        m_SessionQosQ.Remove(&SessionID);
        if (XNetQosListen(&SessionID, (BYTE*)m_QosResponse.m_pvData,
                          m_QosResponse.m_wLength, dwBitsPerSec, 0xD) != 0)
        {
            ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 498);
        }
        m_bListening = 1;
    }
    else if (m_bListening != 0)
    {
        m_bListening = 0;
        if (dwBitsPerSec == 1)
        {
            Print(L"Qos listening stopped");
            if (XNetQosListen(&SessionID, nullptr, 0, 0, 0x10) != 0)
            {
                ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 514);
            }
        }
        else
        {
            DWORD TickCount = GetTickCount();
            m_SessionQosQ.Add(&SessionID, TickCount);
            if (XNetQosListen(&SessionID, nullptr, 0, dwBitsPerSec, 0xA) != 0)
            {
                ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 525);
            }
            Print(L"Qos probe rejection period started for 0x%x",
                  m_SessionQosQ.m_pTail);
        }
    }
}

// ea: 0x7241F0
void CSession::SetQosResponse(CBlob Value)
{
    m_QosResponse = Value;
    if (m_bListening != 0
        && XNetQosListen(&SessionID, (BYTE*)Value.m_pvData,
                         m_QosResponse.m_wLength, 0, 4) != 0)
    {
        ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 562);
    }
}

// ea: 0x7260D0
CBlob CSession::GetQosResponse()
{
    return m_QosResponse;
}

// ea: 0x725DD0
HRESULT CSession::Delete()
{
    if (m_State == STATE_CREATING)
    {
        if (m_hSessionTask != nullptr)
        {
            XOnlineTaskClose(m_hSessionTask);
            m_hSessionTask = nullptr;
            m_State = STATE_IDLE;
        }
        m_bUpdate = 0;
        return 0;
    }
    if (m_State != STATE_UPDATING)
    {
        if (m_State == STATE_ACTIVE)
        {
            if (m_bListening != 0)
            {
                Listen(0, 0);
            }
            else if (XNetUnregisterKey(&SessionID) != 0)
            {
                ASSERT("iResult == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 204);
            }
            XONLINETASK_HANDLE__* m_hSessionTask = this->m_hSessionTask;
            m_bKeyRegistered = 0;
            if (m_hSessionTask != nullptr)
            {
                XOnlineTaskClose(m_hSessionTask);
                this->m_hSessionTask = nullptr;
                m_State = STATE_IDLE;
            }
            XNKID SessionID = this->SessionID;
            m_bUpdate = 0;
            HRESULT result = XOnlineMatchSessionDelete(
                SessionID, nullptr, &this->m_hSessionTask);
            if (result < 0)
            {
                Print(L"Session Delete Failed with 0x%x", result);
                return result;
            }
            m_State = STATE_DELETING;
            return result;
        }
        return 0;
    }
    if (m_hSessionTask != nullptr)
    {
        XOnlineTaskClose(m_hSessionTask);
        m_hSessionTask = nullptr;
        m_State = STATE_IDLE;
    }
    m_bUpdate = 0;
    Listen(0, 0);
    return 0;
}

// ea: 0x725F10
HRESULT CSession::ProcessStateCreateSession()
{
    int v2 = XOnlineTaskContinue(m_hSessionTask);
    HRESULT hr = v2;
    if (v2 == 0)
        return v2;
    if (v2 < 0)
    {
        if (m_hSessionTask != nullptr)
        {
            XOnlineTaskClose(m_hSessionTask);
            m_hSessionTask = nullptr;
            m_State = STATE_IDLE;
        }
        m_bUpdate = 0;
        Print(L"Session Creation Failed with 0x%x", v2);
        return v2;
    }
    if (XOnlineMatchSessionGetInfo(m_hSessionTask, &SessionID,
                                   &KeyExchangeKey) < 0)
    {
        ASSERT("((HRESULT)(hrGet) >= 0)",
               "c:\\cod\\code\\game\\xboxMatch.cpp", 252);
    }
    m_State = STATE_ACTIVE;
    int iKeyRegistered = XNetRegisterKey(&SessionID, &KeyExchangeKey);
    if (iKeyRegistered == 10102)
    {
        Print(L"Out of keys... Purging SessionQ and trying again");
        PurgeSessionQHead();
        iKeyRegistered = XNetRegisterKey(&SessionID, &KeyExchangeKey);
    }
    if (iKeyRegistered != 0)
    {
        ASSERT("iKeyRegistered == 0L",
               "c:\\cod\\code\\game\\xboxMatch.cpp", 269);
    }
    m_bKeyRegistered = iKeyRegistered == 0;
    Listen(1, 0);
    if (m_bUpdate != 0)
        return Update();
    return hr;
}

// ea: 0x726090
void CSession::PurgeSessionQ(int fRemoveAll)
{
    while (1)
    {
        QosQEntry* m_pHead = m_SessionQosQ.m_pHead;
        if (m_pHead == nullptr
            || fRemoveAll == 0
                && GetTickCount() - m_pHead->dwStartTick < 0x3A98)
        {
            break;
        }
        PurgeSessionQHead();
    }
}

// ea: 0x7265C0
void CSession::Reset()
{
    if (m_hSessionTask != nullptr)
    {
        XOnlineTaskClose(m_hSessionTask);
        m_hSessionTask = nullptr;
        m_State = STATE_IDLE;
    }
    bool v2 = m_bListening == 0;
    m_bUpdate = 0;
    if (!v2)
    {
        m_bListening = 0;
        Print(L"Qos listening stopped");
        if (XNetQosListen(&SessionID, nullptr, 0, 0, 0x10) != 0)
        {
            ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 514);
        }
    }
    while (m_SessionQosQ.m_pHead != nullptr)
        PurgeSessionQHead();
    if (m_bKeyRegistered != 0)
    {
        XNetUnregisterKey(&SessionID);
        m_bKeyRegistered = 0;
    }
}

// ea: 0x7266B0
HRESULT CSession::Process()
{
    while (1)
    {
        QosQEntry* m_pHead = m_SessionQosQ.m_pHead;
        if (m_pHead == nullptr
            || GetTickCount() - m_pHead->dwStartTick < 0x3A98)
        {
            break;
        }
        PurgeSessionQHead();
    }
    HRESULT result;
    switch (m_State)
    {
    case STATE_IDLE:
        // disasm: mov esi, offset 0x1500F0 (literal success-ish HRESULT)
        return 0x1500F0;
    case STATE_CREATING:
        result = ProcessStateCreateSession();
        break;
    case STATE_UPDATING:
        result = ProcessStateUpdateSession();
        break;
    case STATE_DELETING:
        result = ProcessStateDeleteSession();
        break;
    case STATE_ACTIVE:
        result = ProcessStateActiveSession();
        break;
    default:
        ASSERT("0", "c:\\cod\\code\\game\\xboxMatch.cpp", 389);
        return -2147418113;
    }
    if (result < 0)
        Reset();
    return result;
}
