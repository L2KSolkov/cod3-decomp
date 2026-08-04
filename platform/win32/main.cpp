// ============================================================================
// COD3 Win32 Entry Point — Phase 0
// Links with all stubs; will print message and exit.
// ============================================================================

#include <windows.h>
#include <cstdio>

int main()
{
    OutputDebugStringA("========================================\n");
    OutputDebugStringA("  Call of Duty 3 MP — Decompilation\n");
    OutputDebugStringA("  Phase 0: Stub build (green-link)\n");
    OutputDebugStringA("========================================\n");

    MessageBoxA(NULL,
        "Call of Duty 3 MP Decompilation\n\n"
        "Phase 0 build successful — all stubs linked.\n"
        "Progress = shrinking stub files.\n\n"
        "See DECOMP_PLAN.md for details.",
        "COD3MP — Phase 0",
        MB_OK | MB_ICONINFORMATION);

    return 0;
}
