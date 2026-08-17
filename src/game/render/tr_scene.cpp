// ============================================================================
// tr_scene.cpp - render.o scene/shadow/lod helpers (tr_main.cpp, tr_shadow.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_scene.h"
#include "ngl/ngl_lighting.h"

#include <math.h>
#include <string.h>

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
namespace apsMemory {
    void Reset();
    unsigned int AddPool(int numBlocks, int blockSize);
}

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
    int    reType;          // +0x00
    int    renderfx;        // +0x04
    uint8_t _pad0[0x38 - 0x08];
    float scale;           // +0x38
    float origin[3];       // +0x3C
};

// orientationr_t (IDA type; size 0x7C)
struct orientationr_t {
    float origin[3];        // +0x00
    float axis[3][3];       // +0x0C
    float viewOrigin[3];    // +0x30
    float modelMatrix[16];  // +0x3C
};

// viewParms_t (IDA type; align 16, size 0x1D4)
struct viewParms_t {
    orientationr_t or;          // +0x00
    orientationr_t world;       // +0x7C
    float pvsOrigin[3];         // +0xF8
    int isPortal;               // +0x104
    int isMirror;               // +0x108
    int frameCount;             // +0x10C
    uint8_t _pad[0x130 - 0x110];
    float fovX;                 // +0x130
    float fovY;                 // +0x134
    float lodBias;              // +0x138
    float lodScale;             // +0x13C
    uint8_t _pad2[0x180 - 0x140];
    float zFar;                 // +0x180
};

// trRefdef_t view (refdef at tr+0x26C; IDA type, size 0x24)
struct trRefdef_t {
    int x;                   // +0x00
    int y;                   // +0x04
    int width;               // +0x08
    int height;              // +0x0C
    float fov_x;             // +0x10
    float fov_y;             // +0x14
    int time;                // +0x18
    int rdflags;             // +0x1C
    short num_world_dlights; // +0x20
    short num_model_dlights; // +0x22
};

// viewModelInfo_t (IDA type; size 0x70)
struct viewModelInfo_t {
    int mDoingRender;       // +0x00
    int mInWorldScene;      // +0x04
    int mScaleWeaponTrans;  // +0x08
    int mDrawBeforeWorld;   // +0x0C
    uint8_t _pad[0x70 - 0x10];
};

struct trGlobals_t {
    int      registered;         // +0x00
    int      worldMapLoaded;     // +0x04
    int      frameCount;         // +0x08
    int      viewCount;          // +0x0C
    viewParms_t viewParms;       // +0x10
    uint8_t  _pad0[0x26C - 0x10 - 0x184];
    trRefdef_t refdef;           // +0x26C
    uint8_t  _pad1[0x290 - 0x270];
    void*    world;              // +0x290
    uint8_t  _pad2[0x2A0 - 0x294];
    viewModelInfo_t viewModelInfo[1];  // +0x2A0
    int      viewModelInfoIndex;       // +0x310
};
extern trGlobals_t tr;           // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

struct polyVert_t;

// reserved_dlist<trRefEntity> / backEndData views (render.o data)
struct trRefEntityNode {
    trRefEntityNode* m_next;  // +0x00
    trRefEntityNode* m_prev;  // +0x04
};
struct reserved_dlist_trRefEntity {
    int              m_size;  // +0x00
    trRefEntityNode* m_head;  // +0x04
    trRefEntityNode* m_end;   // +0x08
    trRefEntityNode* m_tail;  // +0x0C
};
struct backEndData_t {
    reserved_dlist_trRefEntity entities;    // +0x00
    reserved_dlist_trRefEntity viewmodels;  // +0x10
};
backEndData_t* backEndData;  // ?backEndData@@3PAUbackEndData_t@@A @ 0xDFA438

// refimport_t view (Printf +0x00, Error +0x04, Milliseconds +0x08)
struct refimport_t {
    void (*Printf)(int, const char*, ...);
    void (*Error)(int, const char*, ...);
    int (*Milliseconds)();
};
extern refimport_t ri;         // ?ri@@3Urefimport_t@@A @ 0xF741E8

// ============================================================================
// RE_AddRefEntityToScene - ea: 0x006D2CD0
// ============================================================================
void RE_AddRefEntityToScene(refEntity_t* ent, int iCellNum)
{
    if (tr.registered == 0)
        return;
    if (ent->reType < 0 || ent->reType >= 16)
        ri.Error(1, "RE_AddRefEntityToScene: bad reType %i", ent->reType);
    trRefEntityNode* node = (trRefEntityNode*)((char*)ent - 8);
    *(int*)((char*)node + 0xE8) = iCellNum;
    *(char*)((char*)node + 0xFA) = 0;
    node->m_next = backEndData->entities.m_end;
    node->m_prev = backEndData->entities.m_tail;
    backEndData->entities.m_tail->m_next = node;
    backEndData->entities.m_tail = node;
    ++backEndData->entities.m_size;
}

// ============================================================================
// RE_AddViewModelToScene - ea: 0x006D2D30
// ============================================================================
void RE_AddViewModelToScene(refEntity_t* ent)
{
    if (tr.registered == 0)
        return;
    trRefEntityNode* node = (trRefEntityNode*)((char*)ent - 8);
    *(int*)((char*)node + 0xE8) = 0;
    *(char*)((char*)node + 0xFA) = 0;
    node->m_next = backEndData->viewmodels.m_end;
    node->m_prev = backEndData->viewmodels.m_tail;
    backEndData->viewmodels.m_tail->m_next = node;
    backEndData->viewmodels.m_tail = node;
    ++backEndData->viewmodels.m_size;
}

