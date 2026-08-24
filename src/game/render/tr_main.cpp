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
enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
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
    uint8_t _pad1[0xF0 - 0x68];
    float mAlpha;            // +0xF0
    uint8_t _pad2[0xFA - 0xF4];
    unsigned char cull;      // +0xFA
    unsigned char iAmVisible : 1;  // +0xFB
    unsigned char noShadow : 1;    // +0xFB
    unsigned char iflIndex;  // +0xFB

    // ea: 0x005E9A80
    void set_transparent(unsigned char transparent)
    {
        if (transparent != 0)
        {
            if (transparent == 1)
                mAlpha = 0.5f;
            else
                mAlpha = transparent * 0.0039215689f;
        }
        else
        {
            mAlpha = 1.0f;
        }
    }
};

// viewParms_t (IDA type; size 0x1E0)
struct viewParms_t {
    orientationr_t or;              // +0x00
    orientationr_t world;           // +0x7C
    float pvsOrigin[3];             // +0xF8
    int isPortal;                   // +0x104
    int isMirror;                   // +0x108
    int frameCount;                 // +0x10C
    uint8_t _pad[0x130 - 0x110];    // portalPlane (dpvs_plane_t)
    float fovX;                     // +0x130
    float fovY;                     // +0x134
    float lodBias;                  // +0x138
    float lodScale;                 // +0x13C
    float projectionMatrix[16];     // +0x140
    float zFar;                     // +0x180
    uint8_t _frustum[0x1E0 - 0x184];  // frustum (cplane_s[4])
};
static_assert(sizeof(viewParms_t) == 0x1E0, "viewParms_t size mismatch");

// trRefdef_t view (refdef at tr+0x26C; IDA type, size 0x24)
struct trRefdef_t {
    int x;                     // +0x00
    int y;                     // +0x04
    int width;                 // +0x08
    int height;                // +0x0C
    float fov_x;               // +0x10
    float fov_y;               // +0x14
    int time;                  // +0x18
    int rdflags;               // +0x1C
    short num_world_dlights;   // +0x20
    short num_model_dlights;   // +0x22
};
static_assert(sizeof(trRefdef_t) == 0x24, "trRefdef_t size mismatch");

struct BspTreeView;
struct world_t {
    char name[128];            // +0x00
    char baseName[128];        // +0x80
    BspTreeView* bspTree;      // +0x100
};

// trGlobals_t (IDA type; size 0x3A0)
struct trGlobals_t {
    int registered;            // +0x00
    int worldMapLoaded;        // +0x04
    int frameCount;            // +0x08
    int viewCount;             // +0x0C
    viewParms_t viewParms;     // +0x10
    orientationr_t orr;        // +0x1F0 (tr.or)
    trRefdef_t refdef;         // +0x26C
    world_t* world;            // +0x290
    uint8_t _pad1[0x2A0 - 0x294];
    uint8_t viewModelInfo[0x70];  // +0x2A0 (viewModelInfo_t[1])
    int viewModelInfoIndex;    // +0x310
    uint8_t debug[0x8C];       // +0x314 (trDebug_t)
};
static_assert(sizeof(trGlobals_t) == 0x3A0, "trGlobals_t size mismatch");
trGlobals_t tr{};              // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

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
extern char* va(const char* fmt, ...);       // core.o
extern void Swap_Init();                    // render.o
extern void R_ToggleSmpFrame();             // render.o
extern void HackUpGLConfig();               // render.o
extern void R_SetViewModelScale(int a1, float a2, float a3, int a4, int a5,
                                math::Mat43* a6);  // render.o
extern void R_ScreenShot_f();               // screenshot.cpp
extern void R_ScreenShotHigh_f();           // render.o
extern int g_bOptimize;                     // ?g_bOptimize@@3HA @ 0xF743D0

