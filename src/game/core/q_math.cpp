// ============================================================================
// q_math.cpp - idTech math + Treyarch math helpers (core.o com_math.cpp)
// Reconstructed from IDA release decompiles (ea comments below).
// ============================================================================

#include "game/core/core_types.h"
#include "game/core/core_systems.h"
#include "game/core/core_globals.h"

// ?bytedirs@@3PAY02MA (core.o @ 0x11C7790) - 162 precomputed byte->dir vectors
static float s_bytedirs[162][3];
float (*bytedirs)[3] = s_bytedirs;  // filled at runtime by table init

#include <math.h>
#include <string.h>

// ============================================================================
// Externs (core.o data / libc)
// ============================================================================
extern int holdrand;

// FastSinCos shared scratch (core.o data at 0xF03200-0xF03214)
extern float sr;
extern float cr;
extern float psin;
extern float cp;
extern float sy;
extern float cy;
extern void FastSinCos(float radians, float* psin, float* pcos);
extern void Com_Error(int code, const char* fmt, ...);

// Forward declarations (mutually recursive angle/axis helpers)
float vectoyaw(float* vec);
float vectosignedyaw(float* vec);
float vectopitch(const float* vec);
float vectosignedpitch(const float* vec);
void vectoangles(float* vec, float* angles);
void vectosignedangles(float* vec, float* angles);

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

// ============================================================================
// Random / scalar helpers
// ============================================================================

// ea: 0x004B5FE0
int Q_rand(int* seed)
{
    int result = 69069 * *seed + 1;
    *seed = result;
    return result;
}

// ea: 0x004B6000
float Q_random(int* seed)
{
    int v1 = 69069 * *seed + 1;
    *seed = v1;
    float seeda = (float)(v1 & 0xFFFF);
    return seeda / 65536.0f;
}

// ea: 0x004B6030
float Q_crandom(int* seed)
{
    int v1 = 69069 * *seed + 1;
    *seed = v1;
    float seeda = (float)(v1 & 0xFFFF);
    return 2.0f * (seeda / 65536.0f - 0.5f);
}

// ea: 0x004B6070
int Q_log2(int val)
{
    int v1 = val >> 1;
    int result = 0;
    for (; v1 != 0; ++result)
        v1 >>= 1;
    return result;
}

// ea: 0x004B6090
float Q_acos(float c)
{
    float angle = acosf(c);
    if (angle <= 3.1415927f && angle >= -3.1415927f)
        return angle;
    return 3.1415927f;
}

// ea: 0x004B60D0
char ClampChar(int i)
{
    char result = (char)i;
    if (i < -128)
        return (char)0x80;
    if (i > 127)
        return 127;
    return result;
}

// ea: 0x004B60F0
short ClampShort(int i)
{
    short result = (short)i;
    if (i < -32768)
        return (short)0x8000;
    if (i > 0x7FFF)
        return 0x7FFF;
    return result;
}

// ea: 0x004B6110
unsigned char DirToByte(const float* dir)
{
    unsigned char result = 0;
    if (dir != nullptr)
    {
        float v2 = 0.0f;
        unsigned char v3 = 0;
        do
        {
            float dot = bytedirs[v3][0] * *dir
                      + bytedirs[v3][1] * dir[1]
                      + bytedirs[v3][2] * dir[2];
            if (dot > v2)
            {
                v2 = dot;
                result = v3;
            }
            ++v3;
        }
        while (v3 < 0xA2u);
    }
    return result;
}

// ea: 0x004B6170
void ByteToDir(unsigned int b, float* dir)
{
    if (b > 0xA1)
    {
        *dir = 0.0f;
        dir[1] = 0.0f;
        dir[2] = 0.0f;
    }
    else
    {
        *dir = bytedirs[b][0];
        dir[1] = bytedirs[b][1];
        dir[2] = bytedirs[b][2];
    }
}

// ============================================================================
// Vector helpers
// ============================================================================

// ea: 0x004B61C0
float _DotProduct(const float* v1, const float* v2)
{
    return v1[2] * v2[2] + v1[1] * v2[1] + *v1 * *v2;
}

// ea: 0x004B61E0
void _VectorSubtract(const float* veca, const float* vecb, float* out)
{
    *out = *veca - *vecb;
    out[1] = veca[1] - vecb[1];
    out[2] = veca[2] - vecb[2];
}

// ea: 0x004B6220
void _VectorAdd(const float* veca, const float* vecb, float* out)
{
    *out = *veca + *vecb;
    out[1] = veca[1] + vecb[1];
    out[2] = veca[2] + vecb[2];
}

// ea: 0x004B6260
void _VectorCopy(const float* in, float* out)
{
    *out = *in;
    out[1] = in[1];
    out[2] = in[2];
}

// ea: 0x004B6280
void _VectorScale(const float* in, float scale, float* out)
{
    *out = *in * scale;
    out[1] = in[1] * scale;
    out[2] = in[2] * scale;
}

// ea: 0x004B62C0
void _VectorMA(const float* veca, float scale, const float* vecb, float* vecc)
{
    *vecc = *vecb * scale + *veca;
    vecc[1] = vecb[1] * scale + veca[1];
    vecc[2] = vecb[2] * scale + veca[2];
}

// ea: 0x004B6310
int VectorCompareEpsilon(const float* v1, float* v2)
{
    float* eax1 = v2;
    int v3 = 0;
    while ((eax1[v1 - v2] - *eax1) * (eax1[v1 - v2] - *eax1) <= 0.000024999999f)
    {
        ++v3;
        ++eax1;
        if (v3 >= 3)
            return 1;
    }
    return 0;
}

// ea: 0x004B6360
float _VectorLength(const float* v)
{
    return sqrtf(*v * *v + v[1] * v[1] + v[2] * v[2]);
}

// ea: 0x004B63B0
float VectorDistance(const float* v1, const float* v2)
{
    float dir = *v2 - *v1;
    float v4 = v2[1] - v1[1];
    float v5 = v2[2] - v1[2];
    return sqrtf(v5 * v5 + v4 * v4 + dir * dir);
}

// ea: 0x004B6400
float VectorDistanceSquared(const float* p1, const float* p2)
{
    float v = *p2 - *p1;
    float v4 = p2[1] - p1[1];
    float v5 = p2[2] - p1[2];
    return v5 * v5 + v4 * v4 + v * v;
}

// ea: 0x004B6440
float VectorDistance2D(const float* v1, const float* v2)
{
    float dir = *v2 - *v1;
    float dir_4 = v2[1] - v1[1];
    return sqrtf(dir_4 * dir_4 + dir * dir);
}

// ea: 0x004B6480
float VectorDistanceSquared2D(const float* p1, const float* p2)
{
    float v = *p2 - *p1;
    float v4 = p2[1] - p1[1];
    return v4 * v4 + v * v;
}

// ea: 0x004B64B0
float VectorDistanceSquared2D(const math::Position3* p1, const math::Position3* p2)
{
    float v = p2->v.m128_f32[0] - p1->v.m128_f32[0];
    float v4 = p2->v.m128_f32[1] - p1->v.m128_f32[1];
    return v4 * v4 + v * v;
}

// ea: 0x004B64E0
void CrossProduct(const float* v1, const float* v2, float* cross)
{
    *cross = v2[2] * v1[1] - v1[2] * v2[1];
    cross[1] = v1[2] * *v2 - *v1 * v2[2];
    cross[2] = *v1 * v2[1] - *v2 * v1[1];
}

// ea: 0x004B6540
void CrossProductUp(const float* v1, float* cross)
{
    *cross = v1[1];
    cross[1] = 0.0f - *v1;
    cross[2] = 0.0f;
}

// ea: 0x004B6570
float VectorNormalize2(const math::Dir3* in, math::Dir3* out)
{
    float v6 = in->v.m128_f32[0] * in->v.m128_f32[0]
             + in->v.m128_f32[1] * in->v.m128_f32[1]
             + in->v.m128_f32[2] * in->v.m128_f32[2];
    if (v6 == 0.0f)
    {
        out->v.m128_f32[0] = 0.0f;
        out->v.m128_f32[1] = 0.0f;
        out->v.m128_f32[2] = 0.0f;
        return 0.0f;
    }
    float v7 = sqrtf(v6);
    float inv = 1.0f / v7;
    out->v.m128_f32[0] = in->v.m128_f32[0] * inv;
    out->v.m128_f32[1] = in->v.m128_f32[1] * inv;
    out->v.m128_f32[2] = in->v.m128_f32[2] * inv;
    return v7;
}

// ea: 0x004B6620
float VectorNormalize(math::Dir3* v)
{
    float v4 = sqrtf(v->v.m128_f32[0] * v->v.m128_f32[0]
                   + v->v.m128_f32[1] * v->v.m128_f32[1]
                   + v->v.m128_f32[2] * v->v.m128_f32[2]);
    if (v4 != 0.0f)
    {
        v->v.m128_f32[0] /= v4;
        v->v.m128_f32[1] /= v4;
        v->v.m128_f32[2] /= v4;
    }
    return v4;
}

// ea: 0x004B66B0
float VectorNormalize(float* v)
{
    float length = sqrtf(*v * *v + v[1] * v[1] + v[2] * v[2]);
    if (length != 0.0f)
    {
        float inv = 1.0f / length;
        *v = *v * inv;
        v[1] = v[1] * inv;
        v[2] = v[2] * inv;
    }
    return length;
}

// ea: 0x004B6740
float VectorNormalize2D(float* v)
{
    float length = sqrtf(*v * *v + v[1] * v[1]);
    if (length != 0.0f)
    {
        float inv = 1.0f / length;
        v[1] = v[1] * inv;
        *v = *v * inv;
    }
    return length;
}

