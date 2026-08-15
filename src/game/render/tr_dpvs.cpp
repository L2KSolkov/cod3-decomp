// ============================================================================
// tr_dpvs.cpp - render.o DPVS mesh-record + debug-axes helpers (tr_dpvs.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/mem_heap.h"
#include "core/PoolAllocator.h"
#include "ngl/nglDebug.h"

#include <string.h>

// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, MJK = 4 };
extern ECoderId gCurrentAuthor;        // ?gCurrentAuthor@AeAssert@@3W4ECoderId@1@A
extern const char* gCurrentFile;       // ?gCurrentFile@AeAssert@@3PBDB
extern int gCurrentLine;               // ?gCurrentLine@AeAssert@@3HA
extern const char* gCurrentExpr;       // ?gCurrentExpr@AeAssert@@3PBDB
bool IsIgnored();                      // ?IsIgnored@AeAssert@@YA_NXZ
bool Assert(const char* fmtstring, ...);  // ?Assert@AeAssert@@YA_NPBDZZ
bool Warning(const char* fmtstring, ...); // ?Warning@AeAssert@@YA_NPBDZZ
}

// Exact-tag forward decls (binary: nglMeshNode=class PAV, nglShaderParamSet=struct PAU)
class nglMeshNode;
struct nglMesh;
class nglMeshParams;
struct nglShaderParamSet;
struct nglScene;

enum nglSceneParamType {
    NGLSCENE_DEFAULTS = 0,
    NGLSCENE_PARENT = 1,
    NGLSCENE_ROOT = 2,
};

// ngl.o signatures (exact binary manglings; ngl_mesh.o/ngl_scene.o)
nglMeshNode* nglListAddMesh(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                            nglMeshParams* MeshParams, nglShaderParamSet* ShaderParams,
                            void (*fn)(nglMeshNode*));
nglScene* nglListBeginScene(nglSceneParamType ParamSource);
void nglListEndScene();
extern nglScene* nglBuildScene;

// ngl_debug.o
extern nglPerfInfoStruct nglPerfInfo;

// Forward decls for render entity / DObj views (defined below)
class DObj;
class trRefEntity;

// ============================================================================
// MeshRecord (IDA type; size 0x50) + mesh-list globals (render.o data)
// ============================================================================
struct MeshRecord {
    math::Mat43      localToWorld;   // +0x00
    nglMesh*         mesh;           // +0x40
    nglMeshParams*   meshParams;     // +0x44
    nglShaderParamSet* shaderParams; // +0x48
    bool             isCulled;       // +0x4C
};
static_assert(sizeof(MeshRecord) == 0x50, "MeshRecord size mismatch");

bool gLockMeshList;              // ?gLockMeshList@@3_NA @ 0xF743F0
bool gRecordMeshList;            // ?gRecordMeshList@@3_NA @ 0xF74288
bool gEnableMeshFlash;           // ?gEnableMeshFlash@@3_NA @ 0xF743F1
MeshRecord* gMeshRecordList;     // ?gMeshRecordList@@3PAUMeshRecord@@A @ 0xF74280
int gMaxMeshRecordListEntries;   // ?gMaxMeshRecordListEntries@@3HA @ 0xDFA440
int gMeshListCounter;            // ?gMeshListCounter@@3HA @ 0xF741C8
int gMeshListCount;              // ?gMeshListCount@@3HA @ 0xF743A4
static bool lockMeshListPrev;    // render.o @ 0xF78368
static int flashMeshCounter;     // render.o @ 0xF7836C

// ============================================================================
// DPVS globals (dpvs_t; g_dpvs plain C symbol @ 0xF75600)
// ============================================================================
struct trModelCellRef_t {
    math::Vector4     sphere;   // +0x00
    trRefEntity*      re;       // +0x10
    trModelCellRef_t* next;     // +0x14
    int               viewCount;// +0x18
    char              pad[4];   // +0x1C
};
static_assert(sizeof(trModelCellRef_t) == 0x20, "trModelCellRef_t size mismatch");

