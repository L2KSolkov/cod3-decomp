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
    uint8_t _pad1[0x314 - 0x284];
    uint8_t debug[0x80];       // +0x314 (trDebug_t)
};
extern trGlobals_t tr;         // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

// cvar_t view (integer +0x20)
struct cvar_t {
    const char* name;          // +0x00
    uint8_t _pad[0x1C - 0x04];
    float value;               // +0x1C
    int integer;               // +0x20
};

// refimport_t view (Printf +0x00)
struct refimport_t {
    void (*Printf)(int, const char*, ...);
    void (*Error)(int, const char*, ...);
    uint8_t _pad1[0x20 - 0x08];
    cvar_t* (*Cvar_Get)(const char*, const char*, int);   // +0x20
    uint8_t _pad2[0x28 - 0x24];
    void (*Cvar_Set)(const char*, const char*);           // +0x28
    void (*Cmd_AddCommand)(const char*, void (__cdecl*)());  // +0x2C
};
extern refimport_t ri;         // ?ri@@3Urefimport_t@@A @ 0xF741E8

// cvar globals (render.o data; R_Register)
cvar_t* r_cheats;               // ?r_cheats@@3PAUcvar_t@@A @ 0xF742C4
cvar_t* r_texturebits;          // ?r_texturebits@@3PAUcvar_t@@A @ 0xF742D8
cvar_t* r_mode;                 // ?r_mode@@3PAUcvar_t@@A @ 0xF741C0
cvar_t* r_fullscreen;           // ?r_fullscreen@@3PAUcvar_t@@A @ 0xF7418C
cvar_t* r_weightMipMaps;        // ?r_weightMipMaps@@3PAUcvar_t@@A @ 0xF7429C
cvar_t* r_uiFullScreen;         // ?r_uiFullScreen@@3PAUcvar_t@@A @ 0xF74278
cvar_t* r_fullbright;           // ?r_fullbright@@3PAUcvar_t@@A @ 0xF741E0
cvar_t* r_lodbias;              // ?r_lodbias@@3PAUcvar_t@@A @ 0xF741B0
cvar_t* r_flares;               // ?r_flares@@3PAUcvar_t@@A @ 0xF74180
cvar_t* r_znear;                // ?r_znear@@3PAUcvar_t@@A @ 0xF743A0
cvar_t* r_zfar;                 // ?r_zfar@@3PAUcvar_t@@A @ 0xF741A0
cvar_t* r_dynamiclight;         // ?r_dynamiclight@@3PAUcvar_t@@A @ 0xF742A8
cvar_t* r_finish;               // ?r_finish@@3PAUcvar_t@@A @ 0xF742D4
cvar_t* r_textureMode;          // ?r_textureMode@@3PAUcvar_t@@A @ 0xF741AC
cvar_t* r_swapDelay;            // ?r_swapDelay@@3PAUcvar_t@@A @ 0xF742F8
cvar_t* r_LightScale;           // ?r_LightScale@@3PAUcvar_t@@A @ 0xF741B4
cvar_t* r_maxEntLights;         // ?r_maxEntLights@@3PAUcvar_t@@A @ 0xF741D0
cvar_t* r_minEntLightIntensity; // ?r_minEntLightIntensity@@3PAUcvar_t@@A @ 0xF742BC
cvar_t* r_vc_makelog;           // ?r_vc_makelog@@3PAUcvar_t@@A @ 0xF74270
cvar_t* r_vc_showlog;           // ?r_vc_showlog@@3PAUcvar_t@@A @ 0xF742E8
cvar_t* r_vc_compile;           // ?r_vc_compile@@3PAUcvar_t@@A @ 0xF742B4
cvar_t* r_fog;                  // ?r_fog@@3PAUcvar_t@@A @ 0xF743B4
cvar_t* r_portalOnly;           // ?r_portalOnly@@3PAUcvar_t@@A @ 0xF741A8
cvar_t* r_lodscale;             // ?r_lodscale@@3PAUcvar_t@@A @ 0xF741D4
cvar_t* r_norefresh;            // ?r_norefresh@@3PAUcvar_t@@A @ 0xF7417C
cvar_t* r_drawentities;         // ?r_drawentities@@3PAUcvar_t@@A @ 0xF74190
cvar_t* r_nocull;               // ?r_nocull@@3PAUcvar_t@@A @ 0xF74284
cvar_t* r_outsideMapEnts;       // ?r_outsideMapEnts@@3PAUcvar_t@@A @ 0xF742E0
cvar_t* r_speeds;               // ?r_speeds@@3PAUcvar_t@@A @ 0xF742E4
cvar_t* r_verbose;              // ?r_verbose@@3PAUcvar_t@@A @ 0xF73F34
cvar_t* r_logFile;              // ?r_logFile@@3PAUcvar_t@@A @ 0xF742F0
cvar_t* r_profileDrawElements;  // ?r_profileDrawElements@@3PAUcvar_t@@A @ 0xF74194
cvar_t* r_showtris;             // ?r_showtris@@3PAUcvar_t@@A @ 0xF74198
cvar_t* r_showtricounts;        // ?r_showtricounts@@3PAUcvar_t@@A @ 0xF741C4
cvar_t* r_showsurfcounts;       // ?r_showsurfcounts@@3PAUcvar_t@@A @ 0xF74184
cvar_t* r_clear;                // ?r_clear@@3PAUcvar_t@@A @ 0xF7427C
cvar_t* r_offsetFactor;         // ?r_offsetFactor@@3PAUcvar_t@@A @ 0xF741CC
cvar_t* r_offsetUnits;          // ?r_offsetUnits@@3PAUcvar_t@@A @ 0xF742A0
cvar_t* r_lockpvs;              // ?r_lockpvs@@3PAUcvar_t@@A @ 0xF7428C
cvar_t* r_noportals;            // ?r_noportals@@3PAUcvar_t@@A @ 0xF74298
cvar_t* r_portalsky;            // ?r_portalsky@@3PAUcvar_t@@A @ 0xF742CC
cvar_t* r_showportals;          // ?r_showportals@@3PAUcvar_t@@A @ 0xF741B8
cvar_t* r_cullBModels;          // ?r_cullBModels@@3PAUcvar_t@@A @ 0xF742F4
cvar_t* r_cullXModels;          // ?r_cullXModels@@3PAUcvar_t@@A @ 0xF74178
cvar_t* r_showSkeletons;        // ?r_showSkeletons@@3PAUcvar_t@@A @ 0xF743B8
cvar_t* r_showLocationalDamage; // ?r_showLocationalDamage@@3PAUcvar_t@@A @ 0xF741DC
cvar_t* r_singlecell;           // ?r_singlecell@@3PAUcvar_t@@A @ 0xF74294
cvar_t* r_testshadow;           // ?r_testshadow@@3PAUcvar_t@@A @ 0xF742C0
cvar_t* r_testlight;            // ?r_testlight@@3PAUcvar_t@@A @ 0xF742EC
cvar_t* r_showlightgrid;        // ?r_showlightgrid@@3PAUcvar_t@@A @ 0xF743A8
cvar_t* r_drawworld;            // ?r_drawworld@@3PAUcvar_t@@A @ 0xF741D8

