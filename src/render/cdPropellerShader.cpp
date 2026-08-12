// ============================================================================
// cdPropellerShader.cpp — propeller shader (6 non-inline funcs).
// Source: source/cdPropellerShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdPropellerShader.o):
//   cdPropellerShaderMat::ctor @0x7D0D50
//   InitCDPropellerShader  @0x7D0DE0
//   ToggleCDPropellerShader @0x7D0E30
//   cdPropellerShader::Register @0x7D0E50
//   cdPropellerShader::AddNode @0x7D0E90
// ============================================================================
#include "cdPropellerShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdPropellerShader* gCDPropellerShader = nullptr;  // ?gCDPropellerShader@@3PAVcdPropellerShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdPropellerRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
}
namespace cdPropellerPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
}
namespace cdPropellerFullbrightPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
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
// cdPropellerShaderMat::cdPropellerShaderMat — bind texture + shader.
// ea: 0x7D0D50
// ============================================================================
cdPropellerShaderMat::cdPropellerShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdPropellerShader* v3 = gCDPropellerShader;
    if (gCDPropellerShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdPropellerShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "gCDPropellerShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDPropellerShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDPropellerShader;
}

// ============================================================================
// InitCDPropellerShader — allocate the shader and link into the init list.
// ea: 0x7D0DE0
// ============================================================================
void InitCDPropellerShader() {
    cdPropellerShader* result = (cdPropellerShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdPropellerShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~1;
        gCDPropellerShader = result;
    } else {
        gCDPropellerShader = NULL;
    }
}

// ============================================================================
// ToggleCDPropellerShader — toggle propeller-shader enable bit (bit 0).
// ea: 0x7D0E30
// ============================================================================
void ToggleCDPropellerShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ ~byte) & 1) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;
}

// ============================================================================
// cdPropellerShader::Register — register the propeller vertex/pixel shaders.
// ea: 0x7D0E50
// ============================================================================
void cdPropellerShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdPropellerRender::VS, cdPropellerRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdPropellerPixel::PS, cdPropellerPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdPropellerFullbrightPixel::PS, cdPropellerFullbrightPixel::PShaderTable, 0);
}

// ============================================================================
// cdPropellerShader::AddNode — add a propeller node to the transparent list.
// ea: 0x7D0E90
// ============================================================================
void cdPropellerShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 1) == 0) {
        cdPropellerShaderNode* node = (cdPropellerShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdPropellerShaderNode
            node->mMaterial = (cdPropellerShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortDist = node->GetDist(nglBuildScene->WorldToView);
        node->Next = nglBuildScene->TransRenderList;
        nglBuildScene->TransRenderList = node;
        ++nglBuildScene->TransListCount;
    }
}
