// ============================================================================
// cdGunSightSpecularShader.cpp — gun sight specular shader (6 non-inline funcs).
// Source: source/cdGunSightSpecularShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGunSightSpecularShader.o):
//   cdGunSightSpecularShaderMat::ctor @0x7CD770
//   InitCDGunSightSpecularShader  @0x7CD800
//   ToggleCDGunSightSpecularShader @0x7CD850
//   cdGunSightSpecularShader::Register @0x7CD870
//   cdGunSightSpecularShader::AddNode @0x7CD8A0
// ============================================================================
#include "cdGunSightSpecularShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "ngl/ngl_lighting.h"
#include "render/ShaderCommon.h"

extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);

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

extern void nglDetermineLights(nglMeshNode* meshNode);
extern void nglGetDirLightMatrix(nglMeshNode* meshNode, math::Mat44* dir,
                                 math::Mat44* color);
extern void GetEyePos(nglMeshNode* meshNode, math::Vector4& eyePos);

#include <cstdint>
#include <cstring>
#include <intrin.h>

// Shader global pointer definitions
cdGunSightSpecularShader* gCDGunSightSpecularShader = nullptr;  // ?gCDGunSightSpecularShader@@3PAVcdGunSightSpecularShader@@A

// ea: 0x007CDCD0
void cdGunSightSpecularRender::RegisterVShader()
{
    for (int index = 0; index != 2; ++index) {
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(&cdGunSightSpecularRender::VS[index]),
                             cdGunSightSpecularRender::VShaderTable[index]);
    }
}

// ea: 0x007CDD00
unsigned int cdGunSightSpecularRender::GetVShader(unsigned int index)
{
    return static_cast<unsigned int>(cdGunSightSpecularRender::VS[index]);
}

// ea: 0x007CDD10
void cdGunSightSpecularPixel::RegisterPShader()
{
    nglDxRegisterPShader(cdGunSightSpecularPixel::PS,
                         cdGunSightSpecularPixel::PShaderTable[0]);
}

// ea: 0x007CDD30
unsigned long* cdGunSightSpecularPixel::GetPShader()
{
    return cdGunSightSpecularPixel::PS[0];
}

// ea: 0x007CDD40
void cdGunSightSpecularFullbrightPixel::RegisterPShader()
{
    nglDxRegisterPShader(cdGunSightSpecularFullbrightPixel::PS,
                         cdGunSightSpecularFullbrightPixel::PShaderTable[0]);
}

// ea: 0x007CDD60
unsigned long* cdGunSightSpecularFullbrightPixel::GetPShader()
{
    return cdGunSightSpecularFullbrightPixel::PS[0];
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
// cdGunSightSpecularShaderMat::ctor — bind textures + shader.
// ea: 0x7CD770
// ============================================================================
cdGunSightSpecularShaderMat::cdGunSightSpecularShaderMat(nglTexture* iDiffuseTexture,
                                                         nglTexture* iSpecularTexture) {
    this->mDiffuseTexture = iDiffuseTexture;
    this->mSpecularTexture = iSpecularTexture;
    cdGunSightSpecularShader* v4 = gCDGunSightSpecularShader;
    if (gCDGunSightSpecularShader != NULL) {
        this->Shader = v4;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdGunSightSpecularShader.cpp";
    AeAssert::gCurrentLine = 17;
    AeAssert::gCurrentExpr = "gCDGunSightSpecularShader";
    if (!AeAssert::IsIgnored()) {
        if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
            __debugbreak();
            this->Shader = gCDGunSightSpecularShader;
            return;
        }
        this->Shader = gCDGunSightSpecularShader;
        return;
    }
    this->Shader = gCDGunSightSpecularShader;
}

// ============================================================================
// InitCDGunSightSpecularShader — allocate the shader and link into the init list.
// ea: 0x7CD800
// ============================================================================
// ea: 0x007CDC80
cdGunSightSpecularShader::cdGunSightSpecularShader() {
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    ShaderCommon::ShaderSwitching.__s0[3] &= ~0x20;
}

// ea: 0x007CDE10
cdGunSightSpecularShader::~cdGunSightSpecularShader() = default;

void InitCDGunSightSpecularShader() {
    cdGunSightSpecularShader* result = (cdGunSightSpecularShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdGunSightSpecularShader;
        gCDGunSightSpecularShader = result;
    } else {
        gCDGunSightSpecularShader = NULL;
    }
}

// ============================================================================
// ToggleCDGunSightSpecularShader — toggle gun-sight specular bit (bit 5).
// ea: 0x7CD850
// ============================================================================
void ToggleCDGunSightSpecularShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (32 * ~(byte >> 5))) & 0x20) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;
}

