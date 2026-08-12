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

#include <intrin.h>

// Shader global pointer definitions
cdSimplePrelitShader* gCDSimplePrelitShader = nullptr;  // ?gCDSimplePrelitShader@@3PAVcdSimplePrelitShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdSimplePrelitRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
}
namespace cdSimplePrelitPixel {
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
            // vftable = cdSimplePrelitShaderNode
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
