// ============================================================================
// cdScratchShader.cpp — scratch shader (6 non-inline funcs).
// Source: source/cdScratchShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdScratchShader.o):
//   cdScratchMaterial::ctor @0x7C5660
//   ToggleCDScratchShader @0x7C5690
//   cdScratchShader::Register @0x7C56B0
//   InitCDScratchShader  @0x7C56E0
//   cdScratchShader::AddNode @0x7C5720
// ============================================================================
#include "cdScratchShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"

#include <cstdint>
#include <intrin.h>

// Shader global pointer definitions
cdScratchShader* gCDScratchShader = nullptr;  // ?gCDScratchShader@@3PAVcdScratchShader@@A

extern unsigned int nglTextureFrameParamID;
extern unsigned int nglZBiasParamID;
extern int nglTextureAnimFrame;
extern unsigned int dword_BC2D80;
extern unsigned int D3D__DirtyFlags;
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdScratchShaderVertex {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
}
namespace cdScratchShaderPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
}

// ============================================================================
// cdScratchMaterial::cdScratchMaterial — bind texture + blend + shader.
// ea: 0x7C5660
// ============================================================================
cdScratchMaterial::cdScratchMaterial(nglTexture* tex, unsigned int BlendMode,
                                     int mapflags, bool HeatHaze) {
    this->Texture = tex;
    this->BlendMode = BlendMode;
    this->MapFlags = mapflags;
    this->HeatHaze = HeatHaze;
    this->Shader = gCDScratchShader;
}

// ============================================================================
// ToggleCDScratchShader — toggle the scratch-shader enable (high bit).
// ea: 0x7C5690
// ============================================================================
void ToggleCDScratchShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    ShaderCommon::ShaderSwitching.__s0[2] =
        (unsigned char)(~byte ^ ((byte ^ ~byte) & 0x7F));
}

// ============================================================================
// cdScratchShader::Register — register the scratch vertex/pixel shaders.
// ea: 0x7C56B0
// ============================================================================
void cdScratchShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdScratchShaderVertex::VS, cdScratchShaderVertex::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdScratchShaderPixel::PS, cdScratchShaderPixel::PShaderTable, 0);
}

// ============================================================================
// InitCDScratchShader — allocate the shader and link into the init list.
// ea: 0x7C56E0
// ============================================================================
void InitCDScratchShader() {
    cdScratchShader* result = (cdScratchShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdScratchShader
        gCDScratchShader = result;
    } else {
        gCDScratchShader = NULL;
    }
}

// ============================================================================
// cdScratchShader::AddNode — add a scratch node to opaque/transparent list.
// ea: 0x7C5720
// ============================================================================
void cdScratchShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                              nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.as_u32 & 0x800000) == 0) {
        cdScratchShaderNode* node = (cdScratchShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdScratchShaderNode
            node->Material = (cdScratchMaterial*)iMat;
        } else {
            node = NULL;
        }
        nglShader* Shader = (nglShader*)((unsigned int*)iMat)[5];
        if (Shader == NULL || Shader == (nglShader*)0x10000) {
            node->SortHash = gCDScratchShader->ID;
            node->Next = nglBuildScene->OpaqueRenderList;
            nglBuildScene->OpaqueRenderList = node;
            ++nglBuildScene->OpaqueListCount;
        } else {
            node->SortDist = node->GetDist(nglBuildScene->WorldToView);
            node->Next = nglBuildScene->TransRenderList;
            nglBuildScene->TransRenderList = node;
            ++nglBuildScene->TransListCount;
        }
    }
}

// ============================================================================
// cdScratchShaderNode::Render — ea: 0x7C57F0
// ============================================================================
void cdScratchShaderNode::Render() {
    const unsigned int* array = this->MeshNode->ShaderParams.Array;
    const unsigned int textureFrameId = nglTextureFrameParamID;
    if ((1u << (textureFrameId & 0x1Fu)) & array[textureFrameId >> 5]) {
        // IDA type: nglTextureFrameParamType::Value is int.
        nglTextureAnimFrame =
            *reinterpret_cast<const int*>(array + textureFrameId + 2);
    } else {
        nglTextureAnimFrame = nglBuildScene->IFLFrame;
    }

    const unsigned int zBiasId = nglZBiasParamID;
    if ((1u << (zBiasId & 0x1Fu)) & array[zBiasId >> 5]) {
        // IDA type: nglZBiasParamType::Value is float; the original converts
        // it through _ftol2 before sending the integer render-state value.
        const float zBias = *reinterpret_cast<const float*>(array + zBiasId + 2);
        const unsigned int value =
            static_cast<unsigned int>(static_cast<int>(zBias));
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZBIAS, value) == 0)
            D3DDevice_SetRenderState_ZBias(value);
    }

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    nglDxState.SetBlendMode(this->Material->BlendMode);
    D3DDevice_SetVertexShaderConstantNotInlineFast(
        6, &this->MeshNode->LocalToScreen, 0x10u);
    nglDxInitShaders(false);

    const unsigned int vertexShader = static_cast<unsigned int>(
        cdScratchShaderVertex::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    nglDxSetTexture(0, this->Material->Texture, this->Material->MapFlags, 3u);
    nglDxSetTextureU(0, (this->Material->MapFlags & 0x40) != 0 ? 3u : 1u);
    nglDxSetTextureV(0, (this->Material->MapFlags & 0x80) != 0 ? 3u : 1u);

    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(cdScratchShaderPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(static_cast<uintptr_t>(pixelShader)));
    }

    nglDxSetupVShaderFog(-86, this->MeshNode, nglBuildScene->FogNear,
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
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }
    nglGpuDrawSection(this->Section);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZBIAS, 0) == 0)
        D3DDevice_SetRenderState_ZBias(0);
}
