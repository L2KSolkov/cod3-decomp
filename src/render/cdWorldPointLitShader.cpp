// IDA ABI: nglMeshNode uses the class tag in render_xboxr exports.
// ============================================================================
// cdWorldPointLitShader.cpp — world point-lit shader (5 non-inline funcs).
// Source: source/cdWorldPointLitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldPointLitShader.o):
//   InitCDWorldPointLitShader  @0x7DBB00
//   ToggleCDWorldPointLitShader @0x7DBB50
//   cdWorldPointLitShader::Register @0x7DBB70
//   cdWorldPointLitShader::AddNode @0x7DBBB0
// ============================================================================
#include "cdWorldPointLitShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdWorldPointLitShader* gCDWorldPointLitShader = nullptr;  // ?gCDWorldPointLitShader@@3PAVcdWorldPointLitShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdWorldPointLitRender {
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = {};
}
namespace cdWorldPointLitProjectedRender {
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = {};
}
namespace cdWorldPointLitPixel {
    unsigned long* PS[2][2] = {};
    unsigned int const* PShaderTable[2][2] = {};
}
namespace cdWorldPointLitProjectedPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
}
namespace cdWorldPointLitSolidColorPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
    unsigned long* Shader = nullptr;
}

// ============================================================================
// InitCDWorldPointLitShader — allocate the shader and link into the init list.
// ea: 0x7DBB00
// ============================================================================
void InitCDWorldPointLitShader() {
    cdWorldPointLitShader* result = (cdWorldPointLitShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdWorldPointLitShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldPointLitShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~0x10;
        gCDWorldPointLitShader = result;
    } else {
        gCDWorldPointLitShader = NULL;
    }
}

// ============================================================================
// ToggleCDWorldPointLitShader — toggle point-lit enable bit (bit 4).
// ea: 0x7DBB50
// ============================================================================
void ToggleCDWorldPointLitShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (16 * ~(byte >> 4))) & 0x10) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;
}

// ============================================================================
// cdWorldPointLitShader::Register — register the point-lit shaders.
// ea: 0x7DBB70
// ============================================================================
tlFixedString cdWorldPointLitShader::GetName() { return tlFixedString("cdWorldPointLit"); }

void cdWorldPointLitShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldPointLitRender::VS[v0], cdWorldPointLitRender::VShaderTable, v0);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldPointLitProjectedRender::VS[v0], cdWorldPointLitProjectedRender::VShaderTable, v0);
    }
    for (int v0 = 0, i = 4; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldPointLitPixel::PS[0][v0], cdWorldPointLitPixel::PShaderTable[0], v0);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldPointLitProjectedPixel::PS[v0], cdWorldPointLitProjectedPixel::PShaderTable, v0);
    }
    nglDxRegisterPShaderSafe((unsigned int**)cdWorldPointLitSolidColorPixel::PS, cdWorldPointLitSolidColorPixel::PShaderTable, 0);
    cdWorldPointLitSolidColorPixel::Shader = cdWorldPointLitSolidColorPixel::PS != nullptr ? cdWorldPointLitSolidColorPixel::PS[0] : 0;
}

// ============================================================================
// cdWorldPointLitShader::AddNode — add a clipped node to the opaque list.
// ea: 0x7DBBB0
// ============================================================================
void cdWorldPointLitShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                    nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 0x10) == 0) {
        int ClipResult = cdGetClipResult(iSection, iMeshNode, nglBuildScene);
        if (ClipResult != -1) {
            cdWorldPointLitShaderNode* node = (cdWorldPointLitShaderNode*)nglListAlloc(0x1C, 0x10);
            if (node != NULL) {
                node->MeshNode = iMeshNode;
                node->Section = iSection;
                // vftable = cdWorldPointLitShaderNode
                node->mMaterial = (cdWorldPointLitShaderMat*)iMat;
            } else {
                node = NULL;
            }
            node->Clip = ClipResult;
            if (((cdWorldPointLitShaderMat*)iMat)->mDiffuse == NULL)
                ((cdWorldPointLitShaderMat*)iMat)->mDiffuse = nglDefaultTex;
            node->SortHash = gCDWorldPointLitShader->ID;
            node->Next = nglBuildScene->OpaqueRenderList;
            nglBuildScene->OpaqueRenderList = node;
            ++nglBuildScene->OpaqueListCount;
        }
    }
}
