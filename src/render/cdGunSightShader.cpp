// ============================================================================
// cdGunSightShader.cpp — gun sight shader (6 non-inline funcs).
// Source: source/cdGunSightShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGunSightShader.o):
//   cdGunSightShaderMat::ctor @0x7CE100
//   InitCDGunSightShader  @0x7CE190
//   ToggleCDGunSightShader @0x7CE1E0
//   cdGunSightShader::Register @0x7CE200
//   cdGunSightShader::AddNode @0x7CE240
// ============================================================================
#include "cdGunSightShader.h"

#include <intrin.h>

namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
    extern ECoderId gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int   gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
}

// ============================================================================
// cdGunSightShaderMat::cdGunSightShaderMat — bind texture + shader.
// ea: 0x7CE100
// ============================================================================
cdGunSightShaderMat::cdGunSightShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdGunSightShader* v3 = gCDGunSightShader;
    if (gCDGunSightShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdGunSightShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "gCDGunSightShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDGunSightShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDGunSightShader;
}

// ============================================================================
// InitCDGunSightShader — allocate the shader and link into the init list.
// ea: 0x7CE190
// ============================================================================
void InitCDGunSightShader() {
    cdGunSightShader* result = (cdGunSightShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdGunSightShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~0x10;
        gCDGunSightShader = result;
    } else {
        gCDGunSightShader = NULL;
    }
}

// ============================================================================
// ToggleCDGunSightShader — toggle gun-sight enable bit (bit 4).
// ea: 0x7CE1E0
// ============================================================================
void ToggleCDGunSightShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (16 * ~(byte >> 4))) & 0x10) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;
}

// ============================================================================
// cdGunSightShader::Register — register the gun-sight vertex/pixel shaders.
// ea: 0x7CE200
// ============================================================================
void cdGunSightShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader((unsigned int*)cdGunSightRender::VS, cdGunSightRender::VShaderTable[0]);
    nglDxRegisterPShader((unsigned int**)cdGunSightPixel::PS, cdGunSightPixel::PShaderTable[0]);
    nglDxRegisterPShader((unsigned int**)cdGunSightFullbrightPixel::PS, cdGunSightFullbrightPixel::PShaderTable[0]);
}

// ============================================================================
// cdGunSightShader::AddNode — add a gun-sight node to the opaque list.
// ea: 0x7CE240
// ============================================================================
void cdGunSightShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                               nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 0x10) == 0) {
        cdGunSightShaderNode* node = (cdGunSightShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdGunSightShaderNode
            node->mMaterial = (cdGunSightShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDGunSightShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