// ea: 0x004B67C0
float VectorNormalize4D(float* v)
{
    float length = sqrtf(*v * *v + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
    if (length != 0.0f)
    {
        float inv = 1.0f / length;
        *v = *v * inv;
        v[1] = v[1] * inv;
        v[2] = v[2] * inv;
        v[3] = v[3] * inv;
    }
    return length;
}

// ea: 0x004B6870
float VectorNormalize2(const float* v, float* out)
{
    float length = sqrtf(*v * *v + v[1] * v[1] + v[2] * v[2]);
    if (length == 0.0f)
    {
        *out = 0.0f;
        out[1] = 0.0f;
        out[2] = 0.0f;
    }
    else
    {
        float inv = 1.0f / length;
        *out = *v * inv;
        out[1] = inv * v[1];
        out[2] = inv * v[2];
    }
    return length;
}

// ea: 0x004B6920
void VectorInverse(float* v)
{
    *v = 0.0f - *v;
    float v1 = 0.0f - v[2];
    v[1] = 0.0f - v[1];
    v[2] = v1;
}

// ea: 0x004B6950
void Vector4Scale(const float* in, float scale, float* out)
{
    *out = *in * scale;
    out[1] = in[1] * scale;
    out[2] = in[2] * scale;
    out[3] = in[3] * scale;
}

// ea: 0x004B69A0
float VectorMax(float* in)
{
    float v1 = *in;
    if (*in < in[1])
        v1 = in[1];
    if (v1 < in[2])
        return in[2];
    return v1;
}

// ea: 0x004B69D0
void VectorRotate(const float* in, const float (*matrix)[3], float* out)
{
    ASSERT("in != out", "c:\\cod\\code\\game\\com_math.cpp", 721);
    *out = (*matrix)[2] * in[2] + (*matrix)[1] * in[1] + *in * (*matrix)[0];
    out[1] = (*matrix)[3] * *in + (*matrix)[5] * in[2] + (*matrix)[4] * in[1];
    out[2] = (*matrix)[6] * *in + (*matrix)[8] * in[2] + (*matrix)[7] * in[1];
}

// ea: 0x004B6AB0
void MakeNormalVectors(const float* forward, float* right, float* up)
{
    right[1] = 0.0f - *forward;
    right[2] = forward[1];
    *right = forward[2];
    float v4 = 0.0f - (*forward * *right + forward[2] * right[2] + right[1] * forward[1]);
    *right = *forward * v4 + *right;
    float v5 = *right;
    right[1] = forward[1] * v4 + right[1];
    float v6 = forward[2] * v4 + right[2];
    float v7 = right[1];
    right[2] = v6;
    float righta = sqrtf(v6 * v6 + v5 * v5 + v7 * v7);
    if (righta != 0.0f)
    {
        float inv = 1.0f / righta;
        *right = *right * inv;
        right[1] = right[1] * inv;
        right[2] = right[2] * inv;
    }
    *up = right[1] * forward[2] - right[2] * forward[1];
    up[1] = right[2] * *forward - forward[2] * *right;
    up[2] = forward[1] * *right - right[1] * *forward;
}

// ea: 0x004B6C10
void GetPerpendicularViewVector(const float* point, const float* p1,
                                const float* p2, float* up)
{
    float v5 = point[2] - p1[2];
    float v6 = point[1] - p1[1];
    float v1_8 = v5;
    float v1_4 = v6;
    float v1 = *point - *p1;
    float pointa = sqrtf(v5 * v5 + v6 * v6 + v1 * v1);
    if (pointa != 0.0f)
    {
        float inv = 1.0f / pointa;
        v1 = inv * v1;
        v1_4 = inv * v6;
        v1_8 = inv * v5;
    }
    float v7 = point[2] - p2[2];
    float v8 = point[1] - p2[1];
    float v2 = *point - *p2;
    float pointb = sqrtf(v7 * v7 + v8 * v8 + v2 * v2);
    float v9, v10, v11;
    if (pointb == 0.0f)
    {
        v9 = *point - *p2;
        v10 = point[1] - p2[1];
        v11 = point[2] - p2[2];
    }
    else
    {
        float inv = 1.0f / pointb;
        v9 = inv * v2;
        v10 = inv * (point[1] - p2[1]);
        v11 = inv * (point[2] - p2[2]);
    }
    float upa = v10 * v1 - v9 * v1_4;
    up[2] = upa;
    float p2a = v9 * v1_8 - v11 * v1;
    up[1] = p2a;
    float p1a = v11 * v1_4 - v10 * v1_8;
    *up = p1a;
    float pointc = sqrtf(upa * upa + p2a * p2a + p1a * p1a);
    if (pointc != 0.0f)
    {
        float inv = 1.0f / pointc;
        *up = p1a * inv;
        up[1] = p2a * inv;
        up[2] = upa * inv;
    }
}

// ea: 0x004B6E50
void ProjectPointOntoVector(float* point, float* vStart, float* vEnd,
                            float* vProj)
{
    float pVec = *point - *vStart;
    float pVec_4 = point[1] - vStart[1];
    float v4 = vEnd[2] - vStart[2];
    float v5 = vEnd[1] - vStart[1];
    float pVec_8 = point[2] - vStart[2];
    float vec = *vEnd - *vStart;
    float pointa = sqrtf(v4 * v4 + v5 * v5 + vec * vec);
    float v6, v7, v8;
    if (pointa == 0.0f)
    {
        v8 = vEnd[2] - vStart[2];
        v7 = vEnd[1] - vStart[1];
        v6 = *vEnd - *vStart;
    }
    else
    {
        float inv = 1.0f / pointa;
        v6 = inv * vec;
        v7 = inv * (vEnd[1] - vStart[1]);
        v8 = inv * (vEnd[2] - vStart[2]);
    }
    float v9 = v8 * pVec_8 + v7 * pVec_4 + v6 * pVec;
    *vProj = v9 * v6 + *vStart;
    vProj[1] = v9 * v7 + vStart[1];
    vProj[2] = v9 * v8 + vStart[2];
}

// ============================================================================
// Matrix helpers
// ============================================================================

// ea: 0x004B6F90
void MatrixMultiply(const float (*in1)[3], const float (*in2)[3],
                    float (*out)[3])
{
    (*out)[0] = (*in1)[0] * (*in2)[0] + (*in1)[1] * (*in2)[3] + (*in2)[6] * (*in1)[2];
    (*out)[1] = (*in1)[1] * (*in2)[4] + (*in2)[7] * (*in1)[2] + (*in2)[1] * (*in1)[0];
    (*out)[2] = (*in1)[1] * (*in2)[5] + (*in2)[8] * (*in1)[2] + (*in2)[2] * (*in1)[0];
    (*out)[3] = (*in1)[4] * (*in2)[3] + (*in1)[3] * (*in2)[0] + (*in1)[5] * (*in2)[6];
    (*out)[4] = (*in1)[5] * (*in2)[7] + (*in1)[4] * (*in2)[4] + (*in1)[3] * (*in2)[1];
    (*out)[5] = (*in1)[5] * (*in2)[8] + (*in1)[4] * (*in2)[5] + (*in1)[3] * (*in2)[2];
    (*out)[6] = (*in1)[6] * (*in2)[0] + (*in2)[6] * (*in1)[8] + (*in1)[7] * (*in2)[3];
    (*out)[7] = (*in2)[7] * (*in1)[8] + (*in2)[4] * (*in1)[7] + (*in2)[1] * (*in1)[6];
    (*out)[8] = (*in2)[8] * (*in1)[8] + (*in2)[5] * (*in1)[7] + (*in2)[2] * (*in1)[6];
}

// ea: 0x004B7120
void MatrixMultiplyEquals(const float (*in)[3], float (*out)[3])
{
    float v2 = (*in)[1] * (*out)[3] + (*in)[0] * (*out)[0] + (*out)[6] * (*in)[2];
    float v3 = (*out)[7] * (*in)[2] + (*in)[1] * (*out)[4] + (*out)[1] * (*in)[0];
    float v4 = (*out)[8] * (*in)[2] + (*in)[0] * (*out)[2] + (*in)[1] * (*out)[5];
    float v5 = (*in)[5] * (*out)[6] + (*in)[3] * (*out)[0] + (*in)[4] * (*out)[3];
    float v6 = (*in)[5] * (*out)[7] + (*in)[4] * (*out)[4] + (*in)[3] * (*out)[1];
    float v7 = (*in)[3] * (*out)[2] + (*out)[8] * (*in)[5] + (*out)[5] * (*in)[4];
    (*out)[6] = (*in)[8] * (*out)[6] + (*in)[6] * (*out)[0] + (*in)[7] * (*out)[3];
    float v8 = (*out)[8];
    (*out)[7] = (*in)[8] * (*out)[7] + (*in)[7] * (*out)[4] + (*in)[6] * (*out)[1];
    (*out)[8] = (*in)[6] * (*out)[2] + v8 * (*in)[8] + (*out)[5] * (*in)[7];
    (*out)[0] = v2;
    (*out)[1] = v3;
    (*out)[2] = v4;
    (*out)[3] = v5;
    (*out)[4] = v6;
    (*out)[5] = v7;
}

// ea: 0x004B72B0
void MatrixMultiply34(const float (*in1)[4], const float (*in2)[4],
                      float (*out)[4])
{
    (*out)[0] = (*in1)[0] * (*in2)[0] + (*in1)[1] * (*in2)[4] + (*in1)[2] * (*in2)[8];
    (*out)[1] = (*in1)[1] * (*in2)[5] + (*in2)[1] * (*in1)[0] + (*in2)[9] * (*in1)[2];
    (*out)[2] = (*in1)[1] * (*in2)[6] + (*in2)[2] * (*in1)[0] + (*in2)[10] * (*in1)[2];
    (*out)[3] = (*in1)[1] * (*in2)[7] + (*in2)[3] * (*in1)[0] + (*in2)[11] * (*in1)[2] + (*in1)[3];
    (*out)[4] = (*in1)[6] * (*in2)[8] + (*in1)[5] * (*in2)[4] + (*in1)[4] * (*in2)[0];
    (*out)[5] = (*in1)[6] * (*in2)[9] + (*in1)[5] * (*in2)[5] + (*in1)[4] * (*in2)[1];
    (*out)[6] = (*in1)[6] * (*in2)[10] + (*in1)[5] * (*in2)[6] + (*in1)[4] * (*in2)[2];
    (*out)[7] = (*in2)[11] * (*in1)[6] + (*in1)[5] * (*in2)[7] + (*in1)[4] * (*in2)[3] + (*in1)[7];
    (*out)[8] = (*in1)[10] * (*in2)[8] + (*in1)[9] * (*in2)[4] + (*in2)[0] * (*in1)[8];
    (*out)[9] = (*in1)[10] * (*in2)[9] + (*in1)[9] * (*in2)[5] + (*in2)[1] * (*in1)[8];
    (*out)[10] = (*in1)[10] * (*in2)[10] + (*in1)[9] * (*in2)[6] + (*in2)[2] * (*in1)[8];
    (*out)[11] = (*in1)[10] * (*in2)[11] + (*in1)[9] * (*in2)[7] + (*in2)[3] * (*in1)[8] + (*in1)[11];
}

// ea: 0x004B74D0
void MatrixMultiply43(const float (*in1)[3], const float (*in2)[3],
                      float (*out)[3])
{
    (*out)[0] = (*in1)[0] * (*in2)[0] + (*in1)[1] * (*in2)[3] + (*in2)[6] * (*in1)[2];
    (*out)[3] = (*in1)[4] * (*in2)[3] + (*in1)[3] * (*in2)[0] + (*in1)[5] * (*in2)[6];
    (*out)[6] = (*in1)[7] * (*in2)[3] + (*in1)[6] * (*in2)[0] + (*in1)[8] * (*in2)[6];
    (*out)[1] = (*in1)[1] * (*in2)[4] + (*in2)[7] * (*in1)[2] + (*in2)[1] * (*in1)[0];
    (*out)[4] = (*in1)[5] * (*in2)[7] + (*in1)[4] * (*in2)[4] + (*in1)[3] * (*in2)[1];
    (*out)[7] = (*in1)[8] * (*in2)[7] + (*in1)[7] * (*in2)[4] + (*in1)[6] * (*in2)[1];
    (*out)[2] = (*in1)[0] * (*in2)[2] + (*in1)[1] * (*in2)[5] + (*in2)[8] * (*in1)[2];
    (*out)[5] = (*in1)[5] * (*in2)[8] + (*in1)[4] * (*in2)[5] + (*in1)[3] * (*in2)[2];
    (*out)[8] = (*in1)[8] * (*in2)[8] + (*in1)[7] * (*in2)[5] + (*in1)[6] * (*in2)[2];
    (*out)[9] = (*in1)[10] * (*in2)[3] + (*in1)[9] * (*in2)[0] + (*in1)[11] * (*in2)[6] + (*in2)[9];
    (*out)[10] = (*in1)[11] * (*in2)[7] + (*in1)[10] * (*in2)[4] + (*in1)[9] * (*in2)[1] + (*in2)[10];
    (*out)[11] = (*in1)[11] * (*in2)[8] + (*in1)[10] * (*in2)[5] + (*in1)[9] * (*in2)[2] + (*in2)[11];
}

// ea: 0x004B76F0
void DObjSkelMatrixMultiply43(const DObjSkelMat* in1,
                              const float (*in2)[3], float (*out)[3])
{
    (*out)[0] = in1->axis[0][0] * (*in2)[0] + in1->axis[0][1] * (*in2)[3] + (*in2)[6] * in1->axis[0][2];
    (*out)[3] = in1->axis[1][1] * (*in2)[3] + in1->axis[1][0] * (*in2)[0] + in1->axis[1][2] * (*in2)[6];
    (*out)[6] = in1->axis[2][1] * (*in2)[3] + in1->axis[2][0] * (*in2)[0] + in1->axis[2][2] * (*in2)[6];
    (*out)[1] = in1->axis[0][1] * (*in2)[4] + (*in2)[7] * in1->axis[0][2] + (*in2)[1] * in1->axis[0][0];
    (*out)[4] = in1->axis[1][2] * (*in2)[7] + in1->axis[1][1] * (*in2)[4] + in1->axis[1][0] * (*in2)[1];
    (*out)[7] = in1->axis[2][2] * (*in2)[7] + in1->axis[2][1] * (*in2)[4] + in1->axis[2][0] * (*in2)[1];
    (*out)[2] = in1->axis[0][0] * (*in2)[2] + in1->axis[0][1] * (*in2)[5] + (*in2)[8] * in1->axis[0][2];
    (*out)[5] = in1->axis[1][2] * (*in2)[8] + in1->axis[1][1] * (*in2)[5] + in1->axis[1][0] * (*in2)[2];
    (*out)[8] = in1->axis[2][2] * (*in2)[8] + in1->axis[2][1] * (*in2)[5] + in1->axis[2][0] * (*in2)[2];
    (*out)[9] = in1->origin[1] * (*in2)[3] + in1->origin[0] * (*in2)[0] + in1->origin[2] * (*in2)[6] + (*in2)[9];
    (*out)[10] = in1->origin[2] * (*in2)[7] + in1->origin[1] * (*in2)[4] + in1->origin[0] * (*in2)[1] + (*in2)[10];
    (*out)[11] = in1->origin[2] * (*in2)[8] + in1->origin[1] * (*in2)[5] + in1->origin[0] * (*in2)[2] + (*in2)[11];
}

// ea: 0x004B8000
void DObjSkel2MatrixMultiply43(const DObjSkelMat* in1,
                               const float (*in2)[3], DObjSkelMat* out)
{
    out->axis[0][0] = in1->axis[0][0] * (*in2)[0] + in1->axis[0][1] * (*in2)[3] + (*in2)[6] * in1->axis[0][2];
    out->axis[1][0] = in1->axis[1][1] * (*in2)[3] + in1->axis[1][0] * (*in2)[0] + in1->axis[1][2] * (*in2)[6];
    out->axis[2][0] = in1->axis[2][1] * (*in2)[3] + in1->axis[2][0] * (*in2)[0] + in1->axis[2][2] * (*in2)[6];
    out->axis[0][1] = in1->axis[0][1] * (*in2)[4] + (*in2)[7] * in1->axis[0][2] + (*in2)[1] * in1->axis[0][0];
    out->axis[1][1] = in1->axis[1][2] * (*in2)[7] + in1->axis[1][1] * (*in2)[4] + in1->axis[1][0] * (*in2)[1];
    out->axis[2][1] = in1->axis[2][2] * (*in2)[7] + in1->axis[2][1] * (*in2)[4] + in1->axis[2][0] * (*in2)[1];
    out->axis[0][2] = in1->axis[0][0] * (*in2)[2] + in1->axis[0][1] * (*in2)[5] + (*in2)[8] * in1->axis[0][2];
    out->axis[1][2] = in1->axis[1][2] * (*in2)[8] + in1->axis[1][1] * (*in2)[5] + in1->axis[1][0] * (*in2)[2];
    out->axis[2][2] = in1->axis[2][2] * (*in2)[8] + in1->axis[2][1] * (*in2)[5] + in1->axis[2][0] * (*in2)[2];
    out->axis[0][3] = 0.0f;
    out->axis[1][3] = 0.0f;
    out->axis[2][3] = 0.0f;
    out->origin[0] = in1->origin[0] * (*in2)[0] + (*in2)[6] * in1->origin[2] + in1->origin[1] * (*in2)[3] + (*in2)[9];
    out->origin[1] = (*in2)[7] * in1->origin[2] + (*in2)[4] * in1->origin[1] + (*in2)[1] * in1->origin[0] + (*in2)[10];
    out->origin[2] = in1->origin[0] * (*in2)[2] + in1->origin[2] * (*in2)[8] + in1->origin[1] * (*in2)[5] + (*in2)[11];
    out->origin[3] = 1.0f;
}

// ea: 0x004B8240
DObjSkelMat* DObjSkelMatrixMultiply(DObjSkelMat* result,
                                    const DObjSkelMat* in1,
                                    const DObjSkelMat* in2)
{
    DObjSkelMat out;
    out.axis[0][0] = in1->axis[0][1] * in2->axis[1][0] + in1->axis[0][0] * in2->axis[0][0] + in1->axis[0][2] * in2->axis[2][0];
    out.axis[1][0] = in1->axis[1][0] * in2->axis[0][0] + in1->axis[1][2] * in2->axis[2][0] + in1->axis[1][1] * in2->axis[1][0];
    out.axis[2][0] = in1->axis[2][2] * in2->axis[2][0] + in1->axis[2][1] * in2->axis[1][0] + in2->axis[0][0] * in1->axis[2][0];
    out.axis[0][1] = in2->axis[2][1] * in1->axis[0][2] + in1->axis[0][1] * in2->axis[1][1] + in2->axis[0][1] * in1->axis[0][0];
    out.axis[1][1] = in1->axis[1][2] * in2->axis[2][1] + in1->axis[1][1] * in2->axis[1][1] + in1->axis[1][0] * in2->axis[0][1];
    out.axis[2][1] = in2->axis[0][1] * in1->axis[2][0] + in1->axis[2][2] * in2->axis[2][1] + in1->axis[2][1] * in2->axis[1][1];
    out.axis[0][2] = in2->axis[2][2] * in1->axis[0][2] + in1->axis[0][1] * in2->axis[1][2] + in2->axis[0][2] * in1->axis[0][0];
    out.axis[1][2] = in1->axis[1][2] * in2->axis[2][2] + in1->axis[1][1] * in2->axis[1][2] + in1->axis[1][0] * in2->axis[0][2];
    out.axis[2][2] = in2->axis[0][2] * in1->axis[2][0] + in1->axis[2][2] * in2->axis[2][2] + in1->axis[2][1] * in2->axis[1][2];
    out.axis[0][3] = 0.0f;
    out.axis[1][3] = 0.0f;
    out.axis[2][3] = 0.0f;
    out.origin[0] = in1->origin[0] * in2->axis[0][0] + in1->origin[2] * in2->axis[2][0] + in1->origin[1] * in2->axis[1][0] + in2->origin[0];
    out.origin[1] = in1->origin[2] * in2->axis[2][1] + in1->origin[1] * in2->axis[1][1] + in1->origin[0] * in2->axis[0][1] + in2->origin[1];
    out.origin[2] = in1->origin[2] * in2->axis[2][2] + in1->origin[1] * in2->axis[1][2] + in1->origin[0] * in2->axis[0][2] + in2->origin[2];
    out.origin[3] = 1.0f;
    *result = out;
    return result;
}

// ea: 0x004B84C0
void MatrixTranspose(const float (*in)[3], float (*out)[3])
{
    (*out)[0] = (*in)[0];
    (*out)[1] = (*in)[3];
    (*out)[2] = (*in)[6];
    (*out)[3] = (*in)[1];
    (*out)[4] = (*in)[4];
    (*out)[5] = (*in)[7];
    (*out)[6] = (*in)[2];
    (*out)[7] = (*in)[5];
    (*out)[8] = (*in)[8];
}

// ea: 0x004B8500
void MatrixInverse(const float (*in)[3], float (*out)[3])
{
    float det = ((*in)[4] * (*in)[8] - (*in)[7] * (*in)[5]) * (*in)[0]
              - ((*in)[1] * (*in)[8] - (*in)[7] * (*in)[2]) * (*in)[3]
              + ((*in)[1] * (*in)[5] - (*in)[2] * (*in)[4]) * (*in)[6];
    float v3 = det;
    if (det == 0.0f)
    {
        ASSERT("det", "c:\\cod\\code\\game\\com_math.cpp", 1471);
        v3 = det;
    }
    float inv = 1.0f / v3;
    (*out)[0] = ((*in)[4] * (*in)[8] - (*in)[7] * (*in)[5]) * inv;
    (*out)[1] = 0.0f - (((*in)[1] * (*in)[8] - (*in)[7] * (*in)[2]) * inv);
    (*out)[2] = ((*in)[1] * (*in)[5] - (*in)[2] * (*in)[4]) * inv;
    (*out)[3] = 0.0f - (((*in)[3] * (*in)[8] - (*in)[6] * (*in)[5]) * inv);
    (*out)[4] = ((*in)[0] * (*in)[8] - (*in)[2] * (*in)[6]) * inv;
    (*out)[5] = 0.0f - (((*in)[0] * (*in)[5] - (*in)[2] * (*in)[3]) * inv);
    (*out)[6] = ((*in)[7] * (*in)[3] - (*in)[6] * (*in)[4]) * inv;
    (*out)[7] = 0.0f - (((*in)[7] * (*in)[0] - (*in)[1] * (*in)[6]) * inv);
    (*out)[8] = ((*in)[4] * (*in)[0] - (*in)[1] * (*in)[3]) * inv;
}

// ea: 0x004B8ED0
void MatrixTransformVector(const float* in1, const float (*in2)[3], float* out)
{
    *out = (*in2)[3] * in1[1] + (*in2)[6] * in1[2] + *in1 * (*in2)[0];
    out[1] = (*in2)[1] * *in1 + (*in2)[4] * in1[1] + (*in2)[7] * in1[2];
    out[2] = (*in2)[2] * *in1 + (*in2)[5] * in1[1] + (*in2)[8] * in1[2];
}

// ea: 0x004B8F60
void MatrixTransposeTransformVector(const float* in1, const float (*in2)[3],
                                    float* out)
{
    *out = (*in2)[1] * in1[1] + (*in2)[2] * in1[2] + *in1 * (*in2)[0];
    out[1] = (*in2)[3] * *in1 + (*in2)[4] * in1[1] + (*in2)[5] * in1[2];
    out[2] = (*in2)[6] * *in1 + (*in2)[7] * in1[1] + (*in2)[8] * in1[2];
}

// ea: 0x004B8FF0
void MatrixTransformVector43(const float* in1, const float (*in2)[3],
                             float* out)
{
    *out = (*in2)[3] * in1[1] + (*in2)[6] * in1[2] + *in1 * (*in2)[0] + (*in2)[9];
    out[1] = (*in2)[1] * *in1 + (*in2)[4] * in1[1] + (*in2)[7] * in1[2] + (*in2)[10];
    out[2] = (*in2)[2] * *in1 + (*in2)[5] * in1[1] + (*in2)[8] * in1[2] + (*in2)[11];
}

// ea: 0x004B9090
void MatrixTransformVector43(const float* in1, const float (*in2)[3],
                             math::Position3* out)
{
    out->v.m128_f32[0] = (*in2)[3] * in1[1] + (*in2)[6] * in1[2] + *in1 * (*in2)[0] + (*in2)[9];
    out->v.m128_f32[1] = (*in2)[1] * *in1 + (*in2)[4] * in1[1] + (*in2)[7] * in1[2] + (*in2)[10];
    out->v.m128_f32[2] = (*in2)[2] * *in1 + (*in2)[5] * in1[1] + (*in2)[8] * in1[2] + (*in2)[11];
}

// ea: 0x004B9130
void DObjSkelMatrixTransformVector43(const float* in1, const DObjSkelMat* in2,
                                     float* out)
{
    *out = in2->axis[2][0] * in1[2] + in2->axis[1][0] * in1[1] + *in1 * in2->axis[0][0] + in2->origin[0];
    out[1] = in2->axis[0][1] * *in1 + in2->axis[2][1] * in1[2] + in2->axis[1][1] * in1[1] + in2->origin[1];
    out[2] = in2->axis[0][2] * *in1 + in2->axis[2][2] * in1[2] + in2->axis[1][2] * in1[1] + in2->origin[2];
}

// ea: 0x004B91D0
void MatrixTransposeTransformVector43(const float* in1, const float (*in2)[3],
                                      float* out)
{
    float v3 = in1[1] - (*in2)[10];
    float v4 = in1[2] - (*in2)[11];
    float v5 = *in1 - (*in2)[9];
    *out = (*in2)[2] * v4 + (*in2)[1] * v3 + (*in2)[0] * v5;
    out[1] = (*in2)[5] * v4 + (*in2)[4] * v3 + (*in2)[3] * v5;
    out[2] = (*in2)[8] * v4 + (*in2)[7] * v3 + (*in2)[6] * v5;
}

// ea: 0x004B9280
void MatrixTransformVector43Equals(float* out, const float* in)
{
    float v2 = in[3] * out[1] + in[6] * out[2] + *out * *in + in[9];
    float v3 = in[4] * out[1] + in[1] * *out + in[7] * out[2] + in[10];
    out[2] = in[5] * out[1] + in[2] * *out + in[8] * out[2] + in[11];
    *out = v2;
    out[1] = v3;
}

// ea: 0x004B9320
void QuatMultiply(const float* in1, const float* in2, float* out)
{
    *out = in2[3] * *in1 + in2[1] * in1[2] + in1[3] * *in2 - in2[2] * in1[1];
    out[1] = in2[3] * in1[1] - *in2 * in1[2] + in2[2] * *in1 + in1[3] * in2[1];
    out[2] = in1[1] * *in2 + in2[3] * in1[2] - *in1 * in2[1] + in1[3] * in2[2];
    out[3] = in2[3] * in1[3] - *in1 * *in2 - in1[1] * in2[1] - in2[2] * in1[2];
}

// ea: 0x004B9410
void QuatInverse(const float* in, float* out)
{
    *out = 0.0f - *in;
    out[1] = 0.0f - in[1];
    out[2] = 0.0f - in[2];
    out[3] = in[3];
}

// ea: 0x004B9450
void ConvertQuatToMat(float* mat)
{
    float v2 = mat[2] * mat[2];
    float v3 = mat[1] * mat[1];
    float v4 = *mat * *mat;
    float v5 = mat[3] * mat[3] + v2 + v3 + v4;
    if (v5 == 0.0f)
    {
        *mat = 1.0f;
        mat[1] = 0.0f;
        mat[2] = 0.0f;
        mat[3] = 0.0f;
        mat[4] = 1.0f;
        mat[5] = 0.0f;
        mat[6] = 0.0f;
        mat[7] = 0.0f;
        mat[8] = 1.0f;
    }
    else
    {
        float v6 = 2.0f / v5;
        float v7 = v6 * v4;
        float v8 = mat[1] * v6;
        float v9 = v6 * v2;
        float v10 = *mat * v6;
        float xz = mat[2] * v10;
        float xx = v7;
        float v11 = v6 * v3;
        float v12 = mat[3] * v10;
        float v13 = mat[1] * v10;
        float v14 = mat[2] * v8;
        float xw = v12;
        float v15 = mat[3] * v8;
        float v16 = mat[3] * mat[2] * v6;
        mat[1] = v16 + v13;
        *mat = 1.0f - (v9 + v11);
        mat[2] = xz - v15;
        mat[3] = v13 - v16;
        mat[4] = 1.0f - (v9 + xx);
        mat[5] = v14 + xw;
        mat[6] = v15 + xz;
        mat[7] = v14 - xw;
        mat[8] = 1.0f - (v11 + xx);
    }
}

// ea: 0x004B95E0
float QuatEigenTrace(float* quat)
{
    float v1 = quat[2] * quat[2];
    float v2 = quat[1] * quat[1];
    float v3 = *quat * *quat;
    float v4 = quat[3] * quat[3] + v1 + v2 + v3;
    if (v4 == 0.0f)
        return 0.0f;
    return (1.0f / v4) * v1 + (1.0f / v4) * v2 + (1.0f / v4) * v3;
}

// ea: 0x004B9670
float AngleEigenTrace(float angle)
{
    float s = sinf(angle * 0.017453292f);
    return s * s;
}

// ea: 0x004B96A0
float QuatRatioEigenTrace(float* quat1, float* quat2)
{
    float v2 = quat2[3];
    float v3 = 0.0f - *quat2;
    float v4 = 0.0f - quat2[1];
    float v5 = 0.0f - quat2[2];
    float v6 = quat1[2];
    float v7 = v6 * v4 + quat1[3] * v3 + *quat1 * v2 - quat1[1] * v5;
    float v8 = quat1[1] * v2 - v6 * v3 + quat1[3] * v4;
    float v9 = quat1[1] * v3 + v6 * v2 - *quat1 * v4 + quat1[3] * v5;
    float v10 = quat1[3] * v2 - *quat1 * v3 - quat1[1] * v4 - v6 * v5;
    float v11 = v7 * v7;
    float v12 = v10 * v10;
    float v14 = v9 * v9;
    float v15 = (v8 + *quat1 * v5) * (v8 + *quat1 * v5);
    float v16 = v12 + v14 + v15 + v11;
    if (v16 != 0.0f)
        return (1.0f / v16) * v14 + (1.0f / v16) * v15 + (1.0f / v16) * v11;
    return 0.0f;
}

// ea: 0x004B97E0
unsigned int ColorBytes3(float r, float g, float b)
{
    unsigned int i = 0;
    i = (unsigned char)(r * 255.0f);
    i |= ((unsigned int)(unsigned char)(g * 255.0f)) << 8;
    i |= ((unsigned int)(unsigned char)(b * 255.0f)) << 16;
    i |= 0xFF000000u;
    return i;
}

// ea: 0x004B9820
unsigned int ColorBytes4(float r, float g, float b, float a)
{
    unsigned int i = 0;
    i = (unsigned char)(r * 255.0f);
    i |= ((unsigned int)(unsigned char)(g * 255.0f)) << 8;
    i |= ((unsigned int)(unsigned char)(b * 255.0f)) << 16;
    i |= ((unsigned int)(unsigned char)(a * 255.0f)) << 24;
    return i;
}

// ea: 0x004B9870
float NormalizeColor(float* in, float* out)
{
    float v4 = in[1];
    float max = *in;
    float v3 = max;
    if (v4 > max)
    {
        v3 = v4;
        max = v4;
    }
    if (in[2] > v3)
    {
        v3 = in[2];
        max = v3;
    }
    float v5 = 0.0f;
    float result = max;
    if (v3 == 0.0f)
    {
        out[1] = 0.0f;
        *out = 0.0f;
    }
    else
    {
        *out = *in / v3;
        out[1] = in[1] / v3;
        v5 = in[2] / v3;
    }
    out[2] = v5;
    return result;
}

// ea: 0x004B98F0
float AngleMod(float a)
{
    float aa = a * 182.04445f;
    return aa * 0.0054931641f;
}

// ea: 0x004B9920
float LerpAngle(float from, float to, float frac)
{
    float v3 = to;
    if (to - from > 180.0f)
    {
        v3 = to - 360.0f;
        to = to - 360.0f;
    }
    if (v3 - from < -180.0f)
        to = v3 + 360.0f;
    return (to - from) * frac + from;
}

// ea: 0x004B9990
float AngleSubtract(float a1, float a2)
{
    float a = fmodf(a1 - a2, 360.0f);
    float v2 = a;
    if (a > 180.0f)
    {
        v2 = a - 360.0f;
        a = a - 360.0f;
    }
    if (v2 < -180.0f)
        return v2 + 360.0f;
    return a;
}

// ea: 0x004B99F0
void AnglesSubtract(const math::Position3* v1, const math::Position3* v2,
                    math::Position3* v3)
{
    float v2a = fmodf(v1->v.m128_f32[0] - v2->v.m128_f32[0], 360.0f);
    float v4 = v2a;
    if (v2a > 180.0f)
        v4 = v2a - 360.0f;
    if (v4 < -180.0f)
        v4 = v4 + 360.0f;
    v3->v.m128_f32[0] = v4;
    float v2b = fmodf(v1->v.m128_f32[1] - v2->v.m128_f32[1], 360.0f);
    float v5 = v2b;
    if (v2b > 180.0f)
        v5 = v2b - 360.0f;
    if (v5 < -180.0f)
        v5 = v5 + 360.0f;
    v3->v.m128_f32[1] = v5;
    float v2c = fmodf(v1->v.m128_f32[2] - v2->v.m128_f32[2], 360.0f);
    float v6 = v2c;
    if (v2c > 180.0f)
        v6 = v2c - 360.0f;
    if (v6 < -180.0f)
        v6 = v6 + 360.0f;
    v3->v.m128_f32[2] = v6;
}

// ea: 0x004B9B20
void AnglesSubtract(const float* v1, const float* v2, float* v3)
{
    float v2a = fmodf(*v1 - *v2, 360.0f);
    float v4 = v2a;
    if (v2a > 180.0f)
        v4 = v2a - 360.0f;
    if (v4 < -180.0f)
        v4 = v4 + 360.0f;
    *v3 = v4;
    float v2b = fmodf(v1[1] - v2[1], 360.0f);
    float v5 = v2b;
    if (v2b > 180.0f)
        v5 = v2b - 360.0f;
    if (v5 < -180.0f)
        v5 = v5 + 360.0f;
    v3[1] = v5;
    float v2c = fmodf(v1[2] - v2[2], 360.0f);
    float v6 = v2c;
    if (v2c > 180.0f)
        v6 = v2c - 360.0f;
    if (v6 < -180.0f)
        v6 = v6 + 360.0f;
    v3[2] = v6;
}

// ea: 0x004B9C50
float AngleNormalize360(float angle)
{
    float anglea = angle * 182.04445f;
    return anglea * 0.0054931641f;
}

// ea: 0x004B9C80
float AngleNormalize180(float angle)
{
    float v1 = (angle * 182.04445f) * 0.0054931641f;
    float a = v1;
    if (v1 > 180.0f)
        return v1 - 360.0f;
    return a;
}

// ea: 0x004B9CD0
float AngleDelta(float angle1, float angle2)
{
    float v2 = ((angle1 - angle2) * 182.04445f) * 0.0054931641f;
    float angle1a = v2;
    if (v2 > 180.0f)
        return v2 - 360.0f;
    return angle1a;
}

// ea: 0x004B9D20
float RadiusFromBounds(const math::Position3* mins, const math::Position3* maxs)
{
    float v6 = fabsf(mins->v.m128_f32[0]);
    float v2 = v6;
    float b = fabsf(maxs->v.m128_f32[0]);
    if (v6 <= b)
        v2 = b;
    float bb = fabsf(mins->v.m128_f32[1]);
    float a = bb;
    float v3 = bb;
    float ba = fabsf(maxs->v.m128_f32[1]);
    if (a <= ba)
        v3 = ba;
    float v9 = fabsf(mins->v.m128_f32[2]);
    float v4 = v9;
    float v7 = fabsf(maxs->v.m128_f32[2]);
    if (v9 <= v7)
        v4 = v7;
    return sqrtf(v4 * v4 + v3 * v3 + v2 * v2);
}

// ea: 0x004B9E20
void ClearBounds(float* mins, float* maxs)
{
    mins[2] = 262144.0f;
    mins[1] = 262144.0f;
    *mins = 262144.0f;
    maxs[2] = -262144.0f;
    maxs[1] = -262144.0f;
    *maxs = -262144.0f;
}

// ea: 0x004B9E60
void AxisClear(float (*axis)[3])
{
    (*axis)[0] = 1.0f;
    (*axis)[1] = 0.0f;
    (*axis)[2] = 0.0f;
    (*axis)[3] = 0.0f;
    (*axis)[4] = 1.0f;
    (*axis)[5] = 0.0f;
    (*axis)[6] = 0.0f;
    (*axis)[7] = 0.0f;
    (*axis)[8] = 1.0f;
}

// ea: 0x004B9EA0
void AxisCopy(const float (*in)[3], float (*out)[3])
{
    (*out)[0] = (*in)[0];
    (*out)[1] = (*in)[1];
    (*out)[2] = (*in)[2];
    (*out)[3] = (*in)[3];
    (*out)[4] = (*in)[4];
    (*out)[5] = (*in)[5];
    (*out)[6] = (*in)[6];
    (*out)[7] = (*in)[7];
    (*out)[8] = (*in)[8];
}

// ea: 0x004B9EE0
int PlaneFromPoints(float* plane, const float* a, const float* b, const float* c)
{
    float v4 = c[1] - a[1];
    float v5 = c[2] - a[2];
    float v6 = b[1] - a[1];
    float v7 = b[2] - a[2];
    float v8 = *b - *a;
    float v9 = v4 * v7 - v5 * v6;
    float v10 = *c - *a;
    float planea = v10 * v6 - v4 * v8;
    plane[2] = planea;
    *plane = v9;
    float ca = v5 * v8 - v10 * v7;
    plane[1] = ca;
    float ba = sqrtf(planea * planea + ca * ca + v9 * v9);
    if (ba == 0.0f)
        return 0;
    float inv = 1.0f / ba;
    float v12 = v9 * inv;
    float v13 = ca * inv;
    float v14 = planea * inv;
    plane[2] = v14;
    *plane = v12;
    plane[1] = v13;
    plane[3] = *a * v12 + v14 * a[2] + a[1] * v13;
    return 1;
}

// ea: 0x004BA030
void SetPlaneSignbits(cplane_s* out)
{
    unsigned char v1 = out->normal[0] < 0.0f;
    if (out->normal[1] < 0.0f)
        v1 |= 2u;
    if (out->normal[2] < 0.0f)
        v1 |= 4u;
    out->signbits = v1;
}

// ea: 0x004BA060
int BoxDistSqrdExceeds(const float* absmin, const float* absmax,
                       const float* org, float fogOpaqueDistSqrd)
{
    float v4 = *absmax - *org;
    float v5 = *absmin - *org;
    float v6 = absmin[1] - org[1];
    float v7 = absmin[2] - org[2];
    float v8 = absmax[1] - org[1];
    float v9 = absmax[2] - org[2];
    float v12;
    if (v4 * v5 <= 0.0f)
    {
        v12 = 0.0f;
    }
    else
    {
        float v10 = v5 * v5;
        float v11 = v4 * v4;
        if (v10 > v11)
            v10 = v11;
        v12 = v10 + 0.0f;
    }
    if (v8 * v6 > 0.0f)
    {
        float v13 = v6 * v6;
        float v14 = v8 * v8;
        if (v13 > v14)
            v13 = v14;
        v12 = v13 + v12;
    }
    if (v9 * v7 > 0.0f)
    {
        float v15 = v7 * v7;
        float v16 = v9 * v9;
        if (v15 > v16)
            v15 = v16;
        v12 = v15 + v12;
    }
    return v12 > fogOpaqueDistSqrd;
}

// ea: 0x004BA130
void Vec10Copy(const float* in, float* out)
{
    *out = *in;
    out[1] = in[1];
    out[2] = in[2];
    out[3] = in[3];
    out[4] = in[4];
    out[5] = in[5];
    out[6] = in[6];
    out[7] = in[7];
    out[8] = in[8];
    out[9] = in[9];
}

// ea: 0x004BA180
float Q_rint(float in)
{
    float v1 = in + 0.5f;
    v1 = floorf(v1);
    return v1;
}

// ea: 0x004BA1B0
float ColorNormalize(float* in, float* out)
{
    float v4 = in[1];
    float max = *in;
    float v3 = max;
    if (v4 > max)
    {
        v3 = v4;
        max = v4;
    }
    if (in[2] > v3)
    {
        v3 = in[2];
        max = v3;
    }
    if (v3 == 0.0f)
    {
        out[2] = 1.0f;
        out[1] = 1.0f;
        *out = 1.0f;
        return 0.0f;
    }
    float inv = 1.0f / v3;
    *out = *in * inv;
    out[1] = in[1] * inv;
    out[2] = in[2] * inv;
    return max;
}

// ea: 0x004BA250
void VectorSnap(float* v)
{
    float va = *v + 0.5f;
    va = floorf(va);
    *v = va;
    float vb = v[1] + 0.5f;
    vb = floorf(vb);
    v[1] = vb;
    float vc = v[2] + 0.5f;
    vc = floorf(vc);
    v[2] = vc;
}

// ea: 0x004BA2C0
void _Vector5Add(const float* va, const float* vb, float* out)
{
    *out = *va + *vb;
    out[1] = va[1] + vb[1];
    out[2] = va[2] + vb[2];
    out[3] = va[3] + vb[3];
    out[4] = va[4] + vb[4];
}

// ea: 0x004BA320
void _Vector5Scale(const float* v, float scale, float* out)
{
    *out = *v * scale;
    out[1] = v[1] * scale;
    out[2] = v[2] * scale;
    out[3] = v[3] * scale;
    out[4] = v[4] * scale;
}

// ea: 0x004BA380
void _Vector53Copy(const float* in, float* out)
{
    *out = *in;
    out[1] = in[1];
    out[2] = in[2];
}

// ea: 0x004BA3A0
float RoundFloat(float fIn, int iNumPlaces)
{
    float v6 = (float)iNumPlaces;
    float dValuea = powf(10.0f, v6);
    float dValueb = dValuea * fIn;
    float intptr;
    float v4 = modff(dValueb, &intptr);
    float dValue = intptr;
    float dFrac = v4;
    if (dFrac >= 0.5f)
    {
        dValue = dValue + 1.0f;
    }
    else if (dFrac <= -0.5f)
    {
        dValue = dValue - 1.0f;
    }
    float dFraca = powf(0.1000000014901161f, v6);
    return dFraca * dValue;
}

// ea: 0x004BA450
void Rand_Init(int seed)
{
    holdrand = seed;
}

// ea: 0x004BA460
float flrand(float min, float max)
{
    holdrand = 214013 * holdrand + 2531011;
    float v3 = (float)(holdrand >> 17);
    return (max - min) * v3 / 32768.0f + min;
}

// ea: 0x004BA4B0
int irand(int min, int max)
{
    holdrand = 214013 * holdrand + 2531011;
    return min + ((max - min) * (holdrand >> 17) >> 15);
}

// ea: 0x004BA4E0
float Q_SwayRand(float x, float y, float time)
{
    float timea = time / 1000.0f;
    float ya = cosf(timea * y * 6.2831855f);
    float timeb = sinf(timea * x * 6.2831855f);
    return timeb * ya;
}

// ea: 0x004BA7B0
float DiffTrack(float tgt, float cur, float rate, float deltaTime)
{
    float v4 = (tgt - cur) * rate * deltaTime;
    float ratea = fabsf(tgt - cur);
    if (ratea <= 0.0049999999f)
        return tgt;
    float err = fabsf(v4);
    if (err > ratea)
        return tgt;
    return v4 + cur;
}

// ea: 0x004BA820
void InterpolateAngles(math::Position3* curAngles,
                       const math::Position3* initialAngles,
                       const math::Position3* targetAngles, float t)
{
    float v4 = targetAngles->v.m128_f32[1];
    float v5 = targetAngles->v.m128_f32[2];
    float newDeltaYaw = targetAngles->v.m128_f32[0];
    float v12 = fabsf(newDeltaYaw);
    if (v12 > 90.0f)
    {
        do
        {
            float v13 = 1.0f;
            if (newDeltaYaw <= 0.0f)
                v13 = -1.0f;
            newDeltaYaw = newDeltaYaw - v13 * 180.0f;
            float v10 = fabsf(newDeltaYaw);
            if (v10 <= 90.0f)
                break;
        }
        while (1);
    }
    float v7 = v4 - initialAngles->v.m128_f32[1];
    float v8 = newDeltaYaw - initialAngles->v.m128_f32[0];
    float v9 = v5 - initialAngles->v.m128_f32[2];
    float newDeltaYawa = v7;
    float initialAnglesb = fabsf(v7);
    if (initialAnglesb > 180.0f)
    {
        do
        {
            float initialAnglesa = 1.0f;
            if (v7 <= 0.0f)
                initialAnglesa = -1.0f;
            newDeltaYawa = newDeltaYawa - initialAnglesa * 360.0f;
            float v11 = fabsf(newDeltaYawa);
            if (v11 <= 180.0f)
                break;
        }
        while (1);
        v7 = newDeltaYawa;
    }
    curAngles->v.m128_f32[0] = initialAngles->v.m128_f32[0] + v8 * t;
    curAngles->v.m128_f32[1] = v7 * t + initialAngles->v.m128_f32[1];
    curAngles->v.m128_f32[2] = v9 * t + initialAngles->v.m128_f32[2];
}

// ea: 0x004BA960
void InterpolateAnglesSmooth(float* curAngles, float* initialAngles,
                             float* targetAngles, float t)
{
    float v4 = initialAngles[1];
    float v5 = initialAngles[2];
    float v6 = *targetAngles;
    float v7 = targetAngles[1];
    float v8 = targetAngles[2];
    float newStartPitch = *initialAngles;
    float newDeltaYawc = fabsf(newStartPitch);
    if (newDeltaYawc > 90.0f)
    {
        do
        {
            float newDeltaYaw;
            if (newStartPitch <= 0.0f)
                newDeltaYaw = -1.0f;
            else
                newDeltaYaw = 1.0f;
            newStartPitch = newStartPitch - newDeltaYaw * 180.0f;
            float v13 = fabsf(newStartPitch);
            if (v13 <= 90.0f)
                break;
        }
        while (1);
    }
    float newDeltaYawa = v6;
    float v14 = fabsf(v6);
    if (v14 > 90.0f)
    {
        do
        {
            float v15;
            if (v6 <= 0.0f)
                v15 = -1.0f;
            else
                v15 = 1.0f;
            newDeltaYawa = newDeltaYawa - v15 * 180.0f;
            v6 = newDeltaYawa;
            float v10 = fabsf(newDeltaYawa);
            if (v10 <= 90.0f)
                break;
        }
        while (1);
    }
    float v9 = v7 - v4;
    float v11 = fabsf(v9);
    float newDeltaYawb = v9;
    if (v11 > 180.0f)
    {
        do
        {
            float v16 = 1.0f;
            if (v9 <= 0.0f)
                v16 = -1.0f;
            newDeltaYawb = newDeltaYawb - v16 * 360.0f;
            float v12 = fabsf(newDeltaYawb);
            if (v12 <= 180.0f)
                break;
        }
        while (1);
    }
    float time = cosf(t * 3.1415927f - 3.1415927f);
    float timea = (time + 1.0f) * 0.5f;
    *curAngles = newStartPitch + timea * (v6 - newStartPitch);
    curAngles[1] = timea * newDeltaYawb + v4;
    curAngles[2] = timea * (v8 - v5) + v5;
}

// ea: 0x004BAB70
void InterpolatePositionSmooth(float* curPos, const float* initialPos,
                               const float* targetPos, float t)
{
    float time = cosf(t * 3.1415927f - 3.1415927f);
    float timea = (time + 1.0f) * 0.5f;
    float v4 = (targetPos[1] - initialPos[1]) * timea;
    float v5 = (targetPos[2] - initialPos[2]) * timea;
    *curPos = *initialPos + (*targetPos - *initialPos) * timea;
    curPos[1] = initialPos[1] + v4;
    curPos[2] = initialPos[2] + v5;
}

// ea: 0x004BC250
void toMatrix(const quat_t* src, mat3_t* dst)
{
    float y = src->y;
    float v3 = src->x * 2.0f;
    float v4 = src->z * 2.0f;
    float v5 = src->x * v3;
    float v6 = src->w * v3;
    float v7 = src->w * (y * 2.0f);
    float v8 = y * (y * 2.0f);
    float v9 = src->w * v4;
    float xy = src->x * (y * 2.0f);
    float v10 = src->x * v4;
    float yz = y * v4;
    float v11 = src->z * v4;
    dst->mat[0].x = 1.0f - (v11 + v8);
    dst->mat[0].y = xy - v9;
    dst->mat[0].z = v7 + v10;
    dst->mat[1].x = v9 + xy;
    dst->mat[1].y = 1.0f - (v11 + v5);
    dst->mat[1].z = yz - v6;
    dst->mat[2].x = v10 - v7;
    dst->mat[2].y = v6 + yz;
    dst->mat[2].z = 1.0f - (v8 + v5);
}

// ea: 0x004BC6A0
void toQuat(idVec3* src, quat_t* dst)
{
    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
    dst->w = 0.0f;
}

// ea: 0x004BCF70
float interpolate(float min, float max, float percentage)
{
    return (max - min) * percentage + min;
}

// ea: 0x004BCF90
float ComputeIntensity(float min_distance, float max_distance, float distance)
{
    ASSERT("min_distance >= 0.0f", "c:\\cod\\code\\game\\RumbleManager.cpp", 30);
    ASSERT("max_distance >= 0.0f", "c:\\cod\\code\\game\\RumbleManager.cpp", 31);
    ASSERT("min_distance <= max_distance", "c:\\cod\\code\\game\\RumbleManager.cpp", 32);
    ASSERT("distance >= 0.0f", "c:\\cod\\code\\game\\RumbleManager.cpp", 33);
    if (min_distance >= distance)
        return 1.0f;
    if (distance >= max_distance)
        return 0.0f;
    float range = max_distance - min_distance;
    float distancea = distance - min_distance;
    return 1.0f - distancea / range;
}

// ea: 0x004BD140
float ConvertCharToIntensity(char c)
{
    float ca = (float)(c - 97);
    return ca / 25.0f;
}

// ea: 0x004BD1C0
float clamp_0_to_1(float f)
{
    if (f < 0.0f)
        return 0.0f;
    float fa = 1.0f;
    if (f <= 1.0f)
        return f;
    return fa;
}

// ea: 0x004BDF70
void VectorNormalizeFast(float* v)
{
    float v2 = 1.0f / sqrtf(*v * *v + v[1] * v[1] + v[2] * v[2]);
    *v = *v * v2;
    float v3 = v2 * v[1];
    float v4 = v2 * v[2];
    v[1] = v3;
    v[2] = v4;
}

// ea: 0x004BF8C0
void MatrixInverseOrthogonal43(const float (*in)[3], float (*out)[3])
{
    (*out)[0] = (*in)[0];
    (*out)[1] = (*in)[3];
    (*out)[2] = (*in)[6];
    (*out)[3] = (*in)[1];
    float v2 = (*out)[3];
    (*out)[4] = (*in)[4];
    (*out)[5] = (*in)[7];
    (*out)[6] = (*in)[2];
    float v3 = (*out)[6];
    (*out)[7] = (*in)[5];
    (*out)[8] = (*in)[8];
    float v4 = 0.0f - (*in)[9];
    float v5 = 0.0f - (*in)[11];
    float v6 = 0.0f - (*in)[10];
    (*out)[9] = v2 * v6 + v3 * v5 + (*out)[0] * v4;
    (*out)[10] = (*out)[4] * v6 + (*out)[7] * v5 + v4 * (*out)[1];
    (*out)[11] = (*out)[8] * v5 + (*out)[2] * v4 + v6 * (*out)[5];
}

// ea: 0x004BFCE0
float AngleNormalize360Accurate(float angle)
{
    float v1 = angle;
    if (angle >= 0.0f)
    {
        if (angle >= 360.0f)
        {
            do
                v1 = v1 - 360.0f;
            while (v1 >= 360.0f);
            return v1;
        }
        return angle;
    }
    do
        v1 = v1 + 360.0f;
    while (v1 < 0.0f);
    return v1;
}

// ea: 0x004BFD90
float AngleNormalize180Accurate(float angle)
{
    float v1 = angle;
    if (angle > -180.0f)
    {
        if (angle > 180.0f)
        {
            do
                v1 = v1 - 360.0f;
            while (v1 > 180.0f);
            return v1;
        }
        return angle;
    }
    do
        v1 = v1 + 360.0f;
    while (v1 <= -180.0f);
    return v1;
}

// ea: 0x004BFE60
void ProjectPointOnPlane(float* dst, const float* p, const float* normal)
{
    float lengthSqrd = *normal * *normal + normal[1] * normal[1] + normal[2] * normal[2];
    ASSERT("lengthSqrd", "c:\\cod\\code\\game\\com_math.cpp", 2497);
    float lengthSqrda = 1.0f / lengthSqrd;
    float v4 = (p[2] * normal[2] + *p * *normal + p[1] * normal[1]) * lengthSqrda;
    float v5 = lengthSqrda * normal[1];
    float v6 = lengthSqrda * normal[2];
    *dst = *p - (*normal * lengthSqrda) * v4;
    dst[1] = p[1] - v5 * v4;
    dst[2] = p[2] - v6 * v4;
}

// ea: 0x004BFFD0
void NormalToLatLong(float* normal, unsigned char* bytes)
{
    float v2 = 0.0f;
    if (*normal == 0.0f && normal[1] == 0.0f)
    {
        float v4 = normal[2];
        bytes[1] = 0;
        if (v4 <= 0.0f)
            *bytes = 0x80;
        else
            *bytes = 0;
    }
    else
    {
        float v5 = normal[1];
        float normala = *normal;
        float v18 = fabsf(normala);
        float v19 = fabsf(v5);
        if (v19 + v18 != 0.0f)
        {
            float v11 = sqrtf(v5 * v5 + normala * normala);
            float v12 = 1.0f / v11;
            float v7;
            if (v19 <= v18)
            {
                float v8 = v12 * v19;
                if (v12 * v19 >= 0.5f)
                {
                    float v16 = sqrtf(fabsf((1.0f - v8) * 0.5f));
                    v7 = v16 * v16 * v16 * v16 * v16 * v16 * -0.1079625f
                       - v16 * v16 * v16 * v16 * 0.15000001f
                       - v16 * v16 * v16 * 0.33333331f
                       - v16 * 2.0f
                       + 1.570796f;
                }
                else
                {
                    v7 = v8 * v8 * v8 * 0.1666667f
                       + v8 * v8 * v8 * v8 * v8 * v8 * 0.053981241f
                       + v8 * v8 * v8 * v8 * 0.075000003f
                       + v8;
                }
                v2 = v7;
            }
            else
            {
                float v6 = v12 * v18;
                if (v12 * v18 >= 0.5f)
                {
                    float v14 = sqrtf(fabsf((1.0f - v6) * 0.5f));
                    v2 = v14 * v14 * v14 * v14 * v14 * v14 * -0.1079625f
                       - v14 * v14 * v14 * v14 * 0.15000001f
                       - v14 * v14 * v14 * 0.33333331f
                       - v14 * 2.0f
                       + 1.570796f;
                }
                else
                {
                    v2 = v6 * v6 * v6 * 0.1666667f
                       + v6 * v6 * v6 * v6 * v6 * v6 * 0.053981241f
                       + v6 * v6 * v6 * v6 * 0.075000003f
                       + v6;
                }
                v2 = 1.5707964f - v2;
            }
            if (normala < 0.0f)
                v2 = 3.1415927f - v2;
            if (v5 < 0.0f)
                v2 = 0.0f - v2;
        }
        float v17 = acosf(normal[2]);
        *bytes = (unsigned char)(v17 * 180.0f / 3.1415927f * 0.70833331f);
        bytes[1] = (unsigned char)(v2 * 180.0f / 3.1415927f * 0.70833331f);
    }
}

// ea: 0x004C05A0
void toMatrix(const angles_t* src, mat3_t* dst)
{
    float sy, cy, psin, cp, sr, cr;
    float angle = src->yaw * 0.017453292f;
    FastSinCos(angle, &sy, &cy);
    float anglea = src->pitch * 0.017453292f;
    FastSinCos(anglea, &psin, &cp);
    float angleb = src->roll * 0.017453292f;
    FastSinCos(angleb, &sr, &cr);
    float v3 = 0.0f - psin;
    float v4 = cp * sy;
    dst->mat[0].x = cp * cy;
    dst->mat[0].z = v3;
    dst->mat[0].y = v4;
    float v5 = sy;
    float v6 = cr;
    float v7 = cy;
    float v8 = sr * psin;
    dst->mat[1].z = sr * cp;
    dst->mat[1].y = v8 * v5 + v6 * v7;
    dst->mat[1].x = v8 * v7 - v6 * v5;
    float v9 = cr * cp;
    float v10 = (cr * psin) * sy - sr * cy;
    dst->mat[2].x = (cr * psin) * cy + sr * sy;
    dst->mat[2].y = v10;
    dst->mat[2].z = v9;
}

// ea: 0x004B7910
void MatrixMultiplyRT(const math::Mat43* in1, const math::Mat43* in2,
                      math::Mat43* out)
{
    for (int i = 0; i < 3; ++i)
    {
        out->x.v.m128_f32[i] =
            in2->x.v.m128_f32[i] * in1->x.v.m128_f32[0]
            + in2->y.v.m128_f32[i] * in1->x.v.m128_f32[1]
            + in2->z.v.m128_f32[i] * in1->x.v.m128_f32[2];
        out->y.v.m128_f32[i] =
            in2->x.v.m128_f32[i] * in1->y.v.m128_f32[0]
            + in2->y.v.m128_f32[i] * in1->y.v.m128_f32[1]
            + in2->z.v.m128_f32[i] * in1->y.v.m128_f32[2];
        out->z.v.m128_f32[i] =
            in2->x.v.m128_f32[i] * in1->z.v.m128_f32[0]
            + in2->y.v.m128_f32[i] * in1->z.v.m128_f32[1]
            + in2->z.v.m128_f32[i] * in1->z.v.m128_f32[2];
    }
    out->w.v.m128_f32[0] =
        in2->x.v.m128_f32[3] * in1->w.v.m128_f32[0]
        + in2->y.v.m128_f32[3] * in1->w.v.m128_f32[1]
        + in2->z.v.m128_f32[3] * in1->w.v.m128_f32[2]
        + in2->w.v.m128_f32[0];
    out->w.v.m128_f32[1] =
        in2->x.v.m128_f32[3] * in1->w.v.m128_f32[1]
        + in2->y.v.m128_f32[3] * in1->w.v.m128_f32[2]
        + in2->z.v.m128_f32[3] * in1->w.v.m128_f32[0]
        + in2->w.v.m128_f32[1];
    out->w.v.m128_f32[2] =
        in2->x.v.m128_f32[3] * in1->w.v.m128_f32[2]
        + in2->y.v.m128_f32[3] * in1->w.v.m128_f32[0]
        + in2->z.v.m128_f32[3] * in1->w.v.m128_f32[1]
        + in2->w.v.m128_f32[2];
    out->x.v.m128_f32[3] = 0.0f;
    out->y.v.m128_f32[3] = 0.0f;
    out->z.v.m128_f32[3] = 0.0f;
    out->w.v.m128_f32[3] = 0.0f;
}

// ea: 0x004B8710
void MatrixInverse44(float* mat, float* dst)
{
    float src_8 = mat[8];
    float src_24 = mat[9];
    float v2 = mat[10];
    float src_56 = mat[11];
    float src_12 = mat[12];
    float src_28 = mat[13];
    float v3 = mat[7];
    float src_44 = mat[14];
    float src_60 = mat[15];
    float v4 = mat[3];
    float src_20 = mat[5];
    float v5 = mat[6];
    float v6 = *mat;
    float v7 = mat[4];
    float src_16 = mat[1];
    float v8 = mat[2];
    *dst = (src_28 * (v5 * src_56) + src_24 * (v3 * src_44) + src_20 * (src_60 * v2))
         - (src_28 * (v3 * v2) + src_24 * (v5 * src_60) + src_20 * (src_56 * src_44));
    dst[1] = (src_28 * (v4 * v2) + src_24 * (v8 * src_60) + src_16 * (src_56 * src_44))
           - (src_28 * (v8 * src_56) + src_24 * (v4 * src_44) + src_16 * (src_60 * v2));
    dst[2] = (src_28 * (v8 * v3) + src_20 * (v4 * src_44) + src_16 * (v5 * src_60))
           - (src_28 * (v4 * v5) + src_20 * (v8 * src_60) + src_16 * (v3 * src_44));
    float* v9 = dst + 2;
    dst[3] = (src_24 * (v4 * v5) + src_20 * (v8 * src_56) + src_16 * (v3 * v2))
           - (src_24 * (v8 * v3) + src_20 * (v4 * v2) + src_16 * (v5 * src_56));
    dst[4] = (src_12 * (v3 * v2) + src_8 * (v5 * src_60) + v7 * (src_56 * src_44))
           - (src_12 * (v5 * src_56) + src_8 * (v3 * src_44) + v7 * (src_60 * v2));
    dst[5] = (src_12 * (v8 * src_56) + v6 * (src_60 * v2) + src_8 * (v4 * src_44))
           - (src_12 * (v4 * v2) + v6 * (src_56 * src_44) + src_8 * (v8 * src_60));
    dst[6] = (src_12 * (v4 * v5) + v6 * (v3 * src_44) + v7 * (v8 * src_60))
           - (src_12 * (v8 * v3) + v6 * (v5 * src_60) + v7 * (v4 * src_44));
    dst[7] = (v6 * (v5 * src_56) + src_8 * (v8 * v3) + v7 * (v4 * v2))
           - (v6 * (v3 * v2) + src_8 * (v4 * v5) + v7 * (v8 * src_56));
    dst[8] = (src_12 * src_20 * src_56 + v3 * (src_8 * src_28) + (v7 * src_24) * src_60)
           - (v3 * (src_12 * src_24) + (v7 * src_28) * src_56 + (src_8 * src_20) * src_60);
    dst[9] = (v4 * (src_12 * src_24) + (v6 * src_28) * src_56 + (src_8 * src_16) * src_60)
           - (src_12 * src_16 * src_56 + v4 * (src_8 * src_28) + (v6 * src_24) * src_60);
    dst[10] = (src_12 * src_16 * v3 + v4 * (v7 * src_28) + (v6 * src_20) * src_60)
            - (v4 * (src_12 * src_20) + (v6 * src_28) * v3 + (v7 * src_16) * src_60);
    dst[11] = (v4 * (src_8 * src_20) + (v6 * src_24) * v3 + (v7 * src_16) * src_56)
            - (v4 * (v7 * src_24) + (src_8 * src_16) * v3 + (v6 * src_20) * src_56);
    dst[12] = (src_8 * src_20 * src_44 + v5 * (src_12 * src_24) + (v7 * src_28) * v2)
            - ((v7 * src_24) * src_44 + v5 * (src_8 * src_28) + src_12 * src_20 * v2);
    dst[13] = ((v6 * src_24) * src_44 + v8 * (src_8 * src_28) + (src_12 * src_16) * v2)
            - ((src_8 * src_16) * src_44 + v8 * (src_12 * src_24) + (v6 * src_28) * v2);
    dst[14] = (v8 * (src_12 * src_20) + (v6 * src_28) * v5 + (v7 * src_16) * src_44)
            - (src_12 * src_16 * v5 + v8 * (v7 * src_28) + (v6 * src_20) * src_44);
    float v10 = (v8 * (v7 * src_24) + (src_8 * src_16) * v5 + (v6 * src_20) * v2)
              - (v8 * (src_8 * src_20) + (v6 * src_24) * v5 + (v7 * src_16) * v2);
    float v11 = dst[2];
    dst[15] = v10;
    float v12 = 1.0f / (v11 * src_8 + dst[3] * src_12 + v6 * *dst + dst[1] * v7);
    for (int i = 2; i != 0; --i)
    {
        *(v9 - 2) = *(v9 - 2) * v12;
        *(v9 - 1) = *(v9 - 1) * v12;
        *v9 = *v9 * v12;
        v9[1] = v12 * v9[1];
        v9[2] = v9[2] * v12;
        v9[3] = v9[3] * v12;
        v9[4] = v9[4] * v12;
        v9[5] = v12 * v9[5];
        v9 += 8;
    }
}

// ea: 0x004BA530
void nglMatrixCreateXYZ(math::Mat43* mat, math::Dir3* rot, math::Position3* trans)
{
    float sinx = sinf(rot->v.m128_f32[0]);
    float cosx = cosf(rot->v.m128_f32[0]);
    float siny = sinf(rot->v.m128_f32[1]);
    float cosy = cosf(rot->v.m128_f32[1]);
    float sinz = sinf(rot->v.m128_f32[2]);
    float cosz = cosf(rot->v.m128_f32[2]);
    float tx = 0.0f, ty = 0.0f, tz = 0.0f, tw = 0.0f;
    if (trans != nullptr)
    {
        tx = trans->v.m128_f32[0];
        ty = trans->v.m128_f32[1];
        tz = trans->v.m128_f32[2];
        tw = trans->v.m128_f32[3];
    }
    mat->x.v.m128_f32[0] = cosz * cosy;
    mat->x.v.m128_f32[1] = sinz * cosy;
    mat->x.v.m128_f32[2] = 0.0f - siny;
    mat->x.v.m128_f32[3] = 0.0f;
    mat->y.v.m128_f32[0] = cosz * siny * sinx - sinz * cosx;
    mat->y.v.m128_f32[1] = sinz * siny * sinx + cosz * cosx;
    mat->y.v.m128_f32[2] = cosy * sinx;
    mat->y.v.m128_f32[3] = 0.0f;
    mat->z.v.m128_f32[0] = cosz * siny * cosx + sinz * sinx;
    mat->z.v.m128_f32[1] = sinz * siny * cosx - cosz * sinx;
    mat->z.v.m128_f32[2] = cosy * cosx;
    mat->z.v.m128_f32[3] = 0.0f;
    mat->w.v.m128_f32[0] = tx;
    mat->w.v.m128_f32[1] = ty;
    mat->w.v.m128_f32[2] = tz;
    mat->w.v.m128_f32[3] = tw;
}

// ea: 0x004BC4B0
void mat3_t::Transpose(mat3_t& matrix)
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
            matrix.mat[i].x = this->mat[j].x;
    }
}

