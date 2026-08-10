// ============================================================================
// g_camerashake.cpp - camera shake + noise system (game2.o CameraShake.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_camerashake.h"
#include "game/logic/g_local.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace AeAssert {
extern bool IsIgnored();
extern bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                       \
    do {                                                               \
        if (!AeAssert::IsIgnored()                                     \
            && AeAssert::Assert("old cod assert"))                     \
            __debugbreak();                                            \
    } while (0)

extern int currCl;                       // ?currCl
extern int cgGlobal_time;                // cgGlobal.time
extern NoiseManager g_noise;             // ?g_noise (game2.o)
extern const math::Mat43& nglGetMatrix_ViewToWorld(void* Scene);
extern void* nglBuildScene;
extern void StartCameraShake_glue(int type, void* worldPos, float size,
                                  float timeOverride, float nextDelay);
extern int cgGlobal_time;             // ?cgGlobal@@3UcgGlobal_t@@A
extern float g_ShakeTestMag;          // ?g_ShakeTestMag@@3MA (game2.o)
extern float g_ShakeTestFreq;         // ?g_ShakeTestFreq@@3MA (game2.o)
extern float g_ShakeTestTime;         // ?g_ShakeTestTime@@3MA (game2.o)
extern int g_ShakeTest2d;             // ?g_ShakeTest2d@@3HA (game2.o)

// ============================================================================
// NoiseManager tables
// ============================================================================
int NoiseManager::m_initialised = 0;
int g_noiseP[256];
float g_noiseG[256];
int dword_F037B4[256];
int dword_F03BB0[256];
int dword_F03BB4[256];
int dword_F03BB8[256];
int dword_F03BBC[256];
int dword_F03BC0[256];
int dword_F03BC4[256];
float dword_F04048[256];
float dword_F0404C[256];
float dword_F04050[256];
float dword_F04054[256];
float dword_F04058[256];
float dword_F0405C[256];
float dword_F04448[256];
float dword_F0444C[256];
float dword_F04450[256];
float dword_F04454[256];
float dword_F04458[256];
float dword_F0445C[256];

CameraShake g_cameraShake[4];
CameraShakeType shakeTable[];

// ============================================================================
// NoiseManager
// ============================================================================

// ea: 0x4F55A0
void NoiseManager::Init()
{
    if (m_initialised == 0)
    {
        m_initialised = 1;
        for (int i = 0; i < 256; ++i)
        {
            g_noiseP[i] = i;
            g_noiseG[i] = (float)(rand() % 512 - 256) * 0.00390625f;
        }
        for (int j = 255; j != 0; g_noiseP[0] = 0)
        {
            int v3 = g_noiseP[j];
            int v4 = rand() % 256;
            dword_F037B4[--j] = g_noiseP[v4];
            g_noiseP[v4] = v3;
        }
        for (int k = 0; k < 258; k += 6)
        {
            dword_F03BB0[k] = g_noiseP[k];
            dword_F04448[k] = g_noiseG[k];
            dword_F03BB4[k] = dword_F037B4[k];
            dword_F0444C[k] = dword_F0404C[k];
            dword_F03BB8[k] = dword_F037B8[k];
            dword_F04450[k] = dword_F04050[k];
            dword_F03BBC[k] = dword_F037BC[k];
            dword_F04454[k] = dword_F04054[k];
            dword_F03BC0[k] = dword_F037C0[k];
            dword_F04458[k] = dword_F04058[k];
            dword_F03BC4[k] = dword_F037C4[k];
            dword_F0445C[k] = dword_F0405C[k];
        }
    }
}

// ea: 0x4F56E0
float NoiseManager::Noise(float arg)
{
    if (m_initialised == 0)
        Init();
    int idx = (int)(arg + 4096.0f);
    float rx0 = (arg + 4096.0f) - (float)idx;
    float v3 = g_noiseG[g_noiseP[idx]];
    float t = rx0;
    float y0 = v3 * rx0;
    float y1 = (rx0 - 1.0f) * g_noiseG[g_noiseP[(idx + 1)]];
    float v = (y1 - y0) * ((3.0f - (t + t)) * rx0 * rx0) + y0;
    return v;
}

