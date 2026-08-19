// ============================================================================
// cdGunSightShader.cpp — gun sight shader (6 non-inline funcs).
// Source: source/cdGunSightShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGunSightShader.o):
//   cdGunSightShaderMat::ctor @0x7CE100
//   InitCDGunSightShader  @0x7CE190
//   ToggleCDGunSightShader @0x7CE1E0
//   cdGunSightShader::Register @0x7CE200
//   cdGunSightShader::AddNode @0x7CE240
// ============================================================================
#include "cdGunSightShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "ngl/ngl_lighting.h"
#include "render/ShaderCommon.h"

#include <cstdint>
#include <intrin.h>

// Shader global pointer definitions
cdGunSightShader* gCDGunSightShader = nullptr;  // ?gCDGunSightShader@@3PAVcdGunSightShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdGunSightRender {
    unsigned int VS[1] = {};
    unsigned int const* VShaderTable[1] = {};
}
namespace cdGunSightPixel {
    unsigned int* PS[1] = {};
    unsigned int const* PShaderTable[1] = {};
}
namespace cdGunSightFullbrightPixel {
    unsigned int* PS[1] = {};
    unsigned int const* PShaderTable[1] = {};
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
// cdGunSightShaderMat::cdGunSightShaderMat — bind texture + shader.
// ea: 0x7CE100
// ============================================================================
cdGunSightShaderMat::cdGunSightShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdGunSightShader* v3 = gCDGunSightShader;
    if (gCDGunSightShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdGunSightShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "gCDGunSightShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDGunSightShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDGunSightShader;
}

// ============================================================================
// InitCDGunSightShader — allocate the shader and link into the init list.
// ea: 0x7CE190
// ============================================================================
void InitCDGunSightShader() {
    cdGunSightShader* result = (cdGunSightShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdGunSightShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdGunSightShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~0x10;
        gCDGunSightShader = result;
    } else {
        gCDGunSightShader = NULL;
    }
}

// ============================================================================
// ToggleCDGunSightShader — toggle gun-sight enable bit (bit 4).
// ea: 0x7CE1E0
// ============================================================================
void ToggleCDGunSightShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (16 * ~(byte >> 4))) & 0x10) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;
}

// ============================================================================
// cdGunSightShader::Register — register the gun-sight vertex/pixel shaders.
// ea: 0x7CE200
// ============================================================================
tlFixedString cdGunSightShader::GetName() { return tlFixedString("cdGunSight"); }

void cdGunSightShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdGunSightRender::VS, cdGunSightRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdGunSightPixel::PS, cdGunSightPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdGunSightFullbrightPixel::PS, cdGunSightFullbrightPixel::PShaderTable, 0);
}

// ============================================================================
// cdGunSightShader::AddNode — add a gun-sight node to the opaque list.
// ea: 0x7CE240
// ============================================================================
void cdGunSightShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                               nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 0x10) == 0) {
        cdGunSightShaderNode* node = (cdGunSightShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdGunSightShaderNode
            node->mMaterial = (cdGunSightShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDGunSightShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ============================================================================
// cdGunSightShaderNode::Render — ea: 0x7CE2B0
// ============================================================================
void cdGunSightShaderNode::Render() {
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
    math::Mat44 dir;
    math::Mat44 color;
    nglGetDirLightMatrix(this->MeshNode, &dir, &color);

    SimpleContext context;
    context.mLToS = this->MeshNode->LocalToScreen;
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &context, 0x30u);

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
    const unsigned int vertexShader = cdGunSightRender::VS[0];
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    const _D3DPixelShaderDef* pixelShader =
        reinterpret_cast<const _D3DPixelShaderDef*>(
            reinterpret_cast<uintptr_t>(cdGunSightFullbrightPixel::PS[0]));
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = reinterpret_cast<const _D3DPixelShaderDef*>(
            reinterpret_cast<uintptr_t>(cdGunSightPixel::PS[0]));
    if (pixelShader != reinterpret_cast<const _D3DPixelShaderDef*>(
                          static_cast<uintptr_t>(gpuHashPixelShader))) {
        gpuHashPixelShader = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pixelShader));
        D3DDevice_SetPixelShaderProgram(pixelShader);
    }

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
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }
    nglGpuDrawSection(this->Section);
}
