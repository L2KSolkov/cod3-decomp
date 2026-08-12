// ============================================================================
// cdWaterShader.cpp — water shader (6 non-inline funcs).
// Source: source/cdWaterShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWaterShader.o):
//   cdWaterShaderMat::ctor @0x7D95E0
//   InitCDWaterShader  @0x7D9670
//   ToggleCDWaterShader @0x7D96C0
//   cdWaterShader::Register @0x7D96E0
//   cdWaterShader::AddNode @0x7D9720
// ============================================================================
#include "cdWaterShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdWaterShader* gCDWaterShader = nullptr;  // ?gCDWaterShader@@3PAVcdWaterShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdWaterRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
}
namespace cdWaterPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
}
namespace cdWaterFullbrightPixel {
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
// cdWaterShaderMat::cdWaterShaderMat — bind textures + shader.
// ea: 0x7D95E0
// ============================================================================
cdWaterShaderMat::cdWaterShaderMat(nglTexture* iTexture) {
    this->mDiffuse = iTexture;
    this->mLightmap = NULL;
    cdWaterShader* v3 = gCDWaterShader;
    if (gCDWaterShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdWaterShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "gCDWaterShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDWaterShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDWaterShader;
}

// ============================================================================
// InitCDWaterShader — allocate the shader and link into the init list.
// ea: 0x7D9670
// ============================================================================
void InitCDWaterShader() {
    cdWaterShader* result = (cdWaterShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWaterShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~0x80;
        gCDWaterShader = result;
    } else {
        gCDWaterShader = NULL;
    }
}

// ============================================================================
// ToggleCDWaterShader — toggle water-shader enable (high bit).
// ea: 0x7D96C0
// ============================================================================
void ToggleCDWaterShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    ShaderCommon::ShaderSwitching.__s0[0] =
        (unsigned char)(~byte ^ ((byte ^ ~byte) & 0x7F));
}

// ============================================================================
// cdWaterShader::Register — register the water vertex/pixel shaders.
// ea: 0x7D96E0
// ============================================================================
void cdWaterShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdWaterRender::VS, cdWaterRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdWaterPixel::PS, cdWaterPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdWaterFullbrightPixel::PS, cdWaterFullbrightPixel::PShaderTable, 0);
}

// ============================================================================
// cdWaterShader::AddNode — add a water node to the opaque list.
// ea: 0x7D9720
// ============================================================================
void cdWaterShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 0x80) == 0) {
        cdWaterShaderNode* node = (cdWaterShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdWaterShaderNode
            node->mMaterial = (cdWaterShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = -1;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
