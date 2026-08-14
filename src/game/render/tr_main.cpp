// ============================================================================
// tr_main.cpp - render.o rotate/frame/mirror helpers (tr_main.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"

#include <math.h>
#include <float.h>
#include <string.h>

// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

// refEntityType_t (IDA enum; codmp_xboxr.xbe.h)
enum refEntityType_t {
    RT_BRUSHMODEL = 0x0,
    RT_XMODEL = 0x1,
    RT_STATICMODEL = 0x2,
};

// orientation_t (IDA type; size 0x30)
struct orientation_t {
    float origin[3];   // +0x00
    float axis[3][3];  // +0x0C
};
static_assert(sizeof(orientation_t) == 0x30, "orientation_t size mismatch");

// orientationr_t (IDA type; size 0x7C)
struct orientationr_t {
    float origin[3];       // +0x00
    float axis[3][3];      // +0x0C
    float viewOrigin[3];   // +0x30
    float modelMatrix[16]; // +0x3C
};
static_assert(sizeof(orientationr_t) == 0x7C, "orientationr_t size mismatch");

// trRefEntity / refEntity_t views (IDA layouts; e at +0x08)
class DObj;
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

class trRefEntity {
public:
    uint8_t _pad0[0x08];
    refEntity_t e;           // +0x08
};

// viewParms_t (IDA type; size 0x1E0)
struct viewParms_t {
    orientationr_t or;              // +0x00
    orientationr_t world;           // +0x7C
    uint8_t _pad[0x140 - 0xF8];     // pvsOrigin/isPortal/isMirror/frameCount/portalPlane/fov/lod
    float projectionMatrix[16];     // +0x140
    float zFar;                     // +0x180
    uint8_t _pad2[0x1E0 - 0x184];   // frustum
};
static_assert(sizeof(viewParms_t) == 0x1E0, "viewParms_t size mismatch");

// trRefdef_t view (refdef at tr+0x26C)
struct trRefdef_t {
    int x;                     // +0x00
    int y;                     // +0x04
    int width;                 // +0x08
    int height;                // +0x0C
    float fov_x;               // +0x10
    float fov_y;               // +0x14
    int time;                  // +0x18
    int rdflags;               // +0x1C
};

struct trGlobals_t {
    uint8_t _pad0[0x10];
    viewParms_t viewParms;     // +0x10
    orientationr_t orr;        // +0x1F0 (tr.or)
    trRefdef_t refdef;         // +0x26C
};
extern trGlobals_t tr;         // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

// cvar_t view (integer +0x20)
struct cvar_t {
    uint8_t _pad[0x1C];
    float value;               // +0x1C
    int integer;               // +0x20
};
extern cvar_t* r_znear;        // ?r_znear@@3PAUcvar_t@@A @ 0xF743A0
extern cvar_t* r_speeds;       // ?r_speeds@@3PAUcvar_t@@A @ 0xF742E4

// refimport_t view (Printf +0x00)
struct refimport_t {
    void (*Printf)(int, const char*, ...);
};
extern refimport_t ri;         // ?ri@@3Urefimport_t@@A @ 0xF741E8

// glfog_t / glfogType_t (tr_fog.cpp defines the data)
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
enum glfogType_t {
    FOG_NONE = 0x0,
    FOG_SKY = 0x1,
    FOG_PORTALVIEW = 0x2,
    FOG_MAP = 0x3,
    FOG_SERVER = 0x4,
    FOG_CURRENT = 0x5,
    FOG_LAST = 0x6,
    FOG_TARGET = 0x7,
    FOG_CMD_SWITCHFOG = 0x8,
    NUM_FOGS = 0x9,
};
extern glfog_t glfogsettings[NUM_FOGS];  // ?glfogsettings@@3PAUglfog_t@@A

// r_stubs.cpp
void myGlMultMatrix(const float* a, const float* b, float* out);

