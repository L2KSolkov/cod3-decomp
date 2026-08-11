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
// cdDebugShaderMat::cdDebugShaderMat — default material, bind the shader.
// ea: 0x7C6360
// ============================================================================
cdDebugShaderMat::cdDebugShaderMat() {
    if (gCDDebugShader == NULL) {
        AeAssert::gCurrentAuthor = 0;
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
void cdDebugShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader((unsigned int*)cdDebugShaderRender::VS, cdDebugShaderRender::VShaderTable[0]);
    nglDxRegisterPShader((unsigned int**)cdDebugPixel::PS, cdDebugPixel::PShaderTable[0]);
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
