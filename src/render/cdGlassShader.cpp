// ============================================================================
// cdGlassShader.cpp — glass shader (4 non-inline funcs).
// Source: source/cdGlassShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGlassShader.o):
//   InitCDGlassShader  @0x7CFF90
//   ToggleCDGlassShader @0x7CFFE0
//   cdGlassShader::Register @0x7D0000
//   cdGlassShader::AddNode @0x7D0970 (inline COMDAT)
// ============================================================================
#include "cdGlassShader.h"

#include <intrin.h>

// ============================================================================
// InitCDGlassShader — allocate the shader and link into the init list.
// ea: 0x7CFF90
// ============================================================================
void InitCDGlassShader() {
    cdGlassShader* result = (cdGlassShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdGlassShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~0x40;
        gCDGlassShader = result;
    } else {
        gCDGlassShader = NULL;
    }
}

// ============================================================================
// ToggleCDGlassShader — toggle glass-shader enable bit (bit 6).
// ea: 0x7CFFE0
// ============================================================================
void ToggleCDGlassShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (~(byte >> 6) << 6)) & 0x40) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;
}

// ============================================================================
// cdGlassShader::Register — register the glass vertex/pixel shaders.
// ea: 0x7D0000
// ============================================================================
void cdGlassShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 4; i != 0; --i, ++v0) {
        nglDxRegisterVShader(&cdGlassRender::VS[0][v0], cdGlassRender::VShaderTable[0][v0]);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShader(&cdGlassPixel::PS[v0], cdGlassPixel::PShaderTable[v0]);
    }
    nglDxRegisterPShader(cdGlassSolidColorPixel::PS, cdGlassSolidColorPixel::PShaderTable[0]);
}

// ============================================================================
// cdGlassShader::AddNode — add a glass node to the transparent list.
// ea: 0x7D0970 (inline COMDAT)
// ============================================================================
void cdGlassShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if (nglBuildScene->RenderTarget != gProjShadowTex &&
        (ShaderCommon::ShaderSwitching.__s0[3] & 0x40) == 0) {
        cdGlassShaderNode* node = (cdGlassShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdGlassShaderNode
            node->mMaterial = (cdGlassShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortDist = node->GetDist(nglBuildScene->WorldToView);
        node->Next = nglBuildScene->TransRenderList;
        nglBuildScene->TransRenderList = node;
        ++nglBuildScene->TransListCount;
    }
}