extern cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                        int flags);  // ?Cvar_Get@@YAPAUcvar_t@@PBD0H@Z
extern void AssertCvarRange(cvar_t* cv, int shouldBeIntegral, float minVal,
                            float maxVal);  // render.o 0x6C0310
extern int Swap_Init();                     // render.o
extern void R_ToggleSmpFrame();             // render.o
extern void HackUpGLConfig();               // render.o
extern void R_SetViewModelScale(int a1, float a2, float a3, int a4, int a5,
                                math::Mat43* a6);  // render.o
extern void R_ScreenShot_f();               // screenshot.cpp
extern void R_ScreenShotHigh_f();           // render.o
extern int g_bOptimize;                     // ?g_bOptimize@@3HA @ 0xF743D0

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
extern glfogType_t glfogNum;             // ?glfogNum@@3W4glfogType_t@@A (tr_fog.cpp)

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

// ============================================================================
// R_Register / R_Init - ea: 0x006D22B0 / 0x006D27F0
// ============================================================================
void R_Register()
{
    r_cheats = Cvar_Get("sv_cheats", "0", 72);
    r_texturebits = ri.Cvar_Get("r_texturebits", "0", 33);
    r_mode = ri.Cvar_Get("r_mode", "3", 33);
    r_fullscreen = ri.Cvar_Get("r_fullscreen", "1", 33);
    r_weightMipMaps = ri.Cvar_Get("r_weightMipMaps", "0", 32);
    r_uiFullScreen = ri.Cvar_Get("r_uifullscreen", "0", 0);
    r_fullbright = ri.Cvar_Get("r_fullbright", "0", 544);
    r_lodbias = ri.Cvar_Get("r_lodbias", "0", 1);
    r_flares = ri.Cvar_Get("r_flares", "1", 1);
    r_znear = ri.Cvar_Get("r_znear", "6.0", 256);
    AssertCvarRange(r_znear, 1, 0.001f, 200.0f);
    r_zfar = ri.Cvar_Get("r_zfar", "8192", 512);
    r_dynamiclight = ri.Cvar_Get("r_dynamiclight", "1", 1);
    r_finish = ri.Cvar_Get("r_finish", "0", 1);
    r_textureMode = ri.Cvar_Get("r_textureMode", "GL_LINEAR_MIPMAP_NEAREST", 1);
    r_swapDelay = ri.Cvar_Get("r_swapDelay", "0", 1);
    r_LightScale = ri.Cvar_Get("r_LightScale", "1.0", 256);
    r_maxEntLights = ri.Cvar_Get("r_maxEntLights", "8", 1);
    r_minEntLightIntensity = ri.Cvar_Get("r_minEntLightIntensity", "0.02", 513);
    r_vc_makelog = ri.Cvar_Get("r_vc_makelog", "0", 32);
    r_vc_showlog = ri.Cvar_Get("r_vc_showlog", "0", 0);
    r_vc_compile = ri.Cvar_Get("r_vc_compile", "0", 32);
    r_fog = ri.Cvar_Get("r_fog", "1", 512);
    r_portalOnly = ri.Cvar_Get("r_portalOnly", "0", 512);
    r_lodscale = ri.Cvar_Get("r_lodscale", "1", 1);
    r_norefresh = ri.Cvar_Get("r_norefresh", "0", 512);
    r_drawentities = ri.Cvar_Get("r_drawentities", "1", 512);
    r_nocull = ri.Cvar_Get("r_nocull", "0", 512);
    r_outsideMapEnts = ri.Cvar_Get("outsideMapEnts", "0", 512);
    r_speeds = ri.Cvar_Get("r_speeds", "0", 512);
    r_verbose = ri.Cvar_Get("r_verbose", "0", 0);
    r_logFile = ri.Cvar_Get("r_logFile", "0", 0);
    r_profileDrawElements = ri.Cvar_Get("r_profileDrawElements", "0", 512);
    r_showtris = ri.Cvar_Get("r_showtris", "0", 512);
    r_showtricounts = ri.Cvar_Get("r_showtricounts", "0", 512);
    r_showsurfcounts = ri.Cvar_Get("r_showsurfcounts", "0", 512);
    r_clear = ri.Cvar_Get("r_clear", "0", 512);
    r_offsetFactor = ri.Cvar_Get("r_offsetfactor", "-1", 512);
    r_offsetUnits = ri.Cvar_Get("r_offsetunits", "-2", 512);
    r_lockpvs = ri.Cvar_Get("r_lockpvs", "0", 512);
    r_noportals = ri.Cvar_Get("r_noportals", "0", 512);
    r_portalsky = ri.Cvar_Get("cg_skybox", "1", 0);
    r_showportals = ri.Cvar_Get("r_showportals", "0", 512);
    r_cullBModels = ri.Cvar_Get("r_cullBModels", "1", 0);
    r_cullXModels = ri.Cvar_Get("r_cullXModels", "1", 0);
    r_showSkeletons = ri.Cvar_Get("r_showSkeletons", "0", 512);
    r_showLocationalDamage = ri.Cvar_Get("r_showLocationalDamage", "0", 512);
    r_singlecell = ri.Cvar_Get("r_singlecell", "0", 512);
    ri.Cmd_AddCommand("screenshot", R_ScreenShot_f);
    ri.Cmd_AddCommand("screenshot_high", R_ScreenShotHigh_f);
    r_testshadow = ri.Cvar_Get("r_testshadow", "1", 256);
    r_testlight = ri.Cvar_Get("r_testlight", "0", 512);
    r_showlightgrid = ri.Cvar_Get("r_showlightgrid", "0", 512);
    r_drawworld = ri.Cvar_Get("r_drawworld", "1", 512);
}

