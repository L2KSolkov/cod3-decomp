// ============================================================================
// xboxMatch.cpp - matchmaking session queries (game_xbox.o xboxMatch.cpp)
// CFromIDQuery + CDefaultQuery against the XOnline matchmaking API.
// The XOnline SDK entry points are shimmed (see NEEDED.md); this file ports
// the game's call sites verbatim.
// ============================================================================

#include "MPLiveEngine.h"

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

static void Print(const wchar_t* strFormat, ...)
{
    (void)strFormat;
}

// ============================================================================
// Attribute specs (g.o data, verified via XBE read)
// ============================================================================
_XONLINE_ATTRIBUTE_SPEC FromIDAttributeSpec[7] = {
    { 0x1000, 0x22 },  // 0xD170E0
    { 0, 8 }, { 0, 8 }, { 0, 8 }, { 0, 8 }, { 0, 8 },
    { 0x1000, 0x22 },
};
_XONLINE_ATTRIBUTE_SPEC DefaultAttributeSpec[8] = {
    { 0x1000, 0x22 },  // 0xD17118
    { 0, 8 }, { 0, 8 }, { 0, 8 }, { 0, 8 }, { 0, 8 }, { 0, 8 },
    { 0x1000, 0x22 },
};

// ============================================================================
// CFromIDQuery - query a single session by its ID
// ============================================================================

// ea: 0x724280
CFromIDQuery::CFromIDQuery()
{
    Results.m_dwSize = 0;
    m_State = STATE_IDLE;
    m_hrQuery = 1;  // S_FALSE
    m_hSearchTask = nullptr;
    m_pXnQos = nullptr;
}

// ea: 0x7242B0
CFromIDQuery::~CFromIDQuery()
{
    Cancel();
}

// ea: 0x720950
void CFromIDQuery::Cancel()
{
    if (m_State == STATE_IDLE)
        return;
    if (m_State > STATE_DONE)
    {
        ASSERT("0", "c:\\cod\\code\\game\\xboxMatch.cpp", 972);
    }
    else
    {
        if (m_hSearchTask != nullptr)
        {
            XOnlineTaskClose(m_hSearchTask);
            m_hSearchTask = nullptr;
        }
        Clear();
        m_State = STATE_IDLE;
        m_hrQuery = 1;
    }
}

// ea: 0x7209F0
void CFromIDQuery::Clear()
{
    if (m_pXnQos != nullptr)
    {
        XNetQosRelease(m_pXnQos);
        m_pXnQos = nullptr;
    }
    Results.m_dwSize = 0;
}

// ea: 0x720A20
HRESULT CFromIDQuery::Query(unsigned __int64 SessionID)
{
    if (m_State == STATE_DONE)
    {
        Clear();
        m_State = STATE_IDLE;
    }
    if (m_State != STATE_IDLE)
    {
        ASSERT("m_State == STATE_IDLE", "c:\\cod\\code\\game\\xboxMatch.cpp",
               1009);
        if (m_State != STATE_IDLE)
            return 0x8000FFFF;  // E_PENDING-ish fail
    }
    XONLINE_ATTRIBUTE QueryParameters;
    QueryParameters.dwAttributeID = 0;
    QueryParameters.fChanged = 0;
    QueryParameters.info.integer.qwValue = SessionID;
    unsigned int matched = XOnlineMatchSearchResultsLen(
        1, 7, FromIDAttributeSpec);
    HRESULT result = XOnlineMatchSearch(
        2, 1, 1, &QueryParameters, matched, nullptr, &m_hSearchTask);
    if (result >= 0)
        m_State = STATE_RUNNING;
    return result;
}