// ============================================================================
// R_RotateForModelEntity - ea: 0x006C1080
// ============================================================================
void R_RotateForModelEntity(trRefEntity* ent, const viewParms_t* viewParms,
                            orientationr_t* orr)
{
    if (ent->e.reType != RT_XMODEL && ent->e.reType != RT_STATICMODEL
        && ent->e.reType != RT_BRUSHMODEL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_main.cpp";
        AeAssert::gCurrentLine = 715;
        AeAssert::gCurrentExpr =
            "ent->e.reType == RT_XMODEL || ent->e.reType == RT_STATICMODEL || ent->e.reType == RT_BRUSHMODEL";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    orr->origin[0] = ent->e.origin[0];
    orr->origin[1] = ent->e.origin[1];
    orr->origin[2] = ent->e.origin[2];
    orr->axis[0][0] = ent->e.axis[0][0];
    orr->axis[0][1] = ent->e.axis[0][1];
    orr->axis[0][2] = ent->e.axis[0][2];
    orr->axis[1][0] = ent->e.axis[1][0];
    orr->axis[1][1] = ent->e.axis[1][1];
    orr->axis[1][2] = ent->e.axis[1][2];
    orr->axis[2][0] = ent->e.axis[2][0];
    orr->axis[2][1] = ent->e.axis[2][1];
    orr->axis[2][2] = ent->e.axis[2][2];

    float glMatrix[16];
    glMatrix[0] = orr->axis[0][0];
    glMatrix[1] = orr->axis[0][1];
    glMatrix[2] = orr->axis[0][2];
    glMatrix[3] = 0.0f;
    glMatrix[4] = orr->axis[1][0];
    glMatrix[5] = orr->axis[1][1];
    glMatrix[6] = orr->axis[1][2];
    glMatrix[7] = 0.0f;
    glMatrix[8] = orr->axis[2][0];
    glMatrix[9] = orr->axis[2][1];
    glMatrix[10] = orr->axis[2][2];
    glMatrix[11] = 0.0f;
    glMatrix[12] = orr->origin[0];
    glMatrix[13] = orr->origin[1];
    glMatrix[14] = orr->origin[2];
    glMatrix[15] = 1.0f;
    myGlMultMatrix(glMatrix, viewParms->world.modelMatrix, orr->modelMatrix);

    float v16 = viewParms->or.origin[0] - orr->origin[0];
    float v17 = viewParms->or.origin[2] - orr->origin[2];
    float v18 = viewParms->or.origin[1] - orr->origin[1];
    orr->viewOrigin[0] = ((orr->axis[0][0] * v16) + (orr->axis[0][2] * v17))
                       + (v18 * orr->axis[0][1]);
    orr->viewOrigin[1] = ((orr->axis[1][1] * v18) + (orr->axis[1][0] * v16))
                       + (v17 * orr->axis[1][2]);
    orr->viewOrigin[2] = ((orr->axis[2][2] * v17) + (orr->axis[2][1] * v18))
                       + (orr->axis[2][0] * v16);
}

// ============================================================================
// R_RotateForEntity - ea: 0x006C1270
// ============================================================================
void R_RotateForEntity(trRefEntity* ent, const viewParms_t* viewParms,
                       orientationr_t* orr)
{
    if (ent->e.reType == RT_XMODEL || ent->e.reType == RT_STATICMODEL
        || ent->e.reType == RT_BRUSHMODEL)
        R_RotateForModelEntity(ent, viewParms, orr);
    else
        *orr = viewParms->world;
}

// ============================================================================
// R_RotateForViewer - ea: 0x006C12B0
// ============================================================================
// CameraShake (cg.o; g_cameraShake[4] @ 0xF056E8)
class CameraShake {
public:
    math::Mat43* CreateCameraShakeMatrix(math::Mat43* pCamLocal);  // ?CreateCameraShakeMatrix@CameraShake@@QAEPAVMat43@math@@PAV23@@Z
};
extern CameraShake* g_cameraShake;  // ?g_cameraShake@@3PAVCameraShake@@A @ 0xF056E8
extern int currCl;  // ?currCl@@3HA @ 0xF1579C

// s_flipMatrix (render.o @ 0xDFB0F8; IDA bytes)
static const float s_flipMatrix[16] = {
    0.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};

void R_RotateForViewer()
{
    memset(&tr.orr, 0, sizeof(tr.orr));
    tr.orr.axis[0][0] = 1.0f;
    tr.orr.axis[1][1] = 1.0f;
    tr.orr.axis[2][2] = 1.0f;
    memcpy(tr.orr.viewOrigin, &tr.viewParms, sizeof(tr.orr.viewOrigin));

    float viewerMatrix[16];
    viewerMatrix[14] = tr.viewParms.or.origin[0];
    viewerMatrix[15] = tr.viewParms.or.origin[1];
    float camZ = tr.viewParms.or.origin[2];

    // pCamLocal = 4x3 camera matrix (3 axis rows + origin row)
    math::Mat43 camLocal;
    camLocal.x.v = _mm_setr_ps(tr.viewParms.or.axis[0][0],
                               tr.viewParms.or.axis[0][1],
                               tr.viewParms.or.axis[0][2], 0.0f);
    camLocal.y.v = _mm_setr_ps(tr.viewParms.or.axis[1][0],
                               tr.viewParms.or.axis[1][1],
                               tr.viewParms.or.axis[1][2], 0.0f);
    camLocal.z.v = _mm_setr_ps(tr.viewParms.or.axis[2][0],
                               tr.viewParms.or.axis[2][1],
                               tr.viewParms.or.axis[2][2], 0.0f);
    camLocal.w.v = _mm_setr_ps(tr.viewParms.or.origin[0],
                               tr.viewParms.or.origin[1],
                               tr.viewParms.or.origin[2], 0.0f);
    CameraShake* shake = &g_cameraShake[currCl];
    math::Mat43 shaken;
    if (shake != nullptr)
    {
        math::Mat43* p = shake->CreateCameraShakeMatrix(&camLocal);
        if (p != nullptr)
            shaken = *p;
        else
            shaken = camLocal;
    }
    else
    {
        shaken = camLocal;
    }

    __m128 row0 = shaken.x.v;
    __m128 row1 = shaken.y.v;
    __m128 row2 = shaken.z.v;
    __m128 row0y = _mm_shuffle_ps(row0, row0, 85);
    __m128 row0z = _mm_shuffle_ps(row0, row0, 170);
    viewerMatrix[5] = row0z.m128_f32[0];
    viewerMatrix[1] = row0y.m128_f32[0];
    viewerMatrix[9] = 0.0f - (((row0z.m128_f32[0] * camZ)
                              + (row0y.m128_f32[0] * viewerMatrix[15]))
                             + (row0.m128_f32[0] * viewerMatrix[14]));

    __m128 row1y = _mm_shuffle_ps(row1, row1, 85);
    __m128 row1z = _mm_shuffle_ps(row1, row1, 170);
    viewerMatrix[2] = row1y.m128_f32[0];
    viewerMatrix[6] = row1z.m128_f32[0];
    viewerMatrix[10] = 0.0f - (((row1z.m128_f32[0] * camZ)
                               + (row1y.m128_f32[0] * viewerMatrix[15]))
                              + (row1.m128_f32[0] * viewerMatrix[14]));

    __m128 row2y = _mm_shuffle_ps(row2, row2, 85);
    __m128 row2z = _mm_shuffle_ps(row2, row2, 170);
    viewerMatrix[7] = row2z.m128_f32[0];
    viewerMatrix[3] = row2y.m128_f32[0];
    viewerMatrix[0] = 0.0f;
    viewerMatrix[4] = 0.0f;
    viewerMatrix[8] = 0.0f;
    viewerMatrix[11] = 0.0f - (((row2z.m128_f32[0] * camZ)
                               + (row2y.m128_f32[0] * viewerMatrix[15]))
                              + (row2.m128_f32[0] * viewerMatrix[14]));
    viewerMatrix[12] = 1.0f;
    (void)viewerMatrix;

    // mat4 from the shaken 4x3 rows
    float mat4[16];
    memcpy(&mat4[0], &shaken.x, 64);
    myGlMultMatrix(mat4, s_flipMatrix, tr.orr.modelMatrix);
    tr.viewParms.world = tr.orr;

    if ((_fpclass((double)tr.orr.axis[0][0]) & 0x297) != 0
        || (_fpclass((double)tr.viewParms.world.axis[0][1]) & 0x297) != 0
        || (_fpclass((double)tr.viewParms.world.axis[0][2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_main.cpp";
        AeAssert::gCurrentLine = 860;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tr.viewParms.world.axis[0])[0]) && !IS_NAN((tr.viewParms.world.axis[0])[1]) && !IS_NAN((tr.viewParms.world.axis[0])[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
}

// ============================================================================
// R_SetFrameFog - ea: 0x006C1630
// ============================================================================
void R_SetFrameFog()
{
    if (glfogsettings[FOG_TARGET].registered != 0)
    {
        float end;
        if (glfogsettings[FOG_TARGET].finishTime != 0
            && glfogsettings[FOG_TARGET].finishTime >= tr.refdef.time)
        {
            int v0 = glfogsettings[FOG_TARGET].finishTime
                   - glfogsettings[FOG_TARGET].startTime;
            if (v0 <= 0)
                v0 = 1;
            float v1 = (float)(tr.refdef.time - glfogsettings[FOG_TARGET].startTime)
                     / (float)v0;
            if (v1 > 1.0f)
                v1 = 1.0f;
            glfogsettings[FOG_CURRENT].start =
                ((glfogsettings[FOG_TARGET].start - glfogsettings[FOG_LAST].start) * v1)
                + glfogsettings[FOG_LAST].start;
            end =
                ((glfogsettings[FOG_TARGET].end - glfogsettings[FOG_LAST].end) * v1)
                + glfogsettings[FOG_LAST].end;
            glfogsettings[FOG_CURRENT].density =
                ((glfogsettings[FOG_TARGET].density - glfogsettings[FOG_LAST].density) * v1)
                + glfogsettings[FOG_LAST].density;
            glfogsettings[FOG_CURRENT].color[0] =
                ((glfogsettings[FOG_TARGET].color[0] - glfogsettings[FOG_LAST].color[0]) * v1)
                + glfogsettings[FOG_LAST].color[0];
            glfogsettings[FOG_CURRENT].color[1] =
                ((glfogsettings[FOG_TARGET].color[1] - glfogsettings[FOG_LAST].color[1]) * v1)
                + glfogsettings[FOG_LAST].color[1];
            glfogsettings[FOG_CURRENT].end = end;
            glfogsettings[FOG_CURRENT].color[2] =
                ((glfogsettings[FOG_TARGET].color[2] - glfogsettings[FOG_LAST].color[2]) * v1)
                + glfogsettings[FOG_LAST].color[2];
            glfogsettings[FOG_CURRENT].registered = 1;
            glfogsettings[FOG_CURRENT].clearscreen = 0;
            if (glfogsettings[FOG_TARGET].clearscreen != 0
                || glfogsettings[FOG_LAST].clearscreen != 0)
                glfogsettings[FOG_CURRENT].clearscreen = 1;
            glfogsettings[FOG_CURRENT].dirty = 1;
        }
        else
        {
            glfogsettings[FOG_CURRENT] = glfogsettings[FOG_TARGET];
            end = glfogsettings[FOG_TARGET].end;
            glfogsettings[FOG_CURRENT].dirty = 0;
        }
        if (tr.viewParms.zFar > end)
            tr.viewParms.zFar = end;
        if (r_speeds->integer == 5)
        {
            ri.Printf(0,
                "farclip fog - den: %0.1f  calc zFar: %0.1f  fog zfar: %0.1f\n",
                glfogsettings[FOG_CURRENT].density, tr.viewParms.zFar,
                glfogsettings[FOG_CURRENT].end);
        }
    }
    else if (r_speeds->integer == 5)
    {
        ri.Printf(0, "no fog - calc zFar: %0.1f\n", tr.viewParms.zFar);
    }
}

// ============================================================================
// R_SetupProjection - ea: 0x006C1830
// ============================================================================
void R_SetupProjection()
{
    float v1 = r_znear->value * -2.0f;
    tr.viewParms.projectionMatrix[4] = 0.0f;
    tr.viewParms.projectionMatrix[12] = 0.0f;
    tr.viewParms.projectionMatrix[1] = 0.0f;
    tr.viewParms.projectionMatrix[13] = 0.0f;
    tr.viewParms.projectionMatrix[2] = 0.0f;
    tr.viewParms.projectionMatrix[6] = 0.0f;
    tr.viewParms.projectionMatrix[14] = v1;
    tr.viewParms.projectionMatrix[3] = 0.0f;
    tr.viewParms.projectionMatrix[7] = 0.0f;
    tr.viewParms.projectionMatrix[15] = 0.0f;
    float ymax = tanf(tr.refdef.fov_y * 0.0087266462f);
    float xmax = tanf(tr.refdef.fov_x * 0.0087266462f);
    tr.viewParms.projectionMatrix[8] =
        ((0.0f - xmax) + xmax) * (1.0f / (xmax - (0.0f - xmax)));
    float v2 = 1.0f / (ymax - (0.0f - ymax));
    tr.viewParms.projectionMatrix[9] = ((0.0f - ymax) + ymax) * v2;
    tr.viewParms.projectionMatrix[0] =
        (1.0f / (xmax - (0.0f - xmax))) * 2.0f;
    tr.viewParms.projectionMatrix[5] = v2 * 2.0f;
    tr.viewParms.projectionMatrix[10] = -1.0f;
    tr.viewParms.projectionMatrix[11] = -1.0f;
}

// ============================================================================
// R_MirrorPoint - ea: 0x006C1960
// ============================================================================
void R_MirrorPoint(float* const in, orientation_t* surface,
                   orientation_t* camera, float* const out)
{
    float v4 = in[1] - surface->origin[1];
    float v5 = in[2] - surface->origin[2];
    float v6 = *in - surface->origin[0];
    float v7 = ((surface->axis[0][2] * v5) + (surface->axis[0][1] * v4))
             + (surface->axis[0][0] * v6);
    float v8 = camera->axis[0][0] * v7;
    float v9 = camera->axis[0][1] * v7;
    float v10 = camera->axis[0][2] * v7;
    float v11 = ((surface->axis[1][2] * v5) + (surface->axis[1][1] * v4))
              + (surface->axis[1][0] * v6);
    float v12 = (camera->axis[1][0] * v11) + v8;
    float v13 = (camera->axis[1][1] * v11) + v9;
    float v14 = camera->axis[1][2] * v11;
    float v15 = ((surface->axis[2][2] * v5) + (surface->axis[2][1] * v4))
              + (surface->axis[2][0] * v6);
    float v16 = camera->axis[2][1];
    float v17 = camera->axis[2][2] * v15;
    *out = camera->origin[0] + ((camera->axis[2][0] * v15) + v12);
    out[1] = camera->origin[1] + ((v16 * v15) + v13);
    out[2] = camera->origin[2] + (v17 + (v14 + v10));
}

// ============================================================================
// R_MirrorVector - ea: 0x006C1A90
// ============================================================================
void R_MirrorVector(float* const in, orientation_t* surface,
                    orientation_t* camera, float* const out)
{
    out[2] = 0.0f;
    out[1] = 0.0f;
    *out = 0.0f;
    float v6 = ((surface->axis[0][2] * in[2]) + (surface->axis[0][1] * in[1]))
             + (surface->axis[0][0] * *in);
    *out = camera->axis[0][0] * v6;
    out[1] = camera->axis[0][1] * v6;
    out[2] = camera->axis[0][2] * v6;
    float v7 = ((surface->axis[1][2] * in[2]) + (surface->axis[1][1] * in[1]))
             + (surface->axis[1][0] * *in);
    *out = (camera->axis[1][0] * v7) + *out;
    out[1] = (camera->axis[1][1] * v7) + out[1];
    out[2] = (camera->axis[1][2] * v7) + out[2];
    float v8 = ((surface->axis[2][2] * in[2]) + (surface->axis[2][1] * in[1]))
             + (surface->axis[2][0] * *in);
    *out = (camera->axis[2][0] * v8) + *out;
    out[1] = (camera->axis[2][1] * v8) + out[1];
    out[2] = (camera->axis[2][2] * v8) + out[2];
}

// ============================================================================
// R_ClearAlpha - ea: 0x006C1F40
// ============================================================================
void R_ClearAlpha()
{
}