// ============================================================================
// RE_AddPolyToScene - ea: 0x006D29C0
// ============================================================================
struct polyVert_t {
    float xyz[3];
    float st[2];
    float lightmap[2];
    unsigned char modulate[4];
};

struct nglMeshSection;
struct gpuVertexFormat;
struct nglMaterial;
struct nglMeshNode;
class cdScratchMaterial {
public:
    cdScratchMaterial(nglTexture* tex, unsigned int BlendMode, int a3,
                      bool a4);  // ??0cdScratchMaterial@@QAE@PAUnglTexture@@IH_N@Z
};
extern gpuVertexFormat cdscratch_vertex_format;  // ?cdscratch_vertex_format@@3UgpuVertexFormat@@A
extern void* nglListAlloc(unsigned int size, unsigned int align);  // ngl.o
extern nglMesh* auxCreateScratchMesh(int flags, int num);          // aux.o
extern nglMeshSection* nglCreateScratchSection(int Prim, int NIndices,
                                               int NVertices,
                                               gpuVertexFormat* Fmt);  // ngl.o
extern void nglAddMeshSection(nglMesh* Mesh, nglMeshSection* Section,
                              nglMaterial* Mat, int Count);  // ngl.o
extern void* nglLockSectionIndices(nglMeshSection* Section);   // ngl.o
extern void* nglLockSectionVertices(nglMeshSection* Section);  // ngl.o
extern nglMesh* auxCloseScratchMesh(nglMesh* Mesh);            // aux.o
extern nglMeshNode* nglListAddMesh(nglMesh* Mesh, const math::Mat43& Mat,
                                   nglMeshParams* params,
                                   nglShaderParamSet* shaderParams,
                                   void (*callback)(nglMeshNode*));  // ngl.o
extern void j_nullsub_27(nglMeshSection* Section);  // nullsub
extern void j_nullsub_67(nglMeshSection* Section);  // nullsub

static inline unsigned int PackModulate(const unsigned char* m)
{
    return m[2] | (m[1] << 8) | (m[0] << 16) | (m[3] << 24);
}

void RE_AddPolyToScene(nglTexture* tex, int numVerts, const polyVert_t* verts)
{
    if (numVerts != 4)
        return;
    void* matMem = nglListAlloc(0x20u, 0x10u);
    cdScratchMaterial* mat = nullptr;
    if (matMem != nullptr)
        mat = new (matMem) cdScratchMaterial(tex, 0x64078600u, 1, false);
    nglMesh* TempMesh = auxCreateScratchMesh(0x40000, 1);
    nglMeshSection* Section = nglCreateScratchSection(6, 4, 4,
                                                      &cdscratch_vertex_format);
    nglAddMeshSection(TempMesh, Section, (nglMaterial*)mat, 1);
    unsigned short* indices = (unsigned short*)nglLockSectionIndices(Section);
    float* v7 = (float*)nglLockSectionVertices(Section);

    v7[0] = verts[0].xyz[0];
    v7[1] = verts[0].xyz[1];
    v7[2] = verts[0].xyz[2];
    v7[3] = verts[0].st[0];
    v7[4] = verts[0].st[1];
    *(unsigned int*)&v7[5] = PackModulate(verts[0].modulate);
    indices[0] = 0;

    v7 += 6;
    v7[0] = verts[1].xyz[0];
    v7[1] = verts[1].xyz[1];
    v7[2] = verts[1].xyz[2];
    v7[3] = verts[1].st[0];
    v7[4] = verts[1].st[1];
    *(unsigned int*)&v7[5] = PackModulate(verts[1].modulate);
    indices[1] = 1;

    v7 += 6;
    v7[0] = verts[3].xyz[0];
    v7[1] = verts[3].xyz[1];
    v7[2] = verts[3].xyz[2];
    v7[3] = verts[3].st[0];
    v7[4] = verts[3].st[1];
    *(unsigned int*)&v7[5] = PackModulate(verts[3].modulate);
    indices[2] = 2;

    v7 += 6;
    v7[0] = verts[2].xyz[0];
    v7[1] = verts[2].xyz[1];
    v7[2] = verts[2].xyz[2];
    v7[3] = verts[2].st[0];
    v7[4] = verts[2].st[1];
    *(unsigned int*)&v7[5] = PackModulate(verts[2].modulate);
    indices[3] = 3;

    j_nullsub_67(Section);
    j_nullsub_27(Section);

    math::Mat43 v38;
    v38.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v38.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    v38.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    v38.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    nglMesh* v36 = auxCloseScratchMesh(TempMesh);
    nglListAddMesh(v36, v38, nullptr, nullptr, nullptr);
}

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
// cdProjShadow_Begin - ea: 0x006C2420
// ============================================================================
extern nglScene* nglBuildScene;
void nglListAddDirProjectorLight(unsigned int LightCat,
                                 const math::Mat43* PO,
                                 const math::Position3* Scale,
                                 unsigned int BlendMode,
                                 nglTexture* Tex);

