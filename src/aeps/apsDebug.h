// ============================================================================
// apsDebug — APS debug/print facility (2 non-inline funcs + 1 data).
// Source: c:\cod\code\tl\aeps\source\apsDebug.cpp
// Verified against IDA (aeps_xboxr:apsDebug.o):
//   PrintWarning @0x800060  (?PrintWarning@apsDebug@@SAXW4eWarningLevel@1@PBDZZ)
//   Print        @0x8000C0  (?Print@apsDebug@@SAXPBDZZ)
//   Enabled      @0x800050  (?Enabled@apsDebug@@SAIXZ)      inline COMDAT
//   SetEnabled   @0x7EB360  (?SetEnabled@apsDebug@@SAXI@Z)  inline COMDAT (apsCommon.o)
//   mbEnabled    @0x14CEE80 (data, unsigned int)
// ============================================================================
#ifndef COD3_AEPS_APSDEBUG_H
#define COD3_AEPS_APSDEBUG_H

// ============================================================================
// apsDebug — global APS print/assert toggles.
// ============================================================================
class apsDebug {
public:
    enum eWarningLevel {
        BLUE_ALERT = 0,
        GREEN_ALERT = 1,
        YELLOW_ALERT = 2,
        RED_ALERT = 3,
    };

    static unsigned int Enabled() { return mbEnabled; }                       // ?Enabled@apsDebug@@SAIXZ
    static void         SetEnabled(unsigned int bEnabled) { mbEnabled = bEnabled; }  // ?SetEnabled@apsDebug@@SAXI@Z
    static void         PrintWarning(eWarningLevel level, const char* format, ...);  // @@SAX...PBDZZ
    static void         Print(const char* format, ...);    // ?Print@apsDebug@@SAXPBDZZ

private:
    static unsigned int mbEnabled;                         // ?mbEnabled@apsDebug@@0IA
};

#endif // COD3_AEPS_APSDEBUG_H