// ea: 0x720B00
HRESULT CFromIDQuery::Probe()
{
    if (m_State != STATE_DONE)
    {
        ASSERT("m_State == STATE_DONE",
               "c:\\cod\\code\\game\\xboxMatch.cpp", 1041);
        if (m_State != STATE_DONE)
            return 0x8000FFFF;
    }
    if (m_pXnQos != nullptr)
    {
        XNetQosRelease(m_pXnQos);
        m_pXnQos = nullptr;
    }
    unsigned int m_dwSize = Results.m_dwSize;
    if (m_dwSize == 0)
        return 0;
    for (unsigned int i = 0; i < m_dwSize; ++i)
    {
        XOnlineMatchSessionRecord& rec = Results[i];
        m_rgpXnAddr[i] = &rec.HostAddress;
        m_rgpXnKid[i] = &rec.SessionID;
        m_rgpXnKey[i] = &rec.KeyExchangeKey;
        rec.qosInfo = 0;
    }
    if (XNetQosLookup(m_dwSize, m_rgpXnAddr, m_rgpXnKid, m_rgpXnKey, 0,
                      nullptr, nullptr, 2, 0xFA00, 0, nullptr,
                      &m_pXnQos) == 0)
    {
        m_State = STATE_PROBING_BANDWIDTH;
        return 0;
    }
    ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 1066);
    return 0x80004005;  // E_FAIL
}

// ea: 0x720C50
HRESULT CFromIDQuery::Process()
{
    switch (m_State)
    {
    case STATE_IDLE:
        return 0;
    case STATE_DONE:
        return m_hrQuery;
    case STATE_PROBING_BANDWIDTH:
        if (Results.m_dwSize != 0)
        {
            for (unsigned int i = 0; i < Results.m_dwSize; ++i)
            {
                if (Results[i].qosInfo == 0
                    && (m_pXnQos->axnqosinfo[i].bFlags & 1) != 0)
                {
                    Results[i].qosInfo =
                        (unsigned int)&m_pXnQos->axnqosinfo[i];
                }
            }
        }
        if (m_pXnQos->cxnqosPending != 0)
            return 0;
        m_State = STATE_DONE;
        return m_hrQuery;
    case STATE_PROBING_CONNECTIVITY:
        if (m_pXnQos->cxnqosPending != 0)
            return 0;
        if (Results.m_dwSize != 0)
        {
            unsigned int kept = 0;
            for (unsigned int i = 0; i < Results.m_dwSize; ++i)
            {
                unsigned char flags =
                    m_pXnQos->axnqosinfo[i].bFlags;
                if ((flags & 2) == 0 || (flags & 4) != 0)
                {
                    Print(L"Removing Matching Session %lu (host not "
                          L"reachable or disabled)\n",
                          (unsigned long)kept);
                    Results.Remove(kept);
                }
                else
                {
                    Results[kept].qosInfo =
                        (unsigned int)&m_pXnQos->axnqosinfo[i];
                    ++kept;
                }
            }
        }
        m_State = STATE_DONE;
        return m_hrQuery;
    default:
        break;
    }
    HRESULT result = XOnlineTaskContinue(m_hSearchTask);
    if (result == 0)
        return result;
    m_hrQuery = result;
    m_State = STATE_DONE;
    if (result < 0)
        goto close_task;
    _XONLINE_MATCH_SEARCHRESULT** ppSearchResults = nullptr;
    DWORD dwNumSessions = 0;
    result = XOnlineMatchSearchGetResults(m_hSearchTask, ppSearchResults,
                                          &dwNumSessions);
    if (result >= 0)
    {
        Results.m_dwSize = dwNumSessions;
        for (unsigned int i = 0; i < dwNumSessions; ++i)
        {
            _XONLINE_MATCH_SEARCHRESULT* pRes = ppSearchResults[i];
            XOnlineMatchSessionRecord& rec = Results[i];
            memcpy(&rec.SessionID, &pRes->SessionID, sizeof(XNKID));
            memcpy(&rec.KeyExchangeKey, &pRes->KeyExchangeKey,
                   sizeof(XNKEY));
            memcpy(&rec.HostAddress, &pRes->HostAddress,
                   sizeof(XNADDR));
            rec.dwPublicOpen = pRes->dwPublicOpen;
            rec.dwPrivateOpen = pRes->dwPrivateOpen;
            rec.dwPublicFilled = pRes->dwPublicFilled;
            rec.dwPrivateFilled = pRes->dwPrivateFilled;
            result = XOnlineMatchSearchParse(
                pRes, 7, FromIDAttributeSpec, &rec);
            if (result < 0)
            {
                ASSERT("((HRESULT)(hr) >= 0)",
                       "c:\\cod\\code\\game\\xboxMatch.cpp", 1177);
            }
            m_rgpXnAddr[i] = &rec.HostAddress;
            m_rgpXnKid[i] = &rec.SessionID;
            m_rgpXnKey[i] = &rec.KeyExchangeKey;
            rec.qosInfo = 0;
        }
        if (dwNumSessions != 0)
        {
            if (XNetQosLookup(dwNumSessions, m_rgpXnAddr, m_rgpXnKid,
                              m_rgpXnKey, 0, nullptr, nullptr, 0, 0xFA00,
                              0, nullptr, &m_pXnQos) != 0)
            {
                ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp",
                       1190);
            }
            else
            {
                m_State = STATE_PROBING_CONNECTIVITY;
            }
        }
    }
