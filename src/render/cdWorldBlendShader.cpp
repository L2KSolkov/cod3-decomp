// IDA ABI: nglMeshNode uses the class tag in render_xboxr exports.
// ============================================================================
// cdWorldBlendShader.cpp — world blend shader (5 non-inline funcs).
// Source: source/cdWorldBlendShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldBlendShader.o):
//   InitCDWorldBlendShader  @0x7DD2A0
//   ToggleCDWorldBlendShader @0x7DD2F0
//   cdWorldBlendShader::Register @0x7DD310
//   cdWorldBlendShader::AddNode @0x7DD350
// ============================================================================
#include "cdWorldBlendShader.h"
#include "cdWorldShader.h"

#include "ngl/ngl_lighting.h"
#include "ngl/nglRenderNode.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "ngl/nglTexture.h"
#include "render/ngl_aux.h"

#include <intrin.h>

// Shader global pointer definitions
cdWorldBlendShader* gCDWorldBlendShader = nullptr;  // ?gCDWorldBlendShader@@3PAVcdWorldBlendShader@@A
nglTexture* gDynamicLightTex_1 = nullptr;           // ?gDynamicLightTex_1@@3PAVnglTexture@@A

extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D38;
extern unsigned int dword_BC2D80;
extern unsigned int dword_BC2D08;
extern unsigned int dword_BC2D0C;
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;
extern float gProjShadowAlpha;
extern nglTexture* nglGetTexture(const tlFixedString& FileName);