// ea: 0x006C0310
void AssertCvarRange(cvar_t* cv, int shouldBeIntegral, float minVal,
                     float maxVal)
{
    if (shouldBeIntegral != 0 && cv->value != cv->integer)
    {
        ri.Printf(2, "WARNING: cvar '%s' must be integral (%f)\n", cv->name,
                  cv->value);
        ri.Cvar_Set(cv->name, va("%d", cv->integer));
    }
    if (minVal <= cv->value)
    {
        if (cv->value > maxVal)
        {
            ri.Printf(2, "WARNING: cvar '%s' out of range (%f > %f)\n",
                      cv->name, cv->value, maxVal);
            ri.Cvar_Set(cv->name, va("%f", maxVal));
        }
    }
    else
    {
        ri.Printf(2, "WARNING: cvar '%s' out of range (%f < %f)\n", cv->name,
                  cv->value, minVal);
        ri.Cvar_Set(cv->name, va("%f", minVal));
    }
}

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
// R_AddXModelSurfaces - ea: 0x006D1940
// ============================================================================
struct scr_vehicle_t;
struct nglScene;
class DObj;
class Entity;
struct EntityView {
    uint8_t _pad[0x260];
    scr_vehicle_t* scr_vehicle;  // +0x260
};
struct DObjView {
    uint8_t _pad[0xCE];
    unsigned char numModels;  // +0xCE
    unsigned char numBones;   // +0xCF
    uint8_t _pad3[0xD8 - 0xD0];
    int mLOD;             // +0xD8
    int mLODOverride;     // +0xDC
    int mLODAnim;         // +0xE0
    unsigned int mFlags;  // +0xE4
};
extern int g_DOBJF_NOT_RENDERED_LAST_FRAME;  // ?g_DOBJF_NOT_RENDERED_LAST_FRAME@@3HA (core.o)
struct cdl_proftimer {
    float value;       // +0x00
    uint8_t _pad[0x10 - 0x04];
    static void start(cdl_proftimer*) {}
    static void stop(cdl_proftimer*) {}
};
extern cdl_proftimer cdl_proftimer_temp2;  // ?cdl_proftimer_temp2@@3Ucdl_proftimer@@A (tr_stats.cpp)
extern void nglValidateMatrices(nglScene* Scene);  // ngl.o
extern nglScene* nglBuildScene;  // ?nglBuildScene@@3PAUnglScene@@A
extern int R_AddVehicleSurfaces(DObj* obj, Entity* entity,
                                const math::Mat43& matrix, float alpha,
                                bool render_shadow);  // 0x6D0B50
extern int R_AddNonVehicleSurfaces(DObj* obj, Entity* entity,
                                   const math::Mat43& matrix, float alpha,
                                   bool render_shadow,
                                   bool maxLod);  // 0x6D02D0

void R_AddXModelSurfaces(trRefEntity* ent)
{
    EntityView* entity = (EntityView*)ent->e.entity;
    if (entity == nullptr || ((unsigned int)entity & 0x200000) == 0)
    {
        DObj* obj = ent->e.obj;
        if (obj == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_xmodel.cpp";
            AeAssert::gCurrentLine = 1425;
            AeAssert::gCurrentExpr = "obj";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("we need valid DObj here"))
                __debugbreak();
        }
        if ((g_DOBJF_NOT_RENDERED_LAST_FRAME & ((DObjView*)obj)->mFlags) != 0
            && ((ent->e.renderfx & 2) == 0 || tr.viewParms.isPortal != 0))
        {
            math::Mat43 matrix;
            matrix.x.v = _mm_setr_ps(ent->e.axis[0][0], ent->e.axis[0][1],
                                     ent->e.axis[0][2], 0.0f);
            matrix.y.v = _mm_setr_ps(ent->e.axis[1][0], ent->e.axis[1][1],
                                     ent->e.axis[1][2], 0.0f);
            matrix.z.v = _mm_setr_ps(ent->e.axis[2][0], ent->e.axis[2][1],
                                     ent->e.axis[2][2], 0.0f);
            matrix.w.v = _mm_setr_ps(ent->e.origin[0], ent->e.origin[1],
                                     ent->e.origin[2], 0.0f);
            cdl_proftimer::start(&cdl_proftimer_temp2);
            nglValidateMatrices(nglBuildScene);
            if (((DObjView*)obj)->numModels >= 8u)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\tr_xmodel.cpp";
                AeAssert::gCurrentLine = 1447;
                AeAssert::gCurrentExpr = "DObjGetNumModels(obj)<DOBJ_MAX_SUBMODELS";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("buffer overflow about to happen!"))
                    __debugbreak();
            }
            bool v12 = false;
            if (ent->noShadow != 0 || (r_testshadow->integer == 0))
                v12 = false;
            else
                v12 = true;
            if (ent->e.entity == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\tr_xmodel.cpp";
                AeAssert::gCurrentLine = 1464;
                AeAssert::gCurrentExpr = "ent->e.entity";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                        "this function handles models with valid entity pointer"))
                    __debugbreak();
            }
            EntityView* v6 = (EntityView*)ent->e.entity;
            float mAlpha = ent->mAlpha;
            bool v8;
            if (v6->scr_vehicle != nullptr)
                v8 = R_AddVehicleSurfaces(obj, (Entity*)v6, matrix, mAlpha,
                                          v12);
            else
                v8 = R_AddNonVehicleSurfaces(obj, (Entity*)v6, matrix, mAlpha,
                                             v12, false);
            ent->iAmVisible = (unsigned char)v8;
            cdl_proftimer::stop(&cdl_proftimer_temp2);
        }
    }
}

