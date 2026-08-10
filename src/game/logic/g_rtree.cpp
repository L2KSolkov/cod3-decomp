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
    int             pad_24;               // +0x24 (unknown; TODO)
    int             top_level_aabb_count; // +0x28
    int             nsimd_levels;         // +0x2C
};

// subdivision_visitor is the rtree_visitor_t base (defined in g_cm_load.cpp);
// the overlap helpers below only need the rtree_node_t layout.

// ea: 0x006F6570
bool rtree_node_overlap_test(const rtree_node_t* line,
                             unsigned int node_offset,
                             const rtree_node_t* simd_tree)
{
    // node_offset is a byte offset into the SIMD tree (disasm: [eax+ecx+2]).
    const rtree_node_t* node =
        (const rtree_node_t*)((const char*)simd_tree + node_offset);
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

// ============================================================================
// traverse_rtree - ea: 0x6F6620 (rtree.cpp)
// ============================================================================

// f32766 - {32767,32767,32767,0} clamp constant (game.o @ 0xF916A0)
static __m128 f32766;
static bool f32766_inited;  // $S70_1 guard flag (game.o @ 0xF916B4)

// Position3 -> vec4 (x,y,z,1) - shuffle sequence from the traverse_rtree disasm
static inline __m128 expand_pos4(const math::Position3& p)
{
    return _mm_shuffle_ps(p.v, _mm_shuffle_ps(_mm_set_ss(1.0f), p.v, 0xA0),
                          0x34);
}

// subdivision_visitor - vftable slot 0 is visit(int) (rtree_visitor_t::visit).
struct subdivision_visitor {
    virtual void visit(int cluster_offset);
};

struct simd_stack_entry {
    unsigned int offset;  // +0x00
    unsigned int level;   // +0x04
};

extern void absolutely_fatal_irrecoverable_error_infinite_loop();  // physics.o

// ea: 0x006F6620
void traverse_rtree(const math::Position3& p0, const math::Position3& p1,
                    const rtree_root_t& root, subdivision_visitor& visitor)
{
    if (!rtree_query_size_error_handler(p0, p1))
        return;

    __m128 reg_center = root.region_center.v;
    __m128 reg_half_inv32k = root.region_halfsize_inv32k.v;

    if (!f32766_inited)
    {
        f32766 = _mm_set_ps(0.0f, 32767.0f, 32767.0f, 32767.0f);
        f32766_inited = true;
    }
    __m128 v = f32766;

    // Clamp both endpoints into the region; reg_min/reg_max is the AABB.
    __m128 neg_v = _mm_xor_ps(_mm_set1_ps(-0.0f), v);  // Float4_SignMask_12
    __m128 t0 = _mm_mul_ps(_mm_sub_ps(expand_pos4(p0), reg_center),
                           reg_half_inv32k);
    t0 = _mm_min_ps(_mm_max_ps(t0, neg_v), v);
    __m128 t1 = _mm_mul_ps(_mm_sub_ps(expand_pos4(p1), reg_center),
                           reg_half_inv32k);
    t1 = _mm_min_ps(_mm_max_ps(t1, neg_v), v);
    __m128 reg_min = _mm_min_ps(t0, t1);
    __m128 reg_max = _mm_max_ps(t0, t1);

    rtree_node_t line;
    line.maxx = (int16_t)(int)reg_min.m128_f32[0];
    line.minx = (int16_t)(int)(-reg_max.m128_f32[0]);
    line.maxy = (int16_t)(int)reg_min.m128_f32[1];
    line.miny = (int16_t)(int)(-reg_max.m128_f32[1]);
    line.maxz = (int16_t)(int)reg_min.m128_f32[2];
    line.minz = (int16_t)(int)(-reg_max.m128_f32[2]);

    unsigned int stack_guard[2];  // below the stack array (underflow sentinel)
    simd_stack_entry stack_array[257];
    simd_stack_entry* p_level = &stack_array[256];
    stack_guard[0] = 0xBEEFBEEF;  // [ebp-0x868]
    stack_guard[1] = 0xDEADDEAD;  // [ebp-0x864]

    rtree_node_t* tree0 = root.simd_tree;
    int top_level_aabb_count = root.top_level_aabb_count;
    if (top_level_aabb_count != 0)
    {
        int v21 = 16 * (top_level_aabb_count - 1);
        int count = top_level_aabb_count;
        do
        {
            --p_level;
            p_level->offset = (unsigned int)v21;
            p_level->level = 1;
            if (p_level < &stack_array[0])
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\rtree.cpp";
                AeAssert::gCurrentLine = 138;
                AeAssert::gCurrentExpr = "stack_cur1 >= stack_bottom";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            v21 -= 16;
        } while (--count != 0);
    }

    int nsimd_levels = root.nsimd_levels;
    while (p_level <= &stack_array[255])
    {
        unsigned int node_offset = p_level->offset;
        if (rtree_node_overlap_test(&line, node_offset, tree0))
        {
            int cluster_offset =
                ((const rtree_node_t*)((const char*)tree0 + node_offset))->offs;
            if (cluster_offset == -1)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\rtree.cpp";
                AeAssert::gCurrentLine = 238;
                AeAssert::gCurrentExpr = "cluster_offset != -1";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            unsigned int level = p_level->level;
            if (level < (unsigned int)nsimd_levels)
            {
                ++p_level;
                unsigned int v34 = level + 1;
                unsigned int cur = (unsigned int)cluster_offset;
                int i = 0;
                do
                {
                    --p_level;
                    if (p_level <= &stack_array[0])
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\rtree.cpp";
                        AeAssert::gCurrentLine = 247;
                        AeAssert::gCurrentExpr = "stack_cur > stack_bottom";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                    p_level->level = v34;
                    if (cluster_offset + i == -1)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\rtree.cpp";
                        AeAssert::gCurrentLine = 249;
                        AeAssert::gCurrentExpr =
                            "cluster_offset + i < 0xFFFFFFFF";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                    p_level->offset = cur;
                    ++i;
                    cur += 16;
                } while (i < 4);
                continue;
            }
            if (cluster_offset == -1)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\rtree.cpp";
                AeAssert::gCurrentLine = 255;
                AeAssert::gCurrentExpr = "cluster_offset != -1";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            visitor.visit(cluster_offset);
        }
        ++p_level;
    }

    if (stack_guard[1] != 0xDEADDEAD || stack_guard[0] != 0xBEEFBEEF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\rtree.cpp";
        AeAssert::gCurrentLine = 265;
        AeAssert::gCurrentExpr =
            "\"REALLY FATAL UNSKIPPABLE ERROR: RTREE STACK UNDERFLOW. "
            "EXECUTION CANNOT BE CONTINUED. CHECK QUERY SOURCE AND RADIUS.\" "
            "&& 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        absolutely_fatal_irrecoverable_error_infinite_loop();
    }
}
