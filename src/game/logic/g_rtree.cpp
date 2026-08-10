// ============================================================================
// g_rtree.cpp - physics.o rtree traversal (rtree.cpp / rtree_client.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <string.h>

// rtree_node_t - SIMD tree node (16 bytes) - verified against IDA
struct rtree_node_t {
    int16_t minx;  // +0x00
    int16_t maxx;  // +0x02
    int16_t miny;  // +0x04
    int16_t maxy;  // +0x06
    int16_t minz;  // +0x08
    int16_t maxz;  // +0x0A
    int     offs;  // +0x0C
};

// rtree_root_t - SIMD tree root (verified from traverse_rtree disasm)
struct rtree_root_t {
    math::Position3 region_center;        // +0x00
    math::Position3 region_halfsize_inv32k;  // +0x10
    rtree_node_t*   simd_tree;            // +0x20
    int             nsimd_levels;         // +0x24
    int             top_level_aabb_count; // +0x28
};

// subdivision_visitor is the rtree_visitor_t base (defined in g_cm_load.cpp);
// the overlap helpers below only need the rtree_node_t layout.

// ea: 0x006F6570
bool rtree_node_overlap_test(const rtree_node_t* line,
                             unsigned int node_offset,
                             const rtree_node_t* simd_tree)
{
    const rtree_node_t* node = &simd_tree[node_offset];
    if (line->maxx > node->maxx || line->minx > node->minx
        || line->maxy > node->maxy || line->miny > node->miny
        || line->maxz > node->maxz || line->minz > node->minz)
        return false;
    if (node->offs == -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\rtree.cpp";
        AeAssert::gCurrentLine = 51;
        AeAssert::gCurrentExpr = "rect.offs != -1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return true;
}

// ea: 0x006F6AD0
bool rtree_query_size_error_handler(const math::Position3& p0,
                                    const math::Position3& p1)
{
    __m128 v2 = _mm_sub_ps(p1.v, p0.v);
    __m128 v3 = _mm_shuffle_ps(v2, _mm_shuffle_ps(_mm_setzero_ps(), v2, 0xF0),
                               0xC4);
    __m128 v4 = _mm_mul_ps(v3, v3);
    bool v5 = (v4.m128_f32[0]
               + (v4.m128_f32[1] + v4.m128_f32[2]))
        < 0.000099999997f;
    __m128 v6 = _mm_sub_ps(p0.v, p1.v);
    __m128 v7 = _mm_mul_ps(v6, v6);
    float v8 = v7.m128_f32[0] + (v7.m128_f32[1] + v7.m128_f32[2]);
    if (v5)
    {
        if (v8 <= 1000000.0f)
            return true;
    }
    else if (v8 < 16978100.0f)
    {
        return true;
    }
    AeAssert::gCurrentAuthor = AeAssert::JSV;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\rtree_client.cpp";
    AeAssert::gCurrentLine = 15;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert(
            "rtree query size is too large. Please inspect the callstack and fix/reengineer the calling code. Probably line "
            "check needs to be clamped or split into shorter line checks and distributed over multiple frames. Thank you."))
        __debugbreak();
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\rtree_client.cpp";
    AeAssert::gCurrentLine = 20;
    AeAssert::gCurrentExpr =
        "\"rtree query size is too large. Please inspect the callstack and fix/reengineer the calling code. Probably line check needs to be clamped or split into shorter line checks and distributed over multiple frames. Thank you.\" && 0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
        __debugbreak();
    return false;
}

// traverse_rtree (0x6F6620) is deferred: it walks the multi-level SIMD node
// stack with a level-tracking visitor (per-4-child offsets), which needs the
// full rtree.cpp traversal semantics. Revisit after the surrounding
// rtree/visitor infra is verified.
