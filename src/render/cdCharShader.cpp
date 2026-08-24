// ============================================================================
// cdCharShader.cpp — character shader (5 non-inline funcs).
// Source: source/cdCharShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdCharShader.o):
//   InitCDCharShader  @0x7D2B70
//   ToggleCDCharShader @0x7D2BC0
//   cdCharShader::Register @0x7D2BE0
//   cdCharShader::AddNode @0x7D2C40
// ============================================================================
#include "cdCharShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdCharShader* gCDCharShader = nullptr;  // ?gCDCharShader@@3PAVcdCharShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdCharShaderRender {
    static const unsigned int VShaderMicrocode[185] = {
        0x002e2078, 0x00000000, 0x0080061b, 0x38001954, 0x3f000000,
        0x00000000, 0x0062601a, 0x08001468, 0xfeb00000, 0x00000000,
        0x03a00400, 0x0400106c, 0xa000f84c, 0x00000000, 0x002020aa,
        0x1c001000, 0x22400000, 0x00000000, 0x004c0800, 0x48361800,
        0x2fa00002, 0x00000000, 0x004c2800, 0x48363800, 0x2f200002,
        0x00000000, 0x004c4800, 0x48365800, 0x2f300002, 0x00000000,
        0x01a00055, 0x04001000, 0x20000000, 0x00000000, 0x00a0001a,
        0xb4356800, 0x28b00000, 0x00000000, 0x008c0855, 0x4836186e,
        0x9fa00002, 0x00000000, 0x008c2855, 0x4836386c, 0x9f200002,
        0x00000000, 0x008c4855, 0x4836586c, 0xdf300002, 0x00000000,
        0x09a000aa, 0x04001002, 0xd0080000, 0x00000000, 0x008c08aa,
        0x4836186e, 0x9fa00002, 0x00000000, 0x008c28aa, 0x4836386c,
        0x9f200002, 0x00000000, 0x008c48aa, 0x4836586c, 0xdf300002,
        0x00000000, 0x01a000ff, 0x04001000, 0x20000000, 0x00000000,
        0x00824000, 0x14016d54, 0xb8000000, 0x00000000, 0x008c08ff,
        0x4836186e, 0x9fb00002, 0x00000000, 0x008c28ff, 0x4836386c,
        0x9f200002, 0x00000000, 0x008c48ff, 0x4836586c, 0xdf300002,
        0x00000000, 0x00a0021a, 0x18356800, 0x28a00000, 0x00000000,
        0x00a0021a, 0x18344800, 0x24a00000, 0x00000000, 0x00a0021a,
        0x18346800, 0x22a00000, 0x00000000, 0x00e0001b, 0x08376800,
        0x28400000, 0x00000000, 0x00a0001a, 0xa4354800, 0x21500000,
        0x00000000, 0x00e0001b, 0x08364800, 0x24400000, 0x00000000,
        0x08e0001b, 0x08366bfd, 0x51410000, 0x00000000, 0x00424000,
        0x05545800, 0x28000000, 0x00000000, 0x0040001a, 0xa5fe2800,
        0x2ea00000, 0x00000000, 0x00a1401a, 0xa4355800, 0x28b00000,
        0x00000000, 0x00a1601a, 0xa4357800, 0x24b00000, 0x00000000,
        0x00a1801a, 0xa4359800, 0x22b00000, 0x00000000, 0x00a1a01a,
        0xa435b800, 0x21b00000, 0x00000000, 0x00e1201e, 0x44373800,
        0x20a01800, 0x00000000, 0x0140201b, 0xb4003800, 0x2fb00000,
        0x00000000, 0x00824000, 0x05fe5800, 0xb8000000, 0x00000000,
        0x06e1c01b, 0xb437dbff, 0x18680000, 0x00000000, 0x00e1e01b,
        0xb437f800, 0x24600000, 0x00000000, 0x00e2001b, 0xb4361800,
        0x22600000, 0x00000000, 0x00e0c01e, 0x4436d800, 0x20b08800,
        0x00000000, 0x00e0e01e, 0x4436f800, 0x20b04800, 0x00000000,
        0x00e1001e, 0x44371800, 0x20b02800, 0x00000000, 0x006020aa,
        0x1c001400, 0x10b0f828, 0x00000000, 0x0062201a, 0x64001068,
        0x70b0e818, 0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}
namespace cdCharPixel {
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
namespace cdCharFullbrightPixel {
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
// InitCDCharShader — allocate the shader and link into the init list.
// ea: 0x7D2B70
// ============================================================================
void InitCDCharShader() {
    cdCharShader* result = (cdCharShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdCharShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdCharShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~2;
        gCDCharShader = result;
    } else {
        gCDCharShader = NULL;

    }

}

// ============================================================================
// ToggleCDCharShader — toggle the character-shader enable bit (bit 1).
// ea: 0x7D2BC0
// ============================================================================
void ToggleCDCharShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (2 * ~(byte >> 1))) & 2) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;

}

// ============================================================================
// cdCharShader::Register — register the character vertex/pixel shaders.
// ea: 0x7D2BE0
// ============================================================================
tlFixedString cdCharShader::GetName() { return tlFixedString("cdChar"); }

void cdCharShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdCharShaderRender::VS, cdCharShaderRender::VShaderTable, 0);
    cdCharShaderRender::Shader = cdCharShaderRender::VS != nullptr ? cdCharShaderRender::VS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdCharPixel::PS, cdCharPixel::PShaderTable, 0);
    cdCharPixel::Shader = cdCharPixel::PS != nullptr ? cdCharPixel::PS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdCharFullbrightPixel::PS, cdCharFullbrightPixel::PShaderTable, 0);
    cdCharFullbrightPixel::Shader = cdCharFullbrightPixel::PS != nullptr ? cdCharFullbrightPixel::PS[0] : 0;
}

// ============================================================================
// cdCharShader::AddNode — add a character node to the opaque list.
// ea: 0x7D2C40
// ============================================================================
void cdCharShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                           nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 2) == 0) {
        cdCharShaderNode* node = (cdCharShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdCharShaderNode
            node->mMaterial = (cdCharShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDCharShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}
