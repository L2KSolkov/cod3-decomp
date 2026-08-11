// ============================================================================
// cdCharShader.cpp — character shader (5 non-inline funcs).
// Source: source/cdCharShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdCharShader.o):
//   InitCDCharShader  @0x7D2B70
//   ToggleCDCharShader @0x7D2BC0
//   cdCharShader::Register @0x7D2BE0
//   cdCharShader::AddNode @0x7D2C40
// ============================================================================
#include "cdCharShader.h"

#include <intrin.h>

// ============================================================================
// InitCDCharShader — allocate the shader and link into the init list.
// ea: 0x7D2B70
// ============================================================================
void InitCDCharShader() {
    cdCharShader* result = (cdCharShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdCharShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~2;
        gCDCharShader = result;
    } else {
        gCDCharShader = NULL;

    }

}

// ============================================================================
// ToggleCDCharShader — toggle the character-shader enable bit (bit 1).
// ea: 0x7D2BC0
// ============================================================================
void ToggleCDCharShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (2 * ~(byte >> 1))) & 2) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;

}

// ============================================================================
// cdCharShader::Register — register the character vertex/pixel shaders.
// ea: 0x7D2BE0
// ============================================================================
void cdCharShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader((unsigned int*)cdCharShaderRender::VS, cdCharShaderRender::VShaderTable[0]);
    cdCharShaderRender::Shader = cdCharShaderRender::VS[0];
    nglDxRegisterPShader((unsigned int**)cdCharPixel::PS, cdCharPixel::PShaderTable[0]);
    cdCharPixel::Shader = cdCharPixel::PS[0];
    nglDxRegisterPShader((unsigned int**)cdCharFullbrightPixel::PS, cdCharFullbrightPixel::PShaderTable[0]);
    cdCharFullbrightPixel::Shader = cdCharFullbrightPixel::PS[0];
}

// ============================================================================
// cdCharShader::AddNode — add a character node to the opaque list.
// ea: 0x7D2C40
// ============================================================================
void cdCharShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                           nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 2) == 0) {
        cdCharShaderNode* node = (cdCharShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdCharShaderNode
            node->mMaterial = (cdCharShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDCharShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
