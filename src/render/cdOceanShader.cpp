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

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdOceanRender {
    static const unsigned int VShaderMicrocode[241] = {
        0x003c2078, 0x00000000, 0x00e1801b, 0x08379800, 0x28000000,
        0x00000000, 0x00e1a01b, 0x0837b800, 0x24000000, 0x00000000,
        0x02202017, 0x080012a8, 0x7d40e81c, 0x00000000, 0x00630011,
        0x0400146e, 0x3f200000, 0x00000000, 0x00632011, 0x0400146e,
        0x7fb00000, 0x00000000, 0x0243421b, 0x24375854, 0x6f20c864,
        0x00000000, 0x0043601b, 0xb4377800, 0x2fb00000, 0x00000000,
        0x0060002a, 0x240011fc, 0x9c300000, 0x00000000, 0x00600002,
        0xb400115e, 0xd3300000, 0x00000000, 0x0082a015, 0x042abefd,
        0x70a0c848, 0x00000000, 0x0140001b, 0x36366800, 0x2f300000,
        0x00000000, 0x0043a01b, 0x3437b800, 0x2fb00000, 0x00000000,
        0x0063e01b, 0xb400106f, 0xffb00000, 0x00000000, 0x0043801b,
        0x34379800, 0x2f500000, 0x00000000, 0x0080401b, 0xb5fe5954,
        0xbf700000, 0x00000000, 0x0120201b, 0x55543800, 0x2f500000,
        0x00000000, 0x0b40201b, 0x54003aa9, 0xdf540000, 0x00000000,
        0x0a000000, 0x080013fd, 0xd0840000, 0x00000000, 0x02000000,
        0x08001154, 0x50880004, 0x00000000, 0x0a000000, 0x08001001,
        0xd0b40000, 0x00000000, 0x0a000000, 0x08001155, 0xd0740000,
        0x00000000, 0x02000000, 0x08001156, 0xd0720004, 0x00000000,
        0x02000000, 0x08001006, 0x10790004, 0x00000000, 0x0083c11b,
        0x5437d86f, 0xbf600000, 0x00000000, 0x00804093, 0x75545c00,
        0xbf700000, 0x00000000, 0x0082c015, 0x042adefd, 0xb0b0c850,
        0x00000000, 0x01000000, 0x7400e800, 0x2c900000, 0x00000000,
        0x01000055, 0x74aae800, 0x2ca00000, 0x00000000, 0x010000aa,
        0x7554e800, 0x2cb00000, 0x00000000, 0x010000ff, 0x75fee800,
        0x24200000, 0x00000000, 0x00400055, 0x94ab2800, 0x22900000,
        0x00000000, 0x00400055, 0xa4ab4800, 0x22a00000, 0x00000000,
        0x00400055, 0xb4ab6800, 0x22b00000, 0x00000000, 0x00400055,
        0x24aa4800, 0x22200000, 0x00000000, 0x00400055, 0x95552800,
        0x21900000, 0x00000000, 0x00400055, 0xa5554800, 0x21a00000,
        0x00000000, 0x00400055, 0xb5556800, 0x21b00000, 0x00000000,
        0x00400055, 0x25544800, 0x21200000, 0x00000000, 0x00e0601b,
        0x94367800, 0x28200000, 0x00000000, 0x00e0601b, 0xa4367800,
        0x24200000, 0x00000000, 0x00e0601b, 0xb4367800, 0x22200000,
        0x00000000, 0x0082e015, 0x042afefd, 0xf0a0c858, 0x00000000,
        0x00e0601b, 0x24367800, 0x21200000, 0x00000000, 0x00e0001b,
        0x2436c800, 0x28b00000, 0x00000000, 0x0080011b, 0x5436486c,
        0x9f300000, 0x00000000, 0x00628000, 0x4c001002, 0xd2000000,
        0x00000000, 0x00e4001b, 0x34361800, 0x20b01818, 0x00000000,
        0x00e24019, 0x04365800, 0x22400000, 0x00000000, 0x0000001b,
        0x0836106c, 0x20a00000, 0x00000000, 0x0060e01a, 0x44001469,
        0xfeb00000, 0x00000000, 0x00e1601b, 0x44377800, 0x20a01800,
        0x00000000, 0x00a0001a, 0xb4356800, 0x28b00000, 0x00000000,
        0x06e1001b, 0x44371bff, 0x10888800, 0x00000000, 0x08e1201b,
        0x44373802, 0xd0944800, 0x00000000, 0x00e1401b, 0x44375800,
        0x20a02800, 0x00000000, 0x0080c055, 0x14016d55, 0xb8800000,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e800, 0x00000000,
        0x0040c000, 0x8554d800, 0x28800000, 0x00000000, 0x0080c000,
        0x85fed801, 0xb8800000, 0x00000000, 0x006020aa, 0x1c001402,
        0x10b0f829,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}
namespace cdOceanPixel {
    static const unsigned int PShader0[60] = {
        0xd1301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
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
        0x00000000, 0x00000000, 0xfffffff1, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader1[60] = {
        0x00000000, 0xd1301010, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0xcccb0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00008001,
        0x00000000, 0x00000000, 0xffffff1f, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader2[60] = {
        0x00000000, 0xdad41010, 0xd1301010, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000a0, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0x00000000, 0xcada20cc, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000c00, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011103, 0x00000401,
        0x00000000, 0x00000000, 0xfffff1ff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader3[60] = {
        0x00000000, 0xdad41010, 0x00000000, 0xd1301010, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000a0, 0x00000000, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0x00000000, 0xcada20cc, 0xcccb0000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000c00, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011104, 0x00008401,
        0x00000000, 0x00000000, 0xffff1fff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader4[60] = {
        0x00000000, 0x00000000, 0xd9d11010, 0xd1301010, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000090, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0xc9c40000, 0x00000000, 0x19c9cc39, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000090, 0x00000000, 0x00000c00, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011104, 0x00000021,
        0x00000000, 0x00000000, 0xffff10ff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader5[60] = {
        0x00000000, 0x00000000, 0xd9d11010, 0x00000000, 0xd1301010,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000090, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0xc9c40000, 0x00000000, 0x19c9cc39, 0xcccb0000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000090, 0x00000000, 0x00000c00, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00011105, 0x00008021,
        0x00000000, 0x00000000, 0xfff1f0ff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader6[60] = {
        0x00000000, 0x00000000, 0xd9d11010, 0x00000000, 0xdad41010,
        0xd1301010, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000090, 0x00000000,
        0x000000a0, 0x000000c0, 0x00000000, 0x00000000, 0xc8c40000,
        0xc9c40000, 0x00000000, 0x19c9cc39, 0x00000000, 0xcada20cc,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000090, 0x00000000, 0x00000c00, 0x00000000,
        0x00000c00, 0x00000000, 0x00000000, 0x00011106, 0x00000421,
        0x00000000, 0x00000000, 0xff1ff0ff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader7[60] = {
        0x00000000, 0x00000000, 0xd9d11010, 0x00000000, 0xdad41010,
        0x00000000, 0xd1301010, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000090, 0x00000000,
        0x000000a0, 0x00000000, 0x000000c0, 0x00000000, 0xc8c40000,
        0xc9c40000, 0x00000000, 0x19c9cc39, 0x00000000, 0xcada20cc,
        0xcccb0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000090, 0x00000000, 0x00000c00, 0x00000000,
        0x00000c00, 0x000000c0, 0x00000000, 0x00011107, 0x00008421,
        0x00000000, 0x00000000, 0xf1fff0ff, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2][2][2] = {};
    unsigned int const* PShaderTable[2][2][2] = {
        { { PShader0, PShader1 }, { PShader2, PShader3 } },
        { { PShader4, PShader5 }, { PShader6, PShader7 } },
    };
}
namespace cdOceanPixel_Fullbright {
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
    static const unsigned int* PShaderTableStorage[2] = { PShaderMicrocode, nullptr };
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = { PShaderMicrocode, nullptr };
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
        ::new (v1) cdOceanShader;
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
tlFixedString cdOceanShader::GetName() { return tlFixedString("cdOcean"); }

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