extern math::Mat43 gProjShadowMat;     // ?gProjShadowMat@@3VMat43@math@@A @ 0xF755B0
float gProjShadowSize = 700.0f;        // ?gProjShadowSize@@3MA (render.o @ 0xDFB144; init 700.0 per IDA bytes)
float gProjShadowAlpha = 0.0f;         // ?gProjShadowAlpha@@3MA (render.o @ 0x11EA640)
extern float gProjShadowZTop;          // ?gProjShadowZTop@@3MA @ 0xDFB14C
extern float gProjShadowZBottom;       // ?gProjShadowZBottom@@3MA @ 0xDFB150
extern nglScene* gProjShadowScene;     // ?gProjShadowScene@@3PAUnglScene@@A @ 0xF74434
extern bool gProjShadowQuad;           // ?gProjShadowQuad@@3_NA @ 0xF74438
nglScene* gProjShadowScene = nullptr;
bool gProjShadowQuad = false;

void cdProjShadow_Begin()
{
    if (gProjShadowTex == nullptr)
    {
        gProjShadowTex = nglCreateTexture(0x4010u, 6u,
                                          (int)gProjShadowTexSize,
                                          (int)gProjShadowTexSize, 0, 1);
    }

    const math::Mat43* vw = nglGetMatrix_ViewToWorld(nglBuildScene);
    // v19: 4x3 view-to-world matrix (x,y,z rows + w row)
    math::Mat43 m;
    m.x.v = vw->x.v;
    m.y.v = vw->y.v;
    m.z.v = vw->z.v;
    m.w.v = vw->w.v;

    // forward = normalize(z row); up = normalize(-(x - fwd*dot)); right = cross(up, fwd)
    __m128 fwd = m.z.v;
    float fwdLen = sqrtf(fwd.m128_f32[0] * fwd.m128_f32[0]
                       + fwd.m128_f32[1] * fwd.m128_f32[1]
                       + fwd.m128_f32[2] * fwd.m128_f32[2]);
    fwd = _mm_div_ps(fwd, _mm_set1_ps(fwdLen));

    __m128 up = _mm_sub_ps(m.y.v, _mm_mul_ps(fwd, _mm_set1_ps(
        fwd.m128_f32[0] * m.y.v.m128_f32[0]
        + fwd.m128_f32[1] * m.y.v.m128_f32[1]
        + fwd.m128_f32[2] * m.y.v.m128_f32[2])));
    float upLen = sqrtf(up.m128_f32[0] * up.m128_f32[0]
                      + up.m128_f32[1] * up.m128_f32[1]
                      + up.m128_f32[2] * up.m128_f32[2]);
    up = _mm_div_ps(up, _mm_set1_ps(upLen));

    math::Mat43 shadowMat;
    shadowMat.z.v = _mm_setr_ps(0.0f, 0.0f, -1.0f, 0.0f);
    shadowMat.y.v = up;
    __m128 right = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(up, up, 9), _mm_shuffle_ps(fwd, fwd, 18)),
        _mm_mul_ps(_mm_shuffle_ps(up, up, 18), _mm_shuffle_ps(fwd, fwd, 9)));
    shadowMat.x.v = right;

    float half = gProjShadowSize * 0.45f;
    shadowMat.w.v = _mm_add_ps(
        _mm_add_ps(
            _mm_sub_ps(m.w.v, _mm_mul_ps(shadowMat.x.v, _mm_set1_ps(half))),
            _mm_mul_ps(shadowMat.y.v, _mm_set1_ps(half))),
        _mm_mul_ps(shadowMat.z.v, _mm_set1_ps(gProjShadowZTop)));
    gProjShadowMat = shadowMat;

    nglListBeginScene(NGLSCENE_DEFAULTS);
    nglSetZWriteEnable(false);
    nglSetZTestEnable(false);
    nglSetRenderTarget(gProjShadowTex);
    nglSetViewport(0.0f, 0.0f, (float)gProjShadowTex->Width,
                   (float)gProjShadowTex->Height);
    nglSetClearFlags(0xF0u);
    nglSetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    nglListEndScene();

    gProjShadowScene = nglListBeginScene(NGLSCENE_DEFAULTS);
    nglSetZWriteEnable(false);
    nglSetZTestEnable(false);
    nglSetRenderTarget(gProjShadowTex);
    nglSetViewport(1.0f, 1.0f, (float)(gProjShadowTex->Width - 2),
                   (float)(gProjShadowTex->Height - 2));
    nglSetClearFlags(0);
    nglSetFBWriteMask(0x1000000u);
    nglSetAspectRatio(1.0f);
    nglSetCameraMatrix(&gProjShadowMat);
    nglSetOrthoMatrix(0.1f, gProjShadowZBottom);
    float half2 = (1.0f / gProjShadowSize) * 2.0f;
    float neg2 = (1.0f / gProjShadowSize) * -2.0f;
    nglSetView(neg2, neg2, half2, half2);
    nglValidateMatrices(nglBuildScene);
    if (gProjShadowQuad)
    {
        nglQuad q;
        nglInitQuad(&q);
        nglSetQuadRect(&q, 192.0f, 192.0f, 320.0f, 320.0f);
        nglSetQuadColor(&q, 0xFF000000u);
        nglListAddQuad(&q);
    }
    nglListEndScene();

    math::Position3 scale;
    scale.v = _mm_setr_ps(gProjShadowSize, -gProjShadowSize,
                          1140457472.0f, 0.0f);
    nglListAddDirProjectorLight(0xFFFFFFFFu, &gProjShadowMat, &scale,
                                0x64CF8600u, gProjShadowTex);
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

// ============================================================================
// RE_RenderScene / R_RenderView (tr_scene.cpp)
// ============================================================================
// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

extern "C" int __fpclass(float);

static bool IS_NAN(float x)
{
    return (__fpclass(x) & 0x297) != 0;
}

// cvar_t view (integer +0x20)
struct cvar_t {
    uint8_t _pad[0x1C];
    float value;   // +0x1C
    int integer;   // +0x20
};

