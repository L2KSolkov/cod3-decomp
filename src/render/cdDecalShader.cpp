// ============================================================================
// cdDecalShader.cpp — decal shader (6 non-inline funcs).
// Source: source/cdDecalShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDecalShader.o):
//   cdDecalShaderMat::ctor @0x7D1720
//   InitCDDecalShader  @0x7D17B0
//   ToggleCDDecalShader @0x7D1800
//   cdDecalShader::Register @0x7D1820
//   cdDecalShader::AddNode @0x7D1860
// ============================================================================
#include "cdDecalShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"

#include <intrin.h>
#include <new>

extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_40344;
extern unsigned int dword_40348;
extern unsigned int dword_40350;
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D08;
extern unsigned int dword_BC2D0C;
extern unsigned int dword_BC2D38;
extern unsigned int dword_BC2D80;
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;

// Shader global pointer definitions
cdDecalShader* gCDDecalShader = nullptr;  // ?gCDDecalShader@@3PAVcdDecalShader@@A
cdDecalShader* g_cdDecalShader = nullptr;  // ?g_cdDecalShader@@3PAVcdDecalShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdDecalRender {
    static const unsigned int VShaderMicrocode[49] = {
        0x000c2078, 0x00000000, 0x0062601a, 0x08001468, 0xfeb00000,
        0x00000000, 0x00e1201b, 0x08373800, 0x20a01800, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x06e0c01b,
        0x0836dbff, 0x10a88800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0a44800, 0x00000000, 0x00e1001b, 0x08371800, 0x20a02800,
        0x00000000, 0x00824055, 0x14016d54, 0xb8000000, 0x00000000,
        0x02000400, 0x0800106c, 0xa0b0f81c, 0x00000000, 0x00424000,
        0x05545800, 0x28000000, 0x00000000, 0x00824000, 0x05fe5800,
        0xb8000000, 0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}
namespace cdDecalPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8d41010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000001,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = nullptr;
}
namespace cdDecalFullbrightPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8200000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000001,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = nullptr;
}
// ============================================================================
// InitCDDecalShader — allocate the shader and link into the init list.
// ea: 0x7D17B0
// ============================================================================
void InitCDDecalShader() {
    cdDecalShader* result = (cdDecalShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdDecalShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdDecalShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~0x20;
        gCDDecalShader = result;
    } else {
        gCDDecalShader = NULL;

    }

}

// ============================================================================
// ToggleCDDecalShader — toggle the decal-shader enable bit (bit 5).
// ea: 0x7D1800
// ============================================================================
void ToggleCDDecalShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (32 * ~(byte >> 5))) & 0x20) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;

}

// ============================================================================
// cdDecalShader::Register — register the decal vertex/pixel shaders.
// ea: 0x7D1820
// ============================================================================
tlFixedString cdDecalShader::GetName() { return tlFixedString("cdDecal"); }

void cdDecalShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdDecalRender::VS, cdDecalRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdDecalPixel::PS, cdDecalPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdDecalFullbrightPixel::PS, cdDecalFullbrightPixel::PShaderTable, 0);
}

// ============================================================================
// cdDecalShader::AddNode — add a decal node to the opaque list.
// ea: 0x7D1860
// ============================================================================
void cdDecalShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 0x20) == 0) {
        cdDecalShaderNode* node = (cdDecalShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            ::new (node) cdDecalShaderNode;
            node->mMaterial = (cdDecalShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDDecalShader->ID | 0x80000000;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ============================================================================
// cdDecalShaderNode::Render — ea: 0x7D18E0
// Ported from the reference C dump. The decal path uses its local-to-screen
// matrix, z-bias, alpha blend state, and the registered decal shader pair.
// ============================================================================
void cdDecalShaderNode::Render() {
    const unsigned int cullMode = this->mMaterial->mCullMode == 2 ? 0u : 0x900u;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, cullMode) == 0)
        D3DDevice_SetRenderState_CullMode(cullMode);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 1u);
        dword_BC2D00 = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x204u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, 0x204u);
        dword_BC2CF8 = 0x204u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0);
        dword_BC2D04 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 1u);
        dword_BC2CFC = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, 0x8006u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40350, 0x8006u);
        dword_BC2D38 = 0x8006u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, 0x302u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40344, 0x302u);
        dword_BC2D08 = 0x302u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, 0x303u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40348, 0x303u);
        dword_BC2D0C = 0x303u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZBIAS, 0xAu) == 0)
        D3DDevice_SetRenderState_ZBias(0xAu);

    nglDxState.PrevBM = static_cast<unsigned int>(-1);
    const float dist = this->GetDist(nglBuildScene->WorldToView);
    const float depth = dist >= 3.0f ? dist : 3.0f;

    math::Mat44 context = this->MeshNode->LocalToScreen;
    context.z.v.m128_f32[3] -= (this->mMaterial->mZBias / depth) * 10000.0f;

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
    const unsigned int vertexShader = static_cast<unsigned int>(cdDecalRender::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const _D3DPixelShaderDef* pixelShader =
        reinterpret_cast<const _D3DPixelShaderDef*>(cdDecalFullbrightPixel::PS[0]);
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = reinterpret_cast<const _D3DPixelShaderDef*>(cdDecalPixel::PS[0]);
    if (pixelShader != reinterpret_cast<const _D3DPixelShaderDef*>(gpuHashPixelShader)) {
        gpuHashPixelShader = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pixelShader));
        D3DDevice_SetPixelShaderProgram(pixelShader);
    }

    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &context, 0x10u);
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
    nglGpuDrawSection(this->Section);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZBIAS, 0) == 0)
        D3DDevice_SetRenderState_ZBias(0);
}
