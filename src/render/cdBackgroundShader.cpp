// ============================================================================
// cdBackgroundShader.cpp — background shader (6 non-inline funcs).
// Source: source/cdBackgroundShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdBackgroundShader.o):
//   cdBackgroundShaderMat::ctor @0x7E0520
//   InitCDBackgroundShader  @0x7E05B0
//   ToggleCDBackgroundShader @0x7E0600
//   cdBackgroundShader::Register @0x7E0620
//   cdBackgroundShader::AddNode @0x7E0680
// ============================================================================
#include "cdBackgroundShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "render/ShaderCommon.h"

#include <intrin.h>
#include <cstdint>

// Shader global pointer definitions
cdBackgroundShader* gCDBackgroundShader = nullptr;  // ?gCDBackgroundShader@@3PAVcdBackgroundShader@@A
cdBackgroundShader* g_cdBackgroundShader = nullptr;  // ?g_cdBackgroundShader@@3PAVcdBackgroundShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdBackgroundRender {
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
namespace cdBackgroundPixel {
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
namespace cdBackgroundFullbrightPixel {
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
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;

// ============================================================================
// cdBackgroundShaderNode::Render — ea: 0x7E06F0
// ============================================================================
void cdBackgroundShaderNode::Render() {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 1u);
        dword_BC2D00 = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0x80u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0x80u);
        dword_BC2D04 = 0x80u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x206u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, 0x206u);
        dword_BC2CF8 = 0x206u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 0);
        dword_BC2CFC = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);

    math::Mat44 context;
    context = this->MeshNode->LocalToScreen;
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
        cdBackgroundRender::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    const unsigned int* pixelShader = reinterpret_cast<const unsigned int*>(
        cdBackgroundFullbrightPixel::PS[0]);
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = reinterpret_cast<const unsigned int*>(cdBackgroundPixel::PS[0]);
    if (pixelShader != reinterpret_cast<const unsigned int*>(
                           static_cast<uintptr_t>(gpuHashPixelShader))) {
        gpuHashPixelShader = static_cast<unsigned int>(
            reinterpret_cast<uintptr_t>(pixelShader));
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(pixelShader));
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
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1u) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }
    nglGpuDrawSection(this->Section);
    nglDxState.PrevBM = static_cast<unsigned int>(-1);
}

// ============================================================================
// cdBackgroundShaderMat::cdBackgroundShaderMat — bind texture + shader.
// ea: 0x7E0520
// ============================================================================
cdBackgroundShaderMat::cdBackgroundShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdBackgroundShader* v3 = g_cdBackgroundShader;
    if (g_cdBackgroundShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdBackgroundShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "g_cdBackgroundShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = g_cdBackgroundShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = g_cdBackgroundShader;
}

// ============================================================================
// InitCDBackgroundShader — allocate the shader and link into the init list.
// ea: 0x7E05B0
// ============================================================================
void InitCDBackgroundShader() {
    cdBackgroundShader* result = (cdBackgroundShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdBackgroundShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdBackgroundShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~0x40;
        gCDBackgroundShader = result;
    } else {
        gCDBackgroundShader = NULL;

    }

}

// ============================================================================
// ToggleCDBackgroundShader — toggle background-shader enable bit (bit 6).
// ea: 0x7E0600
// ============================================================================
void ToggleCDBackgroundShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (~(byte >> 6) << 6)) & 0x40) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;

}

// ============================================================================
// cdBackgroundShader::Register — register the background shaders.
// ea: 0x7E0620
// ============================================================================
tlFixedString cdBackgroundShader::GetName() { return tlFixedString("cdBackground"); }

void cdBackgroundShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdBackgroundRender::VS, cdBackgroundRender::VShaderTable, 0);
    cdBackgroundRender::Shader = cdBackgroundRender::VS != nullptr ? cdBackgroundRender::VS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdBackgroundPixel::PS, cdBackgroundPixel::PShaderTable, 0);
    cdBackgroundPixel::Shader = cdBackgroundPixel::PS != nullptr ? cdBackgroundPixel::PS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdBackgroundFullbrightPixel::PS, cdBackgroundFullbrightPixel::PShaderTable, 0);
    cdBackgroundFullbrightPixel::Shader = cdBackgroundFullbrightPixel::PS != nullptr ? cdBackgroundFullbrightPixel::PS[0] : 0;
}

// ============================================================================
// cdBackgroundShader::AddNode — add a background node to the opaque list.
// ea: 0x7E0680
// ============================================================================
void cdBackgroundShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                 nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 0x40) == 0) {
        cdBackgroundShaderNode* node = (cdBackgroundShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            ::new (node) cdBackgroundShaderNode;
            node->mMaterial = (cdBackgroundShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDBackgroundShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
