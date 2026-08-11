// ============================================================================
// cdSimpleAlphaShader.cpp — simple alpha shader (6 non-inline funcs).
// Source: source/cdSimpleAlphaShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleAlphaShader.o):
//   cdSimpleAlphaShaderMat::ctor @0x7C7E10
//   InitCDSimpleAlphaShader  @0x7C7EA0
//   ToggleCDSimpleAlphaShader @0x7C7EF0
//   cdSimpleAlphaShader::Register @0x7C7F10
//   cdSimpleAlphaShader::AddNode @0x7C7F30
// ============================================================================
#include "cdSimpleAlphaShader.h"

#include <intrin.h>

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
// cdSimpleAlphaShaderMat::cdSimpleAlphaShaderMat — bind the texture + shader.
// ea: 0x7C7E10
// ============================================================================
cdSimpleAlphaShaderMat::cdSimpleAlphaShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdSimpleAlphaShader* v3 = gCDSimpleAlphaShader;
    if (gCDSimpleAlphaShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdSimpleAlphaShader.cpp";
    AeAssert::gCurrentLine = 79;
    AeAssert::gCurrentExpr = "gCDSimpleAlphaShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDSimpleAlphaShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDSimpleAlphaShader;
}

// ============================================================================
// InitCDSimpleAlphaShader — allocate the shader and link into the init list.
// ea: 0x7C7EA0
// ============================================================================
void InitCDSimpleAlphaShader() {
    cdSimpleAlphaShader* result = (cdSimpleAlphaShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSimpleAlphaShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~0x80;
        gCDSimpleAlphaShader = result;
    } else {
        gCDSimpleAlphaShader = NULL;
    }
}

// ============================================================================
// ToggleCDSimpleAlphaShader — toggle the alpha-shader enable (high bit).
// ea: 0x7C7EF0
// ============================================================================
void ToggleCDSimpleAlphaShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    char result = (char)~byte;
    ShaderCommon::ShaderSwitching.__s0[1] =
        (unsigned char)(~byte ^ ((byte ^ ~byte) & 0x7F));
}

// ============================================================================
// cdSimpleAlphaShader::Register — register the alpha vertex/pixel shaders.
// ea: 0x7C7F10
// ============================================================================
void cdSimpleAlphaShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShader((unsigned int*)&cdSimpleAlphaRender::VS[v0], cdSimpleAlphaRender::VShaderTable[v0]);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShader((unsigned int**)&cdSimpleAlphaPixel::PS[v0], cdSimpleAlphaPixel::PShaderTable[v0]);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShader((unsigned int**)&cdSimpleAlphaPixel_Fullbright::PS[v0], cdSimpleAlphaPixel_Fullbright::PShaderTable[v0]);
    }
}

// ============================================================================
// cdSimpleAlphaShader::AddNode — add an alpha node to the transparent list.
// ea: 0x7C7F30
// ============================================================================
void cdSimpleAlphaShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                  nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 0x80) == 0) {
        cdSimpleAlphaShaderNode* node = (cdSimpleAlphaShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdSimpleAlphaShaderNode
            node->mMaterial = (cdSimpleAlphaShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortDist = node->GetDist(nglBuildScene->WorldToView);
        node->Next = nglBuildScene->TransRenderList;
        nglBuildScene->TransRenderList = node;
        ++nglBuildScene->TransListCount;
    }
}
