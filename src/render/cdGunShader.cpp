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

#include <intrin.h>

namespace AeAssert {
    extern int   gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int   gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
}

// ============================================================================
// cdGunShaderMat::cdGunShaderMat — bind the texture + shader.
// ea: 0x7CEA70
// ============================================================================
cdGunShaderMat::cdGunShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdGunShader* v3 = gCDGunShader;
    if (gCDGunShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = 0;
    AeAssert::gCurrentFile = "cdGunShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "gCDGunShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDGunShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDGunShader;
}

// ============================================================================
// InitCDGunShader — allocate the shader and link into the init list.
// ea: 0x7CEB00
// ============================================================================
void InitCDGunShader() {
    cdGunShader* result = (cdGunShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdGunShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~8;
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

// ============================================================================
// cdGunShader::Register — register the gun vertex/pixel shaders.
// ea: 0x7CEB70
// ============================================================================
void cdGunShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader(cdGunRender::VS, cdGunRender::VShaderTable[0]);
    cdGunRender::Shader = cdGunRender::VS[0];
    nglDxRegisterPShader(cdGunPixel::PS, cdGunPixel::PShaderTable[0]);
    cdGunPixel::Shader = cdGunPixel::PS[0];
    nglDxRegisterPShader(cdGunFullbrightPixel::PS, cdGunFullbrightPixel::PShaderTable[0]);
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
