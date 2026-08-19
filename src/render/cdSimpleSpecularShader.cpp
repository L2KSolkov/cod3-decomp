// ============================================================================
// cdSimpleSpecularShader.cpp — simple specular shader (7 non-inline funcs).
// Source: source/cdSimpleSpecularShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleSpecularShader.o):
//   cdSimpleSpecularShaderMat::ctor @0x7D4DD0
//   InitCDSimpleSpecularShader  @0x7D4E60
//   ToggleCDSimpleSpecularShader @0x7D4EB0
//   cdSimpleSpecularShader::Register @0x7D4ED0
//   GetEyePos @0x7D4F00
//   cdSimpleSpecularShader::AddNode @0x7D4FF0
// ============================================================================
#include "cdSimpleSpecularShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "ngl/ngl_lighting.h"
#include "render/ShaderCommon.h"

#include <cstdint>
#include <cstring>
#include <intrin.h>

// Shader global pointer definitions
cdSimpleSpecularShader* gCDSimpleSpecularShader = nullptr;  // ?gCDSimpleSpecularShader@@3PAVcdSimpleSpecularShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdSimpleSpecularRender {
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = {};
}
namespace cdSimpleSpecularPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
}
namespace cdSimpleSpecularFullbrightPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
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
// cdSimpleSpecularShaderMat::cdSimpleSpecularShaderMat — bind textures+shader.
// ea: 0x7D4DD0
// ============================================================================
cdSimpleSpecularShaderMat::cdSimpleSpecularShaderMat(nglTexture* iDiffuseTexture,
                                                     nglTexture* iSpecularTexture) {
    this->mDiffuseTexture = iDiffuseTexture;
    this->mSpecularTexture = iSpecularTexture;
    cdSimpleSpecularShader* v4 = gCDSimpleSpecularShader;
    if (gCDSimpleSpecularShader != NULL) {
        this->Shader = v4;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdSimpleSpecularShader.cpp";
    AeAssert::gCurrentLine = 17;
    AeAssert::gCurrentExpr = "gCDSimpleSpecularShader";
    if (!AeAssert::IsIgnored()) {
        if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
            __debugbreak();
            this->Shader = gCDSimpleSpecularShader;
            return;
        }
        this->Shader = gCDSimpleSpecularShader;
        return;
    }
    this->Shader = gCDSimpleSpecularShader;
}

// ============================================================================
// InitCDSimpleSpecularShader — allocate the shader and link into the init list.
// ea: 0x7D4E60
// ============================================================================
void InitCDSimpleSpecularShader() {
    cdSimpleSpecularShader* result = (cdSimpleSpecularShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSimpleSpecularShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~1;
        gCDSimpleSpecularShader = result;
    } else {
        gCDSimpleSpecularShader = NULL;

    }

}

// ============================================================================
// ToggleCDSimpleSpecularShader — toggle specular-shader enable bit (bit 0).
// ea: 0x7D4EB0
// ============================================================================
void ToggleCDSimpleSpecularShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ ~byte) & 1) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;

}

// ============================================================================
// cdSimpleSpecularShader::Register — register the specular shaders.
// ea: 0x7D4ED0
// ============================================================================
void cdSimpleSpecularShader::Register() {
    nglShader::Register();
    cdSimpleSpecularRender::RegisterVShader();
    nglDxRegisterPShader(reinterpret_cast<unsigned int**>(cdSimpleSpecularPixel::PS),
                         cdSimpleSpecularPixel::PShaderTable[0]);
    nglDxRegisterPShader(reinterpret_cast<unsigned int**>(cdSimpleSpecularFullbrightPixel::PS),
                         cdSimpleSpecularFullbrightPixel::PShaderTable[0]);
}