extern cvar_t* r_norefresh;    // ?r_norefresh@@3PAUcvar_t@@A @ 0xF7417C
extern cvar_t* r_dynamiclight; // ?r_dynamiclight@@3PAUcvar_t@@A @ 0xF742A8
extern cvar_t* r_lodscale;     // ?r_lodscale@@3PAUcvar_t@@A @ 0xF741D4
extern cvar_t* r_lodbias;      // ?r_lodbias@@3PAUcvar_t@@A @ 0xF741B0
extern cvar_t* r_zfar;         // ?r_zfar@@3PAUcvar_t@@A @ 0xF741A0
extern cvar_t* r_znear;        // ?r_znear@@3PAUcvar_t@@A @ 0xF743A0
extern cvar_t* r_testshadow;   // ?r_testshadow@@3PAUcvar_t@@A @ 0xF742C0
extern cvar_t* r_testlight;    // ?r_testlight@@3PAUcvar_t@@A @ 0xF742EC

extern int r_numdlights;        // ?r_numdlights@@3HA @ 0xF741E4
extern int r_firstSceneDlight;  // ?r_firstSceneDlight@@3HA @ 0xF74274
extern int r_numpolys;          // ?r_numpolys@@3HA @ 0xF742FC
extern int r_firstScenePoly;    // ?r_firstScenePoly@@3HA @ 0xF742B0
extern int skyboxportal;        // ?skyboxportal@@3HA @ 0xF743B0
extern int drawskyboxportal;    // ?drawskyboxportal@@3HA @ 0xF74188
extern int currCl;              // ?currCl@@3HA @ 0xF1579C
extern float g_zfar;            // ?g_zfar@@3MA @ 0xDFB000
extern void* tempScene;         // ?tempScene@@3PAUnglScene@@A @ 0xF7442C
extern int gParticleBatchGroup; // ?gParticleBatchGroup@@3HA @ 0xF742B8
extern int gRenderSky;          // ?gRenderSky@@3HA @ 0xDFB008
extern int gRenderFX;           // ?gRenderFX@@3HA @ 0xDFB020
extern int gRenderLocalEntities;// ?gRenderLocalEntities@@3HA @ 0xDFB00C
extern int gRenderWorld;        // ?gRenderWorld@@3HA @ 0xDFB004
extern int gRenderEntities;     // ?gRenderEntities@@3HA @ 0xDFB018
extern int gRenderInstanceGroups;  // ?gRenderInstanceGroups@@3HA @ 0xDFB024
extern int gRenderLightGlows;   // ?gRenderLightGlows@@3HA @ 0xDFB028
extern int gRenderDebug;        // ?gRenderDebug@@3HA @ 0xDFB014
extern int gRenderStatusBar;    // ?gRenderStatusBar@@3HA @ 0xDFB010
extern int g_showLightGridDistribution;  // ?g_showLightGridDistribution@@3HA @ 0xF04908
extern int g_showLightGridDebugText;     // ?g_showLightGridDebugText@@3HA @ 0xF04904
extern int g_useOnScreenSoundPosDebugging;  // ?g_useOnScreenSoundPosDebugging@@3HA @ 0xF04970
extern float unk_F6A284[];       // @ 0xF6A284

// vmCvar_t (cg_widescreen; integer +0x0C)
struct vmCvar_t {
    int handle;             // +0x00
    int modificationCount;  // +0x04
    float value;            // +0x08
    int integer;            // +0x0C
};
extern vmCvar_t cg_widescreen;  // ?cg_widescreen@@3UvmCvar_t@@A @ 0xF5CC88

// cgGlobal_t (cubemapShot +0x0C)
struct cgGlobal_t {
    uint8_t _pad[0x0C];
    int cubemapShot;  // +0x0C
};
enum { CUBEMAPSHOT_NONE = -1 };
extern cgGlobal_t cgGlobal;  // ?cgGlobal@@3UcgGlobal_t@@A @ 0xF5FE30

// world_t (bspTree +0x100, mSky +0x108)
struct world_t {
    uint8_t _pad[0x100];
    void* bspTree;  // +0x100
    void* mSky;     // +0x108
};

// FogConfig statics (fogconfig.cpp)
namespace FogConfig {
extern int sEnabled;
extern float sNear, sFar, sStart, sEnd, sRed, sGreen, sBlue;
}

// ShaderCommon statics
namespace ShaderCommon {
extern math::Position3 gGlowSunPosScreen;  // ?gGlowSunPosScreen@ShaderCommon@@3VPosition3@math@@A @ 0x10DDF40
extern float gGlowGodRaysFadeOut;          // ?gGlowGodRaysFadeOut@ShaderCommon@@3MA @ 0x10DDF20
}
math::Position3 ShaderCommon::gGlowSunPosScreen;

// ngl.o exports
extern void nglSetEndOfRenderCallback(void (*Fn)(void*), void* Data);
extern void nglSetFogRange(float Near, float Far, float Min, float Max);
extern void nglSetFogColor(float r, float g, float b);
extern void nglSetAspectRatio(float a);
extern void nglSetPerspectiveMatrix(float fov, float nearz, float farz);
extern void nglSetWorldToViewMatrix(const math::Mat43* WorldToView);
extern void nglListAddPointLight(unsigned int LightCat,
                                 const math::Position3& Pos, float Near,
                                 float Far, const math::Vector4& Color,
                                 bool isVertexPointLight);