// ea: 0x4F5770
void NoiseManager::Normalize2(float* v)
{
    float s = sqrtf(v[0] * v[0] + v[1] * v[1]);
    float inv = 1.0f / s;
    float y = inv * v[1];
    v[0] = v[0] * inv;
    v[1] = y;
}

// ea: 0x4F57C0
void NoiseManager::Normalize3(float* v)
{
    float s = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    float inv = 1.0f / s;
    v[0] = v[0] * inv;
    v[1] = inv * v[1];
    v[2] = inv * v[2];
}

// ea: 0x4F5820
float NoiseManager::GetElapsedTime()
{
    return (float)cgGlobal_time * 0.001f;
}

// ea: 0x4F5830
float NoiseFloat::GetValue()
{
    if (m_seed < 0.0f || m_seed > 16384.0f)
        m_seed = (float)(rand() % 0x4000);
    float t = ((float)cgGlobal_time * 0.001f) * m_freq_mult + m_seed;
    float n = 0.0f;
    float mult = 1.0f;
    for (unsigned int octave = 1; octave <= m_num_octaves;
         mult = mult * 0.5f, ++octave)
    {
        float v7 = (float)octave * t;
        if (NoiseManager::m_initialised == 0)
            g_noise.Init();
        int idx = (int)(v7 + 4096.0f);
        float rx0 = (v7 + 4096.0f) - (float)idx;
        float v5 = g_noiseG[g_noiseP[idx]] * rx0;
        float y1 = (rx0 - 1.0f) * g_noiseG[g_noiseP[(idx + 1)]];
        n = ((((y1 - v5) * (((3.0f - (rx0 * 2.0f)) * rx0) * rx0)) + v5)
             * mult) + n;
    }
    return n * m_range;
}

// ============================================================================
// MsaQuat
// ============================================================================

// ea: 0x4F5190
MsaQuat::MsaQuat(float fX, float fY, float fZ, float fW)
    : x(fX), y(fY), z(fZ), w(fW)
{
}

// ea: 0x4F51C0
MsaQuat::MsaQuat(const MsaQuat& qQuat)
{
    *this = qQuat;
}

// ea: 0x4F51F0
void MsaQuat::Multiply(const MsaQuat& other, MsaQuat& out)
{
    float v3 = ((other.x * w) + (x * other.w) + (other.y * z)) - (other.z * y);
    float v4 = ((w * other.y) + (y * other.w) + (other.z * x)) - (other.x * z);
    float v5 = ((other.x * y) + (other.z * w) + (other.w * z)) - (x * other.y);
    out.w = ((w * other.w) - (other.x * x) + (other.z * z)) + (y * other.y);
    out.x = v3;
    out.y = v4;
    out.z = v5;
}

// ea: 0x4F52E0
float MsaQuat::Normalise()
{
    float length = sqrtf(x * x + y * y + z * z + w * w);
    float inv = 1.0f / length;
    x = inv * x;
    y = y * inv;
    z = inv * z;
    w = inv * w;
    return length;
}

// ea: 0x4F5570
float MsaQuat::Magnitude() const
{
    return sqrtf(x * x + y * y + z * z + w * w);
}

// ea: 0x4F5360
void MsaQuat::QuaternionToMatrix(math::Position3* pos, math::Mat43* mat)
{
    float x2 = x * x;
    float z2 = z * z;
    float y2 = y * y;
    float v6 = 2.0f / ((w * w) + x2 + y2 + z2);
    float v17 = x2 * v6;
    float v7 = (y * x) * v6;
    float v8 = (y * v6) * w;
    float v9 = (x * v6) * z;
    float v10 = (x * v6) * w;
    float v11 = y2 * v6;
    float v12 = (y * v6) * z;
    float v16 = (v6 * w) * z;
    float rx = 1.0f - ((z2 * v6) + v11);
    float ry = v16 + v7;
    float rz = v9 - v8;
    mat->x.v = _mm_set_ps(0.0f, rz, ry, rx);
    float yx = v7 - v16;
    float yy = 1.0f - ((z2 * v6) + v17);
    float yz = v12 + v10;
    mat->y.v = _mm_set_ps(0.0f, yz, yy, yx);
    float zx = v8 + v9;
    float zy = v12 - v10;
    float zz = 1.0f - (v11 + v17);
    mat->z.v = _mm_set_ps(0.0f, zz, zy, zx);
    if (pos != nullptr)
    {
        mat->w.v = _mm_set_ps(pos->v.m128_f32[3], pos->v.m128_f32[2],
                              pos->v.m128_f32[1], pos->v.m128_f32[0]);
    }
    else
    {
        mat->w.v = _mm_setzero_ps();
    }
}