// ============================================================================
// GetEyePos — transform the build-scene eye position into mesh-local space.
// ea: 0x7D4F00
// ============================================================================
void GetEyePos(nglMeshNode* meshNode, math::Vector4& eyePos) {
    const __m128 localToWorldW = meshNode->LocalToWorld.w.v;
    const __m128 localToWorldY = meshNode->LocalToWorld.y.v;
    const __m128 positionWithW = _mm_shuffle_ps(
        localToWorldW,
        _mm_shuffle_ps(_mm_set1_ps(1.0f), localToWorldW, 0xA0), 0x34);
    const __m128 relativeEye = _mm_setr_ps(
        nglBuildScene->ViewPos.v.m128_f32[0] - positionWithW.m128_f32[0],
        nglBuildScene->ViewPos.v.m128_f32[1] - positionWithW.m128_f32[1],
        nglBuildScene->ViewPos.v.m128_f32[2] - positionWithW.m128_f32[2],
        1.0f);

    const __m128 localToWorldZ = meshNode->LocalToWorld.z.v;
    const __m128 zWHigh = _mm_shuffle_ps(localToWorldZ, localToWorldW, 0xEE);
    const __m128 zWLow = _mm_shuffle_ps(localToWorldZ, localToWorldW, 0x44);
    const __m128 xyLow = _mm_shuffle_ps(meshNode->LocalToWorld.x.v, localToWorldY, 0x44);
    const __m128 basisX = _mm_shuffle_ps(xyLow, zWLow, 0x88);
    const __m128 basisY = _mm_shuffle_ps(xyLow, zWLow, 0xDD);
    const __m128 basisZ = _mm_shuffle_ps(
        _mm_shuffle_ps(meshNode->LocalToWorld.x.v, localToWorldY, 0xEE),
        zWHigh, 0x88);

    eyePos.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(relativeEye, relativeEye, 0x00), basisX),
            _mm_mul_ps(_mm_shuffle_ps(relativeEye, relativeEye, 0x55), basisY)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(relativeEye, relativeEye, 0xAA), basisZ),
            _mm_mul_ps(_mm_shuffle_ps(relativeEye, relativeEye, 0xFF),
                       _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f))));
}

// ============================================================================
// cdSimpleSpecularShader::AddNode — add a specular node to the opaque list.
// ea: 0x7D4FF0
// ============================================================================
void cdSimpleSpecularShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                     nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 1) == 0) {
        cdSimpleSpecularShaderNode* node = (cdSimpleSpecularShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdSimpleSpecularShaderNode
            node->mMaterial = (cdSimpleSpecularShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDSimpleSpecularShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ============================================================================
// cdSimpleSpecularShaderNode::Render — ea: 0x7D5060
// ============================================================================
void cdSimpleSpecularShaderNode::Render() {
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

    // The original stack places the constant upload pointer 12 bytes before
    // the typed context. Keep that contiguous layout while retaining 16-byte
    // alignment for the matrix/vector fields used by the lighting helpers.
    alignas(16) unsigned char upload[0x110] = {};
    SimpleSpecularContext* context =
        reinterpret_cast<SimpleSpecularContext*>(upload + 0x10);
    unsigned char* constantData = upload + 4;

    context->params.v = _mm_setr_ps(this->mMaterial->mSpecularPower,
                                    this->mMaterial->mSpecularLevel,
                                    0.0f, 0.0f);
    nglDetermineLights(this->MeshNode);
    nglGetDirLightMatrix(this->MeshNode, &context->mLightMatrices[0],
                         &context->mLightMatrices[1]);
    GetEyePos(this->MeshNode, context->eyePos);
    context->mLToS = this->MeshNode->LocalToScreen;
    std::memcpy(constantData, &context->mLToS, 12);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, constantData, 0x40u);

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
    // IDA's dword_10DE7B8 is the overlapping second element of VS[2].
    const unsigned int vertexShader =
        static_cast<unsigned int>(cdSimpleSpecularRender::VS[1]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    const _D3DPixelShaderDef* pixelShader =
        reinterpret_cast<const _D3DPixelShaderDef*>(
            reinterpret_cast<uintptr_t>(cdSimpleSpecularFullbrightPixel::PS[0]));
    if (ShaderCommon::GetDebugRenderMode() != ShaderCommon::kDebugRenderModeFullbright)
        pixelShader = reinterpret_cast<const _D3DPixelShaderDef*>(
            reinterpret_cast<uintptr_t>(cdSimpleSpecularPixel::PS[0]));
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
    const unsigned int fogColor = c3 | (c2 << 8) | (c1 << 16) | (c0 << 24);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_FOGCOLOR, fogColor) == 0)
        D3DDevice_SetRenderState_FogColor(fogColor);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }
    nglGpuDrawSection(this->Section);
}

void cdSimpleSpecularRender::RegisterVShader() {
    for (int i = 0; i != 2; ++i)
        nglDxRegisterVShader(reinterpret_cast<unsigned int*>(&VS[i]), VShaderTable[i]);
}
