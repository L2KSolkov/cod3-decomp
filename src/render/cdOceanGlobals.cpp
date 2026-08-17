// ============================================================================
// cdOceanGlobals.cpp — ocean globals (14 non-inline funcs).
// Source: source/cdOceanGlobals.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdOceanGlobals.o):
//   SetSeaLevel @0x7C0120
//   SetWaveHeading @0x7C0620
//   SetWaveDistance @0x7C0690
//   SetWaveWavelength @0x7C0700
//   SetWaveAmplitude @0x7C0770
//   SetWavePhase @0x7C07E0
//   SetWaveTimescale @0x7C0850
//   Init @0x7C08C0
//   WaveBank::SetDefaults @0x7BFF00 (inline COMDAT)
// ============================================================================
#include "cdOceanGlobals.h"

#include <intrin.h>

namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
    extern ECoderId gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int   gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
}

namespace cdOceanGlobals {

// ============================================================================
// Data
// ============================================================================
WaveBank sBanks[4];  // ?sBanks@cdOceanGlobals@@3PAUWaveBank@1@A

// ============================================================================
// WaveBank::SetDefaults — reset a bank to its default values.
// ea: 0x7BFF00 (inline COMDAT)
// ============================================================================
void WaveBank::SetDefaults() {
    math::Vector4 v1;
    v1.v.m128_f32[0] = 0.5f;
    v1.v.m128_f32[1] = 0.5f;
    v1.v.m128_f32[2] = 0.25f;
    v1.v.m128_f32[3] = 0.25f;
    this->mParams = v1;
    this->mUVScroll1.v = _mm_setzero_ps();
    this->mUVScroll2.v = _mm_setzero_ps();
    this->mUVScale1.v = _mm_set1_ps(800.0f);
    this->mUVScale2.v = _mm_set1_ps(800.0f);
    this->mOrigin12.v = _mm_setzero_ps();
    this->mOrigin34.v = _mm_setzero_ps();
    this->mHeading.v = _mm_setzero_ps();
    this->mDistance.v = _mm_set1_ps(1.0f);
    this->mWavelength.v = _mm_set1_ps(1.0f);
    this->mAmplitude.v = _mm_setzero_ps();
    this->mPhase.v = _mm_setzero_ps();
    this->mTimescale.v = _mm_setzero_ps();
}

// ============================================================================
// SetSeaLevel — set the sea level for a bank.
// ea: 0x7C0120
// ============================================================================
void SetSeaLevel(int bank, float seaLevel) {
    if (bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 14;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    sBanks[bank].mParams.v.m128_f32[0] = seaLevel;
}

// ============================================================================
// SetLayerAlpha — set the two layer alpha parameters for a bank.
// ea: 0x7C01B0
// ============================================================================
void SetLayerAlpha(int bank, float a, float b) {
    if (bank < 0 || bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 20;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    sBanks[bank].mParams.v.m128_f32[1] = a;
    sBanks[bank].mParams.v.m128_f32[2] = b;
}

// ============================================================================
// SetLayerScroll — set a UV scroll pair for a bank layer.
// ea: 0x7C0260
// ============================================================================
void SetLayerScroll(int bank, int layer, float u, float v) {
    if (bank < 0 || bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 27;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    math::Vector4* target = nullptr;
    switch (layer) {
    case 0:
        target = &sBanks[bank].mUVScroll1;
        target->v.m128_f32[0] = u;
        target->v.m128_f32[1] = v;
        return;
    case 1:
        target = &sBanks[bank].mUVScroll1;
        target->v.m128_f32[2] = u;
        target->v.m128_f32[3] = v;
        return;
    case 2:
        target = &sBanks[bank].mUVScroll2;
        target->v.m128_f32[0] = u;
        target->v.m128_f32[1] = v;
        return;
    default:
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 43;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid layer"))
            __debugbreak();
        return;
    }
}

// ============================================================================
// SetLayerScale — set a UV scale pair for a bank layer.
// ea: 0x7C03B0
// ============================================================================
void SetLayerScale(int bank, int layer, float u, float v) {
    if (bank < 0 || bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 49;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    math::Vector4* target = nullptr;
    switch (layer) {
    case 0:
        target = &sBanks[bank].mUVScale1;
        target->v.m128_f32[0] = u;
        target->v.m128_f32[1] = v;
        return;
    case 1:
        target = &sBanks[bank].mUVScale1;
        target->v.m128_f32[2] = u;
        target->v.m128_f32[3] = v;
        return;
    case 2:
        target = &sBanks[bank].mUVScale2;
        target->v.m128_f32[0] = u;
        target->v.m128_f32[1] = v;
        return;
    default:
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 66;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid layer"))
            __debugbreak();
        return;
    }
}

// ============================================================================
// SetWaveOrigin — set an XY origin pair for one of four waves.
// ea: 0x7C0500
// ============================================================================
void SetWaveOrigin(int bank, int wave, float x, float y) {
    if (bank < 0 || bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 73;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    math::Vector4* target = nullptr;
    switch (wave) {
    case 0:
        target = &sBanks[bank].mOrigin12;
        target->v.m128_f32[0] = x;
        target->v.m128_f32[1] = y;
        return;
    case 1:
        target = &sBanks[bank].mOrigin12;
        target->v.m128_f32[2] = x;
        target->v.m128_f32[3] = y;
        return;
    case 2:
        target = &sBanks[bank].mOrigin34;
        target->v.m128_f32[0] = x;
        target->v.m128_f32[1] = y;
        return;
    case 3:
        target = &sBanks[bank].mOrigin34;
        target->v.m128_f32[2] = x;
        target->v.m128_f32[3] = y;
        return;
    default:
        return;
    }
}

// ============================================================================
// SetWaveHeading — set the heading for a bank's wave.
// ea: 0x7C0620
// ============================================================================
void SetWaveHeading(int bank, int wave, float heading) {
    if (bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 98;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    sBanks[bank].mHeading.v.m128_f32[wave] = heading;
}

// ============================================================================
// SetWaveDistance — set the distance for a bank's wave.
// ea: 0x7C0690
// ============================================================================
void SetWaveDistance(int bank, int wave, float distance) {
    if (bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 104;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    sBanks[bank].mDistance.v.m128_f32[wave] = distance;
}

// ============================================================================
// SetWaveWavelength — set the wavelength for a bank's wave.
// ea: 0x7C0700
// ============================================================================
void SetWaveWavelength(int bank, int wave, float wavelength) {
    if (bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 110;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    sBanks[bank].mWavelength.v.m128_f32[wave] = wavelength;
}

// ============================================================================
// SetWaveAmplitude — set the amplitude for a bank's wave.
// ea: 0x7C0770
// ============================================================================
void SetWaveAmplitude(int bank, int wave, float amplitude) {
    if (bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 116;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    sBanks[bank].mAmplitude.v.m128_f32[wave] = amplitude;
}

// ============================================================================
// SetWavePhase — set the phase for a bank's wave.
// ea: 0x7C07E0
// ============================================================================
void SetWavePhase(int bank, int wave, float phase) {
    if (bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 122;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    sBanks[bank].mPhase.v.m128_f32[wave] = phase;
}

// ============================================================================
// SetWaveTimescale — set the timescale for a bank's wave.
// ea: 0x7C0850
// ============================================================================
void SetWaveTimescale(int bank, int wave, float timescale) {
    if (bank >= 4) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanGlobals.cpp";
        AeAssert::gCurrentLine = 128;
        AeAssert::gCurrentExpr = "(bank >= 0) && (bank < NUM_BANKS)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
            __debugbreak();
    }
    sBanks[bank].mTimescale.v.m128_f32[wave] = timescale;
}

// ============================================================================
// Init — reset all 4 wave banks.
// ea: 0x7C08C0
// ============================================================================
void Init() {
    WaveBank* v0 = sBanks;
    do {
        v0->SetDefaults();
        ++v0;
    } while (v0 < &sBanks[4]);
}

} // namespace cdOceanGlobals