// ea: 0x004BC540
void mat3_t::Transpose()
{
    float tmp;
    tmp = mat[1].x; mat[1].x = mat[0].y; mat[0].y = tmp;
    tmp = mat[2].x; mat[2].x = mat[0].z; mat[0].z = tmp;
    tmp = mat[2].y; mat[2].y = mat[1].z; mat[1].z = tmp;
}

// ea: 0x004BC6D0
void toQuat(mat3_t* src, quat_t* dst)
{
    static const int next[3] = {1, 2, 0};
    float s = src->mat[1].y + src->mat[0].x + src->mat[2].z;
    if (s <= 0.0f)
    {
        int v3 = src->mat[1].y > src->mat[0].x;
        if (src->mat[2].z > src->mat[v3].x)
            v3 = 2;
        int v4 = next[v3];
        int k = next[v4];
        float* m = &src->mat[0].x;
        float sc = m[3 * v3 + v3] - (m[3 * k + k] + m[3 * v4 + v4]) + 1.0f;
        float sd = sqrtf(sc);
        float* q = &dst->x;
        q[v3] = sd * 0.5f;
        float se = 0.5f / sd;
        dst->w = (m[3 * k + v4] - m[3 * v4 + k]) * se;
        q[v4] = (m[3 * v3 + v4] + m[3 * v4 + v3]) * se;
        q[k] = (m[3 * v3 + k] + m[3 * k + v3]) * se;
    }
    else
    {
        float sa = sqrtf(s + 1.0f);
        dst->w = sa * 0.5f;
        dst->x = (src->mat[2].y - src->mat[1].z) * (0.5f / sa);
        dst->y = (src->mat[0].z - src->mat[2].x) * (0.5f / sa);
        dst->z = (src->mat[1].x - src->mat[0].y) * (0.5f / sa);
    }
}