// ============================================================================
// R_AddEntitySurfaces - ea: 0x006D7030
// ============================================================================
// BspTree.mCells = InplaceVector<BspCell> at +0x18 (mSize +0x18, mList +0x1C)
struct trModelCellRef_t {
    math::Vector4 sphere;      // +0x00
    trRefEntity* re;           // +0x10
    trModelCellRef_t* next;    // +0x14
    int viewCount;             // +0x18
    char pad[4];               // +0x1C
};
struct BspCellView {
    uint8_t _pad[0x30];
    int viewCount;             // +0x30
    uint8_t _pad2[0x38 - 0x34];
    trModelCellRef_t* modelRefs;  // +0x38
};
struct BspTreeView {
    uint8_t _pad[0x18];
    unsigned int mCellsSize;   // +0x18 (InplaceVector<BspCell>::mSize)
    BspCellView* mCellsList;   // +0x1C
};
int g_allVisualCount = 0;     // ?g_allVisualCount@@3HA (g.o)
int g_visualCount = 0;        // ?g_visualCount@@3HA (g.o)
int g_xmodelCount = 0;        // ?g_xmodelCount@@3HA (g.o)
int g_alwaysCount = 0;        // ?g_alwaysCount@@3HA (g.o)
extern int g_limitVisualRange;  // ?g_limitVisualRange@@3HA
extern int g_renderSphere;    // ?g_renderSphere@@3HA

// ngl matrix helpers (ngl.o)
enum nglMatrixType {
    NGLMTX_VIEW_TO_WORLD = 0,
};
extern math::Mat44* nglGetMatrix(math::Mat44* result, nglMatrixType ID,
                                 nglScene* Scene);  // ngl.o
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);  // ?_tlAssert@@YA_NPBDH00@Z (tl)

// Entity view: mDObj at +0x23C; CalcRotTranMat43 real in g_entity_misc.cpp
class Entity {
public:
    uint8_t _pad[0x23C];
    DObj* mDObj;                       // +0x23C
    const math::Mat43 CalcRotTranMat43();  // ?CalcRotTranMat43@Entity@@QAE?BVMat43@math@@XZ
};
extern int R_AddXModelSurfaces_DistanceHack(DObj* obj, Entity* entity,
                                            const math::Mat43& matrix);  // 0x6CF200

class EntityHandleDb {
public:
    struct DbElement {
        void* mObject;  // +0x00
        int   mKey;     // +0x04
    };
    uint8_t _pad[0xA8];              // HandleDb BitSet<1344>
    DbElement mElements[0x540];      // +0xA8
    static EntityHandleDb sInst;     // ?sInst@EntityHandleDb@@0V1@A (g.o)
};
template <typename HandleDb, typename T>
class DbLinkedHandle {
public:
    unsigned int mVal;               // +0x00
};
template <typename T, int CAPACITY>
class ae_sized_array {
public:
    T m_elements[CAPACITY];          // +0x00
    int m_size;                      // +sizeof(T)*CAPACITY
};
extern ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 32>
    g_AlwaysRenderEnts;  // ?g_AlwaysRenderEnts@@3V?$ae_sized_array@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@$0CA@@@A (game.o)

// phys_static_array<trRefEntity*,64>: buffer +0x00, slots +0x100, count +0x104
struct PhysArrayHugeModels {
    uint8_t m_buffer[256];           // +0x00
    trRefEntity* m_slot_array[64];   // +0x100
    unsigned int m_alloc_count;      // +0x104
};
PhysArrayHugeModels g_huge_models = {};  // ?g_huge_models@@3V?$phys_static_array@PAVtrRefEntity@@$0EA@@@A (render.o)
class Color {
public:
    float r, g, b, a;
    Color(float _r, float _g, float _b, float _a)
        : r(_r), g(_g), b(_b), a(_a) {}
};
class DebugRender {
public:
    static void RenderSphere(const math::Position3& pos, float radius,
                             const Color& color);  // g_entity_misc.cpp
};

