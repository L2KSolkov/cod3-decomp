// ============================================================================
// ShaderCommon.cpp - shared shader toggle/perf-test helpers (21 funcs).
// Source: src/render/ShaderCommon.cpp (render_xboxr:ShaderCommon.o)
// Verified against IDA (render_xboxr:ShaderCommon.o).
// ============================================================================

#include "ShaderCommon.h"

#include "core/mem_heap.h"
#include "ngl/nglDebug.h"
#include "ngl/ngl_scene.h"
#include "ngl/nglTexture.h"

#include <new>
#include <string.h>

// ShaderCommon shared statics (render_xboxr:ShaderCommon.o)
namespace ShaderCommon {
int   gGlowEnable = 0;      // ?gGlowEnable@ShaderCommon@@3HA
int   gGlowPasses = 0;      // ?gGlowPasses@ShaderCommon@@3HA
bool  gGlowGodRays = false; // ?gGlowGodRays@ShaderCommon@@3_NA
float gGlowIntensity = 0.0f;   // ?gGlowIntensity@ShaderCommon@@3MA
float gGlowBrighten = 0.0f;    // ?gGlowBrighten@ShaderCommon@@3MA
float gGlowExpansion = 0.0f;   // ?gGlowExpansion@ShaderCommon@@3MA
}
int nglShader_NextID = 0;   // ?nglShader_NextID@@3HA

// ============================================================================
// Cross-object externs (shader Init/Setup/Render/Toggle functions).
// ============================================================================
extern void InitCDSkyShader();
extern void InitCDBackgroundShader();
extern void InitCDWorldShader();
extern void InitCDWorldVertexLitShader();
extern void InitCDWorldBlendShader();
extern void InitCDWorldPointLitShader();
extern void InitCDWorldBlendPointLitShader();
extern void InitCDWorldColorShader();
extern void InitCDWaterShader();
extern void InitCDOceanShader();
extern void InitCDRiverShader();
extern void InitCDSimpleShader();
extern void InitCDSimpleColorShader();
extern void InitCDSimplePrelitShader();
extern void InitCDSimpleUVAnimShader();
extern void InitCDSimpleInstanceShader();
extern void InitCDSimpleSpecularShader();
extern void InitCDScratchShader();
extern void InitCDAirplaneMetalShader();
extern void InitCDPrelitShader();
extern void InitCDFlagShader();
extern void InitCDCharShader();
extern void InitCDCharSpecularShader();
extern void InitCDDecalShader();
extern void InitCDDynamicDecalShader();
extern void InitCDWheelMarkShader();
extern void InitCDPropellerShader();
extern void InitCDSimpleAlphaShader();
extern void InitCDGlassShader();
extern void InitCDGlowShader();
extern void InitCDGunShader();
extern void InitCDGunSightShader();
extern void InitCDGunSightSpecularShader();
extern void InitCDDebugShader();

extern void SetupCDGlowShader();
extern void SetupCDHeatHazeShader();
extern void RenderCDHeatHazeShader();
extern void GlowCallback(void*);

extern void ToggleCDGlowShader();
extern void ToggleCDWorldShader();
extern void ToggleCDWorldVertexLitShader();
extern void ToggleCDWorldBlendShader();
extern void ToggleCDWorldPointLitShader();
extern void ToggleCDWorldBlendPointLitShader();
extern void ToggleCDWorldColorShader();
extern void ToggleCDWaterShader();
extern void ToggleCDOceanShader();
extern void ToggleCDRiverShader();
extern void ToggleCDSimpleShader();
extern void ToggleCDSimpleColorShader();
extern void ToggleCDSimpleInstanceShader();
extern void ToggleCDSimplePrelitShader();
extern void ToggleCDSimpleUVAnimShader();
extern void ToggleCDSimpleAlphaShader();
extern void ToggleCDSimpleSpecularShader();
extern void ToggleCDCharShader();
extern void ToggleCDCharSpecularShader();
extern void ToggleCDDebugShader();
extern void ToggleCDSkyShader();
extern void ToggleCDDecalShader();
extern void ToggleCDBackgroundShader();
extern void ToggleCDScratchShader();
extern void ToggleCDPropellerShader();
extern void ToggleCDAirplaneMetalShader();
extern void ToggleCDPrelitShader();
extern void ToggleCDGunShader();
extern void ToggleCDGunSightShader();
extern void ToggleCDGunSightSpecularShader();
extern void ToggleCDGlassShader();
extern void ToggleCDHeatHazeShader();
extern void ToggleCDDynamicDecalShader();
extern void ToggleCDFlagShader();