// ea: 0x004C0700
void toMatrix(const idVec3* src, mat3_t* dst)
{
    angles_t sup;
    sup.pitch = src->x;
    sup.yaw = src->y;
    sup.roll = src->z;
    toMatrix(&sup, dst);
}

// ea: 0x004C0740
void toQuat(angles_t* src, quat_t* dst)
{
    mat3_t temp;
    toMatrix(src, &temp);
    toQuat(&temp, dst);
}

// ea: 0x004C1F00
void gunrandom(float* x, float* y)
{
    float cosT = (rand() / 32768.0f) * 360.0f;
    float r = rand() / 32768.0f;
    float sinT;
    FastSinCos((cosT * 3.1415927f) / 180.0f, &sinT, &cosT);
    *x = cosT * r;
    *y = sinT * r;
}

// ea: 0x004C1F90
void RotateAxisByAxis(float (*axis)[4], int axis_of_rotation, float theta)
{
    float sa, ca;
    FastSinCos(theta, &sa, &ca);
    float* v3 = *axis;
    float* v4 = *axis;
    if (axis_of_rotation != 0)
    {
        if (axis_of_rotation == 1)
        {
            v3 = *axis;
            v4 = *axis + 4;
        }
        else
        {
            if (axis_of_rotation != 2)
                return;
            v3 = *axis;
            v4 = *axis + 8;
        }
    }
    float v5 = sa;
    float v6 = (*v4 * *v4) * (1.0f - ca);
    float v7 = (v4[1] * *v4) * (1.0f - ca);
    float v8 = (v4[1] * (1.0f - ca)) * v4[2];
    float v9 = (*v4 * (1.0f - ca)) * v4[2];
    float v10 = (v4[1] * v4[1]) * (1.0f - ca);
    float v11 = (v4[2] * v4[2]) * (1.0f - ca);
    float sa2 = *v4 * sa;
    float v12 = v4[1] * v5;
    float zsa = v5 * v4[2];
    float v13 = ca;
    *v3 = v6 + ca;
    v3[1] = zsa + v7;
    v3[2] = v9 - v12;
    v3[4] = v7 - zsa;
    float v14 = sa2 + v8;
    float v15 = v8 - sa2;
    v3[5] = v10 + v13;
    v3[6] = v14;
    v3[8] = v12 + v9;
    v3[9] = v15;
    v3[10] = v11 + v13;
}

