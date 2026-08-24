// ============================================================================
// cdSimplePrelitShader.cpp — simple prelit shader (6 non-inline funcs).
// Source: source/cdSimplePrelitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimplePrelitShader.o):
//   cdSimplePrelitShaderMat::ctor @0x7D58E0
//   InitCDSimplePrelitShader  @0x7D5970
//   ToggleCDSimplePrelitShader @0x7D59C0
//   cdSimplePrelitShader::Register @0x7D59E0
//   cdSimplePrelitShader::AddNode @0x7D5A10
// ============================================================================
#include "cdSimplePrelitShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"

#include <intrin.h>

// Shader global pointer definitions
cdSimplePrelitShader* gCDSimplePrelitShader = nullptr;  // ?gCDSimplePrelitShader@@3PAVcdSimplePrelitShader@@A

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

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdSimplePrelitRender {
    static const unsigned int VShaderMicrocode[49] = {
        0x000c2078, 0x00000000, 0x0061601a, 0x0800146a, 0xfeb00000,
        0x00000000, 0x00e1201b, 0x08373800, 0x20a01800, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x06e0c01b,
        0x0836dbff, 0x10a88800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0944800, 0x00000000, 0x00e1001b, 0x08371800, 0x20902800,
        0x00000000, 0x00814055, 0x14016d56, 0xb8000000, 0x00000000,
        0x02000400, 0x0800106c, 0xa0b0f81c, 0x00000000, 0x00414000,
        0x05555800, 0x28000000, 0x00000000, 0x00814000, 0x05ff5802,
        0xb8000000, 0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
}
namespace cdSimplePrelitPixel {
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

// ============================================================================
// cdSimplePrelitShaderMat::cdSimplePrelitShaderMat — bind texture + shader.
// ea: 0x7D58E0
// ============================================================================
cdSimplePrelitShaderMat::cdSimplePrelitShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdSimplePrelitShader* v3 = gCDSimplePrelitShader;
    if (gCDSimplePrelitShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdSimplePrelitShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "gCDSimplePrelitShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDSimplePrelitShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDSimplePrelitShader;
}

// ============================================================================
// InitCDSimplePrelitShader — allocate the shader and link into the init list.
// ea: 0x7D5970
// ============================================================================
void InitCDSimplePrelitShader() {
    cdSimplePrelitShader* result = (cdSimplePrelitShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdSimplePrelitShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSimplePrelitShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~0x20;
        gCDSimplePrelitShader = result;
    } else {
        gCDSimplePrelitShader = NULL;

    }

}

// ============================================================================
// ToggleCDSimplePrelitShader — toggle prelit-shader enable bit (bit 5).
// ea: 0x7D59C0
// ============================================================================
void ToggleCDSimplePrelitShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ (32 * ~(byte >> 5))) & 0x20) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;

}

// ============================================================================
// cdSimplePrelitShader::Register — register the prelit vertex/pixel shaders.
// ea: 0x7D59E0
// ============================================================================
tlFixedString cdSimplePrelitShader::GetName() { return tlFixedString("cdSimplePrelit"); }

void cdSimplePrelitShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdSimplePrelitRender::VS, cdSimplePrelitRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdSimplePrelitPixel::PS, cdSimplePrelitPixel::PShaderTable, 0);
}

// ============================================================================
// cdSimplePrelitShader::AddNode — add a prelit node to the opaque list.
// ea: 0x7D5A10
// ============================================================================
void cdSimplePrelitShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                   nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 0x20) == 0) {
        cdSimplePrelitShaderNode* node = (cdSimplePrelitShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            ::new (node) cdSimplePrelitShaderNode;
            node->mMaterial = (cdSimplePrelitShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDSimplePrelitShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ============================================================================
// cdSimplePrelitShaderNode::Render — ea: 0x7D5A80
// ============================================================================
void cdSimplePrelitShaderNode::Render() {
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

    SimpleContext context;
    context.mLToS = this->MeshNode->LocalToScreen;
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &context, 0x10u);

    nglDxInitShaders(false);
    const unsigned int vertexShader = static_cast<unsigned int>(cdSimplePrelitRender::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

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

    const unsigned int pixelShader =
        static_cast<unsigned int>(reinterpret_cast<uintptr_t>(cdSimplePrelitPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(static_cast<uintptr_t>(pixelShader)));
    }

    nglDxSetupVShaderFog(-86, this->MeshNode, nglBuildScene->FogNear,
                         nglBuildScene->FogFar, nglBuildScene->FogMin,
                         nglBuildScene->FogMax);

    const __m128 fogScaled = _mm_mul_ps(nglBuildScene->FogColor.v, _mm_set1_ps(127.0f));
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
    nglDxState.PrevBM = (unsigned int)-1;
}
