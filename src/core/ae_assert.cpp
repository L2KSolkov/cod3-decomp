// ============================================================================
// AE Assert — assertion system for Treyarch's Angel Engine
// Source: PoolAllocator.cpp (uses these statics), ae_assert system
// ea: 0x7BE240 (IsIgnored), 0x7BEC70-0x7BED1E (Assert/Error/Warning)
// ============================================================================

#include <cstdio>
#include <cstring>
#include <cstdarg>

extern void tlPrintf(const char* fmt, ...);

#ifdef _WIN32
#include <windows.h>
#define COD3_DEBUG_OUT(s) OutputDebugStringA(s)
#else
#define COD3_DEBUG_OUT(s) fprintf(stderr, "%s", s)
#endif

namespace AeAssert {

// Matches the binary's enum-typed global (?gCurrentAuthor@AeAssert@@3W4ECoderId@1@A,
// core_xboxr:AeAssert.o) - declaring/defining as `int` mangles to 3HA and
// leaves every reference unresolved.
enum ECoderId {
    COD3 = 0,
    ARO = 1,
    CD = 2,
    JRS = 3,
    JSV = 10,
};

// Global state
ECoderId gCurrentAuthor = COD3;
const char* gCurrentFile = "";
int gCurrentLine = 0;
const char* gCurrentExpr = "";
int num_ignored_asserts = 0;
bool gInAssert = false;        // ?gInAssert@AeAssert@@3_NA (core_xboxr:AeAssert.o)
bool gAssertsEnabled = true;   // ?gAssertsEnabled@AeAssert@@3_NA (core_xboxr:AeAssert.o)

// Ignored assert table — matches by file + line + optional expression
struct IgnoredAssert {
    int         line;
    char        file[256];
    char        expr[1024];
    IgnoredAssert();
};
#define MAX_IGNORED 500
static IgnoredAssert s_ignored[MAX_IGNORED];

IgnoredAssert::IgnoredAssert() {
    file[0] = 0;
    expr[0] = 0;
}

struct InfiniteRecursionStopper {
    bool* mRecursing;
    explicit InfiniteRecursionStopper(bool* ref);
    ~InfiniteRecursionStopper();
};

InfiniteRecursionStopper::InfiniteRecursionStopper(bool* ref)
    : mRecursing(ref) {
    *ref = true;
}

InfiniteRecursionStopper::~InfiniteRecursionStopper() {
    *mRecursing = false;
}

enum ONSCREEN_MESSAGE_TYPE {
    ONSCREEN_MESSAGE_ASSERT = 0,
    ONSCREEN_MESSAGE_ERROR  = 1,
    ONSCREEN_MESSAGE_WARNING = 2,
};

// ============================================================================
// IsIgnored — check if current assert location is in the ignore list
// ea: 0x7BE240
// ============================================================================
bool IsIgnored() {
    for (int i = 0; i < num_ignored_asserts; ++i) {
        if (s_ignored[i].line == gCurrentLine &&
            strcmp(gCurrentFile, s_ignored[i].file) == 0) {
            if (gCurrentExpr) {
                if (strcmp(gCurrentExpr, s_ignored[i].expr) == 0)
                    return true;
            } else if (!s_ignored[i].expr[0]) {
                return true;
            }
        }
    }
    return false;
}

// ============================================================================
// OnScreenMessageHandler - display assertion on screen / debug output
// ea: 0x7BE500
// ============================================================================
static bool OnScreenMessageHandler(ONSCREEN_MESSAGE_TYPE type, const char* text) {
    const char* prefix = "ASSERT";
    if (type == ONSCREEN_MESSAGE_ERROR) prefix = "ERROR";
    else if (type == ONSCREEN_MESSAGE_WARNING) prefix = "WARNING";

    char buf[2048];
    if (gCurrentExpr && gCurrentExpr[0]) {
        snprintf(buf, sizeof(buf), "%s: %s(%d): %s — %s\n",
                 prefix, gCurrentFile, gCurrentLine, gCurrentExpr, text);
    } else {
        snprintf(buf, sizeof(buf), "%s: %s(%d): %s\n",
                 prefix, gCurrentFile, gCurrentLine, text);
    }

    COD3_DEBUG_OUT(buf);

    // In debug: break. In release: may log and continue.
#ifdef _DEBUG
    return true;  // caller will __debugbreak()
#else
    return false;
#endif
}

// ============================================================================
// Assert — formatted assertion (returns true to trigger breakpoint)
// ea: 0x7BEC70
// ============================================================================
bool Assert(const char* fmt, ...) {
    char text[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);
    return OnScreenMessageHandler(ONSCREEN_MESSAGE_ASSERT, text);
}

// ============================================================================
// Error — formatted error (returns true to trigger breakpoint)
// ea: 0x7BECB0
// ============================================================================
bool Error(const char* fmt, ...) {
    char text[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);
    return OnScreenMessageHandler(ONSCREEN_MESSAGE_ERROR, text);
}

// ============================================================================
// Warning — formatted warning (returns true to trigger breakpoint)
// ea: 0x7BECF0
// ============================================================================
bool Warning(const char* fmt, ...) {
    char text[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);
    return OnScreenMessageHandler(ONSCREEN_MESSAGE_WARNING, text);
}

// ============================================================================
// SetupRouterAssert — emit router assertion records
// ea: 0x7BE430
// ============================================================================
void SetupRouterAssert(int line, const char* pPakFile, const char* pPakResource,
                       const char* pFile, const char* pAuthor,
                       const char* pExpr, const char* pTxt,
                       const char* pType) {
    char pakBuf[256];
    char buf[256];

    _snprintf(pakBuf, sizeof(pakBuf), "%s(%s)", pPakFile, pPakResource);
    _snprintf(buf, sizeof(buf), "RTCMD:O%s", pType);
    tlPrintf(buf);
    _snprintf(buf, sizeof(buf), "RTCMD:I%d", line, pFile, pakBuf, pAuthor, pExpr);
    tlPrintf(buf);
    _snprintf(buf, sizeof(buf), "RTCMD:T%s", pTxt);
    tlPrintf(buf);
}

// ============================================================================
// PumpXenonUI — process Xbox UI for assert dialog (stub on Win32)
// ea: 0x7BE4F0
// ============================================================================
void PumpXenonUI() {
    // NOP on Win32 — Xbox-only
}

} // namespace AeAssert