// ea: 0x004C20C0
void AngleVectors(const float* angles, float* forward, float* right, float* up)
{
    ASSERT("forward && right && up", "c:\\cod\\code\\game\\com_math.cpp", 1064);
    float sy, cy, sp, cp;
    FastSinCos(angles[1] * 0.017453292f, &sy, &cy);
    FastSinCos(*angles * 0.017453292f, &sp, &cp);
    forward[0] = cp * cy;
    forward[1] = cp * sy;
    forward[2] = 0.0f - sp;
    float sr, cr;
    FastSinCos(angles[2] * 0.017453292f, &sr, &cr);
    right[0] = cr * sy - sr * sp * cy;
    right[1] = -(cr * cy) - sr * sp * sy;
    right[2] = -(sr * cp);
    up[0] = cr * sp * cy + sr * sy;
    up[1] = cr * sp * sy - sr * cy;
    up[2] = cr * cp;
}

// ea: 0x004C2260
void AngleVectors(const math::Position3* angles, float* forward, float* right,
                  float* up)
{
    AngleVectors(angles->v.m128_f32, forward, right, up);
}

// ea: 0x004C2270
void AngleVectors(const math::Position3* angles, math::Position3* forward,
                  math::Position3* right, math::Position3* up)
{
    float sy, cy, sp, cp, sr, cr;
    FastSinCos(angles->v.m128_f32[1] * 0.017453292f, &sy, &cy);
    FastSinCos(angles->v.m128_f32[0] * 0.017453292f, &sp, &cp);
    forward->v.m128_f32[0] = cp * cy;
    forward->v.m128_f32[1] = cp * sy;
    forward->v.m128_f32[2] = 0.0f - sp;
    FastSinCos(angles->v.m128_f32[2] * 0.017453292f, &sr, &cr);
    right->v.m128_f32[0] = cr * sy - sr * sp * cy;
    right->v.m128_f32[1] = -(cr * cy) - sr * sp * sy;
    right->v.m128_f32[2] = -(sr * cp);
    up->v.m128_f32[0] = cr * sp * cy + sr * sy;
    up->v.m128_f32[1] = cr * sp * sy - sr * cy;
    up->v.m128_f32[2] = cr * cp;
}