// ea: 0x4F9110
void MsaQuat::AngleToQuaternion(float fAngle, const float (*vAxis)[3])
{
    if (fAngle == 0.0f)
    {
        z = 0.0f;
        y = 0.0f;
        x = 0.0f;
        w = 1.0f;
    }
    else
    {
        float half = fAngle * 0.5f;
        float sinang = sinf(half);
        x = ((*vAxis)[0] * (1.0f / fAngle)) * sinang;
        y = ((*vAxis)[1] * (1.0f / fAngle)) * sinang;
        float vz = ((*vAxis)[2] * (1.0f / fAngle)) * sinang;
        z = vz;
        float cosang = cosf(half);
        float len = sqrtf(x * x + y * y + cosang * cosang + vz * vz);
        x = (1.0f / len) * x;
        y = y * (1.0f / len);
        z = vz * (1.0f / len);
        w = (1.0f / len) * cosang;
    }
}

// ============================================================================
// CameraShake
// ============================================================================

// ea: 0x4F4DB0
void CameraShake::Initialise()
{
    m_scale3D = 0.018f;
    m_scaleCOD = 1.0f;
    m_scaleCOD_onlyADS = 1;
    if (shakeTable[0].m_type != -1)
    {
        CameraShakeType* v1 = shakeTable;
        do
        {
            v1->m_lastShakeTime = -9999.9f;
            ++v1;
        } while (v1->m_type != -1);
    }
    for (int i = 0; i < 5; ++i)
        m_instanceData[i].m_active = 0;
}

// ea: 0x4F4E20
CameraShakeInstance* CameraShake::GetNewShakeInstance()
{
    int v1 = 0;
    while (m_instanceData[v1].m_active != 0)
    {
        ++v1;
        if (v1 >= 5)
            goto too_many;
    }
    memset(&m_instanceData[v1], 0, sizeof(CameraShakeInstance));
    m_instanceData[v1].m_type = 0;
    m_instanceData[v1].m_active = 1;
    return &m_instanceData[v1];
too_many:
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("Too many active Camera Shakes"))
    {
        __debugbreak();
    }
    return nullptr;
}

// ea: 0x4F4EA0
void CameraShakeInstance::SetFalloff(math::Position3* worldPos, float falloff)
{
    float m_scale3D = g_cameraShake[currCl].m_scale3D;
    float v4;
    if (worldPos != nullptr)
    {
        __m128 v5 = _mm_sub_ps(
            nglGetMatrix_ViewToWorld(nglBuildScene).w.v, worldPos->v);
        __m128 v6 = _mm_mul_ps(v5, v5);
        float dist = sqrtf(v6.m128_f32[0]
                           + (v6.m128_f32[1] + v6.m128_f32[2])) * falloff;
        v4 = dist;
        if ((m_scale3D * 2.0f) > dist)
            v4 = m_scale3D * 2.0f;
    }
    else
    {
        v4 = g_cameraShake[currCl].m_scale3D * 2.0f;
    }
    float v7 = 1.0f;
    if (v4 != 0.0f)
        v7 = 1.0f / v4;
    m_invDistance = v7 * m_scale3D;
}

// ea: 0x4F4F90
void CameraShake::StopCameraShake(CameraShakeInstance* pShake)
{
    pShake->m_active = 0;
}