static unsigned int BlendFogColor(const math::Vector4& color) {
    const __m128 scaled = _mm_mul_ps(color.v, _mm_set1_ps(127.0f));
    const unsigned int r = static_cast<unsigned int>(static_cast<int>(scaled.m128_f32[0]));
    const unsigned int g = static_cast<unsigned int>(static_cast<int>(scaled.m128_f32[1]));
    const unsigned int b = static_cast<unsigned int>(static_cast<int>(scaled.m128_f32[2]));
    const unsigned int a = static_cast<unsigned int>(static_cast<int>(scaled.m128_f32[3]));
    return b | (g << 8) | (r << 16) | (a << 24);
}

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdWorldBlendRender {
    static const unsigned int VShader0[54] = {
        0x000d2078, 0x00000000, 0x0062201a, 0x08001468, 0x7eb00000,
        0x00000000, 0x00e1401b, 0x08375800, 0x20a01800, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x06e0e01b,
        0x0836fbff, 0x10a88800, 0x00000000, 0x08e1001b, 0x08371802,
        0xd0a44800, 0x00000000, 0x00e1201b, 0x08373800, 0x20a02800,
        0x00000000, 0x00820055, 0x14016d54, 0x38000000, 0x00000000,
        0x02000600, 0x08001054, 0xe0b0c854, 0x00000000, 0x02420400,
        0x05541854, 0xa800c85c, 0x00000000, 0x02000800, 0x0800106d,
        0x20b0f81c, 0x00000000, 0x00820000, 0x05fe1800, 0x38000000,
        0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828, 0x00000000,
        0x0040001a, 0xc4002800, 0x20b0e801, 0x00000000,
    };
    static const unsigned int VShader1[81] = {
        0x00142078, 0x00000000, 0x0062201a, 0x08001468, 0x7eb00000,
        0x00000000, 0x0020001a, 0x08001000, 0x2e300000, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x00e1401b,
        0x08375800, 0x20a01800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0a88800, 0x00000000, 0x06e1001b, 0x08371bff, 0x10a44800,
        0x00000000, 0x00820000, 0x14016d54, 0x38000000, 0x00000000,
        0x00e1201b, 0x08373800, 0x20b02800, 0x00000000, 0x02420600,
        0x05541854, 0xe800c854, 0x00000000, 0x0041eaaa, 0x5801f800,
        0x28200000, 0x00000000, 0x00820000, 0x05fe1800, 0x38000000,
        0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828, 0x00000000,
        0x02000400, 0x08001054, 0xa0b0c85c, 0x00000000, 0x02000800,
        0x0800106d, 0x20a0f81c, 0x00000000, 0x01402000, 0x24003800,
        0x20a02820, 0x00000000, 0x00c1601b, 0x34377800, 0x20b08860,
        0x00000000, 0x00c1801b, 0x34379800, 0x20b04860, 0x00000000,
        0x00c1a01b, 0x3437b800, 0x20b02860, 0x00000000, 0x00c1c01b,
        0x3437d800, 0x20b01860, 0x00000000, 0x0040001a, 0xc4aa2800,
        0x20b0e801,
    };
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = { VShader0, VShader1 };
}
namespace cdWorldBlendProjectedRender {
    static const unsigned int VShader0[90] = {
        0x00162078, 0x00000000, 0x0062401a, 0x2c001468, 0x2e200000,
        0x00000000, 0x044240ff, 0x2dfe5bfc, 0xb8410000, 0x00000000,
        0x00a00a1a, 0x2434b000, 0x28300000, 0x00000000, 0x00e1401b,
        0x08375800, 0x20a01800, 0x00000000, 0x00824b00, 0x3434b068,
        0xbeb00000, 0x00000000, 0x00600058, 0x08001562, 0xd7400000,
        0x00000000, 0x06400000, 0x34006bff, 0x14b80000, 0x00000000,
        0x01802000, 0x34003800, 0x28b00000, 0x00000000, 0x00802100,
        0x35fe2aa8, 0x78600000, 0x00000000, 0x00600000, 0x44001556,
        0xd8400000, 0x00000000, 0x00400000, 0x64016800, 0x28600000,
        0x00000000, 0x00a00c1a, 0x69b48800, 0x28500000, 0x00000000,
        0x00a00e1a, 0x79b48800, 0x24500000, 0x00000000, 0x09202000,
        0x65543801, 0x18640000, 0x00000000, 0x00e0e01b, 0x0836f800,
        0x20b08800, 0x00000000, 0x00400015, 0x54aa2800, 0x2c500000,
        0x00000000, 0x01402000, 0x64003800, 0x28600000, 0x00000000,
        0x00e1001b, 0x08371800, 0x20b04800, 0x00000000, 0x00e1201b,
        0x08373800, 0x20b02800, 0x00000000, 0x0080c015, 0x5400d801,
        0xb0b0c848, 0x00000000, 0x02000000, 0x08001001, 0x90b0f81c,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801, 0x00000000,
    };
    static const unsigned int VShader1[149] = {
        0x00252078, 0x00000000, 0x0062401a, 0x2c001468, 0x2e200000,
        0x00000000, 0x00626058, 0x3c001560, 0x27b00000, 0x00000000,
        0x04a24a1a, 0x2434b3fc, 0xb8310000, 0x00000000, 0x04a26ada,
        0xb434b3fc, 0xf4380000, 0x00000000, 0x00824b00, 0x3434b068,
        0xbeb00000, 0x00000000, 0x00826b55, 0x34b0b160, 0xf7a00000,
        0x00000000, 0x0060001a, 0x0800146a, 0xde400000, 0x00000000,
        0x004240ff, 0x2dfe5800, 0x21400000, 0x00000000, 0x00400000,
        0x34006800, 0x24b00000, 0x00000000, 0x01802000, 0x34003800,
        0x28b00000, 0x00000000, 0x00802100, 0x35fe2aa8, 0x78600000,
        0x00000000, 0x00600058, 0x0800156e, 0x97a00000, 0x00000000,
        0x004260ff, 0x3dfe7800, 0x22b00000, 0x00000000, 0x00400055,
        0x34aa6800, 0x21b00000, 0x00000000, 0x006000ff, 0x44001556,
        0xd1400000, 0x00000000, 0x00400000, 0x64016800, 0x28600000,
        0x00000000, 0x006000aa, 0xb40017fe, 0xd8b00000, 0x00000000,
        0x00802155, 0x34002aa8, 0x74600000, 0x00000000, 0x08a00c1a,
        0x68348802, 0xd8580000, 0x00000000, 0x00a00e1a, 0x78348800,
        0x24500000, 0x00000000, 0x09202000, 0x65543bfd, 0x18640000,
        0x00000000, 0x00a00c1a, 0x69b54800, 0x22500000, 0x00000000,
        0x00a00e1a, 0x79b54800, 0x21500000, 0x00000000, 0x01202055,
        0x65543800, 0x24600000, 0x00000000, 0x00e1401b, 0x08375800,
        0x20b01800, 0x00000000, 0x00400015, 0x54aa2800, 0x2c500000,
        0x00000000, 0x07402000, 0x64003bff, 0x18640000, 0x00000000,
        0x004000ab, 0x54002800, 0x23500000, 0x00000000, 0x01402055,
        0x64003800, 0x24600000, 0x00000000, 0x00e0e01b, 0x0836f800,
        0x20b08800, 0x00000000, 0x00e1001b, 0x08371800, 0x20b04800,
        0x00000000, 0x00e1201b, 0x08373800, 0x20b02800, 0x00000000,
        0x0080c015, 0x5400d801, 0xb0b0c848, 0x00000000, 0x02000000,
        0x08001001, 0x90b0f81c, 0x00000000, 0x0080c0bf, 0x5400d801,
        0xb0b0c850, 0x00000000, 0x02000000, 0x08001155, 0x90b0f824,
        0x00000000, 0x0040001a, 0xc4aa2800, 0x20b0e801,
    };
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = { VShader0, VShader1 };
}
namespace cdWorldBlendPixel {
    static const unsigned int PShader0[60] = {
        0x00000000, 0xd8301010, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x14c9c834,
        0xccc40000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000c00, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader1[60] = {
        0x00000000, 0x00000000, 0xdbc51010, 0xd8301010, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x000000d0, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x14c9c834,
        0xccc40000, 0x00000000, 0xcc3d0000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000c00, 0x000000c0, 0x00000000, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011104, 0x00008021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader2[60] = {
        0x00000000, 0x00000000, 0xd8301010, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x14c9c834,
        0xccc40000, 0xccca0000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000c00, 0x000000c0, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011103, 0x00000421,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader3[60] = {
        0x00000000, 0x00000000, 0x00000000, 0xdbc51010, 0xd8301010,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000000d0,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x14c9c834,
        0xccc40000, 0xccca0000, 0x00000000, 0xcc3d0000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000c00, 0x000000c0, 0x000000c0, 0x00000000, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00011105, 0x00008421,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2][2] = {};
    unsigned int const* PShaderTable[2][2] = { { PShader0, PShader1 }, { PShader2, PShader3 } };
}
namespace cdWorldBlendProjectedPixel {
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
namespace cdWorldBlendSolidColorPixel {
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
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000001,
        0x00000000, 0x00000000, 0xfffffff0, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = { PShader0, nullptr };
    unsigned long* Shader = nullptr;
}
// ============================================================================
// InitCDWorldBlendShader — allocate the shader and link into the init list.
// ea: 0x7DD2A0
// ============================================================================
void InitCDWorldBlendShader() {
    cdWorldBlendShader* result = (cdWorldBlendShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdWorldBlendShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldBlendShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~8;
        gCDWorldBlendShader = result;
    } else {
        gCDWorldBlendShader = NULL;
    }
}

// ============================================================================
// ToggleCDWorldBlendShader — toggle world-blend enable bit (bit 3).
// ea: 0x7DD2F0
// ============================================================================
void ToggleCDWorldBlendShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (8 * ~(byte >> 3))) & 8) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;
}

// ============================================================================
// cdWorldBlendShader::Register — register the world-blend shaders.
// ea: 0x7DD310
// ============================================================================
tlFixedString cdWorldBlendShader::GetName() { return tlFixedString("cdWorldBlend"); }

void cdWorldBlendShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldBlendRender::VS[v0], cdWorldBlendRender::VShaderTable, v0);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldBlendProjectedRender::VS[v0], cdWorldBlendProjectedRender::VShaderTable, v0);
    }
    for (int v0 = 0, i = 4; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldBlendPixel::PS[0][v0], cdWorldBlendPixel::PShaderTable[0], v0);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldBlendProjectedPixel::PS[v0], cdWorldBlendProjectedPixel::PShaderTable, v0);
    }
    nglDxRegisterPShaderSafe((unsigned int**)cdWorldBlendSolidColorPixel::PS, cdWorldBlendSolidColorPixel::PShaderTable, 0);
    cdWorldBlendSolidColorPixel::Shader = cdWorldBlendSolidColorPixel::PS != nullptr ? cdWorldBlendSolidColorPixel::PS[0] : 0;
}

