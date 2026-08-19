// ============================================================================
// cdSimpleShader.cpp — simple shader (12 non-inline funcs).
// Source: source/cdSimpleShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleShader.o):
//   cdSimpleShaderMat::ctor @0x7D6390
//   InitCDSimpleShader  @0x7D6420
//   ToggleCDSimpleShader @0x7D6470
//   cdSimpleShader::Register @0x7D6490
//   cdSimpleShader::AddNode @0x7D64C0
//   cdSimpleShaderNode::Render @0x7D6540
//   cdSimpleRender/cdSimplePixel/cdSimpleFullbrightPixel helpers @0x7D6910-0x7D69A0
// ============================================================================
#include "cdSimpleShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_lighting.h"
#include "render/ShaderCommon.h"

#include <intrin.h>

// Shader global pointer definitions
cdSimpleShader* gCDSimpleShader = nullptr;  // ?gCDSimpleShader@@3PAVcdSimpleShader@@A

// Shader static data definitions (render_xboxr cdSimpleShader.o).  The
// payload pointers are intentionally null until the Xbox microcode has a
// verified D3D9/HLSL translation.
namespace cdSimpleRender {
    unsigned int VS[2] = {};
    const unsigned int* VShaderTable[2] = {};
}
namespace cdSimplePixel {
    unsigned int* PS[1] = {};
    const unsigned int* PShaderTable[1] = {};
}
namespace cdSimpleFullbrightPixel {
    unsigned int* PS[1] = {};
    const unsigned int* PShaderTable[1] = {};
}

extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D80;
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;

// ============================================================================
// cdSimpleShaderMat::cdSimpleShaderMat — bind the texture + shader.
// ea: 0x7D6390
// ============================================================================
cdSimpleShaderMat::cdSimpleShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    this->Shader = gCDSimpleShader;
}

// ============================================================================
// cdSimpleShader::Register — ea: 0x7D6490
// ============================================================================
void cdSimpleShader::Register() {
    nglShader::Register();
    cdSimpleRender::RegisterVShader();
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(cdSimplePixel::PS), cdSimplePixel::PShaderTable[0]);
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(cdSimpleFullbrightPixel::PS),
                         cdSimpleFullbrightPixel::PShaderTable[0]);
}

// ============================================================================
// InitCDSimpleShader — allocate the shader and link into the init list.
// ea: 0x7D6420
// ============================================================================
void InitCDSimpleShader() {
    cdSimpleShader* result = (cdSimpleShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSimpleShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~4;
        gCDSimpleShader = result;
    } else {
        gCDSimpleShader = NULL;

    }

}

// ============================================================================
// ToggleCDSimpleShader — toggle the simple-shader enable bit (bit 2).
// ea: 0x7D6470
// ============================================================================
void ToggleCDSimpleShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ (4 * ~(byte >> 2))) & 4) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;

}

// ============================================================================
// cdSimpleShader::AddNode — add a simple shader node to the opaque list.
// ea: 0x7D64C0
// ============================================================================
void cdSimpleShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                             nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 4) == 0) {
        cdSimpleShaderNode* node = (cdSimpleShaderNode*)nglListAlloc(0x1C, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdSimpleShaderNode
            node->mMaterial = (cdSimpleShaderMat*)iMat;
            node->hasColorVerts = false;
        } else {
            node = NULL;
        }
        node->SortHash = gCDSimpleShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ============================================================================
// cdSimpleShaderNode::Render — ea: 0x7D6540
// ============================================================================
void cdSimpleShaderNode::Render() {
    const unsigned int cullMode = this->mMaterial->mCullMode == 2 ? 0u : 0x900u;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, cullMode) == 0)
        D3DDevice_SetRenderState_CullMode(cullMode);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 0);
        dword_BC2CFC = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 1);
        dword_BC2D00 = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x204) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, 0x204);
        dword_BC2CF8 = 0x204;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0x80) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0x80);
        dword_BC2D04 = 0x80;
    }

    nglDxState.PrevBM = (unsigned int)-1;
    nglDetermineLights(this->MeshNode);

    math::Mat44 Dir;
    math::Mat44 Color;
    nglGetDirLightMatrix(this->MeshNode, &Dir, &Color);

    SimpleContext context;
    context.mLToS = this->MeshNode->LocalToScreen;
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &context, 0x30u);

    nglTextureAnimFrame = nglBuildScene->IFLFrame;
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
    const unsigned int vertexShader = cdSimpleRender::VS[this->hasColorVerts ? 1 : 0];
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    const _D3DPixelShaderDef* pixelShader =
        reinterpret_cast<const _D3DPixelShaderDef*>(cdSimpleFullbrightPixel::PS[0]);
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = reinterpret_cast<const _D3DPixelShaderDef*>(cdSimplePixel::PS[0]);
    if (pixelShader != reinterpret_cast<const _D3DPixelShaderDef*>(gpuHashPixelShader)) {
        gpuHashPixelShader = (unsigned int)pixelShader;
        D3DDevice_SetPixelShaderProgram(pixelShader);
    }

    nglDxSetupVShaderFog(-78, this->MeshNode, nglBuildScene->FogNear,
                         nglBuildScene->FogFar, nglBuildScene->FogMin,
                         nglBuildScene->FogMax);
    D3DDevice_SetVertexShaderConstant1Fast(22, &nglBuildScene->FogColor);

    const __m128 fogScaled = _mm_mul_ps(nglBuildScene->FogColor.v, _mm_set1_ps(127.0f));
    const unsigned int c0 = (unsigned int)(int)fogScaled.m128_f32[0];
    const unsigned int c1 = (unsigned int)(int)fogScaled.m128_f32[1];
    const unsigned int c2 = (unsigned int)(int)fogScaled.m128_f32[2];
    const unsigned int c3 = (unsigned int)(int)fogScaled.m128_f32[3];
    const unsigned int fogColor = c2 | (c1 << 8) | (c0 << 16) | (c3 << 24);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_FOGCOLOR, fogColor) == 0)
        D3DDevice_SetRenderState_FogColor(fogColor);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }
    nglGpuDrawSection(this->Section);
}

// ============================================================================
// cdSimpleRender/cdSimplePixel/cdSimpleFullbrightPixel helpers
// ============================================================================
void cdSimpleRender::RegisterVShader() {
    for (int i = 0; i != 2; ++i)
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(&VS[i]), VShaderTable[i]);
}

unsigned int cdSimpleRender::GetVShader(unsigned int index) {
    return VS[index];
}

void cdSimplePixel::RegisterPShader() {
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(PS), PShaderTable[0]);
}

unsigned int* cdSimplePixel::GetPShader() {
    return PS[0];
}

void cdSimpleFullbrightPixel::RegisterPShader() {
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(PS), PShaderTable[0]);
}

unsigned int* cdSimpleFullbrightPixel::GetPShader() {
    return PS[0];
}