// ea: 0x4F4FB0
CameraShakeType* CameraShake::GetShakeType(int type)
{
    CameraShakeType* i = shakeTable;
    for (;; ++i)
    {
        if (i->m_type == -1)
        {
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Bad camera shake"))
            {
                __debugbreak();
            }
        }
        if (i->m_type == type)
            break;
    }
    return i;
}

// ea: 0x4F5020
void CameraShakeInstance::SetTime(float time)
{
    m_time = time;
}

// ea: 0x4F5040
void CameraShakeInstance::SetupInstance(int type, float timeOverride)
{
    CameraShakeType* shakeType = g_cameraShake[currCl].GetShakeType(type);
    m_noiseFloats[0].m_seed = (float)(rand() % 0x4000);
    m_noiseFloats[0].m_num_octaves = 2;
    m_noiseFloats[0].m_freq_mult = 1.0f;
    m_noiseFloats[0].m_range = 1.0f;
    m_noiseFloats[0].m_range = shakeType->m_movement;
    m_noiseFloats[0].m_freq_mult = shakeType->m_frequency;
    m_noiseFloats[1].m_seed = (float)(rand() % 0x4000);
    m_noiseFloats[1].m_freq_mult = 1.0f;
    m_noiseFloats[1].m_range = 1.0f;
    m_noiseFloats[1].m_num_octaves = 2;
    m_noiseFloats[1].m_freq_mult = shakeType->m_frequency;
    m_noiseFloats[1].m_range = shakeType->m_movement;
    if (timeOverride == 0.0f)
        m_time = shakeType->m_time;
    else
        m_time = timeOverride;
}

// ea: 0x4F5110
void CameraShakeInstance::OverrideSettings(float frequency, float movement)
{
    if (movement != 0.0f)
    {
        m_noiseFloats[0].m_range = movement;
        m_noiseFloats[1].m_range = movement;
    }
    if (frequency != 0.0f)
    {
        m_noiseFloats[0].m_freq_mult = frequency;
        m_noiseFloats[1].m_freq_mult = frequency;
    }
}

// ea: 0x4F5150
void CameraShakeInstance::SetTargetMagnitude(float endMagnitudeScale)
{
    m_magnitudeInc = ((endMagnitudeScale * m_magnitude) - m_magnitude)
        / g_cameraShake[currCl].GetShakeType(m_type)->m_time;
}

// ============================================================================
// CameraShake::StartCameraShake - ea: 0x4F8FA0
// ============================================================================
CameraShakeInstance* CameraShake::StartCameraShake(int type,
    math::Position3* worldPos, float size, float timeOverride, float nextDelay)
{
    CameraShakeType* ShakeType = GetShakeType(type);
    if (nextDelay != -1.0f)
        ShakeType->m_delayTimeMax = nextDelay;
    float v9 = cgGlobal_time * 0.001f;
    if (ShakeType->m_delayTimeMax > (v9 - ShakeType->m_lastShakeTime))
        return nullptr;
    ShakeType->m_lastShakeTime = v9;
    float time = ShakeType->m_time;
    if (timeOverride != 0.0f)
        time = timeOverride;
    CameraShakeInstance* NewShakeInstance;
    if (ShakeType->m_internalExternal != 0)
    {
        NewShakeInstance = GetNewShakeInstance();
        if (NewShakeInstance == nullptr)
            return nullptr;
        NewShakeInstance->m_magnitude = size;
        NewShakeInstance->m_type = type;
        NewShakeInstance->m_magnitudeInc = 0.0f;
    }
    else
    {
        if (size == 0.0f)
            size = 1.0f;
        NewShakeInstance = GetNewShakeInstance();
        if (NewShakeInstance == nullptr)
            return nullptr;
        NewShakeInstance->m_magnitude = size;
        NewShakeInstance->m_type = type;
        NewShakeInstance->m_magnitudeInc = 0.0f;
        NewShakeInstance->SetFalloff(worldPos, ShakeType->m_fallOff);
    }
    float scale = m_scaleCOD;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    float v13 = 1.0f;
    if (Player == nullptr || (Player->client->ps.pm_flags & 0x20) != 0
        || m_scaleCOD_onlyADS == 0)
        v13 = scale;
    NewShakeInstance->m_magnitude = NewShakeInstance->m_magnitude * v13;
    NewShakeInstance->m_frequency = NewShakeInstance->m_frequency * v13;
    NewShakeInstance->SetupInstance(type, time);
    return NewShakeInstance;
}

