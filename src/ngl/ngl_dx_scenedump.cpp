// ============================================================================
// ngl_dx_scenedump.cpp - host scene-dump file I/O (3 funcs).
// Source: src/xbox/ngl_dx_scenedump.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_dx_scenedump.o).
// ============================================================================

#include "ngl/nglDebug.h"
#include "core/tlFixedString.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void tlWarning(const char* fmt, ...);

// ============================================================================
// nglHostPrintf - ea: 0x8541A0
// ============================================================================
void nglHostPrintf(void* File, const char* Format, ...) {
    char Work[4096];
    va_list ap;
    va_start(ap, Format);
    vsprintf(Work, Format, ap);
    va_end(ap);

    void* v2 = File;
    if (File != NULL) {
        DWORD written = 0;
        if (!WriteFile((HANDLE)File, Work, (DWORD)strlen(Work), &written, NULL)) {
            CloseHandle((HANDLE)v2);
            tlWarning("nglHostPrintf: write error !\n");
        }
    }
}

// ============================================================================
// nglHostOpen - ea: 0x854210
// ============================================================================
void* nglHostOpen(const char* FileName) {
    char Work[512];
    strcpy(Work, "d:\\");
    strcat(Work, FileName);

    HANDLE result = CreateFileA(Work, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                0x8000080u, NULL);
    if (result == INVALID_HANDLE_VALUE) {
        tlWarning("nglHostOpen: cannot open \"%s\" for writing !\n", Work);
        return NULL;
    }
    return (void*)result;
}

// ============================================================================
// nglHostClose - ea: 0x8542B0
// ============================================================================
void nglHostClose(void* File) {
    CloseHandle((HANDLE)File);
}
