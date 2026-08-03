// ============================================================================
// AE Assert — assertion system for Treyarch's Angel Engine
// Source: PoolAllocator.cpp (uses these statics), ae_assert system
// ea: 0x7BE240 (IsIgnored), 0x7BEC70-0x7BED1E (Assert/Error/Warning)
// ============================================================================

#include <cstdio>
#include <cstring>
#include <cstdarg>

#ifdef _WIN32
#include <windows.h>
#define COD3_DEBUG_OUT(s) OutputDebugStringA(s)
#else
#define COD3_DEBUG_OUT(s) fprintf(stderr, "%s", s)
#endif

namespace AeAssert {

// Global state
int gCurrentAuthor = 0;
const char* gCurrentFile = "";
int gCurrentLine = 0;
const char* gCurrentExpr = "";
int num_ignored_asserts = 0;

// Ignored assert table — matches by file + line + optional expression
struct IgnoredAssert {
    int         line;
    const char* file;
    const char* expr;
};
#define MAX_IGNORED 64
static IgnoredAssert s_ignored[MAX_IGNORED];

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
// OnScreenMessageHandler — display assertion on screen / debug output
// ea: referenced by Assert, Error, Warning
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
// SetupRouterAssert — configure assertion routing (static stub)
// ea: 0x7BE430
// ============================================================================
void SetupRouterAssert(int a1, const char* a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    // TODO: reconstruct from ea:0x7BE430
}

// ============================================================================
// PumpXenonUI — process Xbox UI for assert dialog (stub on Win32)
// ea: 0x7BE4F0
// ============================================================================
void PumpXenonUI() {
    // NOP on Win32 — Xbox-only
}

} // namespace AeAssert
