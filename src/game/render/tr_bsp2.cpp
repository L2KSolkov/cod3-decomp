// ============================================================================
// tr_bsp2.cpp - render.o BSP walk / frustum / debug-string / sky/glow helpers
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/color.h"
#include "game/logic/g_local.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_scene.h"
#include "render/ShaderCommon.h"

#include <math.h>

// ============================================================================
// BSP views (IDA types)
// ============================================================================
struct BspPlane {
    math::Vector4 mPlane;      // +0x00
};
struct BspNode {
    short contents;            // +0x00
    short cellNum;             // +0x02
    union {
        struct {
            int children[2];   // +0x04
            BspPlane* plane;   // +0x0C
        } node;
    } u;                       // +0x04
};
static_assert(sizeof(BspNode) == 0x10, "BspNode size mismatch");

struct BspTree {
public:
    uint8_t _pad[8];
    unsigned int mNodesSize;   // +0x08
    BspNode* mNodesList;       // +0x0C
};
extern BspTree* g_bspTree;     // ?g_bspTree@@3PAVBspTree@@A @ 0xF743DC

// ============================================================================
// R_CellForPoint - ea: 0x006C52B0
// ============================================================================
int R_CellForPoint(const math::Position3& pos)
{
    BspNode* node = &g_bspTree->mNodesList[0];
    while (node->contents == -1)
    {
        if (node->cellNum >= 0)
            break;
        __m128 planeV = node->u.node.plane->mPlane.v;
        __m128 v3 = _mm_mul_ps(pos.v, planeV);
        float d = (v3.m128_f32[0]
                   + (_mm_shuffle_ps(v3, v3, 85).m128_f32[0]
                      + _mm_shuffle_ps(v3, v3, 170).m128_f32[0]))
                - _mm_shuffle_ps(pos.v, pos.v, 255).m128_f32[0];
        node = &g_bspTree->mNodesList[node->u.node.children[d <= 0.0f]];
    }
    return node->cellNum;
}

// ============================================================================
// calc_cell_index - ea: 0x006C5380
// ============================================================================
int calc_cell_index(const math::Position3& cur_pos, float (&cached_pos)[4],
                    int& cached_index)
{
    if (cached_pos[3] > 0.0f)
    {
        __m128 cached;
        cached.m128_f32[0] = cached_pos[0];
        cached.m128_f32[1] = cached_pos[1];
        cached.m128_f32[2] = cached_pos[2];
        __m128 v5 = _mm_sub_ps(cached, cur_pos.v);
        __m128 v6 = _mm_mul_ps(v5, v5);
        float distSq = v6.m128_f32[0]
                     + (_mm_shuffle_ps(v6, v6, 85).m128_f32[0]
                        + _mm_shuffle_ps(v6, v6, 170).m128_f32[0]);
        if ((cached_pos[3] * cached_pos[3]) > distSq)
            return cached_index;
    }

    float minDist = 1.0e20f;
    const math::Position3* v4 = &cur_pos;
    BspNode* node = &g_bspTree->mNodesList[0];
    while (node->contents == -1)
    {
        if (node->cellNum >= 0)
            break;
        __m128 planeV = node->u.node.plane->mPlane.v;
        v4 = &cur_pos;
        __m128 v10 = _mm_mul_ps(cur_pos.v, planeV);
        float d = (v10.m128_f32[0]
                   + (_mm_shuffle_ps(v10, v10, 85).m128_f32[0]
                      + _mm_shuffle_ps(v10, v10, 170).m128_f32[0]))
                - _mm_shuffle_ps(planeV, planeV, 255).m128_f32[0];
        node = &g_bspTree->mNodesList[node->u.node.children[d <= 0.0f]];
        float ad = fabsf(d);
        if (ad <= minDist)
            minDist = ad;
    }
    cached_pos[0] = v4->v.m128_f32[0];
    cached_pos[1] = v4->v.m128_f32[1];
    cached_pos[2] = v4->v.m128_f32[2];
    cached_pos[3] = minDist;
    int result = node->cellNum;
    cached_index = result;
    return result;
}

// ============================================================================
// R_SetupFrustum - ea: 0x006C65B0
// ============================================================================
void SetPlaneSignbits(cplane_s* out);  // ?SetPlaneSignbits@@YAXPAUcplane_s@@@Z

struct orientationr_t {
    float origin[3];       // +0x00
    float axis[3][3];      // +0x0C
    float viewOrigin[3];   // +0x30
    float modelMatrix[16]; // +0x3C
};
static_assert(sizeof(orientationr_t) == 0x7C, "orientationr_t size mismatch");

struct viewParms_t {
    orientationr_t orr;        // +0x00
    uint8_t _pad[0x130 - 0x7C];
    float fovX;                // +0x130
    float fovY;                // +0x134
    uint8_t _pad2[0x180 - 0x138];
    float zFar;                // +0x180
    cplane_s frustum[4];       // +0x184
    uint8_t _pad3[0x1E0 - 0x1D4];
};
static_assert(sizeof(viewParms_t) == 0x1E0, "viewParms_t size mismatch");