// dpvs_plane_t (IDA type; size 0x20)
struct dpvs_plane_t {
    math::Vector4 data;          // +0x00
    unsigned char side[3];       // +0x10
    unsigned char frontal;       // +0x13
    static PoolAllocator* sAllocator;  // ?sAllocator@dpvs_plane_t@@2PAVPoolAllocator@@A @ 0xF7442C
};
static_assert(sizeof(dpvs_plane_t) == 0x20, "dpvs_plane_t size mismatch");

// dpvs_t (IDA type; size 0x2180)
struct dpvs_t {
    math::Position3  origin;            // +0x00
    dpvs_plane_t     frustumPlanes[4];  // +0x10
    dpvs_plane_t     viewPlane;         // +0x90
    dpvs_plane_t     fogPlane;          // +0xB0
    dpvs_plane_t*    nearPlane;         // +0xD0
    dpvs_plane_t*    farPlane;          // +0xD4
    float            cullDist;          // +0xD8
    trModelCellRef_t modelRefs[256];    // +0xE0
    int              modelRefCount;     // +0x20E0
    __declspec(align(16)) uint8_t mvp[128];  // +0x20F0 (union)
    int              portalStackLevel;  // +0x2170
};
static_assert(sizeof(dpvs_t) == 0x2180, "dpvs_t size mismatch");
extern "C" {
dpvs_t g_dpvs;  // plain C symbol @ 0xF75600
}

// trRefEntity / refEntity_t views (IDA layouts; e at +0x08)
struct refEntity_t {
    int    reType;           // +0x00
    int    renderfx;         // +0x04
    float  lightingOrigin[3];// +0x08
    float  axis[3][3];       // +0x14
    float  scale;            // +0x38
    float  origin[3];        // +0x3C
    float  oldorigin[3];     // +0x48
    DObj*  obj;              // +0x54
    void*  entity;           // +0x58
    void*  pStaticModel;     // +0x5C

    refEntity_t(int foo);    // ??0refEntity_t@@QAE@H@Z (render.o 0x6C07E0)
    trRefEntity* GetTrRefentity();  // ?GetTrRefentity@refEntity_t@@QAEPAVtrRefEntity@@XZ
};
static_assert(sizeof(refEntity_t) == 0x60, "refEntity_t size mismatch");

// DObj view (numBones +0xCF per g_local.h)
class DObj {
public:
    uint8_t _pad0[0xCF];
    unsigned char numBones;  // +0xCF
};

class trRefEntity {
public:
    uint8_t _pad0[0x08];
    refEntity_t e;           // +0x08
};

struct DObjSkelMat {
    float axis[3][4];        // +0x00
    float origin[4];         // +0x30
};
static_assert(sizeof(DObjSkelMat) == 0x40, "DObjSkelMat size mismatch");

// BspCell view (modelRefs +0x38)
class BspCell {
public:
    uint8_t _pad0[0x38];
    trModelCellRef_t* modelRefs;  // +0x38
};

// core.o / q_math.o helpers
DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex);
void MatrixTransformVector(const float* const in1, const float (*const in2)[3],
                           float* const out);
void CL_AddDebugLine(const float* start, const float* end, const float* color,
                     int depthTest, int duration, int fromServer, int fadeOut);

// ============================================================================
// R_AddModelToCell - ea: 0x006BF830
// ============================================================================
void R_AddModelToCell(BspCell* cell, trRefEntity* re, const math::Vector4& sphere)
{
    if (g_dpvs.modelRefCount < 256)
    {
        trModelCellRef_t* ref = &g_dpvs.modelRefs[g_dpvs.modelRefCount++];
        ref->re = re;
        ref->sphere = sphere;
        ref->next = cell->modelRefs;
        cell->modelRefs = ref;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
        AeAssert::gCurrentLine = 812;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored() && AeAssert::Warning("Max xmodel refs exceeded\n"))
            __debugbreak();
    }
}