// ============================================================================
// CameraShakeInstance::AddNoise_2D_EM - ea: 0x4F9210
// ============================================================================
void CameraShakeInstance::AddNoise_2D_EM(int type3dOR2d,
                                         math::Vector4* shakeEm)
{
    float m_time = this->m_time;
    float v11 = m_magnitude;
    float out[2];
    if (type3dOR2d != 0)
    {
        if (type3dOR2d != 1)
            return;
        if (this->m_time != 0.0f && this->m_time < 0.1f)
            v11 = (this->m_time * 10.0f) * m_magnitude;
        out[1] = m_noiseFloats[1].GetValue() * v11;
        out[0] = m_noiseFloats[0].GetValue() * v11;
    }
    else
    {
        if (this->m_time < 0.1f)
            v11 = (this->m_time * 10.0f) * m_magnitude;
        out[1] = m_noiseFloats[1].GetValue() * m_invDistance * v11 * m_time;
        out[0] = m_noiseFloats[0].GetValue() * m_invDistance * v11 * m_time;
    }
    shakeEm->v.m128_f32[0] += out[0];
    shakeEm->v.m128_f32[1] += out[1];
}

// ============================================================================
// MsaQuat::ExpMapToQuaternion - ea: 0x4F9340
// ============================================================================
void MsaQuat::ExpMapToQuaternion(const math::Vector4& expMap)
{
    float v11 = sqrtf(expMap[0] * expMap[0] + expMap[1] * expMap[1]
                      + expMap[2] * expMap[2]);
    if (v11 == 0.0f)
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 1.0f;
        return;
    }
    float half = v11 * 0.5f;
    float s = sinf(half);
    float c = cosf(half);
    float vx = (expMap[0] * (1.0f / v11)) * s;
    float vy = (expMap[1] * (1.0f / v11)) * s;
    float vz = (expMap[2] * (1.0f / v11)) * s;
    float len = sqrtf(c * c + vz * vz + vy * vy + vx * vx);
    x = (1.0f / len) * vx;
    y = (1.0f / len) * vy;
    z = (1.0f / len) * vz;
    w = (1.0f / len) * c;
}

