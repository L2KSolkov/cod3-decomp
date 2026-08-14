// ============================================================================
// tr_dpvs.cpp - render.o DPVS mesh-record + debug-axes helpers (tr_dpvs.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/mem_heap.h"
#include "ngl/nglDebug.h"

#include <string.h>

// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0 };
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