extern math::Position3 nglProjectPoint(const math::Position3& In,
                                       nglScene* Scene);
extern class nglFont* CL_GetFontInfo(int font, float scale);
class nglFont;
extern nglFont* nglSysFont;  // ?nglSysFont@@3PAVnglFont@@A @ 0x10E3580

// q_math.o
extern void AngleVectors(const float* angles, float* forward, float* right,
                         float* up);  // ?AngleVectors@@YAXQBMQAM11@Z

// render.o scene helpers
void R_RotateForViewer();
void R_SetupFrustum();
void R_RenderSky();
void R_SetupProjection();
void R_RenderGlow();
void R_RenderViewModels(viewParms_t* parms);  // 0x6D7430 (this batch)
void R_AddWorldSurfacesDPVS();
void R_AddEntitySurfaces();
void RB_DrawDebug();
void _codListBeginScene(nglSceneParamType ParamSource);
void _codListEndScene();
void cdProjShadow_Begin();
void LensFlareDraw();
void HandleFullScreenBlur(int viewport);
void XboxNGLMidSceneCallBack(void*);
void XboxNGLPostSceneCallBack(void*);
int apsCheckErrors();

// cg.o / cl.o exports
void CG_AddLocalEntities();
void CG_AddPacketEntities();
void FX_UpdateFX(bool firstClient);
void RenderEffectsInternal();
void FX_BuildSortedParticleEffectList();
namespace LocalClient {
int FirstLocalClientIndex();  // ?FirstLocalClientIndex@LocalClient@@YAHXZ
}

// FEManager (g_femanager object; U tag)
struct FEManager {
    void Draw3DWorldSpace();   // ?Draw3DWorldSpace@FEManager@@QAEXXZ
    void Draw3DScreenSpace();  // ?Draw3DScreenSpace@FEManager@@QAEXXZ
};
extern FEManager g_femanager;  // ?g_femanager@@3UFEManager@@A @ 0xF30E48

// DynamicDecalMgr / DecalSet (IDA layout; vector of DecalSet)
class DynamicDecalSet {
public:
    void Render();  // ?Render@DynamicDecalSet@@QAEXXZ
};
struct DecalSet {
    void* mTexture;          // +0x00
    DynamicDecalSet* mDecalSet;  // +0x04
};
struct DecalSetVector {
    uint8_t _pad[4];   // allocator
    DecalSet* _Myfirst;  // +0x04
    DecalSet* _Mylast;   // +0x08
};
class DynamicDecalMgr {
public:
    static void* sInst;         // ?sInst@DynamicDecalMgr@@2PAXA
    DecalSetVector mDecalSets;  // +0x00
};

// WheelMark / WheelMarkMgr
class WheelMark {
public:
    void Render();  // ?Render@WheelMark@@QAEXXZ
};
class WheelMarkMgr {
public:
    static unsigned int NMarks;   // ?NMarks@WheelMarkMgr@@1IA
    static WheelMark* Marks;      // ?Marks@WheelMarkMgr@@1PAVWheelMark@@A
};

unsigned int WheelMarkMgr::NMarks;

// WorldSpawn / SceneManager (struct tags per sInst mangle PAU1)
class WorldSpawn {
public:
    uint8_t _pad[0x118];
    float sundirection[3];  // +0x118
};
struct SceneManager {
    uint8_t _pad[0x1A0];
    WorldSpawn* mWorldSpawn;         // +0x1A0
    void RenderInstanceGroups();     // ?RenderInstanceGroups@SceneManager@@QAEXXZ
    void RenderLightGlows();         // ?RenderLightGlows@SceneManager@@QAEXXZ
    static SceneManager* sInst;      // ?sInst@SceneManager@@2PAU1@A
};

// LightGridMgr / DebugRender
class LightGridMgr {
public:
    static LightGridMgr* sInst;  // ?sInst@LightGridMgr@@2PAV1@A
    void RenderLightGridDebugSpheres();  // ?RenderLightGridDebugSpheres@LightGridMgr@@QAEXXZ
    void RenderDebugText();              // ?RenderDebugText@LightGridMgr@@QAEXXZ
};
class DebugRender {
public:
    static DebugRender sInst;  // ?sInst@DebugRender@@2V1@A
    void Render();             // ?Render@DebugRender@@QAEXXZ
};

// StatusBar / MemGraph / CG blur callbacks (free-fn manglings)
namespace StatusBar {
void Render();  // ?Render@StatusBar@@YAXXZ
}
namespace MemGraph {
void Render();  // ?Render@MemGraph@@YAXXZ
}
namespace CG_MotionBlur {
void AddPostCallback();  // ?AddPostCallback@CG_MotionBlur@@YAXXZ
}
namespace CG_SceneBlur {
void AddPostCallback();  // ?AddPostCallback@CG_SceneBlur@@YAXXZ
}

// cdl_proftimer (member start/stop)
struct cdl_proftimer {
public:
    unsigned __int64 stamp;
    unsigned __int64 value;
    void start();  // ?start@cdl_proftimer@@QAEXXZ
    void stop();   // ?stop@cdl_proftimer@@QAEXXZ
};
extern cdl_proftimer cdl_proftimer_fx_all;     // ?cdl_proftimer_fx_all@@3Ucdl_proftimer@@A @ 0xF3E970
extern cdl_proftimer cdl_proftimer_fx_render;  // ?cdl_proftimer_fx_render@@3Ucdl_proftimer@@A @ 0xF44FB8