// ============================================================================
// MsaQuat::MatrixToQuaternion - ea: 0x4F94B0
// ============================================================================
int MsaQuat::MatrixToQuaternion(math::Mat43* mat, math::Position3* pos)
{
    if (pos != nullptr)
        *pos = mat->w;
    float mata = mat->y.v.m128_f32[1] + mat->z.v.m128_f32[2];
    if ((mata + mat->x.v.m128_f32[0]) < 0.0f)
    {
        int v5 = mat->y.v.m128_f32[1] > mat->x.v.m128_f32[0];
        if (mat->z.v.m128_f32[2] > mat->x.v.m128_f32[5 * v5])
            v5 = 2;
        if (v5 != 0)
        {
            int v6 = v5 - 1;
            if (v6 == 1)
            {
                float s = sqrtf(mat->z.v.m128_f32[2]
                                - (mat->y.v.m128_f32[1] + mat->x.v.m128_f32[0])
                                + 1.0f);
                float inv = 0.5f / s;
                z = s * 0.5f;
                x = (mat->z.v.m128_f32[0] + mat->x.v.m128_f32[2]) * inv;
                y = (mat->z.v.m128_f32[1] + mat->y.v.m128_f32[2]) * inv;
                w = (mat->x.v.m128_f32[1] - mat->y.v.m128_f32[0]) * inv;
            }
            else if (v6 == 0)
            {
                float s = sqrtf(mat->y.v.m128_f32[1]
                                - (mat->x.v.m128_f32[0] + mat->z.v.m128_f32[2])
                                + 1.0f);
                float inv = 0.5f / s;
                y = s * 0.5f;
                z = (mat->z.v.m128_f32[1] + mat->y.v.m128_f32[2]) * inv;
                x = (mat->y.v.m128_f32[0] + mat->x.v.m128_f32[1]) * inv;
                w = (mat->z.v.m128_f32[0] - mat->x.v.m128_f32[2]) * inv;
            }
        }
        else
        {
            float s = sqrtf(mat->x.v.m128_f32[0] - mata + 1.0f);
            float inv = 0.5f / s;
            x = s * 0.5f;
            y = (mat->y.v.m128_f32[0] + mat->x.v.m128_f32[1]) * inv;
            z = (mat->z.v.m128_f32[0] + mat->x.v.m128_f32[2]) * inv;
            w = (mat->y.v.m128_f32[2] - mat->z.v.m128_f32[1]) * inv;
        }
        return fabsf(1.0f - sqrtf(x * x + y * y + z * z + w * w)) < 0.0001f;
    }
    float s = sqrtf((mata + mat->x.v.m128_f32[0]) + 1.0f);
    w = s * 0.5f;
    x = (mat->y.v.m128_f32[2] - mat->z.v.m128_f32[1]) * (0.5f / s);
    y = (mat->z.v.m128_f32[0] - mat->x.v.m128_f32[2]) * (0.5f / s);
    z = (mat->x.v.m128_f32[1] - mat->y.v.m128_f32[0]) * (0.5f / s);
    return fabsf(1.0f - sqrtf(x * x + y * y + z * z + w * w)) < 0.0001f;
}

// ============================================================================
// FN_ShakeTestFunction - ea: 0x4FEF40
// ============================================================================
CameraShakeInstance* FN_ShakeTestFunction()
{
    CameraShakeInstance* result = g_cameraShake[0].StartCameraShake(
        (g_ShakeTest2d == 0) + 1, nullptr, 1.0f, g_ShakeTestTime, -1.0f);
    float v2 = g_ShakeTestMag;
    float v3 = g_ShakeTestFreq;
    if (result == nullptr)
        return nullptr;
    if (g_ShakeTestMag != 0.0f)
    {
        result->m_noiseFloats[0].m_range = g_ShakeTestMag;
        result->m_noiseFloats[1].m_range = v2;
    }
    if (v3 != 0.0f)
    {
        result->m_noiseFloats[0].m_freq_mult = v3;
        result->m_noiseFloats[1].m_freq_mult = v3;
    }
    return result;
}

// ea: 0x4F8F70
CameraShake::CameraShake()
{
}

// ============================================================================
// CameraShake::Rumble - ea: 0x4FF360
// ============================================================================
void CameraShake::Rumble(float intensity, float duration)
{
    RumbleEffect rumbleEffect;
    rumbleEffect.mRumbleDataArray[0].enabled = true;
    rumbleEffect.mRumbleDataArray[0].delay = 0.0f;
    rumbleEffect.mRumbleDataArray[0].intensity = 1.0f;
    rumbleEffect.mRumbleDataArray[0].ramp_up_duration = 0.0f;
    rumbleEffect.mRumbleDataArray[0].steady_duration = 1.0f;
    rumbleEffect.mRumbleDataArray[0].ramp_down_duration = 0.0f;
    rumbleEffect.mRumbleDataArray[1].enabled = true;
    rumbleEffect.mRumbleDataArray[1].delay = 0.0f;
    rumbleEffect.mRumbleDataArray[1].intensity = 1.0f;
    rumbleEffect.mRumbleDataArray[1].ramp_up_duration = 0.0f;
    rumbleEffect.mRumbleDataArray[1].steady_duration = 1.0f;
    rumbleEffect.mRumbleDataArray[1].ramp_down_duration = 0.0f;
    if (intensity < 0.0f || intensity > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 103;
        AeAssert::gCurrentExpr =
            "new_intensity >= 0.0f && new_intensity <= 1.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Please add a descriptive string"))
            __debugbreak();
    }
    rumbleEffect.mRumbleDataArray[0].intensity = intensity;
    if (duration < 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 124;
        AeAssert::gCurrentExpr = "new_duration >= 0.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Please add a descriptive string"))
            __debugbreak();
    }
    rumbleEffect.mRumbleDataArray[0].steady_duration = duration;
    if (intensity < 0.0f || intensity > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 103;
        AeAssert::gCurrentExpr =
            "new_intensity >= 0.0f && new_intensity <= 1.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Please add a descriptive string"))
            __debugbreak();
    }
    rumbleEffect.mRumbleDataArray[1].intensity = intensity;
    if (duration < 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 124;
        AeAssert::gCurrentExpr = "new_duration >= 0.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Please add a descriptive string"))
            __debugbreak();
    }
    rumbleEffect.mRumbleDataArray[1].steady_duration = duration;
    if (RumbleManager_Inst(currCl) != nullptr)
        RumbleManager_Play(RumbleManager_Inst(currCl), &rumbleEffect, 1.0f);
}