// ============================================================================
// cdWorldBlendShader::AddNode — add a clipped node to the opaque list.
// ea: 0x7DD350
// ============================================================================
void cdWorldBlendShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                 nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 8) == 0 &&
        cdGetClipResult(iSection, iMeshNode, nglBuildScene) != -1) {
        cdWorldBlendShaderNode* node = (cdWorldBlendShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            ::new (node) cdWorldBlendShaderNode;
            node->mMaterial = (cdWorldBlendShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDWorldBlendShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

void cdWorldBlendShaderNode::Render() {
    if (gDynamicLightTex_1 == nullptr) {
        tlFixedString name("dynamiclight");
        gDynamicLightTex_1 = nglGetTexture(name);
    }

    MeshNode->Mesh->Flags |= 0x07000000u;
    nglDetermineLights(MeshNode);

    // The reference collects up to four non-directional lights in the
    // shader's c18-c21 vectors.  The fourth component is the light radius.
    math::Position3 sphereCenter;
    auxGetSphereCenter(&sphereCenter, MeshNode->Mesh);
    math::Position3 worldCenter;
    worldCenter.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 0), MeshNode->LocalToWorld.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 85), MeshNode->LocalToWorld.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 170), MeshNode->LocalToWorld.z.v),
                   MeshNode->LocalToWorld.w.v));

    cdWorldBlendRender::cdWorldBlendParams params = {};
    params.mConsts.v = _mm_setr_ps(0.5f, 2.0f, 0.0f, 0.0f);
    params.mLocalToScreen = MeshNode->LocalToScreen;
    params.mWorldToShadow.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    params.mWorldToShadow.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    params.mWorldToShadow.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    params.mWorldToShadow.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    params.cShadowTint = gProjShadowAlpha;
    params.cFog.v = _mm_setr_ps(
        nglBuildScene->FogMin,
        nglBuildScene->FogNear,
        1.0f / (nglBuildScene->FogFar - nglBuildScene->FogNear),
        nglBuildScene->FogMax - nglBuildScene->FogMin);
    params.cEyePos = nglBuildScene->ViewToWorld.w;
    params.cEyePos.v.m128_f32[3] = 1.0f;

    unsigned int lightCount = 0;
    for (nglLightNode* light = nglSendLightContext->Head.LocalNext;
         light != reinterpret_cast<nglLightNode*>(nglSendLightContext) && lightCount < 4;
         light = light->LocalNext) {
        if (light->Type == NGLLIGHT_DIRECTIONAL)
            continue;
        nglPointLightInfo pointInfo;
        nglPointLightInfo* point = nglGetLightAsPointLight(&pointInfo, light, worldCenter);
        if (point == nullptr || point->isVertexPointLight)
            continue;
        params.mLightInfo[lightCount++].v = _mm_setr_ps(
            point->Pos.v.m128_f32[0], point->Pos.v.m128_f32[1],
            point->Pos.v.m128_f32[2], point->Far);
    }

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 0);
        dword_BC2D00 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0);
        dword_BC2D04 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x206u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, 0x206u);
        dword_BC2CF8 = 0x206u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 0);
        dword_BC2CFC = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);

    cdWorldBlendShaderMat* material = mMaterial;
    nglDxSetTexture(0, material->mDiffuse, 1u, 3u);
    nglDxSetTextureU(0, 1u);
    nglDxSetTextureV(0, 1u);
    nglDxSetTexture(1, material->mBlend, 1u, 3u);
    nglDxSetTextureU(1, 1u);
    nglDxSetTextureV(1, 1u);
    const bool hasLightmap = material->mLightmap != nullptr;
    if (hasLightmap) {
        nglDxSetTexture(2, material->mLightmap, 1u, 3u);
        nglDxSetTextureU(2, 3u);
        nglDxSetTextureV(2, 3u);
    }

    nglDxInitShaders(false);
    const unsigned int vertexShader = static_cast<unsigned int>(cdWorldBlendRender::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(cdWorldBlendPixel::PS[0][hasLightmap ? 2 : 0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(reinterpret_cast<const _D3DPixelShaderDef*>(static_cast<uintptr_t>(pixelShader)));
    }

    const unsigned int fogColor = BlendFogColor(nglBuildScene->FogColor);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_FOGCOLOR, fogColor) == 0)
        D3DDevice_SetRenderState_FogColor(fogColor);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1u) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &params, 0x40u);
    nglGpuDrawSection(Section);
    nglDxState.PrevBM = static_cast<unsigned int>(-1);
}