// TlSystemCallbacks::sWarningsEnabled (private static; @@0 per binary)
class TlSystemCallbacks {
    friend void R_RenderView(viewParms_t* parms);
    static bool sWarningsEnabled;  // ?sWarningsEnabled@TlSystemCallbacks@@0_NA
};
bool TlSystemCallbacks::sWarningsEnabled = true;

// ============================================================================
// RE_RenderScene - ea: 0x006DC6C0
// ============================================================================
struct refdef_s {
    int x;                  // +0x00
    int y;                  // +0x04
    int width;              // +0x08
    int height;             // +0x0C
    float fov_x;            // +0x10
    float fov_y;            // +0x14
    uint8_t _pad[0x20 - 0x18];
    math::Position3 vieworg;   // +0x20
    float viewaxis[3][3];      // +0x30
    uint8_t _pad2[0x54 - 0x3C];
    int time;               // +0x54
    int rdflags;            // +0x58
};

void RE_RenderScene(const refdef_s* fd)
{
    if (tr.registered != 0 && r_norefresh->integer == 0)
    {
        ri.Milliseconds();
        if (tr.world == nullptr && (fd->rdflags & 1) == 0)
            ri.Error(1, "R_RenderScene: NULL worldmodel");  // ERR_DROP
        if (IS_NAN(tr.viewParms.world.modelMatrix[0])
            || IS_NAN(tr.viewParms.world.modelMatrix[1])
            || IS_NAN(tr.viewParms.world.modelMatrix[2]))
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_scene.cpp";
            AeAssert::gCurrentLine = 200;
            AeAssert::gCurrentExpr =
                "!IS_NAN((tr.viewParms.world.modelMatrix)[0]) && !IS_NAN((tr.viewParms.world.modelMatrix)[1]) && !IS_NAN((tr.viewParms.world.modelMatrix)[2])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        tr.refdef.x = fd->x;
        tr.refdef.y = fd->y;
        tr.refdef.width = fd->width;
        tr.refdef.height = fd->height;
        tr.refdef.fov_x = fd->fov_x;
        tr.refdef.fov_y = fd->fov_y;
        tr.refdef.time = fd->time;
        tr.refdef.rdflags = fd->rdflags;
        if ((fd->rdflags & 8) != 0)
            skyboxportal = 1;
        drawskyboxportal = (fd->rdflags & 0x10) != 0;
        short v2 = (short)(r_numdlights - r_firstSceneDlight);
        tr.refdef.num_world_dlights = v2;
        tr.refdef.num_model_dlights = v2;
        if (r_dynamiclight->integer == 0)
            tr.refdef.num_model_dlights = 0;
        if (r_dynamiclight->integer != 1)
        {
            v2 = 0;
            tr.refdef.num_world_dlights = 0;
        }
        if (r_dynamiclight->integer == 3)
            ri.Printf(0, "%i dynamic lights in scene\n", v2);

        viewParms_t v8;
        memset(&v8, 0, sizeof(v8));
        v8.isPortal = 0;
        float value = r_lodscale->value;
        v8.fovX = tr.refdef.fov_x;
        v8.fovY = tr.refdef.fov_y;
        float v3 = 4.0f;
        if (value <= 4.0f)
            v3 = value;
        float v5 = r_lodbias->value;
        v8.lodScale = v3;
        float v6 = 0.0f;
        if (v5 <= 0.0f)
            v6 = v5;
        v8.lodBias = v6;
        v8.zFar = r_zfar->value;
        float fov_x = tr.refdef.fov_x;
        if (tr.refdef.fov_y < tr.refdef.fov_x)
            fov_x = tr.refdef.fov_y;
        v8.or.origin[0] = fd->vieworg.v.m128_f32[0];
        v8.or.origin[1] = fd->vieworg.v.m128_f32[1];
        v8.or.origin[2] = fd->vieworg.v.m128_f32[2];
        memcpy(v8.or.axis, fd->viewaxis, sizeof(v8.or.axis));
        memcpy(v8.pvsOrigin, &fd->vieworg, sizeof(v8.pvsOrigin));
        float v7 = (float)(tan(fov_x * 0.0087266462f)
                           / tan(0.6981317400932312));
        v8.lodScale = v8.lodScale * v7;
        v8.lodBias = v7 * v8.lodBias;
        R_RenderView(&v8);
        r_firstSceneDlight = r_numdlights;
        r_firstScenePoly = r_numpolys;
    }
}

