// ============================================================================
// cdRiverShader.cpp — river shader (10 non-inline funcs).
// Source: source/cdRiverShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdRiverShader.o):
//   cdRiverShaderMat::ctor @0x7D6BC0
//   InitCDRiverShader  @0x7D6BE0
//   ToggleCDRiverShader @0x7D6C30
//   cdRiverShaderNode::GetVShader @0x7D6CA0
//   cdRiverShaderNode::GetFullbrightPShader @0x7D6CB0
//   cdRiverShaderNode::GetPShader @0x7D6CC0
//   cdRiverShaderNode::GetVShaderParamsStartAddress @0x7D6CE0
//   cdRiverShaderNode::GetVShaderFogConstantOffset @0x7D6CF0
//   cdRiverShader::Register @0x7D6C50
//   cdRiverShader::AddNode @0x7D6D00
// ============================================================================
#include "cdRiverShader.h"

#include <intrin.h>

// ============================================================================
// cdRiverShaderMat::cdRiverShaderMat — delegate to the ocean material ctor.
// ea: 0x7D6BC0
// ============================================================================
cdRiverShaderMat::cdRiverShaderMat(nglTexture* iTexture)
    : cdOceanShaderMat(iTexture) {
}

// ============================================================================
// InitCDRiverShader — allocate the shader and link into the init list.
// ea: 0x7D6BE0
// ============================================================================
void InitCDRiverShader() {
    cdRiverShader* result = (cdRiverShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        // vftable = cdOceanShader
        result->Disabled = false;
        ShaderCommon::ShaderSwitching.__s0[1] &= ~1;
        // vftable = cdRiverShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~2;
        gCDRiverShader = result;
    } else {
        gCDRiverShader = NULL;
    }
}

// ============================================================================
// ToggleCDRiverShader — toggle river-shader enable bit (bit 1).
// ea: 0x7D6C30
// ============================================================================
void ToggleCDRiverShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ (2 * ~(byte >> 1))) & 2) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;
}

// ============================================================================
// cdRiverShaderNode::GetVShader — return the river vertex shader.
// ea: 0x7D6CA0
// ============================================================================
unsigned int cdRiverShaderNode::GetVShader() {
    return cdRiverRender::VS[0];
}

// ============================================================================
// cdRiverShaderNode::GetFullbrightPShader — return the fullbright PShader.
// ea: 0x7D6CB0
// ============================================================================
unsigned long* cdRiverShaderNode::GetFullbrightPShader() {
    return cdRiverPixel_Fullbright::PS[0];
}

// ============================================================================
// cdRiverShaderNode::GetPShader — return the indexed PShader.
// ea: 0x7D6CC0
// ============================================================================
unsigned long* cdRiverShaderNode::GetPShader(int l2, int l3, int lm) {
    return cdRiverPixel::PS[l2][l3][lm];
}

// ============================================================================
// cdRiverShaderNode::GetVShaderParamsStartAddress — return -88.
// ea: 0x7D6CE0
// ============================================================================
int cdRiverShaderNode::GetVShaderParamsStartAddress() {
    return -88;
}

// ============================================================================
// cdRiverShaderNode::GetVShaderFogConstantOffset — return -90.
// ea: 0x7D6CF0
// ============================================================================
int cdRiverShaderNode::GetVShaderFogConstantOffset() {
    return -90;
}

// ============================================================================
// cdRiverShader::Register — register the river vertex/pixel shaders.
// ea: 0x7D6C50
// ============================================================================
void cdRiverShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader((unsigned int*)cdRiverRender::VS, cdRiverRender::VShaderTable[0]);
    cdRiverRender::Shader = cdRiverRender::VS[0];
    for (int v0 = 0, i = 8; i != 0; --i, ++v0) {
        nglDxRegisterPShader((unsigned int**)&cdRiverPixel::PS[0][0][v0], cdRiverPixel::PShaderTable[0][0][v0]);
    }
    nglDxRegisterPShader((unsigned int**)cdRiverPixel_Fullbright::PS, cdRiverPixel_Fullbright::PShaderTable[0]);
    cdRiverPixel_Fullbright::Shader = cdRiverPixel_Fullbright::PS[0];
}

// ============================================================================
// cdRiverShader::AddNode — add a river node to the opaque list.
// ea: 0x7D6D00
// ============================================================================
void cdRiverShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 2) == 0) {
        cdRiverShaderNode* node = (cdRiverShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            node->mMaterial = (cdRiverShaderMat*)iMat;
            // vftable = cdRiverShaderNode
        } else {
            node = NULL;
        }
        node->SortHash = -2;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
