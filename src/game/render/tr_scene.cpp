// ============================================================================
// tr_scene.cpp - render.o scene/shadow/lod helpers (tr_main.cpp, tr_shadow.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"

#include <math.h>

struct nglTexture;

// ngl.o helpers
nglTexture* nglCreateTexture(unsigned int Flags, unsigned int Format, int Width,
                             int Height, int Depth, int Levels);
void nglWaitForRendering();
void nglDestroyTexture(nglTexture* Tex);

// q_math.o (returns const float per binary mangling ?A?BMQBM0)
const float VectorDistance(const float* const p1, const float* const p2);

// apsCommon::Report (aepsCommon)
class apsCommon {
public:
    static void Report();  // ?Report@apsCommon@@SAXXZ
};

// apsMemory (aepsMemory)
class apsMemory {
public:
    static void Reset();                // ?Reset@apsMemory@@YAXXZ
    static unsigned int AddPool(int numBlocks, int blockSize);  // ?AddPool@apsMemory@@YAIHH@Z
};

// render.o data (scene counters / proj shadow)
int r_firstSceneDlight;   // ?r_firstSceneDlight@@3HA @ 0xF74274
int r_numdlights;         // ?r_numdlights@@3HA @ 0xF741E4
int r_firstSceneCorona;   // ?r_firstSceneCorona@@3HA @ 0xF742D0
int r_numcoronas;         // ?r_numcoronas@@3HA @ 0xF741BC
int r_firstScenePoly;     // ?r_firstScenePoly@@3HA @ 0xF742B0
int r_numpolys;           // ?r_numpolys@@3HA @ 0xF742FC
unsigned int gProjShadowTexSize;  // ?gProjShadowTexSize@@3IA @ 0xDFB148
extern nglTexture* gProjShadowTex;  // ?gProjShadowTex@@3PAUnglTexture@@A (cdGlassShader.cpp)

// refEntity_t view (origin +0x3C, scale +0x38)
struct refEntity_t {
    uint8_t _pad0[0x38];
    float scale;           // +0x38
    float origin[3];       // +0x3C
};

// orientationr_t view (origin +0x00)
struct orientationr_t {
    float origin[3];       // +0x00
};

// viewParms_t view (or +0x00, lodBias +0x138, lodScale +0x13C)
struct viewParms_t {
    orientationr_t or;           // +0x00
    uint8_t _pad[0x138 - 0x0C];
    float lodBias;               // +0x138
    float lodScale;              // +0x13C
};

struct trGlobals_t {
    uint8_t _pad0[0x10];
    viewParms_t viewParms;       // +0x10
};
extern trGlobals_t tr;           // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

struct polyVert_t;

// ============================================================================
// EndOfRenderCallback - ea: 0x006C2370
// ============================================================================
void EndOfRenderCallback(void* Data)
{
    (void)Data;
}

// ============================================================================
// cdProjShadow_Init - ea: 0x006C23C0
// ============================================================================
nglTexture* cdProjShadow_Init()
{
    nglTexture* result = gProjShadowTex;
    if (gProjShadowTex == nullptr)
    {
        result = nglCreateTexture(0x4010u, 6u, (int)gProjShadowTexSize,
                                  (int)gProjShadowTexSize, 0, 1);
        gProjShadowTex = result;
    }
    return result;
}

// ============================================================================
// cdProjShadow_CleanUp - ea: 0x006C23F0
// ============================================================================
void cdProjShadow_CleanUp()
{
    if (gProjShadowTex != nullptr)
    {
        nglWaitForRendering();
        nglDestroyTexture(gProjShadowTex);
        gProjShadowTex = nullptr;
    }
}

// ============================================================================
// RE_ClearScene - ea: 0x006C29A0
// ============================================================================
void RE_ClearScene()
{
    r_firstSceneDlight = r_numdlights;
    r_firstSceneCorona = r_numcoronas;
    r_firstScenePoly = r_numpolys;
}

// ============================================================================
// RE_AddPolyToScene - ea: 0x006C29D0
// ============================================================================
void RE_AddPolyToScene(int numVerts, int type, const polyVert_t* verts)
{
    (void)numVerts; (void)type; (void)verts;
}

// ============================================================================
// R_GetLodDist(const refEntity_t*) - ea: 0x006C2A00
// ============================================================================
float R_GetLodDist(const refEntity_t* ent)
{
    float v2 = VectorDistance(ent->origin, tr.viewParms.or.origin);
    float scale = ent->scale;
    float dist = tr.viewParms.lodScale * v2 + tr.viewParms.lodBias;
    if (scale != 0.0f)
        return dist / ent->scale;
    return dist;
}

// ============================================================================
// R_GetLodDist(const math::Mat43&, float) - ea: 0x006C2A60
// ============================================================================
float R_GetLodDist(const math::Mat43& matrix, float scale)
{
    __m128 origin;
    origin.m128_f32[0] = tr.viewParms.or.origin[0];
    origin.m128_f32[1] = tr.viewParms.or.origin[1];
    origin.m128_f32[2] = tr.viewParms.or.origin[2];
    origin.m128_f32[3] = 0.0f;
    __m128 v2 = _mm_sub_ps(matrix.w.v, origin);
    __m128 v3 = _mm_mul_ps(v2, v2);
    float len = sqrtf(v3.m128_f32[0] + (v3.m128_f32[1] + v3.m128_f32[2]));
    return (len * tr.viewParms.lodScale + tr.viewParms.lodBias) / scale;
}

// ============================================================================
// apsReportStatus - ea: 0x006C2E50
// ============================================================================
void apsReportStatus()
{
    apsCommon::Report();
}

// ============================================================================
// apsResetPools - ea: 0x006C2E90
// ============================================================================
void apsResetPools()
{
}

// ============================================================================
// apsAddPool - ea: 0x006C2EA0
// ============================================================================
void apsAddPool(int numBlocks, int blockSize)
{
    (void)numBlocks; (void)blockSize;
}

// ============================================================================
// apsCreateDefaultPools - ea: 0x006C2EB0
// ============================================================================
void apsCreateDefaultPools()
{
}

// ============================================================================
// apsCreateBigBufferPools - ea: 0x006C2EC0
// ============================================================================
void apsCreateBigBufferPools()
{
    apsMemory::Reset();
    apsMemory::AddPool(500, 64);
    apsMemory::AddPool(400, 128);
    apsMemory::AddPool(400, 256);
    apsMemory::AddPool(300, 512);
    apsMemory::AddPool(200, 1024);
    apsMemory::AddPool(100, 2048);
    apsMemory::AddPool(60, 3072);
    apsMemory::AddPool(50, 4096);
    apsMemory::AddPool(50, 5120);
    apsMemory::AddPool(30, 6144);
    apsMemory::AddPool(30, 7168);
    apsMemory::AddPool(50, 0x2000);
    apsMemory::AddPool(15, 0x4000);
}

// ============================================================================
// apsInitLevelSpecificParticleMemory - ea: 0x006C3070
// ============================================================================
void apsInitLevelSpecificParticleMemory(const char* name)
{
    (void)name;
}

// ============================================================================
// apsUpdate - ea: 0x006C3080
// ============================================================================
void apsUpdate(float deltaTime)
{
    (void)deltaTime;
}
