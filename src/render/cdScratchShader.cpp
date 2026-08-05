// ============================================================================
// cdScratchShader.cpp — scratch shader (6 non-inline funcs).
// Source: source/cdScratchShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdScratchShader.o):
//   cdScratchMaterial::ctor @0x7C5660
//   ToggleCDScratchShader @0x7C5690
//   cdScratchShader::Register @0x7C56B0
//   InitCDScratchShader  @0x7C56E0
//   cdScratchShader::AddNode @0x7C5720
// ============================================================================
#include "cdScratchShader.h"

#include <intrin.h>

// ============================================================================
// cdScratchMaterial::cdScratchMaterial — bind texture + blend + shader.
// ea: 0x7C5660
// ============================================================================
cdScratchMaterial::cdScratchMaterial(nglTexture* tex, unsigned int BlendMode,
                                     int mapflags, bool HeatHaze) {
    this->Texture = tex;
    this->BlendMode = BlendMode;
    this->MapFlags = mapflags;
    this->HeatHaze = HeatHaze;
    this->Shader = gCDScratchShader;
}

// ============================================================================
// ToggleCDScratchShader — toggle the scratch-shader enable (high bit).
// ea: 0x7C5690
// ============================================================================
void ToggleCDScratchShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    ShaderCommon::ShaderSwitching.__s0[2] =
        (unsigned char)(~byte ^ ((byte ^ ~byte) & 0x7F));
}

// ============================================================================
// cdScratchShader::Register — register the scratch vertex/pixel shaders.
// ea: 0x7C56B0
// ============================================================================
void cdScratchShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader(cdScratchShaderVertex::VS, cdScratchShaderVertex::VShaderTable[0]);
    nglDxRegisterPShader(cdScratchShaderPixel::PS, cdScratchShaderPixel::PShaderTable[0]);
}

// ============================================================================
// InitCDScratchShader — allocate the shader and link into the init list.
// ea: 0x7C56E0
// ============================================================================
void InitCDScratchShader() {
    cdScratchShader* result = (cdScratchShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdScratchShader
        gCDScratchShader = result;
    } else {
        gCDScratchShader = NULL;
    }
}

// ============================================================================
// cdScratchShader::AddNode — add a scratch node to opaque/transparent list.
// ea: 0x7C5720
// ============================================================================
void cdScratchShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                              nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.as_u32 & 0x800000) == 0) {
        cdScratchShaderNode* node = (cdScratchShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdScratchShaderNode
            node->Material = (cdScratchMaterial*)iMat;
        } else {
            node = NULL;
        }
        nglShader* Shader = (nglShader*)((unsigned int*)iMat)[5];
        if (Shader == NULL || Shader == (nglShader*)0x10000) {
            node->SortHash = gCDScratchShader->ID;
            node->Next = nglBuildScene->OpaqueRenderList;
            nglBuildScene->OpaqueRenderList = node;
            ++nglBuildScene->OpaqueListCount;
        } else {
            node->SortDist = node->GetDist(nglBuildScene->WorldToView);
            node->Next = nglBuildScene->TransRenderList;
            nglBuildScene->TransRenderList = node;
            ++nglBuildScene->TransListCount;
        }
    }
}
