// ============================================================================
// tr_fx6.cpp - render.o rain-drop update (FX.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "game/logic/g_local.h"
#include "game/trace_types.h"

#include <math.h>

// render.o rain-drop state (@ 0xF78600 / 0xF784C0 / 0xF784B0 / 0xF784B4 / 0xF78734)
static math::Position3 buffer_pos[16];
static math::Dir3 buffer_normal[16];
static unsigned char bufferHead;
static unsigned char bufferTail;
static unsigned int s_rainInit;  // $S69_2

extern bool gEnableRainDrops;  // ?gEnableRainDrops@@3_NA (tr_fx2.cpp)
extern int currCl;             // ?currCl@@3HA @ 0xF1579C
extern int _rand();            // ?rand@@YAHXZ (CRT)

// Entity view (r +0xE0, mHandle +0x234)
struct EntitySharedView {
    uint8_t _pad[0x70];
    math::Position3 currentOrigin;  // +0x70
};
struct EntityView {
    uint8_t _pad[0xE0];
    EntitySharedView r;            // +0xE0
    uint8_t _pad2[0x234 - 0x230];
    unsigned int mHandle;          // +0x234
};

// PostEffectEventScriptCall (sret Handle; game.o)
struct BrocVec {
    float x;
    float y;
    float z;
};
Handle PostEffectEventScriptCall(const Entity* ent, const char* scriptId,
                                 const BrocVec& pos, const BrocVec& facing,
                                 bool queue, TPakId pakid,
                                 bool important);  // ?PostEffectEventScriptCall@@YA?AVHandle@@PBVEntity@@PBDABUvector@Broc@@2_NW4TPakId@@_N@Z

// ============================================================================
// FX_UpdateRainDrops - ea: 0x006C7C70
// ============================================================================
void FX_UpdateRainDrops(float timeDelta)
{
    if (!gEnableRainDrops || timeDelta == 0.0f)
        return;

    EntityView* Player = (EntityView*)EntityManager::sInst->GetPlayer(currCl);
    if (Player == nullptr)
        return;

    if ((s_rainInit & 1) == 0)
    {
        s_rainInit |= 1u;
        for (int i = 0; i < 16; ++i)
            buffer_pos[i] = math::Position3();
    }
    if ((s_rainInit & 2) == 0)
    {
        s_rainInit |= 2u;
        for (int i = 0; i < 16; ++i)
            buffer_normal[i] = math::Dir3();
    }

    math::Position3 origin;
    origin.v = Player->r.currentOrigin.v;
    __m128 dir = _mm_set1_ps(300.0f);

    __m128 offset;
    float lenSq;
    do
    {
        float rx = ((float)_rand() * 0.000030517578f) * 2.0f - 1.0f;
        float ry = ((float)_rand() * 0.000030517578f) * 2.0f - 1.0f;
        float rz = ((float)_rand() * 0.000061035156f) - 1.0f;
        offset = _mm_setr_ps(rx, ry, rz, 0.0f);
        __m128 v7 = _mm_mul_ps(offset, dir);
        __m128 v8 = _mm_mul_ps(v7, v7);
        lenSq = v8.m128_f32[0]
              + (_mm_shuffle_ps(v8, v8, 85).m128_f32[0]
                 + _mm_shuffle_ps(v8, v8, 170).m128_f32[0]);
    } while (lenSq < 10000.0f);

    math::Position3 start = origin;
    start.v = _mm_add_ps(start.v, offset);

    math::Position3 mins = _mm_setzero_ps();
    math::Position3 maxs = _mm_setzero_ps();
    math::Position3 end;
    end.v = start.v;
    end.v.m128_f32[2] += 200.0f;
    float endZ = start.v.m128_f32[2] - 200.0f;
    math::Position3 endPts;
    endPts.v = _mm_setr_ps(start.v.m128_f32[0], start.v.m128_f32[1],
                           endZ, start.v.m128_f32[3]);

    trace_t trace;
    trace.surfaceFlags = 0;
    trace.contents = 0;
    trace.endpos = math::Position3();

    collision_context_t context;
    context.pass_entity1.mHandle.mVal = 0;
    context.pass_entity2.mHandle.mVal = Player->mHandle;

    g_Trace(&trace, start, mins, maxs, endPts, context);

    if (trace.normal.v.m128_f32[1] >= 1.0f)
    {
        // upward normal: skip (drops on ceiling)
    }
    else
    {
        unsigned char v9 = bufferTail;
        unsigned int v10 = bufferTail;
        math::Position3* v11 = &buffer_pos[bufferTail];
        v11->v.m128_f32[0] = trace.endpos.v.m128_f32[0];
        v11->v.m128_f32[1] = trace.endpos.v.m128_f32[1];
        v11->v.m128_f32[2] = trace.endpos.v.m128_f32[2];
        v11->v.m128_f32[3] = trace.endpos.v.m128_f32[3];
        math::Dir3* v12 = &buffer_normal[v10];
        v12->v.m128_f32[0] = trace.normal.v.m128_f32[0];
        v12->v.m128_f32[1] = trace.normal.v.m128_f32[1];
        v12->v.m128_f32[2] = trace.normal.v.m128_f32[2];
        v12->v.m128_f32[3] = trace.fraction;
        bufferTail = (v9 + 1) & 0xF;
        if (bufferHead == bufferTail)
            bufferHead = (bufferHead + 1) & 0xF;
    }

    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 1065353216;

    int count = (bufferTail - bufferHead) & 0xF;
    if (count != 0 && (timeDelta * 360.0f) != 0.0f)
    {
        __m128 move = _mm_set1_ps(20.0f);
        int n = (int)(timeDelta * 360.0f);
        do
        {
            int idx = (bufferHead + ((_rand() >> 8) % count)) & 0xF;
            math::Position3 pos = buffer_pos[idx];
            float rx = ((float)_rand() * 0.000030517578f) * 2.0f - 1.0f;
            float ry = ((float)_rand() * 0.000030517578f) * 2.0f - 1.0f;
            float rz = ((float)_rand() * 0.000061035156f) - 1.0f;
            __m128 offset2 = _mm_setr_ps(rx, ry, rz, 0.0f);
            __m128 norm = buffer_normal[idx].v;
            __m128 v22 = _mm_mul_ps(offset2, norm);
            float d = v22.m128_f32[0]
                    + (_mm_shuffle_ps(v22, v22, 85).m128_f32[0]
                       + _mm_shuffle_ps(v22, v22, 170).m128_f32[0]);
            pos.v = _mm_add_ps(
                pos.v,
                _mm_mul_ps(
                    _mm_sub_ps(offset2, _mm_mul_ps(norm, _mm_set1_ps(d))),
                    move));

            BrocVec facing = { 0.0f, 0.0f, 1.0f };
            BrocVec bpos = { pos.v.m128_f32[0], pos.v.m128_f32[1],
                             pos.v.m128_f32[2] };
            PostEffectEventScriptCall((Entity*)Player, "rain_splash_md",
                                      bpos, facing, false,
                                      (TPakId)-1, false);
            --n;
        } while (n != 0);
    }
}
