// ============================================================================
// cdPrelitShader.cpp — prelit shader (5 non-inline funcs).
// Source: source/cdPrelitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdPrelitShader.o):
//   InitCDPrelitShader  @0x7D3590
//   ToggleCDPrelitShader @0x7D35E0
//   cdPrelitShader::Register @0x7D3600
//   cdPrelitShader::AddNode @0x7D3B70
// ============================================================================
#include "cdPrelitShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdPrelitShader* gCDPrelitShader = nullptr;  // ?gCDPrelitShader@@3PAVcdPrelitShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdPrelitRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
}
namespace cdPrelitPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
}
namespace cdPrelitFullbrightPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
}
namespace cdPrelitSolidColorPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
}

// ============================================================================
// InitCDPrelitShader — allocate the shader and link into the init list.
// ea: 0x7D3590
// ============================================================================
void InitCDPrelitShader() {
    cdPrelitShader* result = (cdPrelitShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdPrelitShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdPrelitShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~4;
        gCDPrelitShader = result;
    } else {
        gCDPrelitShader = NULL;

    }

}

// ============================================================================
// ToggleCDPrelitShader — toggle the prelit-shader enable bit (bit 2).
// ea: 0x7D35E0
// ============================================================================
void ToggleCDPrelitShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (4 * ~(byte >> 2))) & 4) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;

}

// ============================================================================
// cdPrelitShader::Register — register the prelit vertex/pixel shaders.
// ea: 0x7D3600
// ============================================================================
tlFixedString cdPrelitShader::GetName() { return tlFixedString("cdPrelit"); }

void cdPrelitShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdPrelitRender::VS, cdPrelitRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdPrelitPixel::PS, cdPrelitPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdPrelitFullbrightPixel::PS, cdPrelitFullbrightPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdPrelitSolidColorPixel::PS, cdPrelitSolidColorPixel::PShaderTable, 0);
}

// ============================================================================
// cdPrelitShader::AddNode — add a prelit node to the opaque list.
// ea: 0x7D3B70
// ============================================================================
void cdPrelitShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                             nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 4) == 0) {
        cdPrelitShaderNode* node = (cdPrelitShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdPrelitShaderNode
            node->mMaterial = (cdPrelitShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDPrelitShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