// ============================================================================
// _codListAddMesh - ea: 0x006BFB70
// ============================================================================
nglMeshNode* _codListAddMesh(nglMesh* mesh, const math::Mat43& localToWorld,
                             nglMeshParams* meshParams, nglShaderParamSet* shaderParams,
                             void (*fn)(nglMeshNode*))
{
    if (!gLockMeshList)
        return nglListAddMesh(mesh, localToWorld, meshParams, shaderParams, fn);

    if (gRecordMeshList && mesh != nullptr)
    {
        unsigned int TotalVerts = nglPerfInfo.TotalVerts;
        nglMeshNode* meshNode =
            nglListAddMesh(mesh, localToWorld, meshParams, shaderParams, fn);
        nglScene* nglBuildSceneSave = nglBuildScene;
        nglBuildScene = nullptr;
        if (gMeshListCounter >= gMaxMeshRecordListEntries)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
            AeAssert::gCurrentLine = 1505;
            AeAssert::gCurrentExpr = "gMeshListCounter < gMaxMeshRecordListEntries";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Too many meshes for the mesh record buffer"))
                __debugbreak();
        }
        nglBuildScene = nglBuildSceneSave;
        gMeshRecordList[gMeshListCounter].mesh = mesh;
        gMeshRecordList[gMeshListCounter].localToWorld = localToWorld;
        gMeshRecordList[gMeshListCounter].isCulled =
            nglPerfInfo.TotalVerts == TotalVerts;
        gMeshRecordList[gMeshListCounter].meshParams = meshParams;
        gMeshRecordList[gMeshListCounter++].shaderParams = shaderParams;
        return meshNode;
    }
    return nullptr;
}

// ============================================================================
// _codListBeginScene - ea: 0x006BFD50
// ============================================================================
void _codListBeginScene(nglSceneParamType ParamSource)
{
    if (!gLockMeshList || lockMeshListPrev)
    {
        gRecordMeshList = false;
    }
    else
    {
        MeshRecord* v1 = (MeshRecord*)mem_heap_malloc(16, 80 * gMaxMeshRecordListEntries);
        nglScene* v2 = nglBuildScene;
        gMeshRecordList = v1;
        nglBuildScene = nullptr;
        if (v1 == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
            AeAssert::gCurrentLine = 1538;
            AeAssert::gCurrentExpr = "gMeshRecordList";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Cannot allocate memory for the mesh record buffer"))
                __debugbreak();
        }
        nglBuildScene = v2;
        gRecordMeshList = true;
        gMeshListCounter = 0;
    }
    if (!gLockMeshList && lockMeshListPrev)
        mem_heap_free(gMeshRecordList);
    lockMeshListPrev = gLockMeshList;
    nglListBeginScene(ParamSource);
}

// ============================================================================
// _codListEndScene - ea: 0x006BFE30
// ============================================================================
void _codListEndScene()
{
    if (gLockMeshList)
    {
        if (gRecordMeshList)
        {
            gMeshListCount = gMeshListCounter;
        }
        else
        {
            int i = 0;
            if (gMeshListCount > 0)
            {
                int v0 = 0;
                do
                {
                    nglScene* v1 = nglBuildScene;
                    nglBuildScene = nullptr;
                    if (gMeshListCounter >= gMaxMeshRecordListEntries)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
                        AeAssert::gCurrentLine = 1581;
                        AeAssert::gCurrentExpr =
                            "gMeshListCounter < gMaxMeshRecordListEntries";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Too many meshes for the mesh record buffer"))
                            __debugbreak();
                    }
                    nglBuildScene = v1;
                    if (!gMeshRecordList[v0].isCulled
                        || ((flashMeshCounter & 1) != 0 && gEnableMeshFlash))
                    {
                        nglListAddMesh(gMeshRecordList[v0].mesh,
                                       gMeshRecordList[v0].localToWorld,
                                       nullptr, nullptr, nullptr);
                    }
                    ++v0;
                    ++i;
                } while (i < gMeshListCount);
            }
        }
    }
    ++flashMeshCounter;
    nglListEndScene();
}

