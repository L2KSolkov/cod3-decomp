// ============================================================================
// tr_bsp4.cpp - render.o static-model BSP filter walk (tr_staticmodel.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"

#include <stdint.h>

// AeAssert (game.o)
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

class StaticModel;
struct BspPlane {
    math::Vector4 mPlane;      // +0x00
};
class BspNode {
public:
    short contents;            // +0x00
    short cellNum;             // +0x02
    union {
        struct {
            BspNode* children[2];  // +0x04
            BspPlane* plane;       // +0x0C
        } node;
    } u;                       // +0x04
};

class BspTree {
public:
    uint8_t _pad[0x18];
    unsigned int mCellsSize;   // +0x18
    void* mCellsList;          // +0x1C
};

struct world_t {
    uint8_t _pad[0x100];
    BspTree* bspTree;          // +0x100
};

void R_AddModelToCell(world_t* world, StaticModel* psm, int cellNum);  // tr_staticmodel.cpp

// ============================================================================
// R_FilterModelIntoCells_r - ea: 0x006C6DC0
// ============================================================================
void R_FilterModelIntoCells_r(world_t* world, BspNode* node, StaticModel* psm,
                              const math::Position3& mins,
                              const math::Position3& maxs)
{
    BspNode* v5 = node;
    if (node->cellNum == -2)
    {
        __m128 zero = _mm_setzero_ps();
        do
        {
            __m128 v = v5->u.node.plane->mPlane.v;
            float planeZ = _mm_shuffle_ps(v, v, 170).m128_f32[0];
            float planeY = _mm_shuffle_ps(v, v, 85).m128_f32[0];
            float planeX = v.m128_f32[0];
            float planeW = _mm_shuffle_ps(v, v, 255).m128_f32[0];
            int side = 0;

            // corner A (max-dot): plane[i] <= 0 ? maxs[i] : mins[i]
            float aX = planeX <= 0.0f ? maxs.v.m128_f32[0] : mins.v.m128_f32[0];
            float aY = planeY <= 0.0f ? maxs.v.m128_f32[1] : mins.v.m128_f32[1];
            float aZ = planeZ <= 0.0f ? maxs.v.m128_f32[2] : mins.v.m128_f32[2];
            // corner B (min-dot): plane[i] >= 0 ? maxs[i] : mins[i]
            float bX = planeX >= 0.0f ? maxs.v.m128_f32[0] : mins.v.m128_f32[0];
            float bY = planeY >= 0.0f ? maxs.v.m128_f32[1] : mins.v.m128_f32[1];
            float bZ = planeZ >= 0.0f ? maxs.v.m128_f32[2] : mins.v.m128_f32[2];

            float minDot = (bX * planeX) + (bY * planeY) + (bZ * planeZ);
            float maxDot = (aX * planeX) + (aY * planeY) + (aZ * planeZ);
            if (minDot >= planeW)
                side = 1;
            if (planeW > maxDot)
                side |= 2;

            if (side == 3)
            {
                R_FilterModelIntoCells_r(world, v5->u.node.children[0], psm,
                                         mins, maxs);
                v5 = v5->u.node.children[1];
            }
            else
            {
                if (side != 1 && side != 2)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_staticmodel.cpp";
                    AeAssert::gCurrentLine = 78;
                    AeAssert::gCurrentExpr = "iSide == 1 || iSide == 2";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                v5 = v5->u.node.children[side - 1];
            }
        } while (v5->cellNum == -2);
    }
    short cellNum = v5->cellNum;
    if (cellNum >= 0)
        R_AddModelToCell(world, psm, cellNum);
}
