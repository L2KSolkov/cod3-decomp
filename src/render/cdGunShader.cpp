// ============================================================================
// cdGunShader.cpp — gun shader (6 non-inline funcs).
// Source: source/cdGunShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGunShader.o):
//   cdGunShaderMat::ctor @0x7CEA70
//   InitCDGunShader  @0x7CEB00
//   ToggleCDGunShader @0x7CEB50
//   cdGunShader::Register @0x7CEB70
//   cdGunShader::AddNode @0x7CEBD0
// ============================================================================
#include "cdGunShader.h"

extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);

#include <intrin.h>

// Shader global pointer definitions
cdGunShader* gCDGunShader = nullptr;  // ?gCDGunShader@@3PAVcdGunShader@@A

// ea: 0x007CEA70
cdGunShaderMat::cdGunShaderMat(nglTexture* iTexture)
{
    this->mTexture = iTexture;
    cdGunShader* shader = gCDGunShader;
    if (shader != nullptr) {
        this->Shader = shader;
        return;
    }
    AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(0);
    AeAssert::gCurrentFile = "cdGunShader.cpp";
    AeAssert::gCurrentLine = 14;
    AeAssert::gCurrentExpr = "gCDGunShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDGunShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
        __debugbreak();
        this->Shader = gCDGunShader;
        return;
    }
    this->Shader = gCDGunShader;
}

// ea: 0x007CF020
unsigned long cdGunRender::GetVShader()
{
    return static_cast<unsigned int>(cdGunRender::VS[0]);
}

// ea: 0x007CF030
void cdGunPixel::RegisterShader()
{
    nglDxRegisterPShader(cdGunPixel::PS, cdGunPixel::PShaderTable[0]);
    cdGunPixel::Shader = cdGunPixel::PS[0];
}

// ea: 0x007CF050
void cdGunPixel::RegisterPShader()
{
    cdGunPixel::RegisterShader();
}

// ea: 0x007CF070
unsigned long* cdGunPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(cdGunPixel::PS[0]);
}

// ea: 0x007CF080
void cdGunFullbrightPixel::RegisterShader()
{
    nglDxRegisterPShader(cdGunFullbrightPixel::PS,
                         cdGunFullbrightPixel::PShaderTable[0]);
    cdGunFullbrightPixel::Shader = cdGunFullbrightPixel::PS[0];
}

// ea: 0x007CF0A0
void cdGunFullbrightPixel::RegisterPShader()
{
    cdGunFullbrightPixel::RegisterShader();
}

// ea: 0x007CF0C0
unsigned long* cdGunFullbrightPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(cdGunFullbrightPixel::PS[0]);
}

// ea: 0x007CEF90
cdGunShader::cdGunShader() {
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    ShaderCommon::ShaderSwitching.__s0[3] &= ~8;
}

// ea: 0x007CF170
cdGunShader::~cdGunShader() = default;

// Shader static data definitions are restored in cdGunShaderData.cpp.

// ea: 0x007CF000
void cdGunRender::RegisterVShader()
{
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(cdGunRender::VS),
                         reinterpret_cast<const unsigned int*>(cdGunRender::VShaderTable[0]));
    cdGunRender::Shader = cdGunRender::VS[0];
}
// ============================================================================
// InitCDGunShader — allocate the shader and link into the init list.
// ea: 0x7CEB00
// ============================================================================
void InitCDGunShader() {
    cdGunShader* result = (cdGunShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdGunShader;
        gCDGunShader = result;
    } else {
        gCDGunShader = NULL;

    }

}

// ============================================================================
// ToggleCDGunShader — toggle the gun-shader enable bit (bit 3).
// ea: 0x7CEB50
// ============================================================================
void ToggleCDGunShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (8 * ~(byte >> 3))) & 8) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;

}

// ea: 0x007CEFC0
tlFixedString cdGunShader::GetName() { return tlFixedString("cdGun"); }

// ============================================================================
// cdGunShader::Register — register the gun vertex/pixel shaders.
// ea: 0x7CEB70
// ============================================================================
void cdGunShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(cdGunRender::VS),
                         cdGunRender::VShaderTable[0]);
    cdGunRender::Shader = cdGunRender::VS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(cdGunPixel::PS),
                         cdGunPixel::PShaderTable[0]);
    cdGunPixel::Shader = cdGunPixel::PS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(cdGunFullbrightPixel::PS),
                         cdGunFullbrightPixel::PShaderTable[0]);
    cdGunFullbrightPixel::Shader = cdGunFullbrightPixel::PS[0];
}

// ============================================================================
// cdGunShader::AddNode — add a gun node to the opaque list.
// ea: 0x7CEBD0
// ============================================================================
void cdGunShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                          nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 8) == 0) {
        cdGunShaderNode* node = (cdGunShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdGunShaderNode
            node->mMaterial = (cdGunShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDGunShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ea: 0x007CF0D0
cdGunShaderNode::cdGunShaderNode(
    nglMeshNode* iMeshNode, nglMeshSection* iSection,
    cdGunShaderMat* iMaterial) {
    this->MeshNode = iMeshNode;
    this->Section = iSection;
    this->mMaterial = iMaterial;
}

// ea: 0x007CF130
cdGunShaderNode::~cdGunShaderNode() = default;
