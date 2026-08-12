// ============================================================================
// cdSimpleUVAnimShader.cpp — simple UV anim shader (7 non-inline funcs).
// Source: source/cdSimpleUVAnimShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleUVAnimShader.o):
//   cdSimpleUVAnimShaderMat::ctor @0x7C6D70
//   InitCDSimpleUVAnimShader  @0x7C6E00
//   ToggleCDSimpleUVAnimShader @0x7C6E50
//   cdSimpleUVAnimShader::Register @0x7C6E70
//   cdSimpleUVAnimShader::AddNode @0x7C6ED0
// ============================================================================
#include "cdSimpleUVAnimShader.h"

#include <intrin.h>

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