// ============================================================================
// CameraShake::CreateCameraShakeMatrix - ea: 0x4FF130
// Integrate each active shake instance and accumulate a camera-local matrix.
// ============================================================================
math::Mat43* CameraShake::CreateCameraShakeMatrix(math::Mat43* pCamLocal)
{
    float dt = ServerTime::sInst.mTickDelta;
    MsaQuat viewQuat;
    viewQuat.x = 0.0f;
    viewQuat.y = 0.0f;
    viewQuat.z = 0.0f;
    viewQuat.w = 1.0f;
    for (int i = 0; i < 5; ++i)
    {
        CameraShakeInstance* inst = &m_instanceData[i];
        if (inst->m_active == 0)
            continue;
        CameraShakeType* ShakeType = GetShakeType(inst->m_type);
        if (inst->m_magnitudeInc != 0.0f)
            inst->m_magnitude = (inst->m_magnitudeInc * dt) + inst->m_magnitude;
        if (ShakeType->m_time > 0.0f)
        {
            inst->m_time -= dt;
            if (inst->m_time <= 0.0f)
                inst->m_active = 0;
        }
        if (inst->m_active == 0)
            continue;
        math::Vector4 shakeEm;
        shakeEm.v = _mm_setzero_ps();
        inst->AddNoise_2D_EM(ShakeType->m_internalExternal, &shakeEm);
        MsaQuat shakeQuat;
        shakeQuat.ExpMapToQuaternion(shakeEm);
        MsaQuat viewMatQuat;
        viewMatQuat.MatrixToQuaternion(pCamLocal, nullptr);
        MsaQuat acc;
        // Quaternion multiply: viewQuat * shakeQuat
        acc.x = ((viewMatQuat.w * shakeQuat.x)
                 + (viewMatQuat.x * shakeQuat.w)
                 + (viewMatQuat.y * shakeQuat.z))
                - (viewMatQuat.z * shakeQuat.y);
        acc.y = ((viewMatQuat.w * shakeQuat.y)
                 + (viewMatQuat.y * shakeQuat.w)
                 + (viewMatQuat.z * shakeQuat.x))
                - (viewMatQuat.x * shakeQuat.z);
        acc.z = ((viewMatQuat.w * shakeQuat.z)
                 + (viewMatQuat.z * shakeQuat.w)
                 + (viewMatQuat.x * shakeQuat.y))
                - (viewMatQuat.y * shakeQuat.x);
        acc.w = ((viewMatQuat.w * shakeQuat.w)
                 - (viewMatQuat.x * shakeQuat.x)
                 - (viewMatQuat.y * shakeQuat.y)
                 - (viewMatQuat.z * shakeQuat.z));
        viewQuat = acc;
        math::Mat43 mat;
        viewQuat.QuaternionToMatrix(nullptr, &mat);
        pCamLocal->x = mat.x;
        pCamLocal->y = mat.y;
        pCamLocal->z = mat.z;
    }
    return pCamLocal;
}