// ea: 0x004C23B0
void YawVectors(float yaw, float* forward, float* right)
{
    float sy;
    FastSinCos(yaw * 0.017453292f, &sy, &yaw);
    if (forward != nullptr)
    {
        *forward = yaw;
        forward[1] = sy;
        forward[2] = 0.0f;
    }
    if (right != nullptr)
    {
        *right = sy;
        right[1] = 0.0f - yaw;
        right[2] = 0.0f;
    }
}

// ea: 0x004C2420
void PerpendicularVector(float* dst, float* src)
{
    ASSERT("src[0] || src[1] || src[2]", "c:\\cod\\code\\game\\com_math.cpp",
           1161);
    int v5 = 0;
    float minelem = 1.0f;
    float v9 = fabsf(*src);
    if (v9 < 1.0f)
    {
        minelem = v9;
    }
    float v8 = fabsf(src[1]);
    if (minelem > v8)
    {
        v5 = 1;
        v8 = fabsf(src[1]);
        minelem = v8;
    }
    float srcb = fabsf(src[2]);
    if (minelem > srcb)
        v5 = 2;
    float tempvec[3] = {0.0f, 0.0f, 0.0f};
    tempvec[v5] = 1.0f;
    ProjectPointOnPlane(dst, tempvec, src);
    float srca = sqrtf(*dst * *dst + dst[1] * dst[1] + dst[2] * dst[2]);
    if (srca != 0.0f)
    {
        float inv = 1.0f / srca;
        *dst = *dst * inv;
        dst[1] = dst[1] * inv;
        dst[2] = dst[2] * inv;
    }
}

// ea: 0x004C25F0
void VectorAngleMultiply(float* vec, float angle)
{
    float sy;
    FastSinCos(angle * 0.017453292f, &sy, &angle);
    float v2 = *vec * angle - sy * vec[1];
    vec[1] = *vec * sy + angle * vec[1];
    *vec = v2;
}

// ea: 0x004C2660
void PitchToQuaternion(float pitch, float* quat)
{
    *quat = 0.0f;
    quat[2] = 0.0f;
    float pitcha = pitch * 0.0087266462f;
    FastSinCos(pitcha, quat + 1, quat + 3);
}

// ea: 0x004C26A0
void YawToQuaternion(float yaw, float* quat)
{
    *quat = 0.0f;
    quat[1] = 0.0f;
    float yawa = yaw * 0.0087266462f;
    FastSinCos(yawa, quat + 2, quat + 3);
}

// ea: 0x004C26E0
void RollToQuaternion(float roll, float* quat)
{
    float rolla = roll * 0.0087266462f;
    quat[1] = 0.0f;
    quat[2] = 0.0f;
    FastSinCos(rolla, quat, quat + 3);
}

// ea: 0x004C2720
void AnglesToAxis(const float* angles, float (*axis)[3])
{
    float right[3];
    AngleVectors(angles, *axis, right, &(*axis)[6]);
    (*axis)[3] = 0.0f - right[0];
    float v2 = 0.0f - right[2];
    (*axis)[4] = 0.0f - right[1];
    (*axis)[5] = v2;
}

