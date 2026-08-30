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

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdGunRender {
    static const unsigned int VShaderMicrocode[81] = {
        0x00142078, 0x00000000, 0x0062601a, 0x08001468, 0xfeb00000,
        0x00000000, 0x00a1421a, 0x18355800, 0x28000000, 0x00000000,
        0x02a0041a, 0xb4356854, 0xa8b0c84c, 0x00000000, 0x00a1621a,
        0x18357800, 0x24000000, 0x00000000, 0x08a1821a, 0x18359802,
        0xd2080000, 0x00000000, 0x00a1a21a, 0x1835b800, 0x21000000,
        0x00000000, 0x00824000, 0x14016d54, 0xb8b00000, 0x00000000,
        0x0140201b, 0x04003800, 0x2f000000, 0x00000000, 0x00e1201b,
        0x08373800, 0x20a01800, 0x00000000, 0x00424000, 0xb5545800,
        0x28b00000, 0x00000000, 0x06e0c01b, 0x0836dbff, 0x10b88800,
        0x00000000, 0x00e0e01b, 0x0836f800, 0x20a04800, 0x00000000,
        0x00e1001b, 0x08371800, 0x20a02800, 0x00000000, 0x00824000,
        0xb5fe5800, 0xb8b00000, 0x00000000, 0x00e1c01b, 0x0437d800,
        0x28200000, 0x00000000, 0x00e1e01b, 0x0437f800, 0x24200000,
        0x00000000, 0x00e2001b, 0x04361800, 0x22200000, 0x00000000,
        0x006020aa, 0x1c001402, 0xd0b0f828, 0x00000000, 0x0062201a,
        0x24001068, 0x70b0e818, 0x00000000, 0x0040001a, 0xc4002800,
        0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}

// ea: 0x007CF000
void cdGunRender::RegisterVShader()
{
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(cdGunRender::VS),
                         reinterpret_cast<const unsigned int*>(cdGunRender::VShaderTable[0]));
    cdGunRender::Shader = cdGunRender::VS[0];
}
namespace cdGunPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8d41010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000001,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = nullptr;
}
namespace cdGunFullbrightPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8200000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000001,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = nullptr;
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