// trDebug views (render.o data at tr+0x314)
struct trDebugString_t {
    float xyz[3];       // +0x00
    float color[4];     // +0x0C
    float scale;        // +0x1C
    char szText[96];    // +0x20
    int twoD;           // +0x80
};
struct trDebugLine_t {
    float start[3];   // +0x00
    float end[3];     // +0x0C
    float color[4];   // +0x18
    int depthTest;    // +0x28
};
struct trDebug_t {
    uint8_t _pad0[0x18];
    int numStrings;          // +0x18
    trDebugString_t* strings;// +0x1C
    uint8_t _pad1[0x28 - 0x20];
    int numExternStrings;    // +0x28
    trDebugString_t* externStrings;  // +0x2C
    uint8_t _pad2[0x34 - 0x30];
    int numLines;            // +0x34
    trDebugLine_t* lines;    // +0x38
    int numExternLines;      // +0x3C
    trDebugLine_t* externLines;  // +0x40
};

struct trGlobals_t {
    uint8_t _pad0[0x10];
    viewParms_t viewParms;     // +0x10
    uint8_t _pad1[0x290 - 0x1F0];
    struct world_t* world;     // +0x290
    uint8_t _pad2[0x314 - 0x294];
    trDebug_t debug;           // +0x314
};
extern trGlobals_t tr;         // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

void R_SetupFrustum()
{
    float xc;
    float xs;
    FastSinCos(tr.viewParms.fovX * 0.0087266462f, &xs, &xc);
    tr.viewParms.frustum[0].normal[0] =
        (tr.viewParms.orr.axis[1][0] * xc) + (tr.viewParms.orr.axis[0][0] * xs);
    tr.viewParms.frustum[0].normal[1] =
        (tr.viewParms.orr.axis[1][1] * xc) + (tr.viewParms.orr.axis[0][1] * xs);
    tr.viewParms.frustum[0].normal[2] =
        (tr.viewParms.orr.axis[1][2] * xc) + (tr.viewParms.orr.axis[0][2] * xs);
    tr.viewParms.frustum[1].normal[0] =
        ((0.0f - xc) * tr.viewParms.orr.axis[1][0]) + (tr.viewParms.orr.axis[0][0] * xs);
    tr.viewParms.frustum[1].normal[2] =
        ((0.0f - xc) * tr.viewParms.orr.axis[1][2]) + (tr.viewParms.orr.axis[0][2] * xs);
    tr.viewParms.frustum[1].normal[1] =
        ((0.0f - xc) * tr.viewParms.orr.axis[1][1]) + (tr.viewParms.orr.axis[0][1] * xs);
    FastSinCos(tr.viewParms.fovY * 0.0087266462f, &xs, &xc);
    tr.viewParms.frustum[2].normal[0] =
        (tr.viewParms.orr.axis[2][0] * xc) + (tr.viewParms.orr.axis[0][0] * xs);
    tr.viewParms.frustum[2].normal[1] =
        (tr.viewParms.orr.axis[2][1] * xc) + (tr.viewParms.orr.axis[0][1] * xs);
    tr.viewParms.frustum[2].normal[2] =
        (tr.viewParms.orr.axis[2][2] * xc) + (tr.viewParms.orr.axis[0][2] * xs);
    tr.viewParms.frustum[3].normal[0] =
        ((0.0f - xc) * tr.viewParms.orr.axis[2][0]) + (tr.viewParms.orr.axis[0][0] * xs);
    tr.viewParms.frustum[3].normal[1] =
        ((0.0f - xc) * tr.viewParms.orr.axis[2][1]) + (tr.viewParms.orr.axis[0][1] * xs);
    tr.viewParms.frustum[3].normal[2] =
        ((0.0f - xc) * tr.viewParms.orr.axis[2][2]) + (tr.viewParms.orr.axis[0][2] * xs);

    for (int i = 0; i < 4; ++i)
    {
        tr.viewParms.frustum[i].type = 3;
        tr.viewParms.frustum[i].dist =
            (tr.viewParms.frustum[i].normal[0] * tr.viewParms.orr.origin[0])
          + (tr.viewParms.frustum[i].normal[1] * tr.viewParms.orr.origin[1])
          + (tr.viewParms.frustum[i].normal[2] * tr.viewParms.orr.origin[2]);
        SetPlaneSignbits(&tr.viewParms.frustum[i]);
    }
}

