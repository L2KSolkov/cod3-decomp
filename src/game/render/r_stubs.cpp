// ============================================================================
// r_stubs.cpp - render.o world/render-command layer (tr_world.cpp, renderer)
// ============================================================================

#include "game/logic/g_local.h"
#include "game/render/xsurface.h"

#include <string.h>
#include <intrin.h>

// Minimal IDA-verified views for render.o globals
struct BspTree;
struct world_t {
    void* mSky;      // +0x00
    BspTree* bspTree;  // +0x04
    world_t();       // ??0world_t@@QAE@XZ (render.o 0x6BECC0)
};

struct trGlobals_t {
    uint8_t _pad[0x40];
    world_t* world;   // +0x40
    struct DebugBlock {
        uint8_t _pad[0x80];
        void* externStrings;   // +0x80
        int numExternStrings;  // +0x84
        void* externLines;     // +0x88
        int numExternLines;    // +0x8C
    } debug;          // +0x40 region (offsets via disasm)
};
extern trGlobals_t tr;           // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0
extern world_t s_worldData;      // ?s_worldData@@3Uworld_t@@A @ 0xF74B98
extern BspTree* g_bspTree;       // ?g_bspTree@@3PAVBspTree@@A @ 0xF743DC
extern int sCurColor;            // ?sCurColor@@3IA @ 0xF74290
struct trDebugString_t;
struct trDebugLine_t;

// XModelPartsManager / XModelPartsBank (opaque views)
class XModelPartsBank;
class XModelPartsManager {
public:
private:
    virtual void OnBankUnloaded(XModelPartsBank& xmpBank);  // ?OnBankUnloaded@XModelPartsManager@@EAEXAAVXModelPartsBank@@@Z
};

struct dpvs_plane_t {
    math::Vector4 data;   // +0x00
    int side[3];          // +0x10
};
extern int r_zfar_value;  // float value member of ?r_zfar@@3PAUcvar_t@@A
extern float g_dpvs_cullDist;  // g_dpvs.cullDist @ 0xF75600
struct drawSurf_s;        // tr renderer surface
struct trStatistics_t;    // tr renderer statistics

// ============================================================================
// world
// ============================================================================

// ea: 0x006BECC0
world_t::world_t()
{
    mSky = nullptr;
}

// ea: 0x006BECD0
void RE_LoadWorldMap(const char* name, int* checksum)
{
    (void)name;
    (void)checksum;
    tr.world = &s_worldData;
    s_worldData.bspTree = g_bspTree;
}

// ============================================================================
// render command layer
// ============================================================================

// ea: 0x006BED00
void R_PerformanceCounters()
{
}

// ea: 0x006BED10
void R_IssueRenderCommands(int frameCount)
{
    (void)frameCount;
}

// ea: 0x006BED20
void R_SyncRenderThread()
{
}

// ea: 0x006BED30
void R_AddDrawSurfCmd(drawSurf_s* drawSurf, int surfNum)
{
    (void)drawSurf;
    (void)surfNum;
}

// ea: 0x006BF020
void RE_SaveScreen()
{
}

// ea: 0x006BF030
void RE_TrackStatistics(trStatistics_t* statistics)
{
    (void)statistics;
}

// ea: 0x006BED40
void RE_SetColor(const float* rgba)
{
    if (rgba != nullptr)
    {
        unsigned int alpha = (unsigned int)(rgba[3] * 255.0f);
        unsigned int r = (unsigned int)(rgba[0] * 255.0f);
        unsigned int g = (unsigned int)(rgba[1] * 255.0f);
        unsigned int b = (unsigned int)(rgba[2] * 255.0f);
        sCurColor = (int)((alpha << 24) | (b << 16) | (g << 8) | r);
    }
    else
    {
        sCurColor = -1;
    }
}

// ea: 0x006BF060
void RE_LocateDebugStrings(trDebugString_t* strings, int numStrings)
{
    tr.debug.externStrings = strings;
    tr.debug.numExternStrings = numStrings;
}

// ea: 0x006BF080
void RE_LocateDebugLines(trDebugLine_t* lines, int numLines)
{
    tr.debug.externLines = lines;
    tr.debug.numExternLines = numLines;
}

// ea: 0x006BDC00
void XModelPartsManager::OnBankUnloaded(XModelPartsBank& xmpBank)
{
    (void)xmpBank;
}

// ea: 0x006BF330
void R_SetPlaneSidesDPVS(dpvs_plane_t* plane)
{
    plane->side[0] = plane->data.v.m128_f32[0] <= 0.0f ? 0 : 0xC;
    plane->side[1] = plane->data.v.m128_f32[1] <= 0.0f ? 4 : 16;
    plane->side[2] = plane->data.v.m128_f32[2] <= 0.0f ? 8 : 20;
    plane->data.v.m128_f32[3] =
        plane->data.v.m128_f32[3] - 0.0049999999f;
}

// ea: 0x006BF380
float RE_GetFarPlaneDist()
{
    float zfar = *(float*)&r_zfar_value;
    if (g_dpvs_cullDist > zfar)
        return g_dpvs_cullDist;
    return zfar;
}
