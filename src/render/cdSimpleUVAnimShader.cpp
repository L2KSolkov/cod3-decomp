// ============================================================================
// cdSimpleUVAnimShader.cpp — simple UV anim shader (7 non-inline funcs).
// Source: source/cdSimpleUVAnimShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleUVAnimShader.o):
//   cdSimpleUVAnimShaderMat::ctor @0x7C6D70
//   InitCDSimpleUVAnimShader  @0x7C6E00
//   ToggleCDSimpleUVAnimShader @0x7C6E50
//   cdSimpleUVAnimShader::Register @0x7C6E70
//   cdSimpleUVAnimShader::AddNode @0x7C6ED0
//   cdSimpleUVAnimShaderNode::SetTextureMatrix @0x7C6F40
//   cdSimpleUVAnimShaderNode::Render @0x7C70E0
// ============================================================================
#include "cdSimpleUVAnimShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "ngl/ngl_lighting.h"

#include <intrin.h>
#include <cstring>

extern unsigned int TextureMatrixParamID;  // ?TextureMatrixParamID@@3IA
extern unsigned int isRotatingTextureParamID;  // ?isRotatingTextureParamID@@3IA

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
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// Shader global pointer definitions
cdSimpleUVAnimShader* gCDSimpleUVAnimShader = nullptr;  // ?gCDSimpleUVAnimShader@@3PAVcdSimpleUVAnimShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdSimpleUVAnimRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
    unsigned long Shader = 0;
}
namespace cdSimpleUVAnimPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
    unsigned long* Shader = nullptr;
}
namespace cdSimpleUVAnimFullbrightPixel {
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

// ============================================================================
// cdSimpleUVAnimShaderMat::cdSimpleUVAnimShaderMat — bind texture + shader.
// ea: 0x7C6D70
// ============================================================================
cdSimpleUVAnimShaderMat::cdSimpleUVAnimShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdSimpleUVAnimShader* v3 = gCDSimpleUVAnimShader;
    if (gCDSimpleUVAnimShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdSimpleUVAnimShader.cpp";
    AeAssert::gCurrentLine = 19;
    AeAssert::gCurrentExpr = "gCDSimpleUVAnimShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDSimpleUVAnimShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDSimpleUVAnimShader;
}

// ============================================================================
// InitCDSimpleUVAnimShader — allocate the shader and link into the init list.
// ea: 0x7C6E00
// ============================================================================
void InitCDSimpleUVAnimShader() {
    cdSimpleUVAnimShader* result = (cdSimpleUVAnimShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdSimpleUVAnimShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSimpleUVAnimShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~0x40;
        gCDSimpleUVAnimShader = result;
    } else {
        gCDSimpleUVAnimShader = NULL;

    }

}

// ============================================================================
// ToggleCDSimpleUVAnimShader — toggle UV-anim enable bit (bit 6).
// ea: 0x7C6E50
// ============================================================================
void ToggleCDSimpleUVAnimShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ (~(byte >> 6) << 6)) & 0x40) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;

}

// ============================================================================
// cdSimpleUVAnimShader::Register — register the UV-anim shaders.
// ea: 0x7C6E70
// ============================================================================
tlFixedString cdSimpleUVAnimShader::GetName() { return tlFixedString("cdSimpleUVAnim"); }

void cdSimpleUVAnimShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdSimpleUVAnimRender::VS, cdSimpleUVAnimRender::VShaderTable, 0);
    cdSimpleUVAnimRender::Shader = cdSimpleUVAnimRender::VS != nullptr ? cdSimpleUVAnimRender::VS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdSimpleUVAnimPixel::PS, cdSimpleUVAnimPixel::PShaderTable, 0);
    cdSimpleUVAnimPixel::Shader = cdSimpleUVAnimPixel::PS != nullptr ? cdSimpleUVAnimPixel::PS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdSimpleUVAnimFullbrightPixel::PS, cdSimpleUVAnimFullbrightPixel::PShaderTable, 0);
    cdSimpleUVAnimFullbrightPixel::Shader = cdSimpleUVAnimFullbrightPixel::PS != nullptr ? cdSimpleUVAnimFullbrightPixel::PS[0] : 0;
}