// ea: 0x004C2770
void AnglesToAxis(const math::Position3* angles, float (*axis)[3])
{
    float right[3];
    AngleVectors(angles->v.m128_f32, *axis, right, &(*axis)[6]);
    (*axis)[3] = 0.0f - right[0];
    float v2 = 0.0f - right[2];
    (*axis)[4] = 0.0f - right[1];
    (*axis)[5] = v2;
}

// ea: 0x004C27C0
void AnglesToAxis(const math::Position3* angles, const math::Position3* origin,
                  math::Mat43* mat)
{
    float sy, cy, sp, cp, sr, cr;
    FastSinCos(angles->v.m128_f32[1] * 0.017453292f, &sy, &cy);
    FastSinCos(angles->v.m128_f32[0] * 0.017453292f, &sp, &cp);
    FastSinCos(angles->v.m128_f32[2] * 0.017453292f, &sr, &cr);
    mat->x.v.m128_f32[0] = cp * cy;
    mat->x.v.m128_f32[1] = cp * sy;
    mat->x.v.m128_f32[2] = 0.0f - sp;
    mat->x.v.m128_f32[3] = 0.0f;
    mat->y.v.m128_f32[0] = sr * sp * cy - cr * sy;
    mat->y.v.m128_f32[1] = sr * sp * sy + cr * cy;
    mat->y.v.m128_f32[2] = sr * cp;
    mat->y.v.m128_f32[3] = 0.0f;
    mat->z.v.m128_f32[0] = cr * sp * cy + sr * sy;
    mat->z.v.m128_f32[1] = cr * sp * sy - sr * cy;
    mat->z.v.m128_f32[2] = cr * cp;
    mat->z.v.m128_f32[3] = 0.0f;
    mat->w.v.m128_f32[0] = origin->v.m128_f32[0];
    mat->w.v.m128_f32[1] = origin->v.m128_f32[1];
    mat->w.v.m128_f32[2] = origin->v.m128_f32[2];
    mat->w.v.m128_f32[3] = origin->v.m128_f32[3];
}

// ?AnglesToAxis@@YAXABVPosition3@math@@0AAVMat43@2@@Z (core.o reference form)
void AnglesToAxis(const math::Position3& angles, const math::Position3& origin,
                  math::Mat43& mat)
{
    AnglesToAxis(&angles, &origin, &mat);
}

// ea: 0x004C29C0
void AnglesToAxis(const math::Position3* angles, const math::Position3* origin,
                  math::Mat44* out)
{
    float sy, cy, sp, cp, sr, cr;
    FastSinCos(angles->v.m128_f32[1] * 0.017453292f, &sy, &cy);
    FastSinCos(angles->v.m128_f32[0] * 0.017453292f, &sp, &cp);
    FastSinCos(angles->v.m128_f32[2] * 0.017453292f, &sr, &cr);
    out->x.v.m128_f32[0] = cp * cy;
    out->x.v.m128_f32[1] = cp * sy;
    out->x.v.m128_f32[2] = 0.0f - sp;
    out->x.v.m128_f32[3] = 0.0f;
    out->y.v.m128_f32[0] = sr * sp * cy - cr * sy;
    out->y.v.m128_f32[1] = sr * sp * sy + cr * cy;
    out->y.v.m128_f32[2] = sr * cp;
    out->y.v.m128_f32[3] = 0.0f;
    out->z.v.m128_f32[0] = cr * sp * cy + sr * sy;
    out->z.v.m128_f32[1] = cr * sp * sy - sr * cy;
    out->z.v.m128_f32[2] = cr * cp;
    out->z.v.m128_f32[3] = 0.0f;
    out->w.v.m128_f32[0] = origin->v.m128_f32[0];
    out->w.v.m128_f32[1] = origin->v.m128_f32[1];
    out->w.v.m128_f32[2] = origin->v.m128_f32[2];
    out->w.v.m128_f32[3] = origin->v.m128_f32[3];
}

// ea: 0x004C2B70
void YawToAxis(float yaw, float (*axis)[3])
{
    float psin;
    FastSinCos(yaw * 0.017453292f, &psin, &yaw);
    if (axis != nullptr)
    {
        (*axis)[0] = yaw;
        (*axis)[1] = psin;
        (*axis)[2] = 0.0f;
    }
    (*axis)[8] = 1.0f;
    (*axis)[3] = 0.0f - psin;
    (*axis)[6] = 0.0f;
    (*axis)[7] = 0.0f;
    (*axis)[4] = 0.0f - (0.0f - yaw);
    (*axis)[5] = 0.0f - 0.0f;
}

// ea: 0x004C2C00
void AxisToAngles(const float (*axis)[3], float* angles)
{
    vectoangles((float*)axis[0], angles);
    float v4 = -(angles[1] * 3.1415927f) / 180.0f;
    float right = axis[1][0];
    float v8 = axis[1][1];
    float v9 = axis[1][2];
    float fSin;
    FastSinCos(v4, &fSin, (float*)*axis);
    float v5 = -(*angles * 3.1415927f) / 180.0f;
    float temp = (*axis)[0] * right - fSin * v8;
    v8 = (*axis)[0] * v8 + fSin * right;
    FastSinCos(v5, &fSin, (float*)*axis);
    right = temp * (*axis)[0] + fSin * v9;
    v9 = (*axis)[0] * v9 - temp * fSin;
    float ang = vectosignedpitch(&right);
    if (v8 >= 0.0f)
    {
        angles[2] = 0.0f - ang;
    }
    else
    {
        float v6 = 180.0f;
        if (ang >= 0.0f)
            v6 = -180.0f;
        angles[2] = v6 + ang;
    }
}

// ea: 0x004C2D50
void Axis4ToAngles(const float (*axis)[4], float* angles)
{
    vectoangles((float*)axis[0], angles);
    float v4 = -(angles[1] * 3.1415927f) / 180.0f;
    float right = axis[1][0];
    float v8 = axis[1][1];
    float v9 = axis[1][2];
    float fSin;
    FastSinCos(v4, &fSin, (float*)*axis);
    float v5 = -(*angles * 3.1415927f) / 180.0f;
    float temp = (*axis)[0] * right - fSin * v8;
    v8 = (*axis)[0] * v8 + fSin * right;
    FastSinCos(v5, &fSin, (float*)*axis);
    right = temp * (*axis)[0] + fSin * v9;
    v9 = (*axis)[0] * v9 - temp * fSin;
    float ang = vectosignedpitch(&right);
    if (v8 >= 0.0f)
    {
        angles[2] = 0.0f - ang;
    }
    else
    {
        float v6 = 180.0f;
        if (ang >= 0.0f)
            v6 = -180.0f;
        angles[2] = v6 + ang;
    }
}

// ea: 0x004C2EA0
void AxisToSignedAngles(const float (*axis)[3], float* angles)
{
    vectosignedangles((float*)axis[0], angles);
    float v4 = -(angles[1] * 3.1415927f) / 180.0f;
    float right[3];
    right[0] = axis[1][0];
    right[1] = axis[1][1];
    right[2] = axis[1][2];
    float fSin;
    FastSinCos(v4, &fSin, (float*)*axis);
    float v5 = -(*angles * 3.1415927f) / 180.0f;
    float temp = (*axis)[0] * right[0] - fSin * right[1];
    right[1] = (*axis)[0] * right[1] + fSin * right[0];
    FastSinCos(v5, &fSin, (float*)*axis);
    right[0] = temp * (*axis)[0] + fSin * right[2];
    right[2] = (*axis)[0] * right[2] - temp * fSin;
    float ang = vectosignedpitch(right);
    if (right[1] >= 0.0f)
    {
        angles[2] = 0.0f - ang;
    }
    else
    {
        float v6 = 180.0f;
        if (ang >= 0.0f)
            v6 = -180.0f;
        angles[2] = v6 + ang;
    }
}

// ea: 0x004C2FF0
void VectorRotateAngles(float* vIn, const float* vRotation, float* out)
{
    static const int nIndex[3][2] = {{1, 2}, {2, 0}, {0, 1}};
    float vWork[3] = {vIn[0], vIn[1], vIn[2]};
    for (int v7 = 0; v7 < 3; ++v7)
    {
        float v8 = vRotation[v7];
        if (v8 != 0.0f)
        {
            float radians = (v8 * 3.1415927f) / 180.0f;
            float s, c;
            FastSinCos(radians, &s, &c);
            int v9 = nIndex[v7][0];
            int v11 = nIndex[v7][1];
            float t0 = vWork[v9] * c - vWork[v11] * s;
            float t1 = vWork[v9] * s + vWork[v11] * c;
            vWork[v9] = t0;
            vWork[v11] = t1;
        }
    }
    out[0] = vWork[0];
    out[1] = vWork[1];
    out[2] = vWork[2];
}

// ea: 0x004C3170
void VectorRotateAnglesAroundPoint(const float* vIn, const float* vRotation,
                                   const float* vOrigin, float* out)
{
    float vDelta[3];
    vDelta[0] = *vIn - *vOrigin;
    vDelta[1] = vIn[1] - vOrigin[1];
    vDelta[2] = vIn[2] - vOrigin[2];
    float vRotatedDelta[3];
    VectorRotateAngles(vDelta, vRotation, vRotatedDelta);
    *out = *vOrigin + vRotatedDelta[0];
    out[1] = vOrigin[1] + vRotatedDelta[1];
    out[2] = vOrigin[2] + vRotatedDelta[2];
}

// ea: 0x004C31F0
void VectorPolar(float* v, float radius, float theta, float phi)
{
    float st, ct, psin, cp;
    FastSinCos(theta, &st, &ct);
    FastSinCos(phi, &psin, &cp);
    v[1] = (cp * st) * radius;
    *v = (cp * ct) * radius;
    v[2] = psin * radius;
}

// ea: 0x004C3260
float PitchForYawOnNormal(float fYaw, const float* vNormal)
{
    ASSERT("vNormal[0] || vNormal[1] || vNormal[2]",
           "c:\\cod\\code\\game\\com_math.cpp", 3207);
    float vForward[3];
    float vProjected[3];
    float sy, cy;
    FastSinCos(fYaw * 0.017453292f, &sy, &cy);
    vForward[0] = cy;
    vForward[1] = sy;
    vForward[2] = 0.0f;
    ProjectPointOnPlane(vProjected, vForward, vNormal);
    return vectopitch(vProjected);
}

// ea: 0x004C3340
void make_rotate(math::Mat43* mat, int axis_of_rotation, float theta)
{
    float sa, ca;
    FastSinCos(theta, &sa, &ca);
    math::Dir3 v16;
    math::Mat43* v4 = mat;
    if (axis_of_rotation == 0)
    {
        v16 = mat->x;
    }
    else if (axis_of_rotation == 1)
    {
        v16 = mat->y;
    }
    else if (axis_of_rotation == 2)
    {
        v16 = mat->z;
    }
    else
    {
        return;
    }
    float v7 = v16.v.m128_f32[1];
    float v8 = v16.v.m128_f32[2];
    float yy = (v8 * v16.v.m128_f32[0]) * (1.0f - ca);
    float u_12 = (v7 * v7) * (1.0f - ca);
    float s0 = v16.v.m128_f32[0] * sa;
    float v9 = (v7 * v16.v.m128_f32[0]) * (1.0f - ca);
    float v10 = (v8 * v7) * (1.0f - ca);
    float v11 = (v8 * v8) * (1.0f - ca);
    float v12 = v8 * sa;
    float v13 = v7 * sa;
    float v14 = ca;
    v4->x.v.m128_f32[0] = (v16.v.m128_f32[0] * v16.v.m128_f32[0]) * (1.0f - ca) + ca;
    v4->x.v.m128_f32[1] = v12 + v9;
    v4->x.v.m128_f32[2] = yy - v13;
    v4->x.v.m128_f32[3] = 0.0f;
    v4->y.v.m128_f32[0] = v9 - v12;
    v4->y.v.m128_f32[1] = u_12 + v14;
    v4->y.v.m128_f32[2] = s0 + v10;
    v4->y.v.m128_f32[3] = 0.0f;
    v4->z.v.m128_f32[0] = v13 + yy;
    v4->z.v.m128_f32[1] = v10 - s0;
    v4->z.v.m128_f32[2] = v11 + v14;
    v4->z.v.m128_f32[3] = 0.0f;
}

// ea: 0x004C85C0
void RotatePointAroundVector(float* dst, float* dir, const float* point,
                             float degrees)
{
    ASSERT("dir[0] || dir[1] || dir[2]", "c:\\cod\\code\\game\\com_math.cpp",
           745);
    float rad = degrees * 3.1415927f / 180.0f;
    float rot, psin;
    FastSinCos(rad, &psin, &rot);
    float vup[3];
    PerpendicularVector(vup, dir);
    float vright[3];
    CrossProduct(vup, dir, vright);
    // tmpmat rows: vup, vright, dir
    float tmpmat[3][3];
    tmpmat[0][0] = vup[0];
    tmpmat[0][1] = vup[1];
    tmpmat[0][2] = vup[2];
    tmpmat[1][0] = vright[0];
    tmpmat[1][1] = vright[1];
    tmpmat[1][2] = vright[2];
    tmpmat[2][0] = dir[0];
    tmpmat[2][1] = dir[1];
    tmpmat[2][2] = dir[2];
    float im[3][3];
    memcpy(im, tmpmat, sizeof(im));
    // rotate point into im basis
    float vf = *dir;
    float v40 = dir[1];
    float v41 = dir[2];
    float rotpt[3];
    rotpt[0] = im[0][0] * *point + im[0][1] * point[1] + im[0][2] * point[2];
    rotpt[1] = im[1][0] * *point + im[1][1] * point[1] + im[1][2] * point[2];
    rotpt[2] = im[2][0] * *point + im[2][1] * point[1] + im[2][2] * point[2];
    // zrot (rotation about z = dir)
    float zx = rot * rotpt[0] - psin * rotpt[1];
    float zy = psin * rotpt[0] + rot * rotpt[1];
    float zz = rotpt[2];
    // transform back
    *dst = im[0][0] * zx + im[1][0] * zy + im[2][0] * zz;
    dst[1] = im[0][1] * zx + im[1][1] * zy + im[2][1] * zz;
    dst[2] = im[0][2] * zx + im[1][2] * zy + im[2][2] * zz;
    (void)vf; (void)v40; (void)v41;
}

