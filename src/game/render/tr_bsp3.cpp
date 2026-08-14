// ============================================================================
// tr_bsp3.cpp - render.o BSP decode (tr_bsp.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"

#include <string.h>

// AeAssert (game.o)
namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

class BspTree;
class PakFile;
enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };

extern BspTree* g_bspTree;      // ?g_bspTree@@3PAVBspTree@@A @ 0xF743DC

class BspTreeMethods {
public:
    void Fixup();       // ?Fixup@BspTree@@QAEXXZ (render.o inline COMDAT)
    void post_fixup();  // ?post_fixup@BspTree@@QAEXXZ (render.o inline COMDAT)
};

// refimport_t view (Error +0x04)
struct refimport_t {
    uint8_t _pad[0x04];
    void (*Error)(int code, const char* fmt, ...);  // +0x04
};
extern refimport_t ri;  // ?ri@@3Urefimport_t@@A @ 0xF741E8

// trGlobals_t view (worldMapLoaded +0x04, world +0x290)
struct trGlobals_t {
    int registered;            // +0x00
    int worldMapLoaded;        // +0x04
    uint8_t _pad[0x290 - 0x08];
    void* world;               // +0x290
};
extern trGlobals_t tr;         // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

// world_t (streamer/pakmanager.cpp defines s_worldData)
struct world_t {
    char name[128];            // +0x00
    char baseName[128];        // +0x80
    uint8_t _pad[0x108 - 0x100];
    void* mSky;                // +0x108
};
extern world_t s_worldData;    // ?s_worldData@@3Uworld_t@@A @ 0xF74B98

// glfog_t (tr_fog.cpp defines glfogsettings)
struct glfog_t {
    int mode;           // +0x00
    int hint;           // +0x04
    int startTime;      // +0x08
    int finishTime;     // +0x0C
    float color[4];     // +0x10
    float start;        // +0x20
    float end;          // +0x24
    int useEndForClip;  // +0x28
    float density;      // +0x2C
    int registered;     // +0x30
    int drawsky;        // +0x34
    int clearscreen;    // +0x38
    int dirty;          // +0x3C
};
extern glfog_t glfogsettings[9];  // ?glfogsettings@@3PAUglfog_t@@A (tr_fog.cpp)

// game.o helpers
void Q_strncpyz(char* dest, const char* src, int destsize);
char* Com_SkipPath(char* pathname);
void Com_StripExtension(const char* in, char* out);
void InitEntitiesBSP();

// ============================================================================
// DecodeBSP - ea: 0x006C4D60
// ============================================================================
void DecodeBSP(const char* name, unsigned char* data, int size,
               TPakId pakId, PakFile* pakFile)
{
    (void)size; (void)pakId; (void)pakFile;
    if (g_bspTree != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_bsp.cpp";
        AeAssert::gCurrentLine = 181;
        AeAssert::gCurrentExpr = "g_bspTree==0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("BSP already loaded!"))
            __debugbreak();
    }
    g_bspTree = (BspTree*)data;
    ((BspTreeMethods*)g_bspTree)->Fixup();
    ((BspTreeMethods*)g_bspTree)->post_fixup();
    if (tr.worldMapLoaded != 0)
        ri.Error(1, "ERROR: attempted to redundantly load world map\n");
    tr.worldMapLoaded = 1;
    memset(&s_worldData, 0, sizeof(s_worldData));
    glfogsettings[1].registered = 0;
    glfogsettings[2].registered = 0;
    glfogsettings[3].registered = 0;
    glfogsettings[5].registered = 0;
    glfogsettings[7].registered = 0;
    glfogsettings[4].registered = 0;
    tr.world = nullptr;
    Q_strncpyz(s_worldData.name, name, 128);
    char* v2 = Com_SkipPath(s_worldData.name);
    Q_strncpyz(s_worldData.baseName, v2, 128);
    Com_StripExtension(s_worldData.baseName, s_worldData.baseName);
    tr.world = &s_worldData;
    InitEntitiesBSP();
}
