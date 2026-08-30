// ============================================================================
// cdGunShader.cpp — gun shader (6 non-inline funcs).
// Source: source/cdGunShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGunShader.o):
//   cdGunShaderMat::ctor @0x7CEA70
//   InitCDGunShader  @0x7CEB00
//   ToggleCDGunShader @0x7CEB50
//   cdGunShader::Register @0x7CEB70
//   cdGunShader::AddNode @0x7CEBD0
// ============================================================================
#include "cdGunShader.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "ngl/ngl_lighting.h"

extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);

#include <intrin.h>

// Shader global pointer definitions
cdGunShader* gCDGunShader = nullptr;  // ?gCDGunShader@@3PAVcdGunShader@@A

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

// ea: 0x007CEC40
void cdGunShaderNode::Render()
{
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
    const unsigned int vertexShader = cdGunRender::VS[0];
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const _D3DPixelShaderDef* pixelShader =
        reinterpret_cast<const _D3DPixelShaderDef*>(cdGunFullbrightPixel::PS[0]);
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = reinterpret_cast<const _D3DPixelShaderDef*>(cdGunPixel::PS[0]);
    if (pixelShader != reinterpret_cast<const _D3DPixelShaderDef*>(gpuHashPixelShader)) {
        gpuHashPixelShader = (unsigned int)pixelShader;
        D3DDevice_SetPixelShaderProgram(pixelShader);
    }
    nglDxSetupVShaderFog(-78, this->MeshNode, nglBuildScene->FogNear,
                         nglBuildScene->FogFar, nglBuildScene->FogMin,
                         nglBuildScene->FogMax);
    const __m128 fogScaled = _mm_mul_ps(nglBuildScene->FogColor.v, _mm_set1_ps(127.0f));
    const unsigned int fogColor = (unsigned int)(int)fogScaled.m128_f32[2]
        | ((unsigned int)(int)fogScaled.m128_f32[1] << 8)
        | ((unsigned int)(int)fogScaled.m128_f32[0] << 16)
        | ((unsigned int)(int)fogScaled.m128_f32[3] << 24);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_FOGCOLOR, fogColor) == 0)
        D3DDevice_SetRenderState_FogColor(fogColor);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1u) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }
    nglGpuDrawSection(this->Section);
}

// ea: 0x007CEA70
cdGunShaderMat::cdGunShaderMat(nglTexture* iTexture)
{
    this->mTexture = iTexture;
    cdGunShader* shader = gCDGunShader;
    if (shader != nullptr) {
        this->Shader = shader;
        return;
    }
    AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(0);
    AeAssert::gCurrentFile = "cdGunShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "gCDGunShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDGunShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
        __debugbreak();
        this->Shader = gCDGunShader;
        return;
    }
    this->Shader = gCDGunShader;
}

// ea: 0x007CF020
unsigned long cdGunRender::GetVShader()
{
    return static_cast<unsigned int>(cdGunRender::VS[0]);
}

// ea: 0x007CF030
void cdGunPixel::RegisterShader()
{
    nglDxRegisterPShader(cdGunPixel::PS, cdGunPixel::PShaderTable[0]);
    cdGunPixel::Shader = cdGunPixel::PS[0];
}

// ea: 0x007CF050
void cdGunPixel::RegisterPShader()
{
    cdGunPixel::RegisterShader();
}

// ea: 0x007CF070
unsigned long* cdGunPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(cdGunPixel::PS[0]);
}

// ea: 0x007CF080
void cdGunFullbrightPixel::RegisterShader()
{
    nglDxRegisterPShader(cdGunFullbrightPixel::PS,
                         cdGunFullbrightPixel::PShaderTable[0]);
    cdGunFullbrightPixel::Shader = cdGunFullbrightPixel::PS[0];
}

// ea: 0x007CF0A0
void cdGunFullbrightPixel::RegisterPShader()
{
    cdGunFullbrightPixel::RegisterShader();
}

// ea: 0x007CF0C0
unsigned long* cdGunFullbrightPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(cdGunFullbrightPixel::PS[0]);
}

// ea: 0x007CEF90
cdGunShader::cdGunShader() {
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    ShaderCommon::ShaderSwitching.__s0[3] &= ~8;
}

// ea: 0x007CF170
cdGunShader::~cdGunShader() = default;

// Shader static data definitions are restored in cdGunShaderData.cpp.

// ea: 0x007CF000
void cdGunRender::RegisterVShader()
{
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(cdGunRender::VS),
                         reinterpret_cast<const unsigned int*>(cdGunRender::VShaderTable[0]));
    cdGunRender::Shader = cdGunRender::VS[0];
}
// ============================================================================
// InitCDGunShader — allocate the shader and link into the init list.
// ea: 0x7CEB00
// ============================================================================
void InitCDGunShader() {
    cdGunShader* result = (cdGunShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdGunShader;
        gCDGunShader = result;
    } else {
        gCDGunShader = NULL;

    }

}

// ============================================================================
// ToggleCDGunShader — toggle the gun-shader enable bit (bit 3).
// ea: 0x7CEB50
// ============================================================================
void ToggleCDGunShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (8 * ~(byte >> 3))) & 8) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;

}

// ea: 0x007CEFC0
tlFixedString cdGunShader::GetName() { return tlFixedString("cdGun"); }

// ============================================================================
// cdGunShader::Register — register the gun vertex/pixel shaders.
// ea: 0x7CEB70
// ============================================================================
void cdGunShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(cdGunRender::VS),
                         cdGunRender::VShaderTable[0]);
    cdGunRender::Shader = cdGunRender::VS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(cdGunPixel::PS),
                         cdGunPixel::PShaderTable[0]);
    cdGunPixel::Shader = cdGunPixel::PS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(cdGunFullbrightPixel::PS),
                         cdGunFullbrightPixel::PShaderTable[0]);
    cdGunFullbrightPixel::Shader = cdGunFullbrightPixel::PS[0];
}

// ============================================================================
// cdGunShader::AddNode — add a gun node to the opaque list.
// ea: 0x7CEBD0
// ============================================================================
void cdGunShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                          nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 8) == 0) {
        cdGunShaderNode* node = (cdGunShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdGunShaderNode
            node->mMaterial = (cdGunShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDGunShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ea: 0x007CF0D0
cdGunShaderNode::cdGunShaderNode(
    nglMeshNode* iMeshNode, nglMeshSection* iSection,
    cdGunShaderMat* iMaterial) {
    this->MeshNode = iMeshNode;
    this->Section = iSection;
    this->mMaterial = iMaterial;
}

// ea: 0x007CF130
cdGunShaderNode::~cdGunShaderNode() = default;
