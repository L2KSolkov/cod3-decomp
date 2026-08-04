// ============================================================================
// cdSimpleShader.cpp — simple shader (6 non-inline funcs).
// Source: source/cdSimpleShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleShader.o):
//   cdSimpleShaderMat::ctor @0x7D6390
//   InitCDSimpleShader  @0x7D6420
//   ToggleCDSimpleShader @0x7D6470
//   cdSimpleShader::Register @0x7D6490
//   cdSimpleShader::AddNode @0x7D64C0
// ============================================================================
#include "cdSimpleShader.h"

#include <intrin.h>

// ============================================================================
// cdSimpleShaderMat::cdSimpleShaderMat — bind the texture + shader.
// ea: 0x7D6390
// ============================================================================
cdSimpleShaderMat::cdSimpleShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    this->Shader = gCDSimpleShader;
}

// ============================================================================
// cdSimpleShader::Register — ea: 0x7D6490 (empty)
// ============================================================================
void cdSimpleShader::Register() {
}

// ============================================================================
// InitCDSimpleShader — allocate the shader and link into the init list.
// ea: 0x7D6420
// ============================================================================
void InitCDSimpleShader() {
    cdSimpleShader* result = (cdSimpleShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSimpleShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~4;
        gCDSimpleShader = result;
    } else {
        gCDSimpleShader = NULL;

    }

}

// ============================================================================
// ToggleCDSimpleShader — toggle the simple-shader enable bit (bit 2).
// ea: 0x7D6470
// ============================================================================
void ToggleCDSimpleShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ (4 * ~(byte >> 2))) & 4) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;

}

// ============================================================================
// cdSimpleShader::AddNode — add a simple shader node to the opaque list.
// ea: 0x7D64C0
// ============================================================================
void cdSimpleShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                             nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 4) == 0) {
        cdSimpleShaderNode* node = (cdSimpleShaderNode*)nglListAlloc(0x1C, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdSimpleShaderNode
            node->mMaterial = (cdSimpleShaderMat*)iMat;
            node->hasColorVerts = false;
        } else {
            node = NULL;
        }
        node->SortHash = gCDSimpleShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