// ea: 0x004C8BA0
void RotateAroundDirection(float (*axis)[3], float yaw)
{
    float* v2 = &(*axis)[3];
    PerpendicularVector(&(*axis)[3], *axis);
    if (yaw != 0.0f)
    {
        float temp[3];
        temp[0] = *v2;
        temp[1] = (*axis)[4];
        temp[2] = (*axis)[5];
        RotatePointAroundVector(v2, *axis, temp, yaw);
    }
    (*axis)[6] = (*axis)[5] * (*axis)[1] - (*axis)[2] * (*axis)[4];
    (*axis)[7] = (*axis)[2] * (*axis)[3] - (*axis)[5] * (*axis)[0];
    (*axis)[8] = (*axis)[4] * (*axis)[0] - (*axis)[1] * (*axis)[3];
}

// Fast acos approximation (shared by the vecto* angle helpers; polynomial
// constants from the release com_math.cpp).
static float FastACos(float x)
{
    if (x >= 0.5f)
    {
        float t = sqrtf(fabsf((1.0f - x) * 0.5f));
        return t * t * t * t * t * t * -0.1079625f
             - t * t * t * t * 0.15000001f
             - t * t * t * 0.33333331f
             - t * 2.0f
             + 1.570796f;
    }
    return x * x * x * 0.1666667f
         + x * x * x * x * x * 0.075000003f
         + x * x * x * x * x * x * x * 0.053981241f
         + x;
}

// ea: 0x004BE010
float vectoyaw(float* vec)
{
    if (vec[1] == 0.0f && *vec == 0.0f)
        return 0.0f;
    float v2 = vec[1];
    float yaw = *vec;
    float v16 = fabsf(yaw);
    float v17 = fabsf(v2);
    float v3 = 0.0f;
    if (v17 + v16 != 0.0f)
    {
        float v10 = sqrtf(v2 * v2 + yaw * yaw);
        float v11 = 1.0f / v10;
        if (v17 <= v16)
        {
            float v6 = v11 * v17;
            v3 = FastACos(v6);
        }
        else
        {
            float v4 = v11 * v16;
            float v5 = FastACos(v4);
            v3 = 1.5707964f - v5;
        }
        if (yaw < 0.0f)
            v3 = 3.1415927f - v3;
        if (v2 < 0.0f)
            v3 = 0.0f - v3;
    }
    float v7 = (v3 * 180.0f) / 3.1415927f;
    float yawa = v7;
    if (v7 < 0.0f)
        return v7 + 360.0f;
    return yawa;
}

// ea: 0x004BE300
float vectosignedyaw(float* vec)
{
    if (vec[1] == 0.0f && *vec == 0.0f)
        return 0.0f;
    float v3 = vec[1];
    float yaw = *vec;
    float v15 = fabsf(yaw);
    float v16 = fabsf(v3);
    float v1 = 0.0f;
    if (v16 + v15 != 0.0f)
    {
        float v9 = sqrtf(v3 * v3 + yaw * yaw);
        float v10 = 1.0f / v9;
        if (v16 <= v15)
        {
            float v6 = v10 * v16;
            v1 = FastACos(v6);
        }
        else
        {
            float v4 = v10 * v15;
            float v5 = FastACos(v4);
            v1 = 1.5707964f - v5;
        }
        if (yaw < 0.0f)
            v1 = 3.1415927f - v1;
        if (v3 < 0.0f)
            v1 = 0.0f - v1;
    }
    float yawa = (v1 * 180.0f) / 3.1415927f;
    ASSERT("yaw >= -180", "c:\\cod\\code\\game\\com_math.cpp", 923);
    ASSERT("yaw <= 180", "c:\\cod\\code\\game\\com_math.cpp", 924);
    return yawa;
}

// ea: 0x004BE670
float vectopitch(const float* vec)
{
    if (vec[1] == 0.0f && *vec == 0.0f)
    {
        if (vec[2] <= 0.0f)
            return 90.0f;
        return 270.0f;
    }
    float pitch = sqrtf(vec[1] * vec[1] + *vec * *vec);
    float v18 = vec[2];
    float v16 = fabsf(pitch);
    float v17 = fabsf(v18);
    float v3 = 0.0f;
    if (v17 + v16 != 0.0f)
    {
        float v10 = sqrtf(v18 * v18 + pitch * pitch);
        float v11 = 1.0f / v10;
        if (v17 <= v16)
        {
            float v6 = v11 * v17;
            v3 = FastACos(v6);
        }
        else
        {
            float v4 = v11 * v16;
            float v5 = FastACos(v4);
            v3 = 1.5707964f - v5;
        }
        if (pitch < 0.0f)
            v3 = 3.1415927f - v3;
        if (v18 < 0.0f)
            v3 = 0.0f - v3;
    }
    float v7 = (v3 * -180.0f) / 3.1415927f;
    float pitcha = v7;
    if (v7 < 0.0f)
        return v7 + 360.0f;
    return pitcha;
}

// ea: 0x004BE9B0
float vectosignedpitch(const float* vec)
{
    if (vec[1] == 0.0f && *vec == 0.0f)
    {
        if (vec[2] <= 0.0f)
            return 90.0f;
        return -90.0f;
    }
    float pitch = sqrtf(vec[1] * vec[1] + *vec * *vec);
    float v17 = vec[2];
    float v15 = fabsf(pitch);
    float v16 = fabsf(v17);
    float v3 = 0.0f;
    if (v16 + v15 != 0.0f)
    {
        float v9 = sqrtf(v17 * v17 + pitch * pitch);
        float v10 = 1.0f / v9;
        if (v16 <= v15)
        {
            float v6 = v10 * v16;
            v3 = FastACos(v6);
        }
        else
        {
            float v4 = v10 * v15;
            float v5 = FastACos(v4);
            v3 = 1.5707964f - v5;
        }
        if (pitch < 0.0f)
            v3 = 3.1415927f - v3;
        if (v17 < 0.0f)
            v3 = 0.0f - v3;
    }
    return (v3 * -180.0f) / 3.1415927f;
}

// ea: 0x004BECE0
void vectoangles(float* vec, float* angles)
{
    float v3;
    if (vec[1] == 0.0f && *vec == 0.0f)
    {
        float yaw = 0.0f;
        if (vec[2] <= 0.0f)
            v3 = 90.0f;
        else
            v3 = 270.0f;
        angles[0] = v3;
        angles[1] = yaw;
        angles[2] = 0.0f;
        return;
    }
    float v4 = vec[1];
    float forward = *vec;
    float v34 = fabsf(forward);
    float yawa = fabsf(v4);
    float v5 = 0.0f;
    if (yawa + v34 != 0.0f)
    {
        float v17 = sqrtf(v4 * v4 + forward * forward);
        float v30 = 1.0f / v17;
        float v7;
        if (yawa <= v34)
        {
            float v8 = v30 * yawa;
            v7 = FastACos(v8);
        }
        else
        {
            float v6 = v30 * v34;
            float v9 = FastACos(v6);
            v7 = 1.5707964f - v9;
        }
        if (forward < 0.0f)
            v7 = 3.1415927f - v7;
        if (v4 < 0.0f)
            v7 = 0.0f - v7;
        v5 = v7;
    }
    float v10 = (v5 * 180.0f) / 3.1415927f;
    float yaw = v10;
    if (v10 < 0.0f)
        yaw = v10 + 360.0f;
    float forwarda = sqrtf(vec[1] * vec[1] + *vec * *vec);
    float v36 = vec[2];
    float v35 = fabsf(forwarda);
    float v31 = fabsf(v36);
    float v11 = 0.0f;
    if (v31 + v35 != 0.0f)
    {
        float v24 = sqrtf(v36 * v36 + forwarda * forwarda);
        float v25 = 1.0f / v24;
        if (v31 <= v35)
        {
            float v14 = v25 * v31;
            v11 = FastACos(v14);
        }
        else
        {
            float v12 = v25 * v35;
            float v13 = FastACos(v12);
            v11 = 1.5707964f - v13;
        }
        if (forwarda < 0.0f)
            v11 = 3.1415927f - v11;
        if (v36 < 0.0f)
            v11 = 0.0f - v11;
    }
    v3 = (v11 * -180.0f) / 3.1415927f;
    if (v3 < 0.0f)
        v3 = v3 + 360.0f;
    angles[0] = v3;
    angles[1] = yaw;
    angles[2] = 0.0f;
}

// ea: 0x004BF2C0
void vectoangles(const float* vec, math::Position3* angles)
{
    float tmp[3];
    vectoangles((float*)vec, tmp);
    angles->v.m128_f32[0] = tmp[0];
    angles->v.m128_f32[1] = tmp[1];
    angles->v.m128_f32[2] = tmp[2];
}

// ea: 0x004BF300
void vectosignedangles(float* vec, float* angles)
{
    float v5;
    if (vec[1] == 0.0f && *vec == 0.0f)
    {
        float v4 = 0.0f;
        if (vec[2] <= 0.0f)
            v5 = 90.0f;
        else
            v5 = -90.0f;
        angles[0] = v5;
        angles[1] = v4;
        angles[2] = 0.0f;
        return;
    }
    float v6 = vec[1];
    float forward = *vec;
    float v30 = fabsf(forward);
    float v32 = fabsf(v6);
    float v2 = 0.0f;
    if (v32 + v30 != 0.0f)
    {
        float v18 = sqrtf(v6 * v6 + forward * forward);
        float v19 = 1.0f / v18;
        float v8;
        if (v32 <= v30)
        {
            float v9 = v19 * v32;
            v8 = FastACos(v9);
        }
        else
        {
            float v7 = v19 * v30;
            float v10 = FastACos(v7);
            v8 = 1.5707964f - v10;
        }
        if (forward < 0.0f)
            v8 = 3.1415927f - v8;
        if (v6 < 0.0f)
            v8 = 0.0f - v8;
        v2 = v8;
    }
    float yaw = (v2 * 180.0f) / 3.1415927f;
    float forwarda = sqrtf(vec[1] * vec[1] + *vec * *vec);
    float v35 = vec[2];
    float v31 = fabsf(forwarda);
    float v25 = fabsf(v35);
    float v11 = 0.0f;
    if (v25 + v31 != 0.0f)
    {
        float v33 = sqrtf(v35 * v35 + forwarda * forwarda);
        float v34 = 1.0f / v33;
        if (v25 <= v31)
        {
            float v14 = v34 * v25;
            v11 = FastACos(v14);
        }
        else
        {
            float v12 = v34 * v31;
            float v13 = FastACos(v12);
            v11 = 1.5707964f - v13;
        }
        if (forwarda < 0.0f)
            v11 = 3.1415927f - v11;
        if (v35 < 0.0f)
            v11 = 0.0f - v11;
    }
    float v4 = yaw;
    v5 = (v11 * -180.0f) / 3.1415927f;
    angles[0] = v5;
    angles[1] = v4;
    angles[2] = 0.0f;
}

// ea: 0x004BF990
float RotationToYaw(const float* rot)
{
    float v2 = *rot * *rot;
    float r = rot[1] * rot[1] + v2;
    ASSERT("r", "c:\\cod\\code\\game\\com_math.cpp", 1874);
    float ra = 1.0f - (2.0f / r) * v2;
    float v19 = (rot[1] * *rot) * (2.0f / r);
    float v17 = fabsf(ra);
    float zz = fabsf(v19);
    if (zz + v17 == 0.0f)
        return 0.0f * 57.295776f;
    float v11 = sqrtf(v19 * v19 + ra * ra);
    float v12 = 1.0f / v11;
    float v7;
    if (zz <= v17)
    {
        float v8 = v12 * zz;
        v7 = FastACos(v8);
    }
    else
    {
        float v5 = v12 * v17;
        float v6 = FastACos(v5);
        v7 = 1.5707964f - v6;
    }
    if (ra < 0.0f)
        v7 = 3.1415927f - v7;
    if (v19 < 0.0f)
        v7 = 0.0f - v7;
    return v7 * 57.295776f;
}

// ea: 0x004C8C60
int BoxOnPlaneSide(const float* emins, const float* emaxs, const cplane_s* p)
{
    float dist1, dist2;
    switch (p->signbits)
    {
    case 0:
        dist1 = p->dist - emins[0] * p->normal[0] - emins[1] * p->normal[1] - emins[2] * p->normal[2];
        dist2 = p->dist - emaxs[0] * p->normal[0] - emaxs[1] * p->normal[1] - emaxs[2] * p->normal[2];
        break;
    case 1:
        dist1 = p->dist - emaxs[0] * p->normal[0] - emins[1] * p->normal[1] - emins[2] * p->normal[2];
        dist2 = p->dist - emins[0] * p->normal[0] - emaxs[1] * p->normal[1] - emaxs[2] * p->normal[2];
        break;
    case 2:
        dist1 = p->dist - emins[0] * p->normal[0] - emaxs[1] * p->normal[1] - emins[2] * p->normal[2];
        dist2 = p->dist - emaxs[0] * p->normal[0] - emins[1] * p->normal[1] - emaxs[2] * p->normal[2];
        break;
    case 3:
        dist1 = p->dist - emaxs[0] * p->normal[0] - emaxs[1] * p->normal[1] - emins[2] * p->normal[2];
        dist2 = p->dist - emins[0] * p->normal[0] - emins[1] * p->normal[1] - emaxs[2] * p->normal[2];
        break;
    case 4:
        dist1 = p->dist - emins[0] * p->normal[0] - emins[1] * p->normal[1] - emaxs[2] * p->normal[2];
        dist2 = p->dist - emaxs[0] * p->normal[0] - emaxs[1] * p->normal[1] - emins[2] * p->normal[2];
        break;
    case 5:
        dist1 = p->dist - emaxs[0] * p->normal[0] - emins[1] * p->normal[1] - emaxs[2] * p->normal[2];
        dist2 = p->dist - emins[0] * p->normal[0] - emaxs[1] * p->normal[1] - emins[2] * p->normal[2];
        break;
    case 6:
        dist1 = p->dist - emins[0] * p->normal[0] - emaxs[1] * p->normal[1] - emaxs[2] * p->normal[2];
        dist2 = p->dist - emaxs[0] * p->normal[0] - emins[1] * p->normal[1] - emins[2] * p->normal[2];
        break;
    case 7:
        dist1 = p->dist - emaxs[0] * p->normal[0] - emaxs[1] * p->normal[1] - emaxs[2] * p->normal[2];
        dist2 = p->dist - emins[0] * p->normal[0] - emins[1] * p->normal[1] - emins[2] * p->normal[2];
        break;
    default:
        Com_Error(1, "BoxOnPlaneSide: bad signbits");
        return 3;
    }
    if (dist1 >= 0.0f)
        return 1;
    if (dist2 < 0.0f)
        return 2;
    return 3;
}
