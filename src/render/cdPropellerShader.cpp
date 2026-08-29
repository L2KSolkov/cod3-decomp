// ============================================================================
// cdPropellerShader.cpp — propeller shader (6 non-inline funcs).
// Source: source/cdPropellerShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdPropellerShader.o):
//   cdPropellerShaderMat::ctor @0x7D0D50
//   InitCDPropellerShader  @0x7D0DE0
//   ToggleCDPropellerShader @0x7D0E30
//   cdPropellerShader::Register @0x7D0E50
//   cdPropellerShader::AddNode @0x7D0E90
// ============================================================================
#include "cdPropellerShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "ngl/ngl_lighting.h"

#include <intrin.h>
#include <cstdint>

// Shader global pointer definitions
cdPropellerShader* gCDPropellerShader = nullptr;  // ?gCDPropellerShader@@3PAVcdPropellerShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdPropellerRender {
    // Exact release shader microcode from render_xboxr:cdPropellerShaderVertex.o.
    static const unsigned int VShaderMicrocode[157] = {
        0x00272078u, 0x00000000u, 0x00224000u, 0x2c001000u, 0x28300000u, 0x00000000u, 0x0062e01au, 0x08001469u,
        0xfeb00000u, 0x00000000u, 0x00804000u, 0x35fe5954u, 0xb8400000u, 0x00000000u, 0x00a0001au, 0xb4356800u,
        0x28b00000u, 0x00000000u, 0x0aa1421au, 0x18355801u, 0x18040000u, 0x00000000u, 0x08a1621au, 0x18357802u,
        0xd4080000u, 0x00000000u, 0x00804055u, 0x15545c00u, 0xb8400000u, 0x00000000u, 0x00a1821au, 0x18359800u,
        0x22000000u, 0x00000000u, 0x01000000u, 0x44008800u, 0x2c500000u, 0x00000000u, 0x00400055u, 0x54aaa800u,
        0x22500000u, 0x00000000u, 0x00400055u, 0x5554a800u, 0x21500000u, 0x00000000u, 0x0040001bu, 0x54008800u,
        0x2f400000u, 0x00000000u, 0x00e0801bu, 0x44369800u, 0x24400000u, 0x00000000u, 0x00e0601bu, 0x54367800u,
        0x28400000u, 0x00000000u, 0x00a1a21au, 0x1835b800u, 0x21000000u, 0x00000000u, 0x0082c000u, 0x14016d55u,
        0xb8b00000u, 0x00000000u, 0x0140201bu, 0x04003800u, 0x2f000000u, 0x00000000u, 0x00602405u, 0x28001554u,
        0x76200000u, 0x00000000u, 0x00202000u, 0x1c001000u, 0x28200000u, 0x00000000u, 0x00e1201bu, 0x08373800u,
        0x20a01800u, 0x00000000u, 0x0042c000u, 0xb554d800u, 0x28b00000u, 0x00000000u, 0x06202000u, 0x1c0013ffu,
        0x18680000u, 0x00000000u, 0x00200055u, 0x44001000u, 0x24600000u, 0x00000000u, 0x00200100u, 0x44001000u,
        0x21600000u, 0x00000000u, 0x00202000u, 0x1c001000u, 0x22700000u, 0x00000000u, 0x00200054u, 0x44001000u,
        0x25700000u, 0x00000000u, 0x00e0c01bu, 0x0836d800u, 0x20b08800u, 0x00000000u, 0x00e0e01bu, 0x0836f800u,
        0x20a04800u, 0x00000000u, 0x00e1001bu, 0x08371800u, 0x20a02800u, 0x00000000u, 0x0082c000u, 0xb5fed801u,
        0xb8b00000u, 0x00000000u, 0x00a00060u, 0x24e0c800u, 0x28800000u, 0x00000000u, 0x00a00060u, 0x25b4e800u,
        0x24800000u, 0x00000000u, 0x00e1c01bu, 0x0437d800u, 0x28900000u, 0x00000000u, 0x00e1e01bu, 0x0437f800u,
        0x24900000u, 0x00000000u, 0x00e2001bu, 0x04361800u, 0x22900000u, 0x00000000u, 0x006020aau, 0x1c001402u,
        0xd0b0f828u, 0x00000000u, 0x00602015u, 0x84001154u, 0x70b0c848u, 0x00000000u, 0x0062201au, 0x94001068u,
        0x70b0e818u, 0x00000000u, 0x0040001au, 0xc4002800u, 0x20b0e801u,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
}
namespace cdPropellerPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8d41010u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x130c0300u, 0x00001c80u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0x000000c0u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0xc8c40000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x000000c0u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00011101u, 0x00000001u, 0x00000000u,
        0x00000000u, 0xffffffffu, 0xffffffffu, 0x000001ffu,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
}
namespace cdPropellerFullbrightPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8301010u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x130c0300u, 0x00001c80u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0x000000c0u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0xc8200000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x000000c0u, 0x00000000u, 0x00000000u,
        0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00011101u, 0x00000001u, 0x00000000u,
        0x00000000u, 0xffffffffu, 0xffffffffu, 0x000001ffu,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
}

extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_40340;
extern unsigned int dword_40344;
extern unsigned int dword_40348;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D08;
extern unsigned int dword_BC2D0C;
extern unsigned int dword_BC2D80;
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;

namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
    extern ECoderId gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int   gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
}

// ============================================================================
// cdPropellerShaderMat::cdPropellerShaderMat — bind texture + shader.
// ea: 0x7D0D50
// ============================================================================
cdPropellerShaderMat::cdPropellerShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdPropellerShader* v3 = gCDPropellerShader;
    if (gCDPropellerShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdPropellerShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "gCDPropellerShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDPropellerShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDPropellerShader;
}

// ============================================================================
// InitCDPropellerShader — allocate the shader and link into the init list.
// ea: 0x7D0DE0
// ============================================================================
void InitCDPropellerShader() {
    cdPropellerShader* result = (cdPropellerShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdPropellerShader;
        gCDPropellerShader = result;
    } else {
        gCDPropellerShader = NULL;
    }
}

// ea: 0x7D12B0
cdPropellerShader::cdPropellerShader() {
    next = tlInitList::head;
    tlInitList::head = this;
    Disabled = false;
    ShaderCommon::ShaderSwitching.__s0[3] &= ~1;
}

// ============================================================================
// ToggleCDPropellerShader — toggle propeller-shader enable bit (bit 0).
// ea: 0x7D0E30
// ============================================================================
void ToggleCDPropellerShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ ~byte) & 1) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;
}

// ea: 0x7D1300
void cdPropellerRender::RegisterVShader() {
    nglDxRegisterVShader(cdPropellerRender::VS,
                         cdPropellerRender::VShaderTable[0]);
}

// ea: 0x7D1320
unsigned long cdPropellerRender::GetVShader() {
    return cdPropellerRender::VS[0];
}

// ea: 0x7D1330
void cdPropellerPixel::RegisterPShader() {
    nglDxRegisterPShader(cdPropellerPixel::PS,
                         cdPropellerPixel::PShaderTable[0]);
}

// ea: 0x7D1350
unsigned long* cdPropellerPixel::GetPShader() {
    return cdPropellerPixel::PS[0];
}

// ea: 0x7D1360
void cdPropellerFullbrightPixel::RegisterPShader() {
    nglDxRegisterPShader(cdPropellerFullbrightPixel::PS,
                         cdPropellerFullbrightPixel::PShaderTable[0]);
}

// ea: 0x7D1380
unsigned long* cdPropellerFullbrightPixel::GetPShader() {
    return cdPropellerFullbrightPixel::PS[0];
}

// ea: 0x7D12E0
tlFixedString cdPropellerShader::GetName() { return tlFixedString("cdPropeller"); }

// ============================================================================
// cdPropellerShader::Register — register the propeller vertex/pixel shaders.
// ea: 0x7D0E50
// ============================================================================
void cdPropellerShader::Register() {
    nglShader::Register();
    cdPropellerRender::RegisterVShader();
    cdPropellerPixel::RegisterPShader();
    cdPropellerFullbrightPixel::RegisterPShader();
}