// ============================================================================
// R_XModelDebugAxes - ea: 0x006BE390
// ============================================================================
void R_XModelDebugAxes(trRefEntity* ent, int* partBits)
{
    (void)partBits;

    int numBones = ent->e.obj->numBones;
    DObj* obj = ent->e.obj;
    float translation[3][3];
    translation[0][0] = 6.0f;
    translation[0][1] = 0.0f;
    translation[0][2] = 0.0f;
    translation[1][0] = 0.0f;
    translation[1][1] = 6.0f;
    translation[1][2] = 0.0f;
    translation[2][0] = 0.0f;
    translation[2][1] = 0.0f;
    translation[2][2] = 6.0f;

    DObjSkelMat* MatrixArray = DObjGetMatrixArray(obj, 0);
    if (numBones > 0)
    {
        float* v4 = MatrixArray->axis[1];
        for (int i = numBones; i != 0; --i)
        {
            int j = 0;
            float* v5 = &translation[0][1];
            do
            {
                float color[4];
                color[0] = 0.0f;
                color[1] = 0.0f;
                color[2] = 0.0f;
                color[3] = 0.0f;
                color[j] = 1.0f;

                float vec[3];
                vec[0] = ((*v4 * 0.0f) + (v4[4] * 0.0f) + (*(v4 - 4) * 0.0f)) + v4[8];
                vec[1] = ((v4[1] * 0.0f) + (v4[5] * 0.0f) + (*(v4 - 3) * 0.0f)) + v4[9];
                vec[2] = ((v4[2] * 0.0f) + (v4[6] * 0.0f) + (*(v4 - 2) * 0.0f)) + v4[10];

                float start[3];
                MatrixTransformVector(vec, ent->e.axis, start);
                start[0] += ent->e.origin[0];
                start[1] += ent->e.origin[1];
                start[2] += ent->e.origin[2];

                float v6 = *v5;
                vec[0] = ((*v5 * *v4) + (*(v4 - 4) * *(v5 - 1))) + (v4[4] * v5[1]) + v4[8];
                vec[1] = ((v4[1] * *v5) + (v4[5] * v5[1])) + (*(v5 - 1) * *(v4 - 3)) + v4[9];
                vec[2] = ((v5[1] * v4[6]) + (v6 * v4[2])) + (*(v4 - 2) * *(v5 - 1)) + v4[10];

                float end[3];
                MatrixTransformVector(vec, ent->e.axis, end);
                end[0] += ent->e.origin[0];
                end[1] += ent->e.origin[1];
                end[2] += ent->e.origin[2];

                CL_AddDebugLine(start, end, color, 0, 0, 1, 0);
                v5 += 3;
                ++j;
            } while (j < 3);
            v4 += 16;
        }
    }
}

// ============================================================================
// refEntity_t members (render.o 0x6C07E0 / 0x6C0800)
// ============================================================================
refEntity_t::refEntity_t(int foo)
{
    (void)foo;
    memset(this, 0, sizeof(refEntity_t));
}

trRefEntity* refEntity_t::GetTrRefentity()
{
    return (trRefEntity*)((char*)this - 8);
}

// ============================================================================
// R_FilterModelIntoCells_r (trRefEntity variant) - ea: 0x006C57D0
// ============================================================================
class BspNode {
public:
    short contents;            // +0x00
    short cellNum;             // +0x02
    BspNode* children[2];      // +0x04
    math::Vector4* plane;      // +0x0C
};

// trRefEntity view with mOccupiedCells (+0xF6)
struct trRefEntityFilterView {
    uint8_t _pad[0xF6];
    unsigned char mOccupiedCells[4];  // +0xF6
};

// trGlobals view (refdef +0x26C, world +0x290)
struct trRefdefFilterView {
    uint8_t _pad[0x1C];
    int rdflags;              // +0x1C
    short num_world_dlights;  // +0x20
    short num_model_dlights;  // +0x22
};
struct trGlobalsFilterView {
    uint8_t _pad[0x26C];
    trRefdefFilterView refdef;   // +0x26C
    uint8_t _pad2[0x290 - 0x270];
    void* world;                 // +0x290
};
extern trGlobalsFilterView tr;  // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

// world_t view (bspTree +0x100)
struct worldFilterView {
    uint8_t _pad[0x100];
    void* bspTree;           // +0x100
};

// BspTree view (mCells +0x18)
struct BspTreeFilterView {
    uint8_t _pad[0x18];
    unsigned int mCellsSize;   // +0x18
    void* mCellsList;          // +0x1C
};

// BspCell view (modelRefs +0x38) + R_AddModelToCell(cell, re, sphere)
void R_AddModelToCell(void* cell, trRefEntity* re,
                      const math::Vector4& sphere);  // tr_dpvs.cpp (0x6BF830)