close_task:
    XOnlineTaskClose(m_hSearchTask);
    m_hSearchTask = nullptr;
    return result;
}

// ea: 0x7299F0 (inline, xboxMatch.h:377)
void CFromIDQueryResults::Remove(unsigned int i)
{
    if (i >= m_dwSize)
    {
        ASSERT("i < m_dwSize", "c:\\cod\\code\\game\\xboxMatch.h", 377);
    }
    if (i < m_dwSize)
    {
        --m_dwSize;
        memmove(&v[170 * i], &v[170 * i + 170],
                170 * (m_dwSize - i));
    }
}

// ============================================================================
// CDefaultQuery - full matchmaking query (25 results)
// ============================================================================

// ea: 0x720F80
CDefaultQuery::CDefaultQuery()
{
    Results.m_dwSize = 0;
    m_State = STATE_IDLE;
    m_hrQuery = 1;  // S_FALSE
    m_hSearchTask = nullptr;
    m_pXnQos = nullptr;
}

// ea: 0x7242C0
CDefaultQuery::~CDefaultQuery()
{
    Cancel();
}

// ea: 0x720FB0
void CDefaultQuery::Cancel()
{
    if (m_State == STATE_IDLE)
        return;
    if (m_State > STATE_DONE)
    {
        ASSERT("0", "c:\\cod\\code\\game\\xboxMatch.cpp", 1274);
    }
    else
    {
        if (m_hSearchTask != nullptr)
        {
            XOnlineTaskClose(m_hSearchTask);
            m_hSearchTask = nullptr;
        }
        Clear();
        m_State = STATE_IDLE;
        m_hrQuery = 1;
    }
}

// ea: 0x721050
void CDefaultQuery::Clear()
{
    if (m_pXnQos != nullptr)
    {
        XNetQosRelease(m_pXnQos);
        m_pXnQos = nullptr;
    }
    Results.m_dwSize = 0;
}

