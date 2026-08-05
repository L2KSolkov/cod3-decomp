// ============================================================================
// cdOceanGlobals.h — ocean globals (14 non-inline funcs).
// Source: source/cdOceanGlobals.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdOceanGlobals.o):
//   SetSeaLevel @0x7C0120
//   SetLayerAlpha @0x7C01B0
//   SetLayerScroll @0x7C0260
//   SetLayerScale @0x7C03B0
//   SetWaveOrigin @0x7C0500
//   SetWaveHeading @0x7C0620
//   SetWaveDistance @0x7C0690
//   SetWaveWavelength @0x7C0700
//   SetWaveAmplitude @0x7C0770
//   SetWavePhase @0x7C07E0
//   SetWaveTimescale @0x7C0850
//   Init @0x7C08C0
//   DumpSettings @0x7C08E0
//   GetHeight @0x7C0B70
// ============================================================================
#ifndef COD3_RENDER_CDOCEANGLOBALS_H
#define COD3_RENDER_CDOCEANGLOBALS_H

#include "core/math_types.h"

#include <intrin.h>

// ============================================================================
// cdOceanGlobals — ocean configuration (13 Vector4 params per wave bank).
// ============================================================================
namespace cdOceanGlobals {

// WaveBank — per-bank ocean params (208 bytes)
struct WaveBank {
    math::Vector4 mParams;      // +0x00  (sea level + layer alphas)
    math::Vector4 mUVScroll1;   // +0x10
    math::Vector4 mUVScroll2;   // +0x20
    math::Vector4 mUVScale1;    // +0x30
    math::Vector4 mUVScale2;    // +0x40
    math::Vector4 mOrigin12;    // +0x50
    math::Vector4 mOrigin34;    // +0x60
    math::Vector4 mHeading;     // +0x70
    math::Vector4 mDistance;    // +0x80
    math::Vector4 mWavelength;  // +0x90
    math::Vector4 mAmplitude;   // +0xA0
    math::Vector4 mPhase;       // +0xB0
    math::Vector4 mTimescale;   // +0xC0

    void SetDefaults();  // ?SetDefaults@WaveBank@cdOceanGlobals@@QAEXXZ (inline COMDAT @0x7BFF00)
};
static_assert(sizeof(WaveBank) == 0xD0, "WaveBank size mismatch");

// sBanks — 4 wave banks
extern WaveBank sBanks[4];  // ?sBanks@cdOceanGlobals@@3PAUWaveBank@1@A @0x10DDB50

void SetSeaLevel(int bank, float seaLevel);      // @0x7C0120
void SetWaveHeading(int bank, int wave, float heading);    // @0x7C0620
void SetWaveDistance(int bank, int wave, float distance);  // @0x7C0690
void SetWaveWavelength(int bank, int wave, float wavelength);  // @0x7C0700
void SetWaveAmplitude(int bank, int wave, float amplitude);   // @0x7C0770
void SetWavePhase(int bank, int wave, float phase);           // @0x7C07E0
void SetWaveTimescale(int bank, int wave, float timescale);   // @0x7C0850
void Init();                                             // @0x7C08C0

} // namespace cdOceanGlobals

#endif // COD3_RENDER_CDOCEANGLOBALS_H
