// ============================================================================
// cdGunSightSpecularShader.cpp — gun sight specular shader (6 non-inline funcs).
// Source: source/cdGunSightSpecularShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGunSightSpecularShader.o):
//   cdGunSightSpecularShaderMat::ctor @0x7CD770
//   InitCDGunSightSpecularShader  @0x7CD800
//   ToggleCDGunSightSpecularShader @0x7CD850
//   cdGunSightSpecularShader::Register @0x7CD870
//   cdGunSightSpecularShader::AddNode @0x7CD8A0
// ============================================================================
#include "cdGunSightSpecularShader.h"

#include <intrin.h>

namespace AeAssert {
    extern int   gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int   gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
}

// ============================================================================
// cdGunSightSpecularShaderMat::ctor — bind textures + shader.
// ea: 0x7CD770
// ============================================================================
cdGunSightSpecularShaderMat::cdGunSightSpecularShaderMat(nglTexture* iDiffuseTexture,
                                                         nglTexture* iSpecularTexture) {
    this->mDiffuseTexture = iDiffuseTexture;
    this->mSpecularTexture = iSpecularTexture;
    cdGunSightSpecularShader* v4 = gCDGunSightSpecularShader;
    if (gCDGunSightSpecularShader != NULL) {
        this->Shader = v4;
        return;
    }
    AeAssert::gCurrentAuthor = 0;
    AeAssert::gCurrentFile = "cdGunSightSpecularShader.cpp";
    AeAssert::gCurrentLine = 17;
    AeAssert::gCurrentExpr = "gCDGunSightSpecularShader";
    if (!AeAssert::IsIgnored()) {
        if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
            __debugbreak();
            this->Shader = gCDGunSightSpecularShader;
            return;
        }
        this->Shader = gCDGunSightSpecularShader;
        return;
    }
    this->Shader = gCDGunSightSpecularShader;
}

// ============================================================================
// InitCDGunSightSpecularShader — allocate the shader and link into the init list.
// ea: 0x7CD800
// ============================================================================
void InitCDGunSightSpecularShader() {
    cdGunSightSpecularShader* result = (cdGunSightSpecularShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdGunSightSpecularShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~0x20;
        gCDGunSightSpecularShader = result;
    } else {
        gCDGunSightSpecularShader = NULL;
    }
}

// ============================================================================
// ToggleCDGunSightSpecularShader — toggle gun-sight specular bit (bit 5).
// ea: 0x7CD850
// ============================================================================
void ToggleCDGunSightSpecularShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (32 * ~(byte >> 5))) & 0x20) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;
}

// ============================================================================
// cdGunSightSpecularShader::Register — register the shaders.
// ea: 0x7CD870
// ============================================================================
void cdGunSightSpecularShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShader(&cdGunSightSpecularRender::VS[v0], cdGunSightSpecularRender::VShaderTable[v0]);
    }
    nglDxRegisterPShader(cdGunSightSpecularPixel::PS, cdGunSightSpecularPixel::PShaderTable[0]);
    nglDxRegisterPShader(cdGunSightSpecularFullbrightPixel::PS, cdGunSightSpecularFullbrightPixel::PShaderTable[0]);
}

// ============================================================================
// cdGunSightSpecularShader::AddNode — add a node to the opaque list.
// ea: 0x7CD8A0
// ============================================================================
void cdGunSightSpecularShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                       nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 0x20) == 0) {
        cdGunSightSpecularShaderNode* node = (cdGunSightSpecularShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdGunSightSpecularShaderNode
            node->mMaterial = (cdGunSightSpecularShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDGunSightSpecularShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
