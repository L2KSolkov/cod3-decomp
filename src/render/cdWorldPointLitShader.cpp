// IDA ABI: nglMeshNode uses the class tag in render_xboxr exports.
// ============================================================================
// cdWorldPointLitShader.cpp — world point-lit shader (5 non-inline funcs).
// Source: source/cdWorldPointLitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldPointLitShader.o):
//   InitCDWorldPointLitShader  @0x7DBB00
//   ToggleCDWorldPointLitShader @0x7DBB50
//   cdWorldPointLitShader::Register @0x7DBB70
//   cdWorldPointLitShader::AddNode @0x7DBBB0
// ============================================================================
#include "cdWorldPointLitShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdWorldPointLitShader* gCDWorldPointLitShader = nullptr;  // ?gCDWorldPointLitShader@@3PAVcdWorldPointLitShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdWorldPointLitRender {
    static const unsigned int VShader0[122] = {
        0x001e2078, 0x00000000, 0x0062e01a, 0x7c001468, 0x2e000000,
        0x00000000, 0x00630058, 0x8c001560, 0x27b00000, 0x00000000,
        0x00632058, 0x9c001560, 0x2ba00000, 0x00000000, 0x00634058,
        0xac001560, 0x27900000, 0x00000000, 0x02a0021a, 0x04340854,
        0x6820c84c, 0x00000000, 0x02a004da, 0xb5b56854, 0xa420c854,
        0x00000000, 0x08a000ca, 0xa5954800, 0x92280000, 0x00000000,
        0x08a000da, 0x95b52954, 0x91240000, 0x00000000, 0x08e1401b,
        0x08375aa8, 0x90b21800, 0x00000000, 0x08e0e01b, 0x0836fbfc,
        0x90b18800, 0x00000000, 0x02402000, 0x14004800, 0x7840181c,
        0x00000000, 0x06400055, 0x14aa4bff, 0x14480000, 0x00000000,
        0x004000aa, 0x15544800, 0x22400000, 0x00000000, 0x004000ff,
        0x15fe4800, 0x21400000, 0x00000000, 0x0044001b, 0x44361800,
        0x2f400000, 0x00000000, 0x0062401a, 0x08001468, 0xbeb00000,
        0x00000000, 0x0063e01b, 0xfc00146d, 0x1f400000, 0x00000000,
        0x00a0001a, 0xb4356800, 0x28b00000, 0x00000000, 0x0140201b,
        0x44003800, 0x2f400000, 0x00000000, 0x0920201b, 0x45543802,
        0xdf440000, 0x00000000, 0x00e1001b, 0x08371800, 0x20a04800,
        0x00000000, 0x00822055, 0x14016d54, 0x78500000, 0x00000000,
        0x00e1201b, 0x08373800, 0x20b02800, 0x00000000, 0x00422000,
        0x55543800, 0x28500000, 0x00000000, 0x00822000, 0x55fe3800,
        0x78500000, 0x00000000, 0x00e3601b, 0x44377800, 0x20b08818,
        0x00000000, 0x00e3801b, 0x44379800, 0x20b04818, 0x00000000,
        0x00e3a01b, 0x4437b800, 0x20b02818, 0x00000000, 0x006020aa,
        0x1c001401, 0x50b0f828, 0x00000000, 0x0040001a, 0xc4002800,
        0x20b0e801, 0x00000000,
    };
    static const unsigned int VShader1[149] = {
        0x00252078, 0x00000000, 0x0062e01a, 0x7c001468, 0x2e000000,
        0x00000000, 0x00630058, 0x8c001560, 0x27b00000, 0x00000000,
        0x00632058, 0x9c001560, 0x2ba00000, 0x00000000, 0x00634058,
        0xac001560, 0x27900000, 0x00000000, 0x02a0021a, 0x04340854,
        0x6820c84c, 0x00000000, 0x02a004da, 0xb5b56854, 0xa420c854,
        0x00000000, 0x08a000ca, 0xa5954800, 0x92280000, 0x00000000,
        0x08a000da, 0x95b52954, 0x91240000, 0x00000000, 0x0820001a,
        0x080012a8, 0x9e720000, 0x00000000, 0x08e1401b, 0x08375bfc,
        0x90b11800, 0x00000000, 0x00400000, 0x14004800, 0x28400000,
        0x00000000, 0x06400055, 0x14aa4bff, 0x14480000, 0x00000000,
        0x004000aa, 0x15544800, 0x22400000, 0x00000000, 0x004000ff,
        0x15fe4800, 0x21400000, 0x00000000, 0x0044001b, 0x44361800,
        0x2f400000, 0x00000000, 0x0062401a, 0x08001468, 0xbeb00000,
        0x00000000, 0x0063e01b, 0xfc00146d, 0x1f400000, 0x00000000,
        0x00a0001a, 0xb4356800, 0x28b00000, 0x00000000, 0x0140201b,
        0x44003800, 0x2f400000, 0x00000000, 0x0920201b, 0x45543802,
        0xdf440000, 0x00000000, 0x00e0e01b, 0x0836f800, 0x20a08800,
        0x00000000, 0x00822055, 0x14016d54, 0x78500000, 0x00000000,
        0x00e1001b, 0x08371800, 0x20b04800, 0x00000000, 0x00422000,
        0x55543800, 0x28500000, 0x00000000, 0x00e1201b, 0x08373800,
        0x20b02800, 0x00000000, 0x00822000, 0x55fe3800, 0x78500000,
        0x00000000, 0x0041e6aa, 0x3801f800, 0x28600000, 0x00000000,
        0x00e3601b, 0x44377800, 0x20b08818, 0x00000000, 0x00e3801b,
        0x44379800, 0x20b04818, 0x00000000, 0x00e3a01b, 0x4437b800,
        0x20b02818, 0x00000000, 0x006020aa, 0x1c001401, 0x50a0f828,
        0x00000000, 0x01402000, 0x64003800, 0x20a01818, 0x00000000,
        0x00c1601b, 0x74377800, 0x20b08858, 0x00000000, 0x00c1801b,
        0x74379800, 0x20b04858, 0x00000000, 0x00c1a01b, 0x7437b800,
        0x20b02858, 0x00000000, 0x00c1c01b, 0x7437d800, 0x20b01858,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801,
    };
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = { VShader0, VShader1 };
}
namespace cdWorldPointLitProjectedRender {
    static const unsigned int VShader0[90] = {
        0x00162078, 0x00000000, 0x0062601a, 0x3c001468, 0x2e200000,
        0x00000000, 0x044260ff, 0x3dfe7bfc, 0xf8410000, 0x00000000,
        0x00a0061a, 0x24347000, 0x28300000, 0x00000000, 0x00e1401b,
        0x08375800, 0x20a01800, 0x00000000, 0x00826700, 0x34347068,
        0xfeb00000, 0x00000000, 0x00600058, 0x08001562, 0xd7400000,
        0x00000000, 0x06400000, 0x34006bff, 0x14b80000, 0x00000000,
        0x01802000, 0x34003800, 0x28b00000, 0x00000000, 0x00802100,
        0x35fe2aa8, 0x78600000, 0x00000000, 0x00600000, 0x44001556,
        0xd8400000, 0x00000000, 0x00400000, 0x64016800, 0x28600000,
        0x00000000, 0x00a0081a, 0x49b48800, 0x28500000, 0x00000000,
        0x00a00a1a, 0x59b48800, 0x24500000, 0x00000000, 0x09202000,
        0x65543801, 0x18640000, 0x00000000, 0x00e0e01b, 0x0836f800,
        0x20b08800, 0x00000000, 0x00400015, 0x54aa2800, 0x2c500000,
        0x00000000, 0x01402000, 0x64003800, 0x28600000, 0x00000000,
        0x00e1001b, 0x08371800, 0x20b04800, 0x00000000, 0x00e1201b,
        0x08373800, 0x20b02800, 0x00000000, 0x0080c015, 0x5400d801,
        0xb0b0c848, 0x00000000, 0x02000000, 0x08001001, 0x90b0f81c,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801, 0x00000000,
    };
    static const unsigned int VShader1[157] = {
        0x00272078, 0x00000000, 0x0062601a, 0x3c001468, 0x2e200000,
        0x00000000, 0x00628058, 0x4c001560, 0x27b00000, 0x00000000,
        0x04a2661a, 0x243473fc, 0xf8310000, 0x00000000, 0x04a286da,
        0xb43473fd, 0x34380000, 0x00000000, 0x00826700, 0x34347068,
        0xfeb00000, 0x00000000, 0x00828755, 0x34b07161, 0x37a00000,
        0x00000000, 0x0060001a, 0x0800146a, 0xde400000, 0x00000000,
        0x004260ff, 0x3dfe7800, 0x21400000, 0x00000000, 0x00400000,
        0x34006800, 0x24b00000, 0x00000000, 0x01802000, 0x34003800,
        0x28b00000, 0x00000000, 0x00802100, 0x35fe2aa8, 0x78600000,
        0x00000000, 0x00600058, 0x0800156e, 0x97a00000, 0x00000000,
        0x004280ff, 0x4dfe9800, 0x22b00000, 0x00000000, 0x00400055,
        0x34aa6800, 0x21b00000, 0x00000000, 0x01802055, 0x34003800,
        0x28a00000, 0x00000000, 0x00802155, 0x34002aa8, 0x74600000,
        0x00000000, 0x006000ff, 0x44001556, 0xd1400000, 0x00000000,
        0x00400000, 0x64016800, 0x28600000, 0x00000000, 0x006000aa,
        0xb40017fe, 0xd8b00000, 0x00000000, 0x08400055, 0x64014bfd,
        0x14680000, 0x00000000, 0x08a0081a, 0x48348802, 0xd8540000,
        0x00000000, 0x00a00a1a, 0x58348800, 0x24500000, 0x00000000,
        0x01202000, 0x65543800, 0x28600000, 0x00000000, 0x00a0081a,
        0x49b54800, 0x22500000, 0x00000000, 0x00a00a1a, 0x59b54800,
        0x21500000, 0x00000000, 0x01202055, 0x65543800, 0x24600000,
        0x00000000, 0x00e1401b, 0x08375800, 0x20b01800, 0x00000000,
        0x00400015, 0x54002800, 0x2c500000, 0x00000000, 0x07402000,
        0x64003bff, 0x18680000, 0x00000000, 0x004000ab, 0x54aa2800,
        0x23500000, 0x00000000, 0x01402055, 0x64003800, 0x24600000,
        0x00000000, 0x00e0e01b, 0x0836f800, 0x20b08800, 0x00000000,
        0x00e1001b, 0x08371800, 0x20b04800, 0x00000000, 0x00e1201b,
        0x08373800, 0x20b02800, 0x00000000, 0x0080c015, 0x5400d801,
        0xb0b0c848, 0x00000000, 0x02000000, 0x08001001, 0x90b0f81c,
        0x00000000, 0x0080c0bf, 0x5400d801, 0xb0b0c850, 0x00000000,
        0x02000000, 0x08001155, 0x90b0f824, 0x00000000, 0x0040001a,
        0xc4002800, 0x20b0e801,
    };
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = { VShader0, VShader1 };
}
namespace cdWorldPointLitPixel {
    static const unsigned int PShader0[60] = {
        0xd8301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
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
    static const unsigned int PShader1[60] = {
        0xd9d41010, 0x00000000, 0xd8301010, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000d0, 0x00000000, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xc8c40000, 0xcc3d0000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011103, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader2[60] = {
        0x00000000, 0xd8301010, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0xc8c920cc, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000c00, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader3[60] = {
        0x00000000, 0x00000000, 0xdad41010, 0xd8301010, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x000000d0, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0xc8c920cc, 0x00000000, 0xcc3d0000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000c00, 0x00000000, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011104, 0x00000421,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2][2] = {};
    unsigned int const* PShaderTable[2][2] = { { PShader0, PShader1 }, { PShader2, PShader3 } };
}
namespace cdWorldPointLitProjectedPixel {
    static const unsigned int PShader0[60] = {
        0xd8d41010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x200c2000, 0x00001c80,
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
    static const unsigned int PShader1[60] = {
        0xd8d41010, 0xd9d530dc, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x200c2000, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000c00, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0xc9c520cc, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000c00, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = { PShader0, PShader1 };
}
namespace cdWorldPointLitSolidColorPixel {
    static const unsigned int PShader0[60] = {
        0xd1301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc1200000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000000,
        0x00000000, 0x00000000, 0xfffffff0, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = { PShader0, nullptr };
    unsigned long* Shader = nullptr;
}
// ============================================================================
// InitCDWorldPointLitShader — allocate the shader and link into the init list.
// ea: 0x7DBB00
// ============================================================================
void InitCDWorldPointLitShader() {
    cdWorldPointLitShader* result = (cdWorldPointLitShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdWorldPointLitShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldPointLitShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~0x10;
        gCDWorldPointLitShader = result;
    } else {
        gCDWorldPointLitShader = NULL;
    }
}

// ============================================================================
// ToggleCDWorldPointLitShader — toggle point-lit enable bit (bit 4).
// ea: 0x7DBB50
// ============================================================================
void ToggleCDWorldPointLitShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (16 * ~(byte >> 4))) & 0x10) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;
}

// ============================================================================
// cdWorldPointLitShader::Register — register the point-lit shaders.
// ea: 0x7DBB70
// ============================================================================
tlFixedString cdWorldPointLitShader::GetName() { return tlFixedString("cdWorldPointLit"); }

void cdWorldPointLitShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldPointLitRender::VS[v0], cdWorldPointLitRender::VShaderTable, v0);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldPointLitProjectedRender::VS[v0], cdWorldPointLitProjectedRender::VShaderTable, v0);
    }
    for (int v0 = 0, i = 4; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldPointLitPixel::PS[0][v0], cdWorldPointLitPixel::PShaderTable[0], v0);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldPointLitProjectedPixel::PS[v0], cdWorldPointLitProjectedPixel::PShaderTable, v0);
    }
    nglDxRegisterPShaderSafe((unsigned int**)cdWorldPointLitSolidColorPixel::PS, cdWorldPointLitSolidColorPixel::PShaderTable, 0);
    cdWorldPointLitSolidColorPixel::Shader = cdWorldPointLitSolidColorPixel::PS != nullptr ? cdWorldPointLitSolidColorPixel::PS[0] : 0;
}

// ============================================================================
// cdWorldPointLitShader::AddNode — add a clipped node to the opaque list.
// ea: 0x7DBBB0
// ============================================================================
void cdWorldPointLitShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                    nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 0x10) == 0) {
        int ClipResult = cdGetClipResult(iSection, iMeshNode, nglBuildScene);
        if (ClipResult != -1) {
            cdWorldPointLitShaderNode* node = (cdWorldPointLitShaderNode*)nglListAlloc(0x1C, 0x10);
            if (node != NULL) {
                node->MeshNode = iMeshNode;
                node->Section = iSection;
                // vftable = cdWorldPointLitShaderNode
                node->mMaterial = (cdWorldPointLitShaderMat*)iMat;
            } else {
                node = NULL;
            }
            node->Clip = ClipResult;
            if (((cdWorldPointLitShaderMat*)iMat)->mDiffuse == NULL)
                ((cdWorldPointLitShaderMat*)iMat)->mDiffuse = nglDefaultTex;
            node->SortHash = gCDWorldPointLitShader->ID;
            node->Next = nglBuildScene->OpaqueRenderList;
            nglBuildScene->OpaqueRenderList = node;
            ++nglBuildScene->OpaqueListCount;
        }
    }
}
