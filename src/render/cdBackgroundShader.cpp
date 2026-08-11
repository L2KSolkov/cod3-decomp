// ============================================================================
// cdBackgroundShader.cpp — background shader (6 non-inline funcs).
// Source: source/cdBackgroundShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdBackgroundShader.o):
//   cdBackgroundShaderMat::ctor @0x7E0520
//   InitCDBackgroundShader  @0x7E05B0
//   ToggleCDBackgroundShader @0x7E0600
//   cdBackgroundShader::Register @0x7E0620
//   cdBackgroundShader::AddNode @0x7E0680
// ============================================================================
#include "cdBackgroundShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdBackgroundShader* gCDBackgroundShader = nullptr;  // ?gCDBackgroundShader@@3PAVcdBackgroundShader@@A
cdBackgroundShader* g_cdBackgroundShader = nullptr;  // ?g_cdBackgroundShader@@3PAVcdBackgroundShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdBackgroundRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
    unsigned long Shader = 0;
}
namespace cdBackgroundPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
    unsigned long* Shader = nullptr;
}
namespace cdBackgroundFullbrightPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
    unsigned long* Shader = nullptr;
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
// cdBackgroundShaderMat::cdBackgroundShaderMat — bind texture + shader.
// ea: 0x7E0520
// ============================================================================
cdBackgroundShaderMat::cdBackgroundShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdBackgroundShader* v3 = g_cdBackgroundShader;
    if (g_cdBackgroundShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdBackgroundShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "g_cdBackgroundShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = g_cdBackgroundShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = g_cdBackgroundShader;
}

// ============================================================================
// InitCDBackgroundShader — allocate the shader and link into the init list.
// ea: 0x7E05B0
// ============================================================================
void InitCDBackgroundShader() {
    cdBackgroundShader* result = (cdBackgroundShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdBackgroundShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~0x40;
        gCDBackgroundShader = result;
    } else {
        gCDBackgroundShader = NULL;

    }

}

// ============================================================================
// ToggleCDBackgroundShader — toggle background-shader enable bit (bit 6).
// ea: 0x7E0600
// ============================================================================
void ToggleCDBackgroundShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (~(byte >> 6) << 6)) & 0x40) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;

}

// ============================================================================
// cdBackgroundShader::Register — register the background shaders.
// ea: 0x7E0620
// ============================================================================
void cdBackgroundShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader((unsigned int*)cdBackgroundRender::VS, cdBackgroundRender::VShaderTable[0]);
    cdBackgroundRender::Shader = cdBackgroundRender::VS[0];
    nglDxRegisterPShader((unsigned int**)cdBackgroundPixel::PS, cdBackgroundPixel::PShaderTable[0]);
    cdBackgroundPixel::Shader = cdBackgroundPixel::PS[0];
    nglDxRegisterPShader((unsigned int**)cdBackgroundFullbrightPixel::PS, cdBackgroundFullbrightPixel::PShaderTable[0]);
    cdBackgroundFullbrightPixel::Shader = cdBackgroundFullbrightPixel::PS[0];
}

// ============================================================================
// cdBackgroundShader::AddNode — add a background node to the opaque list.
// ea: 0x7E0680
// ============================================================================
void cdBackgroundShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                 nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 0x40) == 0) {
        cdBackgroundShaderNode* node = (cdBackgroundShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdBackgroundShaderNode
            node->mMaterial = (cdBackgroundShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDBackgroundShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
