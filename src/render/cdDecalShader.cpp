// ============================================================================
// cdDecalShader.cpp — decal shader (6 non-inline funcs).
// Source: source/cdDecalShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDecalShader.o):
//   cdDecalShaderMat::ctor @0x7D1720
//   InitCDDecalShader  @0x7D17B0
//   ToggleCDDecalShader @0x7D1800
//   cdDecalShader::Register @0x7D1820
//   cdDecalShader::AddNode @0x7D1860
// ============================================================================
#include "cdDecalShader.h"

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
// cdDecalShaderMat::cdDecalShaderMat — bind the texture + shader.
// ea: 0x7D1720
// ============================================================================
cdDecalShaderMat::cdDecalShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdDecalShader* v3 = g_cdDecalShader;
    if (g_cdDecalShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = 0;
    AeAssert::gCurrentFile = "cdDecalShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "g_cdDecalShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = g_cdDecalShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = g_cdDecalShader;
}

// ============================================================================
// InitCDDecalShader — allocate the shader and link into the init list.
// ea: 0x7D17B0
// ============================================================================
void InitCDDecalShader() {
    cdDecalShader* result = (cdDecalShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdDecalShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~0x20;
        gCDDecalShader = result;
    } else {
        gCDDecalShader = NULL;

    }

}

// ============================================================================
// ToggleCDDecalShader — toggle the decal-shader enable bit (bit 5).
// ea: 0x7D1800
// ============================================================================
void ToggleCDDecalShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (32 * ~(byte >> 5))) & 0x20) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;

}

// ============================================================================
// cdDecalShader::Register — register the decal vertex/pixel shaders.
// ea: 0x7D1820
// ============================================================================
void cdDecalShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader(cdDecalRender::VS, cdDecalRender::VShaderTable[0]);
    nglDxRegisterPShader(cdDecalPixel::PS, cdDecalPixel::PShaderTable[0]);
    nglDxRegisterPShader(cdDecalFullbrightPixel::PS, cdDecalFullbrightPixel::PShaderTable[0]);
}

// ============================================================================
// cdDecalShader::AddNode — add a decal node to the opaque list.
// ea: 0x7D1860
// ============================================================================
void cdDecalShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 0x20) == 0) {
        cdDecalShaderNode* node = (cdDecalShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdDecalShaderNode
            node->mMaterial = (cdDecalShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDDecalShader->ID | 0x80000000;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
