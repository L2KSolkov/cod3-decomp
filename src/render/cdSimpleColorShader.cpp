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

// Shader global pointer definitions
cdSimpleColorShader* gCDSimpleColorShader = nullptr;  // ?gCDSimpleColorShader@@3PAVcdSimpleColorShader@@A

// ea: 0x007D5EC0
cdSimpleColorShader::cdSimpleColorShader() {
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    ShaderCommon::ShaderSwitching.__s0[1] &= ~8;
}

// ea: 0x007D60B0
cdSimpleColorShader::~cdSimpleColorShader() = default;

// ============================================================================
// cdSimpleColorShader::GetName — ea: 0x7D5EF0
// ============================================================================
// ea: 0x007D5EF0
tlFixedString cdSimpleColorShader::GetName() { return tlFixedString("cdSimpleColor"); }

// ea: 0x007D5F80
void cdSimpleColorShader::Register() { nglShader::Register(); }

// ============================================================================
// InitCDSimpleColorShader — allocate the shader and link into the init list.
// ea: 0x7D5F10
// ============================================================================
void InitCDSimpleColorShader() {
    cdSimpleColorShader* result = (cdSimpleColorShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdSimpleColorShader;
        gCDSimpleColorShader = result;
    } else {
        gCDSimpleColorShader = NULL;

    }

}

// ============================================================================
// ToggleCDSimpleColorShader — toggle the color-shader enable bit (bit 3).
// ea: 0x7D5F60
// ============================================================================
void ToggleCDSimpleColorShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ (8 * ~(byte >> 3))) & 8) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;

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
            ::new (node) cdSimpleShaderNode(iMeshNode, iSection,
                                             (cdSimpleShaderMat*)iMat, true);
        } else {
            node = NULL;
        }
        node->SortHash = gCDSimpleColorShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ea: 0x007D6010
cdSimpleShaderNode::cdSimpleShaderNode(
    nglMeshNode* iMeshNode, nglMeshSection* iSection,
    cdSimpleShaderMat* iMaterial, bool colorVerts) {
    this->MeshNode = iMeshNode;
    this->Section = iSection;
    this->mMaterial = iMaterial;
    this->hasColorVerts = colorVerts;
}

// ea: 0x007D6070
cdSimpleShaderNode::~cdSimpleShaderNode() = default;
