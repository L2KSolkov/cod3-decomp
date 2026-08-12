// ============================================================================
// cdOceanShader.cpp — ocean shader (12 non-inline funcs).
// Source: source/cdOceanShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdOceanShader.o):
//   cdOceanShaderMat::ctor @0x7D7180
//   InitCDOceanShader  @0x7D7200
//   ToggleCDOceanShader @0x7D7250
//   cdOceanShaderNode::GetVShader @0x7D7E50
//   cdOceanShaderNode::GetFullbrightPShader @0x7D7E60
//   cdOceanShaderNode::GetPShader @0x7D7E70
//   cdOceanShaderNode::GetVShaderFogConstantOffset @0x7D7E90
//   cdOceanShaderNode::GetVShaderParamsStartAddress @0x7D7EA0
//   cdOceanShader::Register @0x7D7EB0
//   cdOceanShader::AddNode @0x7D7F00
// ============================================================================
#include "cdOceanShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdOceanShader* gCDOceanShader = nullptr;  // ?gCDOceanShader@@3PAVcdOceanShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdOceanRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
    unsigned long Shader = 0;
}
namespace cdOceanPixel {
    unsigned long* PS[2][2][2] = {};
    unsigned int const* PShaderTable[2][2][2] = {};
}
namespace cdOceanPixel_Fullbright {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
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
// cdOceanShaderMat::cdOceanShaderMat — bind the layer textures + shader.
// ea: 0x7D7180
// ============================================================================
cdOceanShaderMat::cdOceanShaderMat(nglTexture* iTexture) {
    this->mLayers[0].mTexture = iTexture;
    this->mLayers[1].mTexture = NULL;
    this->mLayers[2].mTexture = NULL;
    this->mLightmapTexture = NULL;
    if (gCDOceanShader == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "cdOceanShader.cpp";
        AeAssert::gCurrentLine = 33;
        AeAssert::gCurrentExpr = "gCDOceanShader";
        if (!AeAssert::IsIgnored() &&
            AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
            __debugbreak();
        }
    }
    bool v4 = this->BinaryVersion < 2;
    this->Shader = gCDOceanShader;
    if (v4)
        this->mBankID = 0;
}

// ============================================================================
// InitCDOceanShader — allocate the shader, link the init list, init globals.
// ea: 0x7D7200
// ============================================================================
void InitCDOceanShader() {
    cdOceanShader* v1 = (cdOceanShader*)mem_heap_malloc(0x10);
    if (v1 != NULL) {
        v1->next = tlInitList::head;
        tlInitList::head = v1;
        v1->Disabled = false;
        // vftable = cdOceanShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~1;
        gCDOceanShader = v1;
    } else {
        gCDOceanShader = NULL;
    }
    cdOceanGlobals::Init();
}

// ============================================================================
// ToggleCDOceanShader — toggle ocean-shader enable bit (bit 0).
// ea: 0x7D7250
// ============================================================================
void ToggleCDOceanShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ ~byte) & 1) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;
}

// ============================================================================
// cdOceanShaderNode::GetVShader — return the ocean vertex shader.
// ea: 0x7D7E50
// ============================================================================
unsigned int cdOceanShaderNode::GetVShader() {
    return cdOceanRender::VS[0];
}

// ============================================================================
// cdOceanShaderNode::GetFullbrightPShader — return the fullbright PShader.
// ea: 0x7D7E60
// ============================================================================
unsigned long* cdOceanShaderNode::GetFullbrightPShader() {
    return cdOceanPixel_Fullbright::PS[0];
}

// ============================================================================
// cdOceanShaderNode::GetPShader — return the indexed PShader.
// ea: 0x7D7E70
// ============================================================================
unsigned long* cdOceanShaderNode::GetPShader(int l2, int l3, int lm) {
    return cdOceanPixel::PS[l2][l3][lm];
}

// ============================================================================
// cdOceanShaderNode::GetVShaderFogConstantOffset — return -90.
// ea: 0x7D7E90
// ============================================================================
int cdOceanShaderNode::GetVShaderFogConstantOffset() {
    return -90;
}

// ============================================================================
// cdOceanShaderNode::GetVShaderParamsStartAddress — return -88.
// ea: 0x7D7EA0
// ============================================================================
int cdOceanShaderNode::GetVShaderParamsStartAddress() {
    return -88;
}

// ============================================================================
// cdOceanShader::Register — register the ocean vertex/pixel shaders.
// ea: 0x7D7EB0
// ============================================================================
void cdOceanShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdOceanRender::VS, cdOceanRender::VShaderTable, 0);
    cdOceanRender::Shader = cdOceanRender::VS != nullptr ? cdOceanRender::VS[0] : 0;
    for (int v0 = 0, i = 8; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdOceanPixel::PS[0][0][v0], cdOceanPixel::PShaderTable[0][0], v0);
    }
    nglDxRegisterPShaderSafe((unsigned int**)cdOceanPixel_Fullbright::PS, cdOceanPixel_Fullbright::PShaderTable, 0);
    cdOceanPixel_Fullbright::Shader = cdOceanPixel_Fullbright::PS != nullptr ? cdOceanPixel_Fullbright::PS[0] : 0;
}

// ============================================================================
// cdOceanShader::AddNode — add an ocean node to the opaque list.
// ea: 0x7D7F00
// ============================================================================
void cdOceanShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 1) == 0) {
        cdOceanShaderNode* node = (cdOceanShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdOceanShaderNode
            node->mMaterial = (cdOceanShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = -2;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