void R_FilterModelIntoCells_r(BspNode* startNode, trRefEntity* re,
                              const math::Vector4& sphere)
{
    trRefEntityFilterView* reView = (trRefEntityFilterView*)re;
    reView->mOccupiedCells[0] = 0xFF;
    reView->mOccupiedCells[1] = 0xFF;
    reView->mOccupiedCells[2] = 0xFF;
    reView->mOccupiedCells[3] = 0xFF;

    BspNode* node_stack[1024];
    int size = 1;
    node_stack[0] = startNode;
    float sphereW = _mm_shuffle_ps(sphere.v, sphere.v, 255).m128_f32[0];

    do
    {
        --size;
        BspNode* node = node_stack[size];
        if (node->cellNum == -2)
        {
            math::Vector4* plane = node->plane;
            __m128 v19 = _mm_mul_ps(sphere.v, plane->v);
            float dot = v19.m128_f32[0]
                      + (_mm_shuffle_ps(v19, v19, 85).m128_f32[0]
                         + _mm_shuffle_ps(v19, v19, 170).m128_f32[0]);
            float d = dot - _mm_shuffle_ps(plane->v, plane->v, 255).m128_f32[0];
            if (d > -sphereW)
            {
                node_stack[size++] = node->children[0];
            }
            if (sphereW > d)
            {
                node_stack[size++] = node->children[1];
            }
            if (size >= 1024)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
                AeAssert::gCurrentLine = 904;
                AeAssert::gCurrentExpr = "node_stack_size < STACK_DEPTH";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("R_FilterModelIntoCells_r: stack overflow"))
                    __debugbreak();
            }
        }
        else if (node->cellNum >= 0)
        {
            int cellNum = node->cellNum;
            int i = 0;
            while (1)
            {
                unsigned char v11 = reView->mOccupiedCells[i];
                if (v11 == (unsigned char)cellNum)
                    break;
                if (v11 == 0xFF)
                {
                    reView->mOccupiedCells[i] = (unsigned char)cellNum;
                    if (reView->mOccupiedCells[i] != (unsigned char)cellNum)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
                        AeAssert::gCurrentLine = 885;
                        AeAssert::gCurrentExpr =
                            "re->mOccupiedCells[i] == node->cellNum";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Too many cells in map!"))
                            __debugbreak();
                    }
                    worldFilterView* world = (worldFilterView*)tr.world;
                    BspTreeFilterView* bspTree =
                        (BspTreeFilterView*)world->bspTree;
                    R_AddModelToCell(
                        (char*)bspTree->mCellsList + 0x50 * cellNum,
                        re, sphere);
                    break;
                }
                if (++i >= 4)
                {
                    AeAssert::gCurrentAuthor = AeAssert::ARO;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
                    AeAssert::gCurrentLine = 892;
                    AeAssert::gCurrentExpr = "registered";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Model occupies more than MAX_OCCUPIED_CELLS!"))
                        __debugbreak();
                    break;
                }
            }
        }
    } while (size != 0);
}

// ============================================================================
// R_AddWorldSurfacesDPVS - ea: 0x006D9E00
// ============================================================================
// cvar_t view (integer +0x20)
struct cvar_t {
    uint8_t _pad[0x20];
    int integer;  // +0x20
};

extern cvar_t* r_drawworld;       // ?r_drawworld@@3PAUcvar_t@@A @ 0xF741D8
extern cvar_t* r_outsideMapEnts;  // ?r_outsideMapEnts@@3PAUcvar_t@@A @ 0xF742E0
extern cvar_t* r_singlecell;      // ?r_singlecell@@3PAUcvar_t@@A @ 0xF74294
extern int g_camera_cell;         // ?g_camera_cell@@3HA @ 0x11E993C

// untracked render.o DPVS helpers (internal in binary; stubs until ported)
static void R_SetupDPVS() {}
static int R_CellForCamera(void* frameBase) { (void)frameBase; return -1; }
static void R_FilterModelsIntoCells(void* frameBase, dpvs_plane_t* planes,
                                    int iPlaneCount)
{
    (void)frameBase; (void)planes; (void)iPlaneCount;
}
static void R_AddCellSurfaces(void* frameBase, BspCell* cell,
                              dpvs_plane_t* planes, int iPlaneCount)
{
    (void)frameBase; (void)cell; (void)planes; (void)iPlaneCount;
}
static void R_AddStaticModels(BspCell* cell, int iPlaneCount,
                              dpvs_plane_t* planes)
{
    (void)cell; (void)iPlaneCount; (void)planes;
}
static void R_RecursivePortalWalk(void* frameBase, BspCell* cell,
                                  dpvs_plane_t* parentPlane,
                                  dpvs_plane_t* planes, int iPlaneCount,
                                  int dlightBits, bool root_level)
{
    (void)frameBase; (void)cell; (void)parentPlane; (void)planes;
    (void)iPlaneCount; (void)dlightBits; (void)root_level;
}