extern nglTexture* nglDefaultTex;
extern nglScene* nglBuildScene;
extern void nglSetAnimTime(float Time);
extern nglTexture* nglGetTexture(const tlFixedString& FileName);
extern void tlPrintf(const char* Format, ...);

namespace AeStringSupport {
void CStrToAeStr(char* dst, int* const dstLen, int dstCapacity, const char* src);
bool StrCStrEqu(const char* lhsBuff, int lhsLen, const char* rhsBuff, int rhsLen);
void SubStr(char* dst, int* dstLen, const char* src, int begin, int count, int srcLen);
}

// ============================================================================
// Data
// ============================================================================
namespace ShaderCommon {

ShaderSwitching_t ShaderSwitching = { 0 };  // @0x10DDB10
int  sDebugRenderMode = 0;                 // @0x10DDB1C
int  gTextureSizeMipLevel = 0;             // @0xE3BA10
float gTime = 0.0f;                        // @0x10DDB18
math::Vector4 FarFogColor;                 // @0x10DDB30

ShaderSwitchPair gShaderSwitches[36] = {
    { "cdglow",            ToggleCDGlowShader },
    { "cdworld",           ToggleCDWorldShader },
    { "cdworldvertexlit",  ToggleCDWorldVertexLitShader },
    { "cdworldblend",      ToggleCDWorldBlendShader },
    { "cdworldlit",        ToggleCDWorldPointLitShader },
    { "cdblendpointlit",   ToggleCDWorldBlendPointLitShader },
    { "cdworldcolor",      ToggleCDWorldColorShader },
    { "cdwater",           ToggleCDWaterShader },
    { "cdocean",           ToggleCDOceanShader },
    { "cdriver",           ToggleCDRiverShader },
    { "cdsimple",          ToggleCDSimpleShader },
    { "cdsimplecolor",     ToggleCDSimpleColorShader },
    { "cdsimpleinstance",  ToggleCDSimpleInstanceShader },
    { "cdsimpleprelit",    ToggleCDSimplePrelitShader },
    { "cdsimpleuvanim",    ToggleCDSimpleUVAnimShader },
    { "cdsimplealpha",     ToggleCDSimpleAlphaShader },
    { "cdsimplespecular",  ToggleCDSimpleSpecularShader },
    { "cdchar",            ToggleCDCharShader },
    { "cdcharspecular",    ToggleCDCharSpecularShader },
    { "cddebug",           ToggleCDDebugShader },
    { "cdsky",             ToggleCDSkyShader },
    { "cddecal",           ToggleCDDecalShader },
    { "cdbackground",      ToggleCDBackgroundShader },
    { "cdscratch",         ToggleCDScratchShader },
    { "cdpropeller",       ToggleCDPropellerShader },
    { "cdairplanemetal",   ToggleCDAirplaneMetalShader },
    { "cdprelit",          ToggleCDPrelitShader },
    { "cdgun",             ToggleCDGunShader },
    { "cdgunsight",        ToggleCDGunSightShader },
    { "cdgunsightspecular",ToggleCDGunSightSpecularShader },
    { "cdglass",           ToggleCDGlassShader },
    { "cdheathaze",        ToggleCDHeatHazeShader },
    { "particles",         (void(*)())ToggleParticles },
    { "fog",               (void(*)())ToggleFog },
    { "cdDynamicDecal",    ToggleCDDynamicDecalShader },
    { "cdFlag",            ToggleCDFlagShader },
};

// ============================================================================
// ShotPerfTest::ShaderInfo
// ============================================================================
ShotPerfTest::ShaderInfo::ShaderInfo() {
    sampleTime = 0.5f;
    numSamples = 0;
    renderTime = 0.0f;
    cpuTime = 0.0f;
    nodes = 0.0f;
    polys = 0.0f;
    verts = 0.0f;
    tex = 0;
}

void ShotPerfTest::ShaderInfo::Finalize() {
    int v1 = numSamples - 1;
    float v2 = 1.0f / v1;
    renderTime = v2 * renderTime;
    cpuTime = cpuTime * v2;
    polys = polys * v2;
    verts = verts * v2;
    float v3 = nodes * v2;
    numSamples = v1;
    nodes = v3;
}

void ShotPerfTest::ShaderInfo::Update(float deltaT) {
    int numSamples = this->numSamples;
    if (numSamples > 0) {
        renderTime = nglSyncPerfInfo.RenderMS + renderTime;
        float v3 = nglSyncPerfInfo.ListSendMS + cpuTime;
        cpuTime = v3;
        cpuTime = v3 + nglSyncPerfInfo.ListSubmitMS;
        polys = (float)nglSyncPerfInfo.TotalPolys + polys;
        verts = (float)nglSyncPerfInfo.TotalVerts + verts;
        nodes = (float)nglSyncPerfInfo.NodeCount + nodes;
    }
    float v4 = sampleTime - deltaT;
    int v5 = numSamples + 1;
    numSamples = v5;
    sampleTime = v4;
    if (v4 < 0.0f) {
        int v6 = v5 - 1;
        float v7 = 1.0f / v6;
        renderTime = v7 * renderTime;
        cpuTime = v7 * cpuTime;
        polys = v7 * polys;
        verts = verts * v7;
        float v8 = nodes * v7;
        numSamples = v6;
        nodes = v8;
    }
}

// ============================================================================
// ShotPerfTest
// ============================================================================
ShotPerfTest::ShotPerfTest() {
    mCurShader = kShaderSwitch_cdglow;
    mFinished = false;
    mUnrestricted.sampleTime = 0.5f;
    mUnrestricted.numSamples = 0;
    mUnrestricted.renderTime = 0.0f;
    mUnrestricted.cpuTime = 0.0f;
    mUnrestricted.nodes = 0.0f;
    mUnrestricted.polys = 0.0f;
    mUnrestricted.verts = 0.0f;
    mUnrestricted.tex = 0;
    for (int i = 36; i != 0; --i) {
        mResults[i - 1].sampleTime = 0.5f;
        mResults[i - 1].numSamples = 0;
        mResults[i - 1].renderTime = 0.0f;
        mResults[i - 1].cpuTime = 0.0f;
        mResults[i - 1].nodes = 0.0f;
        mResults[i - 1].polys = 0.0f;
        mResults[i - 1].verts = 0.0f;
        mResults[i - 1].tex = 0;
    }
}

void ShotPerfTest::Update(float deltaT) {
    if (!mFinished) {
        float sampleTime = mUnrestricted.sampleTime;
        if (sampleTime <= 0.0f) {
            if (mResults[mCurShader].sampleTime == 0.5f) {
                ae_fixed_string<64, unsigned char> cmd;
                int oLen;
                AeStringSupport::CStrToAeStr((char*)cmd.mBuff, &oLen, 63, "!");
                cmd.mLength = (unsigned char)oLen;
                cmd += (const unsigned char*)gShaderSwitches[mCurShader].first;
                ToggleShader((const char*)cmd.mBuff);
                tlPrintf("testing %s...\n", gShaderSwitches[mCurShader].first);
            }
            mResults[mCurShader].Update(deltaT);
            int mCurShader = this->mCurShader;
            if (mResults[mCurShader].sampleTime < 0.0f) {
                if (mCurShader == kShaderSwitch_Count)
                    mFinished = true;
                else
                    this->mCurShader = mCurShader + 1;
            }
        } else {
            if (sampleTime == 0.5f)
                tlPrintf("timing unrestricted...\n");
            mUnrestricted.Update(deltaT);
        }
    }
}

void ShotPerfTest::GenerateReport(
    ae_sized_array<ae_fixed_string<512, unsigned short>, 64>* report) {
    ShaderSwitching.as_u32 = 0;
    ae_formatted_string<512, unsigned short> header("\n%-16s,%-8s,%-8s,%-8s,%-8s",
                                                    "shader", "gpu", "nodes", "avg node");
    report->push_back(header);

    ShaderSwitchPair* v5 = gShaderSwitches;
    float* p_renderTime = &mResults[0].renderTime;
    do {
        float avgNode = 0.0f;
        float v7 = p_renderTime[2];
        if (v7 == 0.0f)
            *p_renderTime = 0.0f;
        else
            avgNode = *p_renderTime / v7;
        ae_formatted_string<512, unsigned short> row("%-16s,%5.3f,%8d,%5.3f",
                                                     v5->first, *p_renderTime,
                                                     (int)p_renderTime[2], avgNode);
        report->push_back(row);
        ++v5;
        p_renderTime += 10;
    } while (v5 < &gShaderSwitches[36]);

    ae_formatted_string<512, unsigned short> total("%-16s,%5.3f,%8d,%5.3f",
                                                   "total", mUnrestricted.renderTime,
                                                   (int)mUnrestricted.nodes,
                                                   mUnrestricted.renderTime /
                                                       mUnrestricted.nodes);
    report->push_back(total);
}

// ============================================================================
// ToggleParticles / ToggleFog - ea: 0x7BF3D0 / 0x7BF3F0
// ============================================================================
char ToggleParticles() {
    char result = (char)((ShaderSwitching.as_u32 ^ ~ShaderSwitching.as_u32) & 1u ^
                         ShaderSwitching.as_u32);
    *((unsigned char*)&ShaderSwitching) = (unsigned char)result;
    return result;
}

char ToggleFog() {
    char result = (char)((ShaderSwitching.as_u32 ^ (2 * ~(ShaderSwitching.as_u32 >> 1))) & 2u ^
                         ShaderSwitching.as_u32);
    *((unsigned char*)&ShaderSwitching) = (unsigned char)result;
    return result;
}

// ============================================================================
// Debug render mode / texture mip level - ea: 0x7BF410..0x7BF430
// ============================================================================
void SetDebugRenderMode(EDebugRenderMode iMode) {
    sDebugRenderMode = (int)iMode;
}

EDebugRenderMode GetDebugRenderMode() {
    return (EDebugRenderMode)sDebugRenderMode;
}

void SetTextureSizeMipLevel(int Level) {
    gTextureSizeMipLevel = Level;
}

// ============================================================================
// Perf test helpers - ea: 0x7BF440 / 0x7BF6B0 / 0x7BF820 / 0x7BF840
// ============================================================================
void FinishShotPerfTest(void* perfTestBuff) {
    mem_heap_free(perfTestBuff);
}

ShotPerfTest* StartShotPerfTest() {
    ShotPerfTest* v0 = (ShotPerfTest*)mem_heap_malloc(0x5D0u);
    if (v0 != NULL)
        return new (v0) ShotPerfTest();
    return NULL;
}

bool UpdateShotPerfTest(ShotPerfTest* perfTestBuff, float deltaT) {
    perfTestBuff->Update(deltaT);
    return perfTestBuff->mFinished;
}

void GetShotPerfResults(
    ShotPerfTest* perfTestBuff,
    ae_sized_array<ae_fixed_string<512, unsigned short>, 64>* results) {
    perfTestBuff->GenerateReport(results);
}

// ============================================================================
// InitShaders / RegisterShaders / SetupFrame / SetupSceneCallback
// ============================================================================
void InitShaders() {
    InitCDSkyShader();
    InitCDBackgroundShader();
    InitCDWorldShader();
    InitCDWorldVertexLitShader();
    InitCDWorldBlendShader();
    InitCDWorldPointLitShader();
    InitCDWorldBlendPointLitShader();
    InitCDWorldColorShader();
    InitCDWaterShader();
    InitCDOceanShader();
    InitCDRiverShader();
    InitCDSimpleShader();
    InitCDSimpleColorShader();
    InitCDSimplePrelitShader();
    InitCDSimpleUVAnimShader();
    InitCDSimpleInstanceShader();
    InitCDSimpleSpecularShader();
    InitCDScratchShader();
    InitCDAirplaneMetalShader();
    InitCDPrelitShader();
    InitCDFlagShader();
    InitCDCharShader();
    InitCDCharSpecularShader();
    InitCDDecalShader();
    InitCDDynamicDecalShader();
    InitCDWheelMarkShader();
    InitCDPropellerShader();
    InitCDSimpleAlphaShader();
    InitCDGlassShader();
    InitCDGlowShader();
    InitCDGunShader();
    InitCDGunSightShader();
    InitCDGunSightSpecularShader();
    InitCDDebugShader();
}

void RegisterShaders() {
}

void SetupFrame(float iDelta) {
    gTime = gTime + iDelta;
    nglSetAnimTime(gTime);
}

void SetupSceneCallback() {
}

// ============================================================================
// Texture lookup helpers - ea: 0x7BF550 / 0x7BF5A0
// ============================================================================
nglTexture* ShaderGetTexture(const tlFixedString* name) {
    char buf[1024];
    nglTexture* result;
    if (name->hash == 0 || (result = nglGetTexture(*name)) == NULL) {
        sprintf(buf, "missing texture %s", name->str);
        return nglDefaultTex;
    }
    return result;
}

nglTexture* ShaderGetTextureNoDefault(const tlFixedString* name) {
    nglTexture* result;
    if (name->hash != 0)
        result = nglGetTexture(*name);
    else
        result = NULL;
    if (result == nglDefaultTex)
        return NULL;
    return result;
}

// ============================================================================
// GetFarFogColor - ea: 0x7BF5D0
// ============================================================================
math::Vector4* GetFarFogColor(math::Vector4* result) {
    FarFogColor.v = nglBuildScene->FogColor.v;
    *result = FarFogColor;
    return result;
}

// ============================================================================
// Glow / HeatHaze - ea: 0x7BF630..0x7BF6A0
// ============================================================================
void GlowInit() {
    SetupCDGlowShader();
}

nglScene* GlowRender() {
    nglSortInfo SortInfo;
    SortInfo.Type = nglSortInfo::NGLSORT_TRANSLUCENT;
    SortInfo.Hash = 0;
    nglListBeginScene(NGLSCENE_PARENT);
    nglSetClearFlags(0);
    nglSetZTestEnable(false);
    nglSetZWriteEnable(false);
    nglListAddCustomNode(GlowCallback, NULL, &SortInfo);
    nglListEndScene();
    return nullptr;
}

void HeatHazeInit() {
    SetupCDHeatHazeShader();
}

void HeatHazeCallback(void* Data) {
    (void)Data;
    RenderCDHeatHazeShader();
}

// ============================================================================
// ToggleShader - ea: 0x7BF6D0
// ============================================================================
void ToggleShader(const char* iName) {
    char tmp[64];
    char name[64];
    int oLen;

    AeStringSupport::CStrToAeStr(name, &oLen, 63, iName);
    name[63] = (char)oLen;
    _strlwr(name);
    unsigned char v1 = (unsigned char)name[63];
    if (name[63] != 0) {
        if (name[0] == 33) {  // '!'
            ShaderSwitching.as_u32 = 0xFFFFFFFFu;
            tmp[63] = 0;
            tmp[0] = 0;
            AeStringSupport::SubStr(tmp, &oLen, name, 1, name[63] - 1, 63);
            tmp[63] = (char)oLen;
            memcpy(name, tmp, sizeof(name));
            int v2 = 0;
            while (!AeStringSupport::StrCStrEqu(name, name[63],
                                                gShaderSwitches[v2].first, -1)) {
                if (++v2 >= 0x24)
                    return;
            }
            ShaderSwitching.as_u32 &= ~(1u << v2);
        } else {
            int v3 = 0;
            while (!AeStringSupport::StrCStrEqu(name, v1,
                                                gShaderSwitches[v3].first, -1)) {
                if (++v3 >= 0x24)
                    return;
                v1 = (unsigned char)name[63];
            }
            gShaderSwitches[v3].second();
        }
    } else {
        ShaderSwitching.as_u32 = ~ShaderSwitching.as_u32;
    }
}

} // namespace ShaderCommon
