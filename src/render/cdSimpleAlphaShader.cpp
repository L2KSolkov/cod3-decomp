// ============================================================================
// cdSimpleAlphaShader.cpp — simple alpha shader (6 non-inline funcs).
// Source: source/cdSimpleAlphaShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleAlphaShader.o):
//   cdSimpleAlphaShaderMat::ctor @0x7C7E10
//   InitCDSimpleAlphaShader  @0x7C7EA0
//   ToggleCDSimpleAlphaShader @0x7C7EF0
//   cdSimpleAlphaShader::Register @0x7C7F10
//   cdSimpleAlphaShader::AddNode @0x7C7F30
// ============================================================================
#include "cdSimpleAlphaShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdSimpleAlphaShader* gCDSimpleAlphaShader = nullptr;  // ?gCDSimpleAlphaShader@@3PAVcdSimpleAlphaShader@@A

// ea: 0x007C8DB0
unsigned int cdSimpleAlphaRender::GetVShader(unsigned int index) {
    return static_cast<unsigned int>(cdSimpleAlphaRender::VS[index]);
}

// ea: 0x007C8DF0
unsigned long* cdSimpleAlphaPixel::GetPShader(unsigned int index) {
    return cdSimpleAlphaPixel::PS[index];
}

// ea: 0x007C8E30
unsigned long* cdSimpleAlphaPixel_Fullbright::GetPShader(unsigned int index) {
    return cdSimpleAlphaPixel_Fullbright::PS[index];
}

// ea: 0x007C8FD0
cdSimpleAlphaRender::Params::Params() {}

// ea: 0x007C8F00
void cdSimpleAlphaRender::SetConstants(const cdSimpleAlphaRender::Params& Params) {
    D3DDevice_SetVertexShaderConstantNotInlineFast(8, &Params, 0x34u);
}

// ea: 0x007C8D80
void cdSimpleAlphaRender::RegisterVShader()
{
    for (int index = 0; index != 2; ++index) {
        nglDxRegisterVShader(&cdSimpleAlphaRender::VS[index],
                             cdSimpleAlphaRender::VShaderTable[index]);
    }
}

// ea: 0x007C8DC0
void cdSimpleAlphaPixel::RegisterPShader()
{
    int v0 = 0;
    for (int i = 2; i != 0; --i, ++v0)
        nglDxRegisterPShader(reinterpret_cast<unsigned long**>(&PS[v0]),
                             PShaderTable[v0]);
}

// ea: 0x007C8E00
void cdSimpleAlphaPixel_Fullbright::RegisterPShader()
{
    int v0 = 0;
    for (int i = 2; i != 0; --i, ++v0)
        nglDxRegisterPShader(reinterpret_cast<unsigned long**>(&PS[v0]),
                             PShaderTable[v0]);
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
// cdSimpleAlphaShaderMat::cdSimpleAlphaShaderMat — bind the texture + shader.
// ea: 0x7C7E10
// ============================================================================
cdSimpleAlphaShaderMat::cdSimpleAlphaShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdSimpleAlphaShader* v3 = gCDSimpleAlphaShader;
    if (gCDSimpleAlphaShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdSimpleAlphaShader.cpp";
    AeAssert::gCurrentLine = 79;
    AeAssert::gCurrentExpr = "gCDSimpleAlphaShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDSimpleAlphaShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDSimpleAlphaShader;
}

// ea: 0x007C8D30
cdSimpleAlphaShader::cdSimpleAlphaShader() {
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    ShaderCommon::ShaderSwitching.__s0[1] &= ~0x80;
}

// ea: 0x007C8FC0
cdSimpleAlphaShader::~cdSimpleAlphaShader() = default;

// ============================================================================
// InitCDSimpleAlphaShader — allocate the shader and link into the init list.
// ea: 0x7C7EA0
// ============================================================================
void InitCDSimpleAlphaShader() {
    cdSimpleAlphaShader* result = (cdSimpleAlphaShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdSimpleAlphaShader;
        gCDSimpleAlphaShader = result;
    } else {
        gCDSimpleAlphaShader = NULL;
    }
}

// ============================================================================
// ToggleCDSimpleAlphaShader — toggle the alpha-shader enable (high bit).
// ea: 0x7C7EF0
// ============================================================================
void ToggleCDSimpleAlphaShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    char result = (char)~byte;
    ShaderCommon::ShaderSwitching.__s0[1] =
        (unsigned char)(~byte ^ ((byte ^ ~byte) & 0x7F));
}

// ea: 0x007C8D60
tlFixedString cdSimpleAlphaShader::GetName() { return tlFixedString("cdSimpleAlpha"); }

// ============================================================================
// cdSimpleAlphaShader::Register — register the alpha vertex/pixel shaders.
// ea: 0x7C7F10
// ============================================================================
void cdSimpleAlphaShader::Register() {
    nglShader::Register();
    cdSimpleAlphaRender::RegisterVShader();
    cdSimpleAlphaPixel::RegisterPShader();
    cdSimpleAlphaPixel_Fullbright::RegisterPShader();
}

// ============================================================================
// cdSimpleAlphaShader::AddNode — add an alpha node to the transparent list.
// ea: 0x7C7F30
// ============================================================================
void cdSimpleAlphaShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                  nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 0x80) == 0) {
        cdSimpleAlphaShaderNode* node = (cdSimpleAlphaShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdSimpleAlphaShaderNode
            node->mMaterial = (cdSimpleAlphaShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortDist = node->GetDist(nglBuildScene->WorldToView);
        node->Next = nglBuildScene->TransRenderList;
        nglBuildScene->TransRenderList = node;
        ++nglBuildScene->TransListCount;
    }
}

// ea: 0x007C8F20
cdSimpleAlphaShaderNode::cdSimpleAlphaShaderNode(
    nglMeshNode* iMeshNode, nglMeshSection* iSection,
    cdSimpleAlphaShaderMat* iMaterial) {
    this->MeshNode = iMeshNode;
    this->Section = iSection;
    this->mMaterial = iMaterial;
}

// ea: 0x007C8F80
cdSimpleAlphaShaderNode::~cdSimpleAlphaShaderNode() = default;