void R_AddEntitySurfaces()
{
    if (r_drawentities->integer == 0)
        return;
    BspTreeView* bspTree = (BspTreeView*)tr.world->bspTree;
    unsigned int mSize = bspTree->mCellsSize;
    unsigned int v4 = 0;
    if (mSize != 0)
    {
        do
        {
            trModelCellRef_t* modelRefs =
                bspTree->mCellsList[v4].modelRefs;
            if (modelRefs != nullptr)
            {
                do
                {
                    if (modelRefs->viewCount != tr.viewCount)
                    {
                        trRefEntity* re = modelRefs->re;
                        modelRefs->viewCount = tr.viewCount;
                        if (re->cull != 2)
                        {
                            re->iAmVisible = 0;
                            ++g_allVisualCount;
                            math::Position3 pos;
                            pos.v = _mm_setr_ps(re->e.origin[0],
                                                re->e.origin[1],
                                                re->e.origin[2], 0.0f);
                            math::Mat44 cam;
                            nglGetMatrix(&cam, NGLMTX_VIEW_TO_WORLD,
                                         nglBuildScene);
                            if (g_limitVisualRange != 0)
                            {
                                // camera origin = row 3 of view-to-world
                                __m128 v7 = _mm_sub_ps(cam.w.v, pos.v);
                                float dx = v7.m128_f32[0];
                                float dy = v7.m128_f32[1];
                                if ((dy * dy) + (dx * dx) > 490000.0f)
                                    goto nextRef;
                            }
                            if (g_renderSphere != 0)
                                DebugRender::RenderSphere(
                                    pos, 50.0f,
                                    Color(1.0f, 1.0f, 1.0f, 1.0f));
                            ++g_visualCount;
                            if ((re->e.renderfx & 4) == 0
                                || tr.viewParms.isPortal == 0)
                            {
                                if (re->e.reType != RT_XMODEL)
                                {
                                    AeAssert::gCurrentAuthor =
                                        AeAssert::COD3;
                                    AeAssert::gCurrentFile =
                                        "c:\\cod\\code\\game\\tr_main.cpp";
                                    AeAssert::gCurrentLine = 1613;
                                    AeAssert::gCurrentExpr =
                                        "ent->e.reType == RT_XMODEL";
                                    if (!AeAssert::IsIgnored()
                                        && AeAssert::Assert("bad reType"))
                                        __debugbreak();
                                }
                                R_AddXModelSurfaces(re);
                                ++g_xmodelCount;
                            }
                        }
                    }
                nextRef:
                    modelRefs = modelRefs->next;
                } while (modelRefs != nullptr);
            }
            BspTreeView* v8 = (BspTreeView*)tr.world->bspTree;
            unsigned int v9 = v8->mCellsSize;
            ++v4;
            if (v4 >= v9)
                break;
        } while (true);
    }

    for (int v10 = 0; v10 < g_AlwaysRenderEnts.m_size; ++v10)
    {
        ++g_alwaysCount;
        ++g_visualCount;
        if (v10 >= 0x20)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        unsigned int v11 = g_AlwaysRenderEnts.m_elements[v10].mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v11 >= 0x540
            || g_AlwaysRenderEnts.m_elements[v10].mVal >> 12
                   != (unsigned int)EntityHandleDb::sInst
                          .mElements[v11].mKey
            || (mObject = (Entity*)EntityHandleDb::sInst
                              .mElements[v11]
                              .mObject) == nullptr
            || mObject->mDObj == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_main.cpp";
            AeAssert::gCurrentLine = 1629;
            AeAssert::gCurrentExpr = "ent && ent->GetDObj()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("we need a valid dobj here"))
                __debugbreak();
        }
        if (mObject != nullptr && mObject->mDObj != nullptr)
        {
            math::Mat43 matrix = mObject->CalcRotTranMat43();
            R_AddXModelSurfaces_DistanceHack(mObject->mDObj, mObject,
                                             matrix);
        }
    }
    unsigned int m_alloc_count = g_huge_models.m_alloc_count;
    for (unsigned int j = 0; j < m_alloc_count; ++j)
    {
        if (_tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                108, "i >= 0 && i < m_alloc_count", ""))
            __debugbreak();
        R_AddXModelSurfaces(g_huge_models.m_slot_array[j]);
    }
    g_huge_models.m_alloc_count = 0;
}

// ============================================================================
// RE_EndFrame - ea: 0x006D1DB0
// ============================================================================
class TimerRenderBars {
public:
    void Render();                  // ?Render@TimerRenderBars@@QAEXXZ
    static TimerRenderBars sInst;   // ?sInst@TimerRenderBars@@2U1@A
};
class TestFPS {
public:
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