// ============================================================================
// R_RenderView - ea: 0x006DBE00
// ============================================================================
void R_RenderView(viewParms_t* parms)
{
    nglSetEndOfRenderCallback(EndOfRenderCallback, nullptr);
    TlSystemCallbacks::sWarningsEnabled = false;
    ++tr.viewCount;
    tr.viewParms = *parms;
    tr.viewParms.frameCount = tr.frameCount;
    if (IS_NAN(tr.viewParms.world.axis[0][0])
        || IS_NAN(tr.viewParms.world.axis[0][1])
        || IS_NAN(tr.viewParms.world.axis[0][2]))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_main.cpp";
        AeAssert::gCurrentLine = 3068;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tr.viewParms.world.axis[0])[0]) && !IS_NAN((tr.viewParms.world.axis[0])[1]) && !IS_NAN((tr.viewParms.world.axis[0])[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    R_RotateForViewer();
    R_SetupFrustum();
    nglListBeginScene(NGLSCENE_PARENT);
    nglSetClearFlags(0);
    if (FogConfig::sEnabled != 0)
    {
        nglSetFogRange(FogConfig::sNear, FogConfig::sFar, FogConfig::sStart,
                       FogConfig::sEnd * 0.5f);
        nglSetFogColor(FogConfig::sRed, FogConfig::sGreen, FogConfig::sBlue);
    }
    int v2 = unk_F6A284[802 * currCl];
    float v3;
    if (v2 == 3 || v2 == 4)
        v3 = (cg_widescreen.integer != 0) ? 3.5555556f : 2.6666667f;
    else
        v3 = (cg_widescreen.integer != 0) ? 1.7777778f : 1.3333334f;
    nglSetAspectRatio(v3);
    if (cgGlobal.cubemapShot != CUBEMAPSHOT_NONE)
        nglSetAspectRatio(1.0f);
    float zFar = parms->zFar;
    if (zFar == 0.0f)
        zFar = 8192.0f;
    float value = r_znear->value;
    nglSetPerspectiveMatrix(parms->fovY, value, zFar);
    float v6 = parms->zFar;
    if (v6 == 0.0f)
        v6 = 8192.0f;
    g_zfar = v6;
    if (IS_NAN(tr.viewParms.world.modelMatrix[0])
        || IS_NAN(tr.viewParms.world.modelMatrix[1])
        || IS_NAN(tr.viewParms.world.modelMatrix[2]))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_main.cpp";
        AeAssert::gCurrentLine = 3099;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tr.viewParms.world.modelMatrix)[0]) && !IS_NAN((tr.viewParms.world.modelMatrix)[1]) && !IS_NAN((tr.viewParms.world.modelMatrix)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    float worldToView[16];
    memcpy(worldToView, tr.viewParms.world.modelMatrix, 64);
    nglSetWorldToViewMatrix((const math::Mat43*)worldToView);
    if (((world_t*)tr.world)->mSky != nullptr && gRenderSky != 0)
        R_RenderSky();
    tempScene = nglBuildScene;
    if (gRenderFX != 0)
    {
        cdl_proftimer_fx_all.start();
        FX_UpdateFX(currCl == LocalClient::FirstLocalClientIndex());
        cdl_proftimer_fx_all.stop();
    }
    if (tr.viewModelInfo[tr.viewModelInfoIndex].mDrawBeforeWorld != 0)
        R_RenderViewModels(parms);
    _codListBeginScene(NGLSCENE_PARENT);
    HandleFullScreenBlur(0);
    if (tr.viewModelInfo[tr.viewModelInfoIndex].mDrawBeforeWorld == 0
        || cgGlobal.cubemapShot != CUBEMAPSHOT_NONE)
        nglSetClearFlags(3u);
    if (r_testshadow->integer != 0)
        cdProjShadow_Begin();
    nglSetSceneCallBack(NGLSCENE_MID, XboxNGLMidSceneCallBack, nullptr);
    nglSetSceneCallBack(NGLSCENE_POST, XboxNGLPostSceneCallBack, nullptr);
    LensFlareDraw();
    if (g_showLightGridDistribution != 0)
    {
        nglGetMatrix_ViewToWorld(nglBuildScene);
        LightGridMgr::sInst->RenderLightGridDebugSpheres();
    }
    if (r_testlight->integer != 0)
    {
        const math::Mat43* Matrix_ViewToWorld =
            nglGetMatrix_ViewToWorld(nglBuildScene);
        math::Mat43 mtx = *Matrix_ViewToWorld;
        float sunPos[16];
        memset(&sunPos[4], 0, 32);
        nglListAddPointLight(0x40000000u,
                             *((const math::Position3*)&mtx.w.v), 0.0f, 150.0f,
                             *((const math::Vector4*)&sunPos[20]), false);
        sunPos[4] = 0.0f;
        sunPos[8] = 0.0f;
        sunPos[16] = 0.0f;
        sunPos[12] = 1.0f;
        sunPos[20] = sunPos[4];
        __m128 pos2 = _mm_add_ps(
            mtx.w.v, _mm_mul_ps(mtx.z.v, _mm_set1_ps(300.0f)));
        memcpy(&sunPos[4], &pos2, 16);
        nglListAddPointLight(0x40000000u,
                             *((const math::Position3*)&sunPos[4]), 0.0f,
                             150.0f, *((const math::Vector4*)&sunPos[20]),
                             false);
    }
    if (g_showLightGridDebugText != 0)
        LightGridMgr::sInst->RenderDebugText();
    if (gRenderLocalEntities != 0)
        CG_AddLocalEntities();
    CG_AddPacketEntities();
    g_femanager.Draw3DWorldSpace();
    DynamicDecalMgr* decalMgr = (DynamicDecalMgr*)DynamicDecalMgr::sInst;
    for (DecalSet* Myfirst = decalMgr->mDecalSets._Myfirst,
                  *i = decalMgr->mDecalSets._Mylast;
         Myfirst != i; ++Myfirst)
        Myfirst->mDecalSet->Render();
    if (WheelMarkMgr::NMarks != 0)
    {
        for (unsigned int v11 = 0; v11 < WheelMarkMgr::NMarks; ++v11)
            WheelMarkMgr::Marks[v11].Render();
    }
    float forward[3], right[3], up[3];
    AngleVectors(SceneManager::sInst->mWorldSpawn->sundirection, forward,
                 right, up);
    float SunDir[36];
    float sunWorld[3] = { forward[0] * 100000.0f, forward[1] * 100000.0f,
                          forward[2] * 100000.0f };
    memcpy(&SunDir[20], sunWorld, 12);
    SunDir[32] = 0.0f;
    memcpy(&SunDir[4], &SunDir[20], 12);
    math::Position3 projected =
        nglProjectPoint(*((const math::Position3*)&SunDir[4]), nglBuildScene);
    memcpy(&SunDir[4], &projected, 16);
    SunDir[20] = SunDir[4] * 0.0015625f;
    SunDir[24] = SunDir[5] * 0.0020833334f;
    ShaderCommon::gGlowSunPosScreen.v = *((const __m128*)&SunDir[20]);
    SunDir[20] = forward[0];
    SunDir[24] = forward[1];
    SunDir[28] = forward[2];
    SunDir[32] = 0.0f;
    __m128 v14 = _mm_mul_ps(nglBuildScene->ViewDir.v,
                            *((const __m128*)&SunDir[20]));
    float dot = v14.m128_f32[0]
                + (v14.m128_f32[1] + v14.m128_f32[2]);
    float v15 = dot * 2.5f;
    if (v15 < 0.0f || v15 > 1.0f)
        v15 = (dot * 2.5f < 0.0f) ? 0.0f : 1.0f;
    ShaderCommon::gGlowGodRaysFadeOut = v15;
    if (gRenderWorld != 0)
    {
        R_SetupProjection();
        if (tr.world != nullptr && ((world_t*)tr.world)->bspTree != nullptr)
            R_AddWorldSurfacesDPVS();
        if (gRenderEntities != 0)
            R_AddEntitySurfaces();
    }
    if (gRenderInstanceGroups != 0)
        SceneManager::sInst->RenderInstanceGroups();
    if (gRenderLightGlows != 0)
        SceneManager::sInst->RenderLightGlows();
    if (gRenderDebug != 0)
    {
        RB_DrawDebug();
        DebugRender::sInst.Render();
    }
    if (gRenderFX != 0)
    {
        cdl_proftimer_fx_all.start();
        cdl_proftimer_fx_render.start();
        RenderEffectsInternal();
        cdl_proftimer_fx_render.stop();
        FX_BuildSortedParticleEffectList();
        cdl_proftimer_fx_all.stop();
    }
    _codListEndScene();
    if (g_useOnScreenSoundPosDebugging != 0)
    {
        if (nglSysFont == nullptr)
            nglSysFont = CL_GetFontInfo(4, 1.0f);
    }
    R_RenderGlow();
    if (tr.viewModelInfo[tr.viewModelInfoIndex].mDrawBeforeWorld == 0)
        R_RenderViewModels(parms);
    nglListBeginScene(NGLSCENE_PARENT);
    nglSetClearFlags(0);
    g_femanager.Draw3DScreenSpace();
    nglListEndScene();
    nglListBeginScene(NGLSCENE_PARENT);
    nglSetClearFlags(0);
    if (gRenderStatusBar != 0 && cgGlobal.cubemapShot == CUBEMAPSHOT_NONE)
        StatusBar::Render();
    MemGraph::Render();
    nglListEndScene();
    CG_MotionBlur::AddPostCallback();
    CG_SceneBlur::AddPostCallback();
    nglListEndScene();
    apsCheckErrors();
    TlSystemCallbacks::sWarningsEnabled = true;
}

