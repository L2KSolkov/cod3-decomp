// ============================================================================
// cdCharSpecularShader.cpp — char specular shader (5 non-inline funcs).
// Source: source/cdCharSpecularShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdCharSpecularShader.o):
//   InitCDCharSpecularShader  @0x7D2130
//   ToggleCDCharSpecularShader @0x7D2180
//   cdCharSpecularShader::Register @0x7D21A0
//   cdCharSpecularShader::AddNode @0x7D21F0
// ============================================================================
#include "cdCharSpecularShader.h"

#include <intrin.h>

// ============================================================================
// InitCDCharSpecularShader — allocate the shader and link into the init list.
// ea: 0x7D2130
// ============================================================================
void InitCDCharSpecularShader() {
    cdCharSpecularShader* result = (cdCharSpecularShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdCharSpecularShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~2;
        gcdCharSpecularShader = result;
    } else {
        gcdCharSpecularShader = NULL;
    }
}

// ============================================================================
// ToggleCDCharSpecularShader — toggle char-specular enable bit (bit 1).
// ea: 0x7D2180
// ============================================================================
void ToggleCDCharSpecularShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (2 * ~(byte >> 1))) & 2) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;
}

// ============================================================================
// cdCharSpecularShader::Register — register the char-specular shaders.
// ea: 0x7D21A0
// ============================================================================
void cdCharSpecularShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShader(&cdCharSpecularShaderRender::VS[v0], cdCharSpecularShaderRender::VShaderTable[v0]);
    }
    nglDxRegisterPShader(cdCharSpecularPixel::PS, cdCharSpecularPixel::PShaderTable[0]);
    cdCharSpecularPixel::Shader = cdCharSpecularPixel::PS[0];
    nglDxRegisterPShader(cdCharSpecularFullbrightPixel::PS, cdCharSpecularFullbrightPixel::PShaderTable[0]);
    cdCharSpecularFullbrightPixel::Shader = cdCharSpecularFullbrightPixel::PS[0];
}

// ============================================================================
// cdCharSpecularShader::AddNode — add a char-specular node to the opaque list.
// ea: 0x7D21F0
// ============================================================================
void cdCharSpecularShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                   nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 4) == 0) {
        cdCharSpecularShaderNode* node = (cdCharSpecularShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdCharSpecularShaderNode
            node->mMaterial = (cdCharSpecularShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gcdCharSpecularShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
