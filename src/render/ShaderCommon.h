// ============================================================================
// ShaderCommon.h - shared shader toggle/perf-test helpers.
// Source: src/render/ShaderCommon.cpp (render_xboxr:ShaderCommon.o, 21 funcs)
// Verified against IDA (render_xboxr:ShaderCommon.o).
// ============================================================================

#ifndef COD3_RENDER_SHADERCOMMON_H
#define COD3_RENDER_SHADERCOMMON_H

#include "core/math_types.h"
#include "core/tlFixedString.h"
#include "core/ae_array.h"
#include "core/ae_fixed_string.h"

struct nglScene;
struct nglTexture;

namespace ShaderCommon {

enum EDebugRenderMode {
    kDebugRenderModeNormal = 0,
    kDebugRenderModeFullbright = 1,
    kDebugRenderModeWireframe = 2,
    kDebugRenderModeSolidColor = 3,
    kDebugRenderModeTextureTiling = 5,
};

// Toggle flag bits for each shader (ShaderSwitching, @0x10DDB10).
enum kShaderSwitch {
    kShaderSwitch_cdglow = 0,
    kShaderSwitch_cdworld = 1,
    kShaderSwitch_cdworldvertexlit = 2,
    kShaderSwitch_cdworldblend = 3,
    kShaderSwitch_cdworldlit = 4,
    kShaderSwitch_cdblendpointlit = 5,
    kShaderSwitch_cdworldcolor = 6,
    kShaderSwitch_cdwater = 7,
    kShaderSwitch_cdocean = 8,
    kShaderSwitch_cdriver = 9,
    kShaderSwitch_cdsimple = 10,
    kShaderSwitch_cdsimplecolor = 11,
    kShaderSwitch_cdsimpleinstance = 12,
    kShaderSwitch_cdsimpleprelit = 13,
    kShaderSwitch_cdsimpleuvanim = 14,
    kShaderSwitch_cdsimplealpha = 15,
    kShaderSwitch_cdsimplespecular = 16,
    kShaderSwitch_cdchar = 17,
    kShaderSwitch_cdcharspecular = 18,
    kShaderSwitch_cddebug = 19,
    kShaderSwitch_cdsky = 20,
    kShaderSwitch_cddecal = 21,
    kShaderSwitch_cdbackground = 22,
    kShaderSwitch_cdscratch = 23,
    kShaderSwitch_cdpropeller = 24,
    kShaderSwitch_cdairplanemetal = 25,
    kShaderSwitch_cdprelit = 26,
    kShaderSwitch_cdgun = 27,
    kShaderSwitch_cdgunsight = 28,
    kShaderSwitch_cdgunsightspecular = 29,
    kShaderSwitch_cdglass = 30,
    kShaderSwitch_cdheathaze = 31,
    kShaderSwitch_particles = 32,
    kShaderSwitch_fog = 33,
    kShaderSwitch_cdDynamicDecal = 34,
    kShaderSwitch_cdFlag = 35,
    kShaderSwitch_Count = 36,
};

// ShaderSwitching - 4-byte union of per-shader toggle flags (@0x10DDB10).
union ShaderSwitching_t {
    struct {
        unsigned char __s0[4];
    };
    unsigned int as_u32;  // +0x00
};
extern ShaderSwitching_t ShaderSwitching;

// Perf info + globals (@0x10DDB14.., @0xE3BA10).
extern unsigned int gShaderSwitchesCount;  // not a real symbol; table size
extern int sDebugRenderMode;              // @0x10DDB1C
extern int gTextureSizeMipLevel;          // @0xE3BA10
extern float gTime;                       // @0x10DDB18
extern math::Vector4 FarFogColor;         // @0x10DDB30

struct ShaderSwitchPair {
    const char* first;
    void (*second)();
};
extern ShaderSwitchPair gShaderSwitches[36];  // @0xE3BA18

// ShotPerfTest - shader perf sampling (@0x10DDB50, 0x5D0 bytes).
struct ShotPerfTest {
    struct ShaderInfo {
        float sampleTime;   // +0x00
        int   numSamples;   // +0x04
        float renderTime;   // +0x08
        float cpuTime;      // +0x0C
        float nodes;        // +0x10
        float polys;        // +0x14
        float verts;        // +0x18
        unsigned __int64 tex;  // +0x1C

        ShaderInfo();
        void Finalize();
        void Update(float deltaT);
    };

    int      mCurShader;   // +0x00
    bool     mFinished;    // +0x04
    ShaderInfo mUnrestricted;  // +0x08
    ShaderInfo mResults[36];   // +0x30

    ShotPerfTest();
    bool IsFinished() const { return mFinished; }
    void Update(float deltaT);
    void GenerateReport(ae_sized_array<ae_fixed_string<512, unsigned short>, 64>* report);
};

// Toggle helpers.
char ToggleParticles();
char ToggleFog();
void SetDebugRenderMode(EDebugRenderMode iMode);
EDebugRenderMode GetDebugRenderMode();
void SetTextureSizeMipLevel(int Level);
void FinishShotPerfTest(void* perfTestBuff);
void InitShaders();
void RegisterShaders();
void SetupFrame(float iDelta);
void SetupSceneCallback();
nglTexture* ShaderGetTexture(const tlFixedString* name);
nglTexture* ShaderGetTextureNoDefault(const tlFixedString* name);
math::Vector4* GetFarFogColor(math::Vector4* result);
void GlowInit();
nglScene* GlowRender();
void HeatHazeInit();
void HeatHazeCallback(void* Data);
ShotPerfTest* StartShotPerfTest();
void ToggleShader(const char* iName);
bool UpdateShotPerfTest(ShotPerfTest* perfTestBuff, float deltaT);
void GetShotPerfResults(ShotPerfTest* perfTestBuff,
                        ae_sized_array<ae_fixed_string<512, unsigned short>, 64>* results);

} // namespace ShaderCommon

#endif // COD3_RENDER_SHADERCOMMON_H
