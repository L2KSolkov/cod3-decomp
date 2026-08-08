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
extern int gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = 0;                                     \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

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

// ea: 0x71F7A0
LiveWrapper::~LiveWrapper()
{
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