// ============================================================================
// cdSimpleUVAnimShader::AddNode — add a UV-anim node to the opaque list.
// ea: 0x7C6ED0
// ============================================================================
void cdSimpleUVAnimShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                   nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 0x40) == 0) {
        cdSimpleUVAnimShaderNode* node = (cdSimpleUVAnimShaderNode*)nglListAlloc(0x60, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdSimpleUVAnimShaderNode
            node->mMaterial = (cdSimpleUVAnimShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDSimpleUVAnimShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ============================================================================
// cdSimpleUVAnimShaderNode::SetTextureMatrix — fetch and transpose the
// texture-matrix parameter, or use the identity matrix when it is unset.
// ea: 0x7C6F40
// ============================================================================
void cdSimpleUVAnimShaderNode::SetTextureMatrix(math::Mat44& matOut) {
    const unsigned int id = TextureMatrixParamID;
    const unsigned int* array = this->MeshNode->ShaderParams.Array;
    const math::Mat44* source = nullptr;

    if ((1u << (id & 0x1Fu)) & array[id >> 5]) {
        // nglParamSet::Get<TextureMatrixParamType>() returns Array[id + 2]
        // on the 32-bit target; the outer bit test is the original IsSet()
        // guard, and the getter's null result is handled below.
        source = reinterpret_cast<const math::Mat44*>(
            static_cast<uintptr_t>(array[id + 2]));
    }

    if (source != nullptr) {
        const __m128 v0 = source->x.v;
        const __m128 v1 = source->y.v;
        const __m128 v2 = source->z.v;
        const __m128 v3 = source->w.v;
        const __m128 v4 = _mm_shuffle_ps(v0, v1, 68);
        const __m128 v5 = _mm_shuffle_ps(v0, v1, 238);
        const __m128 v6 = _mm_shuffle_ps(v2, v3, 68);
        matOut.x.v = _mm_shuffle_ps(v4, v6, 136);
        matOut.y.v = _mm_shuffle_ps(v4, v6, 221);
        const __m128 v7 = _mm_shuffle_ps(v2, v3, 238);
        matOut.z.v = _mm_shuffle_ps(v5, v7, 136);
        matOut.w.v = _mm_shuffle_ps(v5, v7, 221);
    } else {
        matOut.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
        matOut.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        matOut.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
        matOut.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

// ============================================================================
// cdSimpleUVAnimShaderNode::Render — configure the UV animation shader and
// draw its mesh section.
// ea: 0x7C70E0
// ============================================================================
void cdSimpleUVAnimShaderNode::Render() {
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

    // IDA's v8[3] immediately precedes the 276-byte context.  Keeping the
    // same contiguous layout preserves the 0x44-dword constant upload.
    alignas(16) unsigned char constants[12 + 276] = {};
    unsigned char* context = constants + 12;
    std::memcpy(constants, &this->MeshNode->LocalToScreen, sizeof(math::Mat44));
    nglGetDirLightMatrix(this->MeshNode,
                         reinterpret_cast<math::Mat44*>(context + 52),
                         reinterpret_cast<math::Mat44*>(context + 116));
    this->SetTextureMatrix(*reinterpret_cast<math::Mat44*>(context + 180));
    *reinterpret_cast<unsigned int*>(context + 240) = 1065353216u;
    D3DDevice_SetVertexShaderConstantNotInlineFast(
        6, reinterpret_cast<unsigned int*>(constants), 0x44u);

    nglDxSetTexture(0, this->mMaterial->mTexture, 1u, 3u);

    const unsigned int rotatingId = isRotatingTextureParamID;
    const unsigned int* array = this->MeshNode->ShaderParams.Array;
    int rotating;
    if ((1u << (rotatingId & 0x1Fu)) & array[rotatingId >> 5]) {
        rotating = array[rotatingId + 2];
    } else {
        const bool assertIgnored = !_tlAssert(
            "c:\\cod\\code\\tl\\ngl\\include\\ngl_params.h", 139,
            "IsSet( Param::GetID() )", "Parameter not set.");
        rotating = array[rotatingId + 2];
        if (!assertIgnored)
            __debugbreak();
    }

    if (rotating != 0) {
        if (nglDxTexCache.Prev[0].WrapU != 3) {
            nglDxTexCache.Prev[0].WrapU = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 3) == 0) {
                D3D__DirtyFlags |= 1u;
                D3D__TextureState[0][D3DTSS_ADDRESSU] = 3;
            }
        }
        if (nglDxTexCache.Prev[0].WrapV != 3) {
            nglDxTexCache.Prev[0].WrapV = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 3) == 0) {
                D3D__DirtyFlags |= 1u;
                D3D__TextureState[0][D3DTSS_ADDRESSV] = 3;
            }
        }
    } else {
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
    }

    nglDxInitShaders(false);
    if (cdSimpleUVAnimRender::VS[0] != gpuHashVertexShader) {
        gpuHashVertexShader = static_cast<unsigned int>(cdSimpleUVAnimRender::VS[0]);
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(
                static_cast<uintptr_t>(cdSimpleUVAnimRender::VS[0])), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    const _D3DPixelShaderDef* pixelShader =
        reinterpret_cast<const _D3DPixelShaderDef*>(cdSimpleUVAnimFullbrightPixel::PS[0]);
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = reinterpret_cast<const _D3DPixelShaderDef*>(cdSimpleUVAnimPixel::PS[0]);
    if (pixelShader != reinterpret_cast<const _D3DPixelShaderDef*>(
                       static_cast<uintptr_t>(gpuHashPixelShader))) {
        gpuHashPixelShader = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pixelShader));
        D3DDevice_SetPixelShaderProgram(pixelShader);
    }

    nglDxSetupVShaderFog(-73, this->MeshNode, nglBuildScene->FogNear,
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
}
