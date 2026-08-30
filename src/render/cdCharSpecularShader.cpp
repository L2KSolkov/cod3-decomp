// ============================================================================
// cdCharSpecularShader.cpp — char specular shader (5 non-inline funcs).
// Source: source/cdCharSpecularShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdCharSpecularShader.o):
//   InitCDCharSpecularShader  @0x7D2130
//   ToggleCDCharSpecularShader @0x7D2180
//   cdCharSpecularShader::Register @0x7D21A0
//   cdCharSpecularShader::AddNode @0x7D21F0
// ============================================================================
#include "cdCharSpecularShader.h"

extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);

#include <intrin.h>

// Shader global pointer definitions
cdCharSpecularShader* gcdCharSpecularShader = nullptr;  // ?gcdCharSpecularShader@@3PAVcdCharSpecularShader@@A

// ea: 0x007D26F0
void cdCharSpecularShaderRender::RegisterShader()
{
    for (int index = 0; index != 2; ++index) {
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(&cdCharSpecularShaderRender::VS[index]),
                             cdCharSpecularShaderRender::VShaderTable[index]);
    }
}

// ea: 0x007D2720
void cdCharSpecularShaderRender::RegisterVShader()
{
    cdCharSpecularShaderRender::RegisterShader();
}

// ea: 0x007D2730
unsigned long cdCharSpecularShaderRender::GetVShader(unsigned int index)
{
    return static_cast<unsigned int>(cdCharSpecularShaderRender::VS[index]);
}

// ea: 0x007D2740
void cdCharSpecularPixel::RegisterShader()
{
    nglDxRegisterPShader(cdCharSpecularPixel::PS,
                         cdCharSpecularPixel::PShaderTable[0]);
    cdCharSpecularPixel::Shader = cdCharSpecularPixel::PS[0];
}

// ea: 0x007D2760
void cdCharSpecularPixel::RegisterPShader()
{
    cdCharSpecularPixel::RegisterShader();
}

// ea: 0x007D2780
unsigned long* cdCharSpecularPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(cdCharSpecularPixel::PS[0]);
}

// ea: 0x007D2790
void cdCharSpecularFullbrightPixel::RegisterShader()
{
    nglDxRegisterPShader(cdCharSpecularFullbrightPixel::PS,
                         cdCharSpecularFullbrightPixel::PShaderTable[0]);
    cdCharSpecularFullbrightPixel::Shader = cdCharSpecularFullbrightPixel::PS[0];
}

// ea: 0x007D27B0
void cdCharSpecularFullbrightPixel::RegisterPShader()
{
    cdCharSpecularFullbrightPixel::RegisterShader();
}

// ea: 0x007D27D0
unsigned long* cdCharSpecularFullbrightPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(cdCharSpecularFullbrightPixel::PS[0]);
}
// ============================================================================
// InitCDCharSpecularShader — allocate the shader and link into the init list.
// ea: 0x7D2130
// ============================================================================
// ea: 0x007D26A0
cdCharSpecularShader::cdCharSpecularShader() {
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    ShaderCommon::ShaderSwitching.__s0[2] &= ~2;
}

// ea: 0x007D2880
cdCharSpecularShader::~cdCharSpecularShader() = default;

void InitCDCharSpecularShader() {
    cdCharSpecularShader* result = (cdCharSpecularShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdCharSpecularShader;
        gcdCharSpecularShader = result;
    } else {
        gcdCharSpecularShader = NULL;
    }
}

// ============================================================================
// ToggleCDCharSpecularShader — toggle char-specular enable bit (bit 1).
// ea: 0x7D2180
// ============================================================================
void ToggleCDCharSpecularShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (2 * ~(byte >> 1))) & 2) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;
}

// ea: 0x007D26D0
tlFixedString cdCharSpecularShader::GetName() { return tlFixedString("cdCharSpecular"); }

// ============================================================================
// cdCharSpecularShader::Register — register the char-specular shaders.
// ea: 0x7D21A0
// ============================================================================
void cdCharSpecularShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdCharSpecularShaderRender::VS[v0], cdCharSpecularShaderRender::VShaderTable, v0);
    }
    nglDxRegisterPShaderSafe((unsigned int**)cdCharSpecularPixel::PS, cdCharSpecularPixel::PShaderTable, 0);
    cdCharSpecularPixel::Shader = cdCharSpecularPixel::PS != nullptr ? cdCharSpecularPixel::PS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdCharSpecularFullbrightPixel::PS, cdCharSpecularFullbrightPixel::PShaderTable, 0);
    cdCharSpecularFullbrightPixel::Shader = cdCharSpecularFullbrightPixel::PS != nullptr ? cdCharSpecularFullbrightPixel::PS[0] : 0;
}

// ============================================================================
// cdCharSpecularShader::AddNode — add a char-specular node to the opaque list.
// ea: 0x7D21F0
// ============================================================================
void cdCharSpecularShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                   nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 4) == 0) {
        cdCharSpecularShaderNode* node = (cdCharSpecularShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdCharSpecularShaderNode
            node->mMaterial = (cdCharSpecularShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gcdCharSpecularShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ea: 0x007D27E0
cdCharSpecularShaderNode::cdCharSpecularShaderNode(
    nglMeshNode* iMeshNode, nglMeshSection* iSection,
    cdCharSpecularShaderMat* iMaterial) {
    this->MeshNode = iMeshNode;
    this->Section = iSection;
    this->mMaterial = iMaterial;
}

// ea: 0x007D2840
cdCharSpecularShaderNode::~cdCharSpecularShaderNode() = default;

// ea: 0x007D2890
CharSpecularContext::CharSpecularContext() {}
