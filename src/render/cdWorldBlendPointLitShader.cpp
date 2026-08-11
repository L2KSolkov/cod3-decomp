// ============================================================================
// cdWorldBlendPointLitShader.cpp — world blend point-lit shader (5 funcs).
// Source: source/cdWorldBlendPointLitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldBlendPointLitShader.o):
//   InitCDWorldBlendPointLitShader  @0x7DA260
//   ToggleCDWorldBlendPointLitShader @0x7DA2B0
//   cdWorldBlendPointLitShader::Register @0x7DA2D0
//   cdWorldBlendPointLitShader::AddNode @0x7DA310
// ============================================================================
#include "cdWorldBlendPointLitShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdWorldBlendPointLitShader* gCDWorldBlendPointLitShader = nullptr;  // ?gCDWorldBlendPointLitShader@@3PAVcdWorldBlendPointLitShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdWorldBlendPointLitRender {
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = {};
}
namespace cdWorldBlendPointLitProjectedRender {
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = {};
}
namespace cdWorldBlendPointLitPixel {
    unsigned long* PS[2][2] = {};
    unsigned int const* PShaderTable[2][2] = {};
}
namespace cdWorldBlendPointLitProjectedPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
}
namespace cdWorldBlendPointLitSolidColorPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
    unsigned long* Shader = nullptr;
}

// ============================================================================
// InitCDWorldBlendPointLitShader — allocate the shader and link the init list.
// ea: 0x7DA260
// ============================================================================
void InitCDWorldBlendPointLitShader() {
    cdWorldBlendPointLitShader* result = (cdWorldBlendPointLitShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldBlendPointLitShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~8;
        gCDWorldBlendPointLitShader = result;
    } else {
        gCDWorldBlendPointLitShader = NULL;
    }
}

// ============================================================================
// ToggleCDWorldBlendPointLitShader — toggle enable bit (bit 5).
// ea: 0x7DA2B0
// ============================================================================
void ToggleCDWorldBlendPointLitShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (32 * ~(byte >> 5))) & 0x20) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;
}

// ============================================================================
// cdWorldBlendPointLitShader::Register — register the shaders.
// ea: 0x7DA2D0
// ============================================================================
void cdWorldBlendPointLitShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShader((unsigned int*)&cdWorldBlendPointLitRender::VS[v0], cdWorldBlendPointLitRender::VShaderTable[v0]);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShader((unsigned int*)&cdWorldBlendPointLitProjectedRender::VS[v0], cdWorldBlendPointLitProjectedRender::VShaderTable[v0]);
    }
    for (int v0 = 0, i = 4; i != 0; --i, ++v0) {
        nglDxRegisterPShader((unsigned int**)&cdWorldBlendPointLitPixel::PS[0][v0], cdWorldBlendPointLitPixel::PShaderTable[0][v0]);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShader((unsigned int**)&cdWorldBlendPointLitProjectedPixel::PS[v0], cdWorldBlendPointLitProjectedPixel::PShaderTable[v0]);
    }
    nglDxRegisterPShader((unsigned int**)cdWorldBlendPointLitSolidColorPixel::PS, cdWorldBlendPointLitSolidColorPixel::PShaderTable[0]);
    cdWorldBlendPointLitSolidColorPixel::Shader = cdWorldBlendPointLitSolidColorPixel::PS[0];
}

// ============================================================================
// cdWorldBlendPointLitShader::AddNode — add a clipped node to the opaque list.
// ea: 0x7DA310
// ============================================================================
void cdWorldBlendPointLitShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                         nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 0x20) == 0 &&
        cdGetClipResult(iSection, iMeshNode, nglBuildScene) != -1) {
        cdWorldBlendPointLitShaderNode* node = (cdWorldBlendPointLitShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdWorldBlendPointLitShaderNode
            node->mMaterial = (cdWorldBlendPointLitShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDWorldBlendPointLitShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
