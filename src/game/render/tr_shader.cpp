// ============================================================================
// tr_shader.cpp - render.o shader/lightgrid/bounds helpers (tr_shader.cpp,
// tr_light.cpp, tr_model.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"

#include <stdint.h>

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

// glfog_t / glfogType_t (tr_fog.cpp defines the data)
struct glfog_t;
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
extern glfogType_t glfogNum;  // ?glfogNum@@3W4glfogType_t@@A

// ngl helpers (ngl.o; exact binary manglings)
void nglSetAmbientLight(float r, float g, float b);   // ?nglSetAmbientLight@@YAXMMM@Z
void nglListAddDirLight(unsigned int LightCat, const math::Dir3& Dir,
                        const math::Vector4& Color);  // ?nglListAddDirLight@@YAXIABVDir3@math@@ABVVector4@2@@Z

// q_math.o
void MatrixTransformVector(const float* const in1, const float (*const in2)[3],
                           float* const out);

// LightGridData (IDA type; class V-tag; size 0x64)
class LightGridData {
public:
    math::Position3::Packed m_ambientColor;      // +0x00
    math::Vector4::Packed   m_directionalColor[3]; // +0x0C
    math::Dir3::Packed      m_directionalDir[3];   // +0x3C
    int m_numDirectional;                          // +0x60
};

// BoundingBox (IDA type; class V-tag; size 0x20)
class BoundingBox {
public:
    math::Position3 vmin;      // +0x00
    math::Position3 vmax;      // +0x10
    void accumulate(const math::Position3& p);  // ?accumulate@BoundingBox@@QAEXABVPosition3@math@@@Z
};

// XModel view (IDA type; mins +0x00, maxs +0x10; class V-tag)
class XModel {
public:
    math::Position3 mins;  // +0x00
    math::Position3 maxs;  // +0x10
};

enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };
template <typename T>
class IVPointer {
public:
    T*           mValue;   // +0x00
    unsigned int mPakId;   // +0x04
};

struct shader_t;

// ============================================================================
// R_InitShaders - ea: 0x006C2BB0
// ============================================================================
void R_InitShaders()
{
    glfogNum = FOG_NONE;
}

// ============================================================================
// R_FindShader - ea: 0x006C2BC0
// ============================================================================
shader_t* R_FindShader(const char* name, int mip, int flags, int q)
{
    (void)name; (void)mip; (void)flags; (void)q;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_shader.cpp";
    AeAssert::gCurrentLine = 78;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
    return nullptr;
}

// ============================================================================
// R_UseCachedLightSample - ea: 0x006C2C10
// ============================================================================
void R_UseCachedLightSample(const LightGridData& lightSample)
{
    nglSetAmbientLight(lightSample.m_ambientColor.x,
                       lightSample.m_ambientColor.y,
                       lightSample.m_ambientColor.z);
    for (int i = 0; i < lightSample.m_numDirectional; ++i)
    {
        math::Dir3 dir;
        dir.v.m128_f32[0] = lightSample.m_directionalDir[i].x;
        dir.v.m128_f32[1] = lightSample.m_directionalDir[i].y;
        dir.v.m128_f32[2] = lightSample.m_directionalDir[i].z;
        dir.v.m128_f32[3] = 0.0f;
        math::Vector4 color;
        color.v.m128_f32[0] = lightSample.m_directionalColor[i].x;
        color.v.m128_f32[1] = lightSample.m_directionalColor[i].y;
        color.v.m128_f32[2] = lightSample.m_directionalColor[i].z;
        color.v.m128_f32[3] = lightSample.m_directionalColor[i].w;
        nglListAddDirLight(0xFFFFFFFFu, dir, color);
    }
}

// ============================================================================
// R_GetXModelBounds - ea: 0x006C28C0
// ============================================================================
void R_GetXModelBounds(XModel* m, float (*const axis)[3], BoundingBox& bounds)
{
    for (int i = 0; i < 8; ++i)
    {
        math::Position3 q;
        q.v.m128_f32[1] = (i & 4) != 0 ? m->mins.v.m128_f32[0]
                                       : m->maxs.v.m128_f32[0];
        q.v.m128_f32[2] = (i & 2) != 0 ? m->mins.v.m128_f32[1]
                                       : m->maxs.v.m128_f32[1];
        q.v.m128_f32[3] = (i & 1) != 0 ? m->mins.v.m128_f32[2]
                                       : m->maxs.v.m128_f32[2];
        math::Position3 v9;
        MatrixTransformVector(&q.v.m128_f32[1], axis, &v9.v.m128_f32[0]);
        bounds.accumulate(v9);
    }
}

// ============================================================================
// R_ModelBounds - ea: 0x006C2970
// ============================================================================
void R_ModelBounds(IVPointer<XModel> model, TPakId pakId,
                   float* const mins, float* const maxs)
{
    (void)model; (void)pakId;
    mins[1] = 0;
    *mins = 0.0f;
    maxs[1] = 0;
    *maxs = 0.0f;
}
