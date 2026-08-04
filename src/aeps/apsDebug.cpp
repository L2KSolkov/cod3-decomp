// ============================================================================
// apsDebug.cpp — APS debug/print facility (2 non-inline funcs + 1 data).
// Source: c:\cod\code\tl\aeps\source\apsDebug.cpp
// Verified against IDA (aeps_xboxr:apsDebug.o):
//   PrintWarning @0x800060  (?PrintWarning@apsDebug@@SAXW4eWarningLevel@1@PBDZZ)
//   Print        @0x8000C0  (?Print@apsDebug@@SAXPBDZZ)
//   mbEnabled    @0x14CEE80 (data, unsigned int)
// ============================================================================
#include "apsDebug.h"

#include <stdarg.h>
#include <stdio.h>

// tl_system.o (tl_xboxr, ported)
extern void tlPrint(const char* text);

// ============================================================================
// Data globals owned by apsDebug.o
// ============================================================================
unsigned int apsDebug::mbEnabled = 0;

// ============================================================================
// apsDebug::PrintWarning — format and print a warning line if enabled.
// ea: 0x800060
// ============================================================================
void apsDebug::PrintWarning(apsDebug::eWarningLevel level, const char* format, ...) {
    char workBuffer[512];
    va_list ap;
    va_start(ap, format);

    if (apsDebug::mbEnabled != 0) {
        vsprintf(workBuffer, format, ap);
        tlPrint("");
        tlPrint("Aeps Warning : ");
        tlPrint(workBuffer);
        tlPrint("\n");
    }
}

// ============================================================================
// apsDebug::Print — format and print a string if enabled.
// ea: 0x8000C0
// ============================================================================
void apsDebug::Print(const char* format, ...) {
    char workBuffer[512];
    va_list ap;
    va_start(ap, format);

    if (apsDebug::mbEnabled != 0) {
        vsprintf(workBuffer, format, ap);
        tlPrint(workBuffer);
    }
}
