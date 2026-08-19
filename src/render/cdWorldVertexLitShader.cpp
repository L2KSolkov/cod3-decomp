// ============================================================================
// cdWorldVertexLitShader.cpp — world vertex-lit shader (6 non-inline funcs).
// Source: source/cdWorldVertexLitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldVertexLitShader.o):
//   cdWorldVertexLitShaderMat::ctor @0x7DE770
//   InitCDWorldVertexLitShader  @0x7DE800
//   ToggleCDWorldVertexLitShader @0x7DE850
//   cdWorldVertexLitShader::Register @0x7DE870
//   cdWorldVertexLitShader::AddNode @0x7DE8D0
// ============================================================================
#include "cdWorldVertexLitShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "render/ShaderCommon.h"

#include <intrin.h>
#include <cstdint>

// Shader global pointer definitions
cdWorldVertexLitShader* gCDWorldVertexLitShader = nullptr;  // ?gCDWorldVertexLitShader@@3PAVcdWorldVertexLitShader@@A
cdWorldVertexLitShader* g_cdWorldVertexLitShader = nullptr;  // ?g_cdWorldVertexLitShader@@3PAVcdWorldVertexLitShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdWorldVertexLitRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
    unsigned long Shader = 0;
}
namespace cdWorldVertexLitPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
    unsigned long* Shader = nullptr;
}
namespace cdWorldVertexLitFullbrightPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
    unsigned long* Shader = nullptr;
}

namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
    extern ECoderId gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int   gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
}

extern unsigned int dword_40300;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D80;
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;

// ============================================================================
// cdWorldVertexLitShaderNode::Render — ea: 0x7DE940
// ============================================================================
void cdWorldVertexLitShaderNode::Render() {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 0);
        dword_BC2D00 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0);
        dword_BC2D04 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x206u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, 0x206u);
        dword_BC2CF8 = 0x206u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);

    nglDxState.SetBlendMode(0x64CF8600u);
    nglDxSetupVShaderFog(-78, this->MeshNode, nglBuildScene->FogNear,
                         nglBuildScene->FogFar, nglBuildScene->FogMin,
                         nglBuildScene->FogMax);

    const __m128 fogScaled =
        _mm_mul_ps(nglBuildScene->FogColor.v, _mm_set1_ps(127.0f));
    const unsigned int c0 = static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[0]));
    const unsigned int c1 = static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[1]));
    const unsigned int c2 = static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[2]));
    const unsigned int c3 = static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[3]));
    const unsigned int fogColor = c2 | (c1 << 8) | (c0 << 16) | (c3 << 24);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_FOGCOLOR, fogColor) == 0)
        D3DDevice_SetRenderState_FogColor(fogColor);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1u) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }

    BackgroundContext context;
    context.mLToS = this->MeshNode->LocalToScreen;
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &context, 0x10u);

    nglDxSetTexture(0, this->mMaterial->mTexture, 1u, 3u);
    if (nglDxTexCache.Prev[0].WrapU != 1) {
        nglDxTexCache.Prev[0].WrapU = 1;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 1) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSU] = 1;
        }
    }
    if (nglDxTexCache.Prev[0].WrapV != 1) {
        nglDxTexCache.Prev[0].WrapV = 1;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 1) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSV] = 1;
        }
    }

    nglDxInitShaders(false);
    const unsigned int vertexShader = static_cast<unsigned int>(
        cdWorldVertexLitRender::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    const unsigned int* pixelShader = reinterpret_cast<const unsigned int*>(
        cdWorldVertexLitFullbrightPixel::PS[0]);
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = reinterpret_cast<const unsigned int*>(cdWorldVertexLitPixel::PS[0]);
    if (pixelShader != reinterpret_cast<const unsigned int*>(
                           static_cast<uintptr_t>(gpuHashPixelShader))) {
        gpuHashPixelShader = static_cast<unsigned int>(
            reinterpret_cast<uintptr_t>(pixelShader));
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(pixelShader));
    }

    nglGpuDrawSection(this->Section);
    nglDxState.PrevBM = static_cast<unsigned int>(-1);
}

// ============================================================================
// cdWorldVertexLitShaderMat::ctor — bind texture + shader.
// ea: 0x7DE770
// ============================================================================
cdWorldVertexLitShaderMat::cdWorldVertexLitShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdWorldVertexLitShader* v3 = g_cdWorldVertexLitShader;
    if (g_cdWorldVertexLitShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdWorldVertexLitShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "g_cdWorldVertexLitShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = g_cdWorldVertexLitShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = g_cdWorldVertexLitShader;
}

// ============================================================================
// InitCDWorldVertexLitShader — allocate the shader and link into the init list.
// ea: 0x7DE800
// ============================================================================
void InitCDWorldVertexLitShader() {
    cdWorldVertexLitShader* result = (cdWorldVertexLitShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldVertexLitShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~4;
        gCDWorldVertexLitShader = result;
    } else {
        gCDWorldVertexLitShader = NULL;
    }
}

// ============================================================================
// ToggleCDWorldVertexLitShader — toggle vertex-lit enable bit (bit 2).
// ea: 0x7DE850
// ============================================================================
void ToggleCDWorldVertexLitShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (4 * ~(byte >> 2))) & 4) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;
}

// ============================================================================
// cdWorldVertexLitShader::Register — register the shaders.
// ea: 0x7DE870
// ============================================================================
void cdWorldVertexLitShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdWorldVertexLitRender::VS, cdWorldVertexLitRender::VShaderTable, 0);
    cdWorldVertexLitRender::Shader = cdWorldVertexLitRender::VS != nullptr ? cdWorldVertexLitRender::VS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdWorldVertexLitPixel::PS, cdWorldVertexLitPixel::PShaderTable, 0);
    cdWorldVertexLitPixel::Shader = cdWorldVertexLitPixel::PS != nullptr ? cdWorldVertexLitPixel::PS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdWorldVertexLitFullbrightPixel::PS, cdWorldVertexLitFullbrightPixel::PShaderTable, 0);
    cdWorldVertexLitFullbrightPixel::Shader = cdWorldVertexLitFullbrightPixel::PS != nullptr ? cdWorldVertexLitFullbrightPixel::PS[0] : 0;
}

// ============================================================================
// cdWorldVertexLitShader::AddNode — add a node to the opaque list.
// ea: 0x7DE8D0
// ============================================================================
void cdWorldVertexLitShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                     nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 4) == 0) {
        cdWorldVertexLitShaderNode* node = (cdWorldVertexLitShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdWorldVertexLitShaderNode
            node->mMaterial = (cdWorldVertexLitShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDWorldVertexLitShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
