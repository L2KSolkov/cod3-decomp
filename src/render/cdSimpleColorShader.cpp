// ============================================================================
// cdSimpleColorShader.cpp — simple color shader (4 non-inline funcs).
// Source: source/cdSimpleColorShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleColorShader.o):
//   InitCDSimpleColorShader @0x7D5F10
//   ToggleCDSimpleColorShader @0x7D5F60
//   cdSimpleColorShader::Register @0x7D5F80
//   cdSimpleColorShader::AddNode @0x7D5F90
// ============================================================================
#include "cdSimpleColorShader.h"

#include <intrin.h>

// ============================================================================
// cdSimpleColorShader::Register — ea: 0x7D5F80 (empty)
// ============================================================================
void cdSimpleColorShader::Register() {
}

// ============================================================================
// InitCDSimpleColorShader — allocate the shader and link into the init list.
// ea: 0x7D5F10
// ============================================================================
cdSimpleColorShader* InitCDSimpleColorShader() {
    cdSimpleColorShader* result = (cdSimpleColorShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        // vftable = tlInitList, link into init list
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSimpleColorShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~8;
        gCDSimpleColorShader = result;
    } else {
        gCDSimpleColorShader = NULL;
        return NULL;
    }
    return result;
}

// ============================================================================
// ToggleCDSimpleColorShader — toggle the color-shader enable bit (bit 3).
// ea: 0x7D5F60
// ============================================================================
char ToggleCDSimpleColorShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ (8 * ~(byte >> 3))) & 8) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;
    return (char)byte;
}

// ============================================================================
// cdSimpleColorShader::AddNode — add a color quad node to the opaque list.
// ea: 0x7D5F90
// ============================================================================
void cdSimpleColorShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                  nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 8) == 0) {
        cdSimpleShaderNode* node = (cdSimpleShaderNode*)nglListAlloc(0x1C, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdSimpleShaderNode
            node->mMaterial = (cdSimpleShaderMat*)iMat;
            node->hasColorVerts = true;
        } else {
            node = NULL;
        }
        node->SortHash = gCDSimpleColorShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