void R_Init()
{
    ri.Printf(0, "----- R_Init -----\n");
    memset(&tr, 0, sizeof(tr));
    Swap_Init();
    R_Register();
    R_ToggleSmpFrame();
    HackUpGLConfig();
    glfogNum = FOG_NONE;
    R_SetViewModelScale(0, 1.0, 1.0, 0, 0, nullptr);
    memset(&tr.debug, 0, sizeof(tr.debug));
    g_bOptimize = 0;
    ri.Printf(0, "----- finished R_Init -----\n");
}

// ============================================================================
// RE_EndFrame - ea: 0x006D1DB0
// ============================================================================
struct TimerRenderBars {
    void Render();                  // ?Render@TimerRenderBars@@QAEXXZ
    static TimerRenderBars sInst;   // ?sInst@TimerRenderBars@@2U1@A
};
struct TestFPS {
    void GatherMetrics();           // ?GatherMetrics@TestFPS@@QAEXXZ
    static TestFPS* sInst;          // ?sInst@TestFPS@@2PAV1@A
};
class ServerTime {
public:
    uint8_t _pad[0x08];
    float mTickDelta;               // +0x08
    static ServerTime sInst;        // ?sInst@ServerTime@@2V1@A
};
extern int gDelayRenderForNFrames;  // ?gDelayRenderForNFrames@@3HA @ 0xE92A68
extern void nglPresent();           // ngl.o
extern void R_ToggleSmpFrame();     // render.o
extern void UpdateShotProf(float mTickDelta);  // g.o

void RE_EndFrame(int* frontEndMsec, int* backEndMsec)
{
    (void)frontEndMsec;
    (void)backEndMsec;
    TimerRenderBars::sInst.Render();
    if (gDelayRenderForNFrames != 0)
        --gDelayRenderForNFrames;
    else
        nglPresent();
    R_ToggleSmpFrame();
    TestFPS::sInst->GatherMetrics();
    UpdateShotProf(ServerTime::sInst.mTickDelta);
}