// ============================================================================
// R_RenderViewModels - ea: 0x006D7430
// ============================================================================
extern int gRenderViewModels;  // ?gRenderViewModels@@3HA @ 0xDFB01C
extern void nglValidateMatrices(nglScene* Scene);
class DObj;
extern int R_AddViewModelSurfaces(int client_index, DObj* obj,
                                  const math::Mat43& matrix);

void R_RenderViewModels(viewParms_t* parms)
{
    if (cgGlobal.cubemapShot == CUBEMAPSHOT_NONE)
    {
        if (tr.viewModelInfo[tr.viewModelInfoIndex].mInWorldScene == 0)
        {
            nglListBeginScene(NGLSCENE_PARENT);
            nglSetPerspectiveMatrix(parms->fovY, 0.5f, 128.0f);
            nglSetClearFlags(3u);
        }
        if (gRenderViewModels != 0)
        {
            trRefEntityNode* m_head = backEndData->viewmodels.m_head;
            trRefEntityNode* v3 =
                (m_head != nullptr) ? m_head->m_next : nullptr;
            if ((void*)m_head != (void*)&backEndData->viewmodels.m_end
                && v3 != nullptr)
            {
                do
                {
                    if ((*(int*)((char*)m_head + 0x0C) & 2) == 0
                        || tr.viewParms.isPortal != 0)
                    {
                        float* f = (float*)m_head;
                        math::Mat43 matrix;
                        matrix.x.v = _mm_setr_ps(f[7], f[8], f[9], 0.0f);
                        matrix.y.v = _mm_setr_ps(f[10], f[11], f[12], 0.0f);
                        matrix.z.v = _mm_setr_ps(f[13], f[14], f[15], 0.0f);
                        matrix.w.v = _mm_setr_ps(f[17], f[18], f[19], 0.0f);
                        nglValidateMatrices(nglBuildScene);
                        unsigned char* visible =
                            &((unsigned char*)m_head)[0xFF];
                        *visible ^= (unsigned char)(
                            R_AddViewModelSurfaces(
                                tr.viewModelInfoIndex,
                                *(DObj**)((char*)m_head + 92), matrix)
                            ^ *visible)
                                   & 1;
                    }
                    m_head = v3;
                    v3 = v3->m_next;
                } while (v3 != nullptr);
            }
        }
        if (tr.viewModelInfo[tr.viewModelInfoIndex].mInWorldScene == 0)
            nglListEndScene();
    }
}
