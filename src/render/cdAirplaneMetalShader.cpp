// ============================================================================
// cdAirplaneMetalShader.cpp — airplane metal shader (5 non-inline funcs).
// Source: source/cdAirplaneMetalShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdAirplaneMetalShader.o):
//   InitCDAirplaneMetalShader  @0x7D4130
//   ToggleCDAirplaneMetalShader @0x7D4180
//   cdAirplaneMetalShader::Register @0x7D41A0
//   cdAirplaneMetalShader::AddNode @0x7D41E0
// ============================================================================
#include "cdAirplaneMetalShader.h"

#include <intrin.h>

// ============================================================================
// InitCDAirplaneMetalShader — allocate the shader and link into the init list.
// ea: 0x7D4130
// ============================================================================
void InitCDAirplaneMetalShader() {
    cdAirplaneMetalShader* result = (cdAirplaneMetalShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdAirplaneMetalShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~2;
        gCDAirplaneMetalShader = result;
    } else {
        gCDAirplaneMetalShader = NULL;
    }
}

// ============================================================================
// ToggleCDAirplaneMetalShader — toggle airplane-metal enable bit (bit 1).
// ea: 0x7D4180
// ============================================================================
void ToggleCDAirplaneMetalShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (2 * ~(byte >> 1))) & 2) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;
}

// ============================================================================
// cdAirplaneMetalShader::Register — register the airplane-metal shaders.
// ea: 0x7D41A0
// ============================================================================
void cdAirplaneMetalShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader(cdAirplaneMetalRender::VS, cdAirplaneMetalRender::VShaderTable[0]);
    nglDxRegisterPShader(cdAirplaneMetalPixel::PS, cdAirplaneMetalPixel::PShaderTable[0]);
    nglDxRegisterPShader(cdAirplaneMetalSolidColorPixel::PS, cdAirplaneMetalSolidColorPixel::PShaderTable[0]);
}

// ============================================================================
// cdAirplaneMetalShader::AddNode — add a node to the opaque list.
// ea: 0x7D41E0
// ============================================================================
void cdAirplaneMetalShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                    nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 2) == 0) {
        cdAirplaneMetalShaderNode* node = (cdAirplaneMetalShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdAirplaneMetalShaderNode
            node->mMaterial = (cdAirplaneMetalShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDAirplaneMetalShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