// ea: 0x007CDCB0
tlFixedString cdGunSightSpecularShader::GetName() { return tlFixedString("cdGunSightSpecular"); }

// ============================================================================
// cdGunSightSpecularShader::Register — register the shaders.
// ea: 0x7CD870
// ============================================================================
void cdGunSightSpecularShader::Register() {
    nglShader::Register();
    cdGunSightSpecularRender::RegisterVShader();
    cdGunSightSpecularPixel::RegisterPShader();
    cdGunSightSpecularFullbrightPixel::RegisterPShader();
}

// ea: 0x007CDD70
cdGunSightSpecularShaderNode::cdGunSightSpecularShaderNode(
    nglMeshNode* iMeshNode, nglMeshSection* iSection,
    cdGunSightSpecularShaderMat* iMaterial) {
    this->MeshNode = iMeshNode;
    this->Section = iSection;
    this->mMaterial = iMaterial;
}

// ea: 0x007CDDD0
cdGunSightSpecularShaderNode::~cdGunSightSpecularShaderNode() = default;

// ea: 0x007CDE20
GunSightSpecularContext::GunSightSpecularContext() = default;

// ea: 0x007CD910
void cdGunSightSpecularShaderNode::Render() {
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

    nglDxState.PrevBM = static_cast<unsigned int>(-1);

    alignas(16) unsigned char upload[0x110] = {};
    const math::Mat44& localToScreen = this->MeshNode->LocalToScreen;
    std::memcpy(upload + 0x00, &localToScreen, sizeof(localToScreen));
    *reinterpret_cast<float*>(upload + 0xE0) = this->mMaterial->mSpecularPower;
    *reinterpret_cast<float*>(upload + 0xE4) = this->mMaterial->mSpecularLevel;

    nglDetermineLights(this->MeshNode);
    nglGetDirLightMatrix(this->MeshNode,
                         reinterpret_cast<math::Mat44*>(upload + 0x40),
                         reinterpret_cast<math::Mat44*>(upload + 0x80));
    GetEyePos(this->MeshNode,
              *reinterpret_cast<math::Vector4*>(upload + 0xF0));
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, upload, 0x40u);

    nglDxSetTexture(0, this->mMaterial->mDiffuseTexture, 1u, 3u);
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
    const unsigned int vertexShader =
        static_cast<unsigned int>(cdGunSightSpecularRender::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    const _D3DPixelShaderDef* pixelShader =
        reinterpret_cast<const _D3DPixelShaderDef*>(
            reinterpret_cast<uintptr_t>(cdGunSightSpecularFullbrightPixel::PS[0]));
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = reinterpret_cast<const _D3DPixelShaderDef*>(
            reinterpret_cast<uintptr_t>(cdGunSightSpecularPixel::PS[0]));
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

// ============================================================================
// cdGunSightSpecularShader::AddNode — add a node to the opaque list.
// ea: 0x7CD8A0
// ============================================================================
void cdGunSightSpecularShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                       nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 0x20) == 0) {
        cdGunSightSpecularShaderNode* node = (cdGunSightSpecularShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdGunSightSpecularShaderNode
            node->mMaterial = (cdGunSightSpecularShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDGunSightSpecularShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
