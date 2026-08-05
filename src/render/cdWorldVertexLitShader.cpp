// ============================================================================
// cdWorldVertexLitShader.cpp — world vertex-lit shader (6 non-inline funcs).
// Source: source/cdWorldVertexLitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldVertexLitShader.o):
//   cdWorldVertexLitShaderMat::ctor @0x7DE770
//   InitCDWorldVertexLitShader  @0x7DE800
//   ToggleCDWorldVertexLitShader @0x7DE850
//   cdWorldVertexLitShader::Register @0x7DE870
//   cdWorldVertexLitShader::AddNode @0x7DE8D0
// ============================================================================
#include "cdWorldVertexLitShader.h"

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
// cdWorldVertexLitShaderMat::ctor — bind texture + shader.
// ea: 0x7DE770
// ============================================================================
cdWorldVertexLitShaderMat::cdWorldVertexLitShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdWorldVertexLitShader* v3 = g_cdWorldVertexLitShader;
    if (g_cdWorldVertexLitShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = 0;
    AeAssert::gCurrentFile = "cdWorldVertexLitShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "g_cdWorldVertexLitShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = g_cdWorldVertexLitShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = g_cdWorldVertexLitShader;
}

// ============================================================================
// InitCDWorldVertexLitShader — allocate the shader and link into the init list.
// ea: 0x7DE800
// ============================================================================
void InitCDWorldVertexLitShader() {
    cdWorldVertexLitShader* result = (cdWorldVertexLitShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldVertexLitShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~4;
        gCDWorldVertexLitShader = result;
    } else {
        gCDWorldVertexLitShader = NULL;
    }
}

// ============================================================================
// ToggleCDWorldVertexLitShader — toggle vertex-lit enable bit (bit 2).
// ea: 0x7DE850
// ============================================================================
void ToggleCDWorldVertexLitShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (4 * ~(byte >> 2))) & 4) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;
}

// ============================================================================
// cdWorldVertexLitShader::Register — register the shaders.
// ea: 0x7DE870
// ============================================================================
void cdWorldVertexLitShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader(cdWorldVertexLitRender::VS, cdWorldVertexLitRender::VShaderTable[0]);
    cdWorldVertexLitRender::Shader = cdWorldVertexLitRender::VS[0];
    nglDxRegisterPShader(cdWorldVertexLitPixel::PS, cdWorldVertexLitPixel::PShaderTable[0]);
    cdWorldVertexLitPixel::Shader = cdWorldVertexLitPixel::PS[0];
    nglDxRegisterPShader(cdWorldVertexLitFullbrightPixel::PS, cdWorldVertexLitFullbrightPixel::PShaderTable[0]);
    cdWorldVertexLitFullbrightPixel::Shader = cdWorldVertexLitFullbrightPixel::PS[0];
}

// ============================================================================
// cdWorldVertexLitShader::AddNode — add a node to the opaque list.
// ea: 0x7DE8D0
// ============================================================================
void cdWorldVertexLitShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                     nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 4) == 0) {
        cdWorldVertexLitShaderNode* node = (cdWorldVertexLitShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdWorldVertexLitShaderNode
            node->mMaterial = (cdWorldVertexLitShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDWorldVertexLitShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
