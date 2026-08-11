// ============================================================================
// cdWorldColorShader.cpp — world color shader (4 non-inline funcs).
// Source: source/cdWorldColorShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldColorShader.o):
//   InitCDWorldColorShader @0x7D9DC0
//   ToggleCDWorldColorShader @0x7D9E10
//   cdWorldColorShader::Register @0x7D9E30
//   cdWorldColorShader::AddNode @0x7D9E40
// ============================================================================
#include "cdWorldColorShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdWorldColorShader* gCDWorldColorShader = nullptr;  // ?gCDWorldColorShader@@3PAVcdWorldColorShader@@A

// ============================================================================
// cdWorldColorShader::Register — ea: 0x7D9E30 (empty)
// ============================================================================
void cdWorldColorShader::Register() {
}

// ============================================================================
// InitCDWorldColorShader — allocate the shader and link into the init list.
// ea: 0x7D9DC0
// ============================================================================
void InitCDWorldColorShader() {
    cdWorldColorShader* result = (cdWorldColorShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldColorShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~0x40;
        gCDWorldColorShader = result;
    } else {
        gCDWorldColorShader = NULL;

    }

}

// ============================================================================
// ToggleCDWorldColorShader — toggle the world-color enable bit (bit 6).
// ea: 0x7D9E10
// ============================================================================
void ToggleCDWorldColorShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (~(byte >> 6) << 6)) & 0x40) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;

}

// ============================================================================
// cdWorldColorShader::AddNode — add a clipped world color node to the list.
// ea: 0x7D9E40
// ============================================================================
void cdWorldColorShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                 nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 0x40) == 0) {
        int ClipResult = cdGetClipResult(iSection, iMeshNode, nglBuildScene);
        if (ClipResult != -1) {
            cdWorldShaderNode* node = (cdWorldShaderNode*)nglListAlloc(0x20, 0x10);
            if (node != NULL) {
                node->MeshNode = iMeshNode;
                node->Section = iSection;
                // vftable = cdWorldShaderNode
                node->mMaterial = (cdWorldShaderMat*)iMat;
                node->hasColorVerts = true;
            } else {
                node = NULL;
            }
            node->Clip = ClipResult;
            node->SortHash = gCDWorldColorShader->ID;
            node->Next = nglBuildScene->OpaqueRenderList;
            nglBuildScene->OpaqueRenderList = node;
            ++nglBuildScene->OpaqueListCount;
        }
    }
}
