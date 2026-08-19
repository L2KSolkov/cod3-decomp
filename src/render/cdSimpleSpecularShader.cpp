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
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdSimpleSpecularRender::VS[v0], cdSimpleSpecularRender::VShaderTable, v0);
    }
    nglDxRegisterPShaderSafe((unsigned int**)cdSimpleSpecularPixel::PS, cdSimpleSpecularPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdSimpleSpecularFullbrightPixel::PS, cdSimpleSpecularFullbrightPixel::PShaderTable, 0);
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
