// ============================================================================
// cdSimpleSpecularShader.cpp — simple specular shader (7 non-inline funcs).
// Source: source/cdSimpleSpecularShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleSpecularShader.o):
//   cdSimpleSpecularShaderMat::ctor @0x7D4DD0
//   InitCDSimpleSpecularShader  @0x7D4E60
//   ToggleCDSimpleSpecularShader @0x7D4EB0
//   cdSimpleSpecularShader::Register @0x7D4ED0
//   cdSimpleSpecularShader::AddNode @0x7D4FF0
// ============================================================================
#include "cdSimpleSpecularShader.h"

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
        nglDxRegisterVShader((unsigned int*)&cdSimpleSpecularRender::VS[v0], cdSimpleSpecularRender::VShaderTable[v0]);
    }
    nglDxRegisterPShader((unsigned int**)cdSimpleSpecularPixel::PS, cdSimpleSpecularPixel::PShaderTable[0]);
    nglDxRegisterPShader((unsigned int**)cdSimpleSpecularFullbrightPixel::PS, cdSimpleSpecularFullbrightPixel::PShaderTable[0]);
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
