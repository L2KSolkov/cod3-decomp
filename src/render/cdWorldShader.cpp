// ============================================================================
// cdWorldShader.cpp — world shader (5 non-inline funcs).
// Source: source/cdWorldShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldShader.o):
//   InitCDWorldShader  @0x7DF0D0
//   ToggleCDWorldShader @0x7DF120
//   cdWorldShader::Register @0x7DF140
//   cdWorldShader::AddNode @0x7DF180
// ============================================================================
#include "cdWorldShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdWorldShader* gCDWorldShader = nullptr;  // ?gCDWorldShader@@3PAVcdWorldShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdWorldRender {
    unsigned long VS[4][2] = {};
    unsigned int const* VShaderTable[4][2] = {};
}
namespace cdWorldProjectedRender {
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = {};
}
namespace cdWorldPixel {
    unsigned long* PS[2][2][2] = {};
    unsigned int const* PShaderTable[2][2][2] = {};
}
namespace cdWorldProjectedPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
}
namespace cdWorldSolidColorPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
    unsigned long* Shader = nullptr;
}

// ============================================================================
// cdWorldRender::RegisterShader — register the 4 world vertex shaders.
// ea: 0x7DFFE0 (inline COMDAT)
// ============================================================================
inline void cdWorldRender_RegisterShader() {
    for (int v0 = 0, i = 4; i != 0; --i, ++v0) {
        nglDxRegisterVShader((unsigned int*)&cdWorldRender::VS[0][v0], cdWorldRender::VShaderTable[0][v0]);
    }
}

// ============================================================================
// cdWorldProjectedRender::RegisterShader — register the 2 projected VShaders.
// ea: 0x7E0040 (inline COMDAT)
// ============================================================================
inline void cdWorldProjectedRender_RegisterShader() {
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShader((unsigned int*)&cdWorldProjectedRender::VS[v0], cdWorldProjectedRender::VShaderTable[v0]);
    }
}

// ============================================================================
// cdWorldPixel::RegisterShader — register the 8 world pixel shaders.
// ea: 0x7E0090 (inline COMDAT)
// ============================================================================
inline void cdWorldPixel_RegisterShader() {
    for (int v0 = 0, i = 8; i != 0; --i, ++v0) {
        nglDxRegisterPShader((unsigned int**)&cdWorldPixel::PS[0][0][v0], cdWorldPixel::PShaderTable[0][0][v0]);
    }
}

// ============================================================================
// cdWorldProjectedPixel::RegisterShader — register the 2 projected PShaders.
// ea: 0x7E00F0 (inline COMDAT)
// ============================================================================
inline void cdWorldProjectedPixel_RegisterShader() {
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShader((unsigned int**)&cdWorldProjectedPixel::PS[v0], cdWorldProjectedPixel::PShaderTable[v0]);
    }
}

// ============================================================================
// InitCDWorldShader — allocate the shader and link into the init list.
// ea: 0x7DF0D0
// ============================================================================
void InitCDWorldShader() {
    cdWorldShader* result = (cdWorldShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~2;
        gCDWorldShader = result;
    } else {
        gCDWorldShader = NULL;

    }

}

// ============================================================================
// ToggleCDWorldShader — toggle the world-shader enable bit (bit 1).
// ea: 0x7DF120
// ============================================================================
void ToggleCDWorldShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (2 * ~(byte >> 1))) & 2) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;

}

// ============================================================================
// cdWorldShader::Register — register all world vertex/pixel shaders.
// ea: 0x7DF140
// ============================================================================
void cdWorldShader::Register() {
    nglShader::Register();
    cdWorldRender_RegisterShader();
    cdWorldProjectedRender_RegisterShader();
    cdWorldPixel_RegisterShader();
    cdWorldProjectedPixel_RegisterShader();
    nglDxRegisterPShader((unsigned int**)cdWorldSolidColorPixel::PS, cdWorldSolidColorPixel::PShaderTable[0]);
    cdWorldSolidColorPixel::Shader = cdWorldSolidColorPixel::PS[0];
}

// ============================================================================
// cdWorldShader::AddNode — add a clipped world node to the opaque list.
// ea: 0x7DF180
// ============================================================================
void cdWorldShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 2) == 0) {
        int ClipResult = cdGetClipResult(iSection, iMeshNode, nglBuildScene);
        if (ClipResult != -1) {
            cdWorldShaderNode* node = (cdWorldShaderNode*)nglListAlloc(0x20, 0x10);
            if (node != NULL) {
                node->MeshNode = iMeshNode;
                node->Section = iSection;
                // vftable = cdWorldShaderNode
                node->mMaterial = (cdWorldShaderMat*)iMat;
                node->hasColorVerts = false;
            } else {
                node = NULL;
            }
            node->Clip = ClipResult;
            node->SortHash = gCDWorldShader->ID;
            node->Next = nglBuildScene->OpaqueRenderList;
            nglBuildScene->OpaqueRenderList = node;
            ++nglBuildScene->OpaqueListCount;
        }
    }
}
