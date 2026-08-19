// ============================================================================
// cdDynamicDecalShader.cpp — dynamic decal shader (6 non-inline funcs).
// Source: source/cdDynamicDecalShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDynamicDecalShader.o):
//   cdDynamicDecalShaderMat::ctor @0x7CBD80
//   InitCDDynamicDecalShader  @0x7CBE10
//   ToggleCDDynamicDecalShader @0x7CBE60
//   cdDynamicDecalShader::Register @0x7CBE80
//   cdDynamicDecalShader::AddNode @0x7CBE90
// ============================================================================
#include "cdDynamicDecalShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdDynamicDecalShader* gCDDynamicDecalShader = nullptr;  // ?gCDDynamicDecalShader@@3PAVcdDynamicDecalShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdDynamicDecalRender {
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = {};
}
namespace cdDynamicDecalPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
}

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
// cdDynamicDecalShaderMat::cdDynamicDecalShaderMat — default material.
// ea: 0x7CBD80
// ============================================================================
cdDynamicDecalShaderMat::cdDynamicDecalShaderMat() {
    this->mTexture = NULL;
    this->mZbias = 0.0020000001f;
    this->mAlphaBlend = false;
    cdDynamicDecalShader* v2 = gCDDynamicDecalShader;
    if (gCDDynamicDecalShader != NULL) {
        this->Shader = v2;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdDynamicDecalShader.cpp";
    AeAssert::gCurrentLine = 18;
    AeAssert::gCurrentExpr = "gCDDynamicDecalShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDDynamicDecalShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
        __debugbreak();
        this->Shader = gCDDynamicDecalShader;
        return;
    }
    this->Shader = gCDDynamicDecalShader;
}

// ============================================================================
// InitCDDynamicDecalShader — allocate the shader and link into the init list.
// ea: 0x7CBE10
// ============================================================================
void InitCDDynamicDecalShader() {
    cdDynamicDecalShader* result = (cdDynamicDecalShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdDynamicDecalShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdDynamicDecalShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~2;
        gCDDynamicDecalShader = result;
    } else {
        gCDDynamicDecalShader = NULL;
    }
}

// ============================================================================
// ToggleCDDynamicDecalShader — toggle dynamic-decal enable bit (bit 2).
// ea: 0x7CBE60
// ============================================================================
void ToggleCDDynamicDecalShader() {
    unsigned char byte = *((unsigned char*)&gShaderSwitchingFlags);
    byte = (unsigned char)(((byte ^ (4 * ~(byte >> 2))) & 4) ^ byte);
    *((unsigned char*)&gShaderSwitchingFlags) = byte;
}

// ============================================================================
// cdDynamicDecalShader::Register — register the dynamic-decal shaders.
// ea: 0x7CBE80
// ============================================================================
tlFixedString cdDynamicDecalShader::GetName() { return tlFixedString("cdDynamicDecal"); }

void cdDynamicDecalShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdDynamicDecalRender::VS[v0], cdDynamicDecalRender::VShaderTable, v0);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdDynamicDecalPixel::PS[v0], cdDynamicDecalPixel::PShaderTable, v0);
    }
}

// ============================================================================
// cdDynamicDecalShader::AddNode — add a dynamic-decal node to the opaque list.
// ea: 0x7CBE90
// ============================================================================
void cdDynamicDecalShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                   nglMaterial* iMat) {
    if ((gShaderSwitchingFlags & 4) == 0) {
        cdDynamicDecalShaderNode* node = (cdDynamicDecalShaderNode*)nglListAlloc(0x1C, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdDynamicDecalShaderNode
            node->mMaterial = (cdDynamicDecalShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->Clip = 0;
        node->SortHash = gCDDynamicDecalShader->ID | 0x80000000;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
