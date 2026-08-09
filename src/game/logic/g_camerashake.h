// ============================================================================
// g_camerashake.h - camera shake system (game2.o CameraShake.cpp)
// Layouts verified against IDA local types.
// ============================================================================

#pragma once

#include <stddef.h>
#include "core/math_types.h"

// ============================================================================
// MsaQuat - quaternion (16 bytes, verified)
// ============================================================================
struct MsaQuat {
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08
    float w;  // +0x0C

    MsaQuat() {}
    MsaQuat(float fX, float fY, float fZ, float fW);  // ea: 0x4F5190
    MsaQuat(const MsaQuat& qQuat);                    // ea: 0x4F51C0
    void Multiply(const MsaQuat& other, MsaQuat& out);  // ea: 0x4F51F0
    float Normalise();                                  // ea: 0x4F52E0
    void QuaternionToMatrix(math::Position3* pos,
                            math::Mat43* mat);    // ea: 0x4F5360
    float Magnitude() const;                            // ea: 0x4F5570
    void AngleToQuaternion(float fAngle,
                           const float (*vAxis)[3]);    // ea: 0x4F9110
    int MatrixToQuaternion(math::Mat43* mat,
                           math::Position3* pos); // ea: 0x4F94B0
    void ExpMapToQuaternion(const math::Vector4& v);  // ea: 0x4F9340
};
static_assert(sizeof(MsaQuat) == 0x10, "MsaQuat size mismatch");

// ============================================================================
// NoiseFloat - per-axis noise params (16 bytes, verified)
// ============================================================================
struct NoiseFloat {
    float m_seed;         // +0x00
    unsigned int m_num_octaves;  // +0x04
    float m_freq_mult;    // +0x08
    float m_range;        // +0x0C
};
static_assert(sizeof(NoiseFloat) == 0x10, "NoiseFloat size mismatch");

// ============================================================================
// CameraShakeInstance - 0x40 (verified)
// ============================================================================
struct CameraShakeInstance {
    int m_type;          // +0x00
    float m_magnitude;   // +0x04
    float m_magnitudeInc;// +0x08
    NoiseFloat m_noiseFloats[2];  // +0x0C
    float m_invDistance; // +0x2C
    float m_time;        // +0x30
    float m_frequency;   // +0x34
    float m_movement;    // +0x38
    int m_active;        // +0x3C

    void SetFalloff(math::Position3* worldPos, float falloff);  // ea: 0x4F4EA0
    void SetTime(float time);          // ea: 0x4F5020
    void SetupInstance(int type, float timeOverride);  // ea: 0x4F5040
    void OverrideSettings(float frequency, float movement);  // ea: 0x4F5110
    void SetTargetMagnitude(float endMagnitudeScale);  // ea: 0x4F5150
    void AddNoise_2D_EM(int, math::Vector4*);   // ea: 0x4F9210
};
static_assert(sizeof(CameraShakeInstance) == 0x40,
              "CameraShakeInstance size mismatch");

// ============================================================================
// CameraShakeType - 0x20 (verified)
// ============================================================================
struct CameraShakeType {
    short m_type;              // +0x00
    short m_internalExternal;  // +0x02
    float m_time;              // +0x04
    float m_frequency;         // +0x08
    float m_movement;          // +0x0C
    float m_fallOff;           // +0x10
    float m_rumbleAmount;      // +0x14
    float m_delayTimeMax;      // +0x18
    float m_lastShakeTime;     // +0x1C
};
static_assert(sizeof(CameraShakeType) == 0x20, "CameraShakeType size mismatch");

// ============================================================================
// CameraShake - 0x14C (verified)
// ============================================================================
class CameraShake {
public:
    float m_scale3D;           // +0x00
    float m_scaleCOD;          // +0x04
    int m_scaleCOD_onlyADS;    // +0x08
    CameraShakeInstance m_instanceData[5];  // +0x0C

    CameraShake();                            // ea: 0x4F8F70
    void Initialise();                        // ea: 0x4F4DB0
    CameraShakeInstance* GetNewShakeInstance();  // ea: 0x4F4E20
    void StopCameraShake(CameraShakeInstance* pShake);  // ea: 0x4F4F90
    CameraShakeType* GetShakeType(int type);  // ea: 0x4F4FB0
    CameraShakeInstance* StartCameraShake(int type,
        math::Position3* worldPos, float size, float timeOverride,
        float nextDelay);                     // ea: 0x4F8FA0
    math::Mat43* CreateCameraShakeMatrix(math::Mat43*);  // ea: 0x4FF130
    void Rumble(float, float);                // ea: 0x4FF360
};
static_assert(sizeof(CameraShake) == 0x14C, "CameraShake size mismatch");

extern CameraShake g_cameraShake[4];  // ?g_cameraShake (game2.o)
extern CameraShakeType shakeTable[];  // ?shakeTable (game2.o)

// ============================================================================
// NoiseManager - permutation/gradient noise (static tables)
// ============================================================================
class NoiseManager {
public:
    static int m_initialised;  // ?m_initialised@NoiseManager@@2HA

    void Init();              // ea: 0x4F55A0
    float Noise(float arg);   // ea: 0x4F56E0
    void Normalize2(float* v);  // ea: 0x4F5770
    void Normalize3(float* v);  // ea: 0x4F57C0
    float GetElapsedTime();   // ea: 0x4F5820
};

extern int g_noiseP[256];     // ?g_noiseP
extern float g_noiseG[256];   // ?g_noiseG
extern int dword_F037B4[256];
extern int dword_F037B8[256];
extern int dword_F037BC[256];
extern int dword_F037C0[256];
extern int dword_F037C4[256];
extern int dword_F03BB0[256];
extern int dword_F03BB4[256];
extern int dword_F03BB8[256];
extern int dword_F03BBC[256];
extern int dword_F03BC0[256];
extern int dword_F03BC4[256];
extern float dword_F04048[256];
extern float dword_F0404C[256];
extern float dword_F04050[256];
extern float dword_F04054[256];
extern float dword_F04058[256];
extern float dword_F0405C[256];
extern float dword_F04448[256];
extern float dword_F0444C[256];
extern float dword_F04450[256];
extern float dword_F04454[256];
extern float dword_F04458[256];
extern float dword_F0445C[256];

// CameraShake helper free functions (game2.o)
void FN_ShakeTestFunction();  // ea: 0x4FEF40