// ea: 0x721080
HRESULT CDefaultQuery::Query(
    unsigned __int64 queryGameType, unsigned __int64 queryGameMap,
    unsigned __int64 queryGameVersion, unsigned __int64 QueryFriendlyFire,
    unsigned __int64 queryTeamBalancing, unsigned __int64 querySubType,
    unsigned __int64 queryMaxPlayers, unsigned __int64 queryMinPlayers)
{
    if (m_State == STATE_DONE)
    {
        Clear();
        m_State = STATE_IDLE;
    }
    if (m_State != STATE_IDLE)
    {
        ASSERT("m_State == STATE_IDLE", "c:\\cod\\code\\game\\xboxMatch.cpp",
               1318);
        if (m_State != STATE_IDLE)
            return 0x8000FFFF;
    }
    XONLINE_ATTRIBUTE QueryParameters[8];
    memset(&QueryParameters[0].fChanged, 0, 0x7C);
    QueryParameters[0].dwAttributeID =
        (queryGameType != 0xFFFFFFFFFFFFFFFFull) ? 0 : 0xF00000;
    QueryParameters[0].info.integer.qwValue = queryGameType;
    QueryParameters[1].dwAttributeID =
        (queryGameMap != 0xFFFFFFFFFFFFFFFFull) ? 0 : 0xF00000;
    QueryParameters[1].info.integer.qwValue = queryGameMap;
    QueryParameters[2].dwAttributeID =
        (queryGameVersion != 0xFFFFFFFFFFFFFFFFull) ? 0 : 0xF00000;
    QueryParameters[2].info.integer.qwValue = queryGameVersion;
    QueryParameters[3].dwAttributeID =
        (QueryFriendlyFire != 0xFFFFFFFFFFFFFFFFull) ? 0 : 0xF00000;
    QueryParameters[3].info.integer.qwValue = QueryFriendlyFire;
    QueryParameters[4].dwAttributeID =
        (queryTeamBalancing != 0xFFFFFFFFFFFFFFFFull) ? 0 : 0xF00000;
    QueryParameters[4].info.integer.qwValue = queryTeamBalancing;
    QueryParameters[5].dwAttributeID =
        (querySubType != 0xFFFFFFFFFFFFFFFFull) ? 0 : 0xF00000;
    QueryParameters[5].info.integer.qwValue = querySubType;
    QueryParameters[6].dwAttributeID =
        (queryMaxPlayers != 0xFFFFFFFFFFFFFFFFull) ? 0 : 0xF00000;
    QueryParameters[6].info.integer.qwValue = queryMaxPlayers;
    QueryParameters[7].dwAttributeID =
        (queryMinPlayers != 0xFFFFFFFFFFFFFFFFull) ? 0 : 0xF00000;
    QueryParameters[7].info.integer.qwValue = queryMinPlayers;
    unsigned int matched =
        XOnlineMatchSearchResultsLen(0x19, 8, DefaultAttributeSpec);
    HRESULT result =
        XOnlineMatchSearch(3, 0x19, 8, QueryParameters, matched, nullptr,
                           &m_hSearchTask);
    if (result >= 0)
        m_State = STATE_RUNNING;
    return result;
}

// ea: 0x721250
HRESULT CDefaultQuery::Probe()
{
    if (m_State != STATE_DONE)
    {
        ASSERT("m_State == STATE_DONE",
               "c:\\cod\\code\\game\\xboxMatch.cpp", 1364);
        if (m_State != STATE_DONE)
            return 0x8000FFFF;
    }
    if (m_pXnQos != nullptr)
    {
        XNetQosRelease(m_pXnQos);
        m_pXnQos = nullptr;
    }
    unsigned int m_dwSize = Results.m_dwSize;
    if (m_dwSize == 0)
        return 0;
    for (unsigned int i = 0; i < m_dwSize; ++i)
    {
        XOnlineMatchSessionRecord& rec = Results[i];
        m_rgpXnAddr[i] = &rec.HostAddress;
        m_rgpXnKid[i] = &rec.SessionID;
        m_rgpXnKey[i] = &rec.KeyExchangeKey;
        rec.qosInfo = 0;
    }
    if (XNetQosLookup(m_dwSize, m_rgpXnAddr, m_rgpXnKid, m_rgpXnKey, 0,
                      nullptr, nullptr, 2, 0xFA00, 0, nullptr,
                      &m_pXnQos) == 0)
    {
        m_State = STATE_PROBING_BANDWIDTH;
        return 0;
    }
    ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp", 1389);
    return 0x80004005;
}