void R_AddWorldSurfacesDPVS()
{
    alignas(16) char pvsPlaneBuffer1[0x8150];
    char* v0 = pvsPlaneBuffer1;
    char* alignedBufferPtr1 = pvsPlaneBuffer1;

    ae_sized_array<PoolAllocator::PoolConfig, 16> cfgList;
    for (int i = 0; i < 16; ++i)
    {
        cfgList.m_elements[i].blockSize = 0;
        cfgList.m_elements[i].numBlocks = 4;
        cfgList.m_elements[i].blockAlign = 0;
        cfgList.m_elements[i].block = nullptr;
    }
    cfgList.m_size = 0;
    PoolAllocator::PoolConfig elt;
    elt.blockSize = 384;
    elt.blockAlign = 16;
    elt.numBlocks = 84;
    elt.block = pvsPlaneBuffer1;
    cfgList.push_back(elt);
    PoolAllocator pvsPlanePool(cfgList, 2u);
    dpvs_plane_t::sAllocator = &pvsPlanePool;

    if (r_drawworld->integer != 0 && (tr.refdef.rdflags & 1) == 0)
    {
        int v4 = (1 << (tr.refdef.num_world_dlights & 0xFF)) - 1;
        R_SetupDPVS();
        R_FilterModelsIntoCells((void*)0, g_dpvs.frustumPlanes, 4);
        int v5 = R_CellForCamera((void*)0);
        g_camera_cell = v5;
        if (v5 < 0)
        {
            BspTreeFilterView* bspTree =
                (BspTreeFilterView*)((worldFilterView*)tr.world)->bspTree;
            unsigned int v8 = 0;
            if (r_outsideMapEnts->integer != 0)
            {
                while (v8 < bspTree->mCellsSize)
                {
                    BspCell* cell = &((BspCell*)bspTree->mCellsList)[v8];
                    R_AddCellSurfaces((void*)0, cell, g_dpvs.frustumPlanes, 4);
                    R_AddStaticModels(cell, 4, g_dpvs.frustumPlanes);
                    ++v8;
                }
            }
            else
            {
                while (v8 < bspTree->mCellsSize)
                {
                    BspCell* cell = &((BspCell*)bspTree->mCellsList)[v8];
                    R_AddCellSurfaces((void*)0, cell, g_dpvs.frustumPlanes, 4);
                    ++v8;
                }
            }
        }
        else
        {
            BspTreeFilterView* bspTree =
                (BspTreeFilterView*)((worldFilterView*)tr.world)->bspTree;
            BspCell* v6 = &((BspCell*)bspTree->mCellsList)[v5];
            BspCell* v7 = v6;
            if (r_singlecell->integer != 0)
            {
                g_dpvs.farPlane = nullptr;
                R_AddCellSurfaces((void*)0, v6, g_dpvs.frustumPlanes, 4);
                R_AddStaticModels(v7, 4, g_dpvs.frustumPlanes);
            }
            else
            {
                R_RecursivePortalWalk((void*)0, v6, &g_dpvs.viewPlane,
                                      g_dpvs.frustumPlanes, 4, v4, true);
            }
            v0 = alignedBufferPtr1;
        }
        dpvs_plane_t::sAllocator = nullptr;
        if (v0 != pvsPlaneBuffer1)
        {
            AeAssert::gCurrentAuthor = AeAssert::MJK;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
            AeAssert::gCurrentLine = 1456;
            AeAssert::gCurrentExpr =
                "alignedBufferPtr1 == (char*)tl_align((uint)pvsPlaneBuffer1, 0x10)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("pvs plane pool overflow, tell matt"))
                __debugbreak();
        }
    }
}
