// ============================================================================
// cdWorldBlendShader.cpp — world blend shader (5 non-inline funcs).
// Source: source/cdWorldBlendShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldBlendShader.o):
//   InitCDWorldBlendShader  @0x7DD2A0
//   ToggleCDWorldBlendShader @0x7DD2F0
//   cdWorldBlendShader::Register @0x7DD310
//   cdWorldBlendShader::AddNode @0x7DD350
// ============================================================================
#include "cdWorldBlendShader.h"

#include <intrin.h>

// ============================================================================
// InitCDWorldBlendShader — allocate the shader and link into the init list.
// ea: 0x7DD2A0
// ============================================================================
void InitCDWorldBlendShader() {
    cdWorldBlendShader* result = (cdWorldBlendShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldBlendShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~8;
        gCDWorldBlendShader = result;
    } else {
        gCDWorldBlendShader = NULL;
    }
}

// ============================================================================
// ToggleCDWorldBlendShader — toggle world-blend enable bit (bit 3).
// ea: 0x7DD2F0
// ============================================================================
void ToggleCDWorldBlendShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (8 * ~(byte >> 3))) & 8) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;
}

// ============================================================================
// cdWorldBlendShader::Register — register the world-blend shaders.
// ea: 0x7DD310
// ============================================================================
void cdWorldBlendShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShader(&cdWorldBlendRender::VS[v0], cdWorldBlendRender::VShaderTable[v0]);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShader(&cdWorldBlendProjectedRender::VS[v0], cdWorldBlendProjectedRender::VShaderTable[v0]);
    }
    for (int v0 = 0, i = 4; i != 0; --i, ++v0) {
        nglDxRegisterPShader(&cdWorldBlendPixel::PS[0][v0], cdWorldBlendPixel::PShaderTable[0][v0]);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShader(&cdWorldBlendProjectedPixel::PS[v0], cdWorldBlendProjectedPixel::PShaderTable[v0]);
    }
    nglDxRegisterPShader(cdWorldBlendSolidColorPixel::PS, cdWorldBlendSolidColorPixel::PShaderTable[0]);
    cdWorldBlendSolidColorPixel::Shader = cdWorldBlendSolidColorPixel::PS[0];
}

// ============================================================================
// cdWorldBlendShader::AddNode — add a clipped node to the opaque list.
// ea: 0x7DD350
// ============================================================================
void cdWorldBlendShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                 nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 8) == 0 &&
        cdGetClipResult(iSection, iMeshNode, nglBuildScene) != -1) {
        cdWorldBlendShaderNode* node = (cdWorldBlendShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdWorldBlendShaderNode
            node->mMaterial = (cdWorldBlendShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDWorldBlendShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