// ea: 0x7213A0
HRESULT CDefaultQuery::Process()
{
    switch (m_State)
    {
    case STATE_IDLE:
        return 0;
    case STATE_DONE:
        return m_hrQuery;
    case STATE_PROBING_BANDWIDTH:
        if (Results.m_dwSize != 0)
        {
            for (unsigned int i = 0; i < Results.m_dwSize; ++i)
            {
                if (Results[i].qosInfo == 0
                    && (m_pXnQos->axnqosinfo[i].bFlags & 1) != 0)
                {
                    Results[i].qosInfo =
                        (unsigned int)&m_pXnQos->axnqosinfo[i];
                }
            }
        }
        if (m_pXnQos->cxnqosPending != 0)
            return 0;
        m_State = STATE_DONE;
        return m_hrQuery;
    case STATE_PROBING_CONNECTIVITY:
        if (m_pXnQos->cxnqosPending != 0)
            return 0;
        if (Results.m_dwSize != 0)
        {
            unsigned int kept = 0;
            for (unsigned int i = 0; i < Results.m_dwSize; ++i)
            {
                unsigned char flags =
                    m_pXnQos->axnqosinfo[i].bFlags;
                if ((flags & 2) == 0 || (flags & 4) != 0)
                {
                    Print(L"Removing Matching Session %lu (host not "
                          L"reachable or disabled)\n",
                          (unsigned long)kept);
                    Results.Remove(kept);
                }
                else
                {
                    Results[kept].qosInfo =
                        (unsigned int)&m_pXnQos->axnqosinfo[i];
                    ++kept;
                }
            }
        }
        m_State = STATE_DONE;
        return m_hrQuery;
    default:
        break;
    }
    HRESULT result = XOnlineTaskContinue(m_hSearchTask);
    if (result == 0)
        return result;
    m_hrQuery = result;
    m_State = STATE_DONE;
    if (result < 0)
        goto close_task;
    _XONLINE_MATCH_SEARCHRESULT** ppSearchResults = nullptr;
    DWORD dwNumSessions = 0;
    result = XOnlineMatchSearchGetResults(m_hSearchTask, ppSearchResults,
                                          &dwNumSessions);
    if (result >= 0)
    {
        Results.m_dwSize = dwNumSessions;
        for (unsigned int i = 0; i < dwNumSessions; ++i)
        {
            _XONLINE_MATCH_SEARCHRESULT* pRes = ppSearchResults[i];
            XOnlineMatchSessionRecord& rec = Results[i];
            memcpy(&rec.SessionID, &pRes->SessionID, sizeof(XNKID));
            memcpy(&rec.KeyExchangeKey, &pRes->KeyExchangeKey,
                   sizeof(XNKEY));
            memcpy(&rec.HostAddress, &pRes->HostAddress,
                   sizeof(XNADDR));
            rec.dwPublicOpen = pRes->dwPublicOpen;
            rec.dwPrivateOpen = pRes->dwPrivateOpen;
            rec.dwPublicFilled = pRes->dwPublicFilled;
            rec.dwPrivateFilled = pRes->dwPrivateFilled;
            result = XOnlineMatchSearchParse(
                pRes, 8, DefaultAttributeSpec, &rec);
            if (result < 0)
            {
                ASSERT("((HRESULT)(hr) >= 0)",
                       "c:\\cod\\code\\game\\xboxMatch.cpp", 1500);
            }
            m_rgpXnAddr[i] = &rec.HostAddress;
            m_rgpXnKid[i] = &rec.SessionID;
            m_rgpXnKey[i] = &rec.KeyExchangeKey;
            rec.qosInfo = 0;
        }
        if (dwNumSessions != 0)
        {
            if (XNetQosLookup(dwNumSessions, m_rgpXnAddr, m_rgpXnKid,
                              m_rgpXnKey, 0, nullptr, nullptr, 0, 0xFA00,
                              0, nullptr, &m_pXnQos) != 0)
            {
                ASSERT("iQos == 0", "c:\\cod\\code\\game\\xboxMatch.cpp",
                       1513);
            }
            else
            {
                m_State = STATE_PROBING_CONNECTIVITY;
            }
        }
    }
close_task:
    XOnlineTaskClose(m_hSearchTask);
    m_hSearchTask = nullptr;
    return result;
}

// ea: 0x7298C0 (inline, xboxMatch.h:293)
void CDefaultQueryResults::Remove(unsigned int i)
{
    if (i >= m_dwSize)
    {
        ASSERT("i < m_dwSize", "c:\\cod\\code\\game\\xboxMatch.h", 293);
    }
    if (i < m_dwSize)
    {
        --m_dwSize;
        memmove(&v[170 * i], &v[170 * i + 170],
                170 * (m_dwSize - i));
    }
}