// ea: 0x7D1390
cdPropellerShaderNode::cdPropellerShaderNode(nglMeshNode* iMeshNode,
                                             nglMeshSection* iSection,
                                             cdPropellerShaderMat* iMaterial) {
    MeshNode = iMeshNode;
    Section = iSection;
    mMaterial = iMaterial;
}

// ea: 0x7D13F0
cdPropellerShaderNode::~cdPropellerShaderNode() = default;

// ea: 0x7D1430
cdPropellerShader::~cdPropellerShader() = default;

// ea: 0x7D1440
PropellerContext::PropellerContext() {}

// ea: 0x7D0F10
void cdPropellerShaderNode::Render() {
    const unsigned int cullMode = mMaterial->mCullMode == 2 ? 0u : 0x900u;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, cullMode) == 0)
        D3DDevice_SetRenderState_CullMode(cullMode);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 1u);
        dword_BC2CFC = 1u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, 0x302u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40344, 0x302u);
        dword_BC2D08 = 0x302u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, 0x303u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40348, 0x303u);
        dword_BC2D0C = 0x303u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 0u);
        dword_BC2D00 = 0u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0x80u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0x80u);
        dword_BC2D04 = 0x80u;
    }

    PropellerContext context;
    nglDetermineLights(MeshNode);
    nglGetDirLightMatrix(MeshNode, &context.mLightMatrices[0],
                         &context.mLightMatrices[1]);
    context.mLToS = MeshNode->LocalToScreen;
    context.mRotation.v.m128_f32[0] =
        (mMaterial->mRotation * ShaderCommon::gTime) * 6.2831855f;
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &context, 0x34u);

    nglDxSetTexture(0, mMaterial->mTexture, 1u, 3u);
    if (nglDxTexCache.Prev[0].WrapU != 3) {
        nglDxTexCache.Prev[0].WrapU = 3;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 3u) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSU] = 3u;
        }
    }
    if (nglDxTexCache.Prev[0].WrapV != 3) {
        nglDxTexCache.Prev[0].WrapV = 3;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 3u) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSV] = 3u;
        }
    }

    nglDxInitShaders(false);
    const unsigned int vertexShader = static_cast<unsigned int>(cdPropellerRender::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    unsigned long* pixelShader = cdPropellerFullbrightPixel::PS[0];
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = cdPropellerPixel::PS[0];
    if (pixelShader != reinterpret_cast<unsigned long*>(
                          static_cast<uintptr_t>(gpuHashPixelShader))) {
        gpuHashPixelShader = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pixelShader));
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(pixelShader));
    }

    nglDxSetupVShaderFog(-74, MeshNode, nglBuildScene->FogNear,
                         nglBuildScene->FogFar, nglBuildScene->FogMin,
                         nglBuildScene->FogMax);
    const __m128 fogScaled =
        _mm_mul_ps(nglBuildScene->FogColor.v, _mm_set1_ps(127.0f));
    const unsigned int c0 = static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[0]));
    const unsigned int c1 = static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[1]));
    const unsigned int c2 = static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[2]));
    const unsigned int c3 = static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[3]));
    const unsigned int fogColor = (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_FOGCOLOR, fogColor) == 0)
        D3DDevice_SetRenderState_FogColor(fogColor);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1u) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1u;
    }
    nglGpuDrawSection(Section);
    nglDxState.PrevBM = static_cast<unsigned int>(-1);
}

// ============================================================================
// cdPropellerShader::AddNode — add a propeller node to the transparent list.
// ea: 0x7D0E90
// ============================================================================
void cdPropellerShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 1) == 0) {
        cdPropellerShaderNode* node = (cdPropellerShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdPropellerShaderNode
            node->mMaterial = (cdPropellerShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortDist = node->GetDist(nglBuildScene->WorldToView);
        node->Next = nglBuildScene->TransRenderList;
        nglBuildScene->TransRenderList = node;
        ++nglBuildScene->TransListCount;
    }
}
