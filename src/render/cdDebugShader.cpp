// ============================================================================
// cdDebugShader.cpp — debug shader (7 non-inline funcs).
// Source: source/cdDebugShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDebugShader.o):
//   cdDebugShaderMat::ctor @0x7C6360
//   InitCDDebugShader  @0x7C63D0
//   ToggleCDDebugShader @0x7C6420
//   cdDebugShader::Register @0x7C6440
//   cdDebugShader::AddNode @0x7C6570
// ============================================================================
#include "cdDebugShader.h"

#include <new>

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "render/ShaderCommon.h"

#include <intrin.h>

// Shader global pointer definitions
cdDebugShader* gCDDebugShader = nullptr;  // ?gCDDebugShader@@3PAVcdDebugShader@@A

extern unsigned int nglTintParamID;
extern unsigned int dword_BC2D80;
extern unsigned int D3D__DirtyFlags;
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdDebugShaderRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
}
namespace cdDebugPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
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
// cdDebugShaderMat::cdDebugShaderMat — default material, bind the shader.
// ea: 0x7C6360
// ============================================================================
cdDebugShaderMat::cdDebugShaderMat() {
    if (gCDDebugShader == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdDebugShader.cpp";
        AeAssert::gCurrentLine = 13;
        AeAssert::gCurrentExpr = "gCDDebugShader";
        if (!AeAssert::IsIgnored() &&
            AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
            __debugbreak();
        }
    }
    this->Name = NULL;
    this->Shader = gCDDebugShader;
    this->BinaryVersion = 0;
    this->RuntimeData = NULL;
}

// ============================================================================
// InitCDDebugShader — allocate the shader and link into the init list.
// ea: 0x7C63D0
// ============================================================================
void InitCDDebugShader() {
    cdDebugShader* result = (cdDebugShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        new (result) cdDebugShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdDebugShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~8;
        gCDDebugShader = result;
    } else {
        gCDDebugShader = NULL;

    }

}

// ============================================================================
// ToggleCDDebugShader — toggle debug-shader enable bit (bit 3).
// ea: 0x7C6420
// ============================================================================
void ToggleCDDebugShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (8 * ~(byte >> 3))) & 8) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;

}

// ============================================================================
// cdDebugShader::Register — register the debug vertex/pixel shaders.
// ea: 0x7C6440
// ============================================================================
tlFixedString cdDebugShader::GetName() {
    return tlFixedString("cdDebug");
}

void cdDebugShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdDebugShaderRender::VS, cdDebugShaderRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdDebugPixel::PS, cdDebugPixel::PShaderTable, 0);
}

// ============================================================================
// cdDebugShader::AddNode — add a debug node to the render list.
// ea: 0x7C6570
// ============================================================================
void cdDebugShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 8) == 0) {
        cdDebugShaderNode* node = (cdDebugShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdDebugShaderNode
            node->mMaterial = (cdDebugShaderMat*)iMat;
            nglListAddNode(node);
        } else {
            nglListAddNode(NULL);
        }
    }
}

// ============================================================================
// cdDebugShaderNode::GetSortInfo — ea: 0x7C6470
// ============================================================================
void cdDebugShaderNode::GetSortInfo(nglSortInfo& si) {
    math::Vector4 color(1.0f);
    const unsigned int paramId = nglTintParamID;
    const unsigned int* array = this->MeshNode->ShaderParams.Array;
    if ((1u << (paramId & 0x1Fu)) & array[paramId >> 5]) {
        const unsigned char* values = reinterpret_cast<const unsigned char*>(array);
        color.v = _mm_loadu_ps(reinterpret_cast<const float*>(values + 8u + 4u * paramId));
    }

    if (_mm_shuffle_ps(color.v, color.v, _MM_SHUFFLE(3, 3, 3, 3)).m128_f32[0] == 1.0f) {
        si.Type = nglSortInfo::NGLSORT_OPAQUE;
        si.Hash = gCDDebugShader->ID;
    } else {
        si.Type = nglSortInfo::NGLSORT_TRANSLUCENT;
        si.Dist = this->GetDist(nglBuildScene->WorldToView);
    }
}

// ============================================================================
// cdDebugShaderNode::Render — ea: 0x7C65C0
// ============================================================================
void cdDebugShaderNode::Render() {
    const unsigned int paramId = nglTintParamID;
    const unsigned int* array = this->MeshNode->ShaderParams.Array;
    math::Vector4 tint;
    if ((1u << (paramId & 0x1Fu)) & array[paramId >> 5]) {
        // nglParamSet::Get<nglTintParamType>() returns Array[id + 2].
        tint.v = reinterpret_cast<const math::Vector4*>(array + paramId + 2)->v;
    } else {
        tint.v = _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.5f);
    }

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    nglDxState.SetBlendMode(0x64CF8600u);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 0) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 0;
    }

    D3DDevice_SetVertexShaderConstantNotInlineFast(6,
                                                    &this->MeshNode->LocalToScreen,
                                                    0x10u);
    D3DDevice_SetVertexShaderConstant1Fast(12, &tint);
    nglDxInitShaders(false);

    const unsigned int vertexShader = static_cast<unsigned int>(
        cdDebugShaderRender::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(cdDebugPixel::PS[0]));
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
}