// ============================================================================
// RB_DrawDebugStrings - ea: 0x006C4EA0
// ============================================================================
void RB_DrawDebugStrings(trDebugString_t* strings, int numStrings)
{
    if (numStrings > 0)
    {
        float* color = strings->color;
        for (int i = numStrings; i != 0; --i)
        {
            if (*(color + 29) != 0)
            {
                int y = *(int*)(color - 2);
                float size = color[4];
                int x = *(int*)(color - 3);
                Color col;
                col.r = color[0];
                col.g = color[1];
                col.b = color[2];
                col.a = color[3];
                DebugRender::RenderText((const char*)(color + 20), x, y, col, 0.0f, size);
            }
            else
            {
                float size = color[4];
                Color col;
                col.r = color[0];
                col.g = color[1];
                col.b = color[2];
                col.a = color[3];
                math::Position3 wpos;
                wpos.v.m128_f32[0] = *(color - 3);
                wpos.v.m128_f32[1] = *(color - 2);
                wpos.v.m128_f32[2] = *(color - 1);
                DebugRender::RenderText3D(wpos, col, size, "%s", color + 20);
            }
            color += 33;
        }
    }
}

// ============================================================================
// RB_DrawDebugLines / RB_DrawDebug - ea: 0x006D9A30 / 0x006D9B20
// ============================================================================
void RB_DrawDebugLines(trDebugLine_t* lines, int numLines)
{
    if (numLines > 0)
    {
        trDebugLine_t* p = lines;
        for (int i = numLines; i != 0; --i)
        {
            math::Position3 pt1;
            pt1.v = _mm_setr_ps(p->start[0], p->start[1], p->start[2], 0.0f);
            math::Position3 pt2;
            pt2.v = _mm_setr_ps(p->end[0], p->end[1], p->end[2], 0.0f);
            Color col(p->color[0], p->color[1], p->color[2], p->color[3]);
            DebugRender::RenderLine(pt1, pt2, col, 0.25f);
            ++p;
        }
    }
}

void RB_DrawDebug()
{
    RB_DrawDebugLines(tr.debug.lines, tr.debug.numLines);
    RB_DrawDebugLines(tr.debug.externLines, tr.debug.numExternLines);
    tr.debug.numLines = 0;
    RB_DrawDebugStrings(tr.debug.strings, tr.debug.numStrings);
    RB_DrawDebugStrings(tr.debug.externStrings, tr.debug.numExternStrings);
    tr.debug.numStrings = 0;
}

// ============================================================================
// R_RenderGlow - ea: 0x006C68B0
// ============================================================================
float gGlowZFactor;  // ?gGlowZFactor@@3MA @ 0x11EA63C
void R_SetWindowQuadRect(nglQuad& q);  // tr_fx.cpp

void R_RenderGlow()
{
    if ((ShaderCommon::ShaderSwitching.as_u32 & 1) == 0)
    {
        nglQuad q;
        nglQuad Quad;
        nglListBeginScene(NGLSCENE_PARENT);
        nglSetClearFlags(0);
        nglSetFBWriteMask(0x1000000u);
        nglSetZWriteEnable(false);
        nglSetZTestEnable(false);
        nglInitQuad(&q);
        R_SetWindowQuadRect(q);
        nglSetQuadColor(&q, 0);
        nglSetQuadBlend(&q, 0);
        nglListAddQuad(&q);
        nglListEndScene();
        nglListBeginScene(NGLSCENE_PARENT);
        nglSetClearFlags(0);
        nglSetFBWriteMask(0x1000000u);
        nglSetZWriteEnable(false);
        nglSetZTestEnable(true);
        nglInitQuad(&Quad);
        R_SetWindowQuadRect(Quad);
        nglSetQuadZ(&Quad, gGlowZFactor * tr.viewParms.zFar);
        nglSetQuadColor(&Quad, 0x80000000u);
        nglSetQuadBlend(&Quad, 0);
        nglListAddQuad(&Quad);
        nglListEndScene();
        ShaderCommon::GlowRender();
    }
}

// ============================================================================
// R_RenderSky - ea: 0x006C69C0
// ============================================================================
struct nglMesh;
class nglMeshParams;
class nglShaderParamSet;
struct nglMeshNode;
nglMeshNode* nglListAddMesh(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                            nglMeshParams* MeshParams, nglShaderParamSet* ShaderParams,
                            void (*fn)(nglMeshNode*));
void R_RenderSky()
{
    struct world_t {
        uint8_t _pad[0x108];
        nglMesh* mSky;             // +0x108
    };
    nglListBeginScene(NGLSCENE_PARENT);
    nglSetZTestEnable(false);
    nglSetClearFlags(0);
    nglSetZWriteEnable(false);

    math::Mat43 m;
    m.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    m.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    m.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    world_t* world = (world_t*)tr.world;
    m.w.v.m128_f32[0] = tr.viewParms.orr.origin[0];
    m.w.v.m128_f32[1] = tr.viewParms.orr.origin[1];
    m.w.v.m128_f32[2] = tr.viewParms.orr.origin[2];
    m.w.v.m128_f32[3] = 0.0f;

    nglListAddMesh(world->mSky, m, nullptr, nullptr, nullptr);
    nglSortScene(nglBuildScene);
    nglListEndScene();
}
