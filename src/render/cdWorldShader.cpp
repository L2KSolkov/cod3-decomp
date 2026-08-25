// IDA ABI: nglMeshNode uses the class tag in render_xboxr exports.
// ============================================================================
// cdWorldShader.cpp — world shader (5 non-inline funcs).
// Source: source/cdWorldShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldShader.o):
//   InitCDWorldShader  @0x7DF0D0
//   ToggleCDWorldShader @0x7DF120
//   cdWorldShader::Register @0x7DF140
//   cdWorldShader::AddNode @0x7DF180
// ============================================================================
#include "cdWorldShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "render/ShaderCommon.h"

#include <intrin.h>
#include <new>

// Shader global pointer definitions
cdWorldShader* gCDWorldShader = nullptr;  // ?gCDWorldShader@@3PAVcdWorldShader@@A

extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D80;
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;
extern float gProjShadowAlpha;

static void SetIdentity(math::Mat44* matrix) {
    matrix->x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    matrix->y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    matrix->z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    matrix->w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
}

// nglMeshNode::GetWToLNoScale, shared by the Xbox shader constant builders.
// This is the same inverse-rotation/translation reconstruction used by the
// NGL backend when it prepares per-node lighting constants.
static math::Mat43* MeshNode_GetWToLNoScale(const nglMeshNode* This,
                                            math::Mat43* result) {
    math::Mat43 localToWorldNoScale;
    if ((This->MeshParams->Flags & 2) != 0) {
        const __m128 scale = This->MeshParams->Scale.v;
        const __m128 reciprocal = _mm_rcp_ps(scale);
        const __m128 inverse = _mm_mul_ps(
            _mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(reciprocal, scale)), reciprocal);
        localToWorldNoScale.x.v = _mm_mul_ps(
            This->LocalToWorld.x.v, _mm_shuffle_ps(inverse, inverse, 0));
        localToWorldNoScale.y.v = _mm_mul_ps(
            This->LocalToWorld.y.v, _mm_shuffle_ps(inverse, inverse, 85));
        localToWorldNoScale.z.v = _mm_mul_ps(
            This->LocalToWorld.z.v, _mm_shuffle_ps(inverse, inverse, 170));
        localToWorldNoScale.w.v = This->LocalToWorld.w.v;
    } else {
        localToWorldNoScale = This->LocalToWorld;
    }

    const __m128 y = localToWorldNoScale.y.v;
    const __m128 z = localToWorldNoScale.z.v;
    const __m128 w = localToWorldNoScale.w.v;
    const __m128 xy = _mm_shuffle_ps(localToWorldNoScale.x.v, y, 0x44);
    const __m128 xz = _mm_shuffle_ps(xy, z, 0xDD);
    const __m128 yz = _mm_shuffle_ps(
        _mm_shuffle_ps(localToWorldNoScale.x.v, y, 0xEE), z, 0xA8);
    const __m128 xx = _mm_shuffle_ps(xy, z, 0x88);
    result->x.v = _mm_shuffle_ps(xx, _mm_setzero_ps(), 0xE4);
    result->y.v = xz;
    result->z.v = yz;
    result->w.v = _mm_xor_ps(
        _mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(w, w, 0), xx),
                       _mm_mul_ps(_mm_shuffle_ps(w, w, 85), xz)),
            _mm_mul_ps(_mm_shuffle_ps(w, w, 170), yz)));
    return result;
}

static unsigned int FogColor(const math::Vector4& color) {
    const __m128 scaled = _mm_mul_ps(color.v, _mm_set1_ps(255.0f));
    const unsigned int r = (unsigned int)(int)scaled.m128_f32[0];
    const unsigned int g = (unsigned int)(int)scaled.m128_f32[1];
    const unsigned int b = (unsigned int)(int)scaled.m128_f32[2];
    const unsigned int a = (unsigned int)(int)scaled.m128_f32[3];
    return b | (g << 8) | (r << 16) | (a << 24);
}

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdWorldRender {
    static const unsigned int VShader0[50] = {
        0x000c2078, 0x00000000, 0x0062401a, 0x08001468, 0xbeb00000,
        0x00000000, 0x00e1401b, 0x08375800, 0x20a01800, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x06e0e01b,
        0x0836fbff, 0x10a88800, 0x00000000, 0x08e1001b, 0x08371802,
        0xd0a44800, 0x00000000, 0x00e1201b, 0x08373800, 0x20a02800,
        0x00000000, 0x00822055, 0x14016d54, 0x78000000, 0x00000000,
        0x02000400, 0x08001054, 0xa0b0c854, 0x00000000, 0x00422000,
        0x05543800, 0x28000000, 0x00000000, 0x00822000, 0x05fe3800,
        0x78000000, 0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801, 0x00000000,
    };
    static const unsigned int VShader1[50] = {
        0x000c2078, 0x00000000, 0x0062401a, 0x08001468, 0xbeb00000,
        0x00000000, 0x00e1401b, 0x08375800, 0x20a01800, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x06e0e01b,
        0x0836fbff, 0x10a88800, 0x00000000, 0x08e1001b, 0x08371802,
        0xd0a44800, 0x00000000, 0x00e1201b, 0x08373800, 0x20a02800,
        0x00000000, 0x00822055, 0x14016d54, 0x78000000, 0x00000000,
        0x02000400, 0x08001054, 0xa0b0c854, 0x00000000, 0x02422600,
        0x0554386c, 0xe800f81c, 0x00000000, 0x0040001a, 0xc4002800,
        0x20b0e800, 0x00000000, 0x00822000, 0x05fe3800, 0x78000000,
        0x00000000, 0x006020aa, 0x1c001400, 0x10b0f829, 0x00000000,
    };
    static const unsigned int VShader2[74] = {
        0x00122078, 0x00000000, 0x0062401a, 0x08001468, 0xbeb00000,
        0x00000000, 0x0020001a, 0x08001000, 0x2e300000, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x00e1401b,
        0x08375800, 0x20a01800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0a88800, 0x00000000, 0x06e1001b, 0x08371bff, 0x10a44800,
        0x00000000, 0x00822000, 0x14016d54, 0x78000000, 0x00000000,
        0x00e1201b, 0x08373800, 0x20b02800, 0x00000000, 0x02422400,
        0x05543854, 0xa800c854, 0x00000000, 0x0041e6aa, 0x3801f800,
        0x28200000, 0x00000000, 0x00822000, 0x05fe3800, 0x78000000,
        0x00000000, 0x006020aa, 0x1c001400, 0x10a0f828, 0x00000000,
        0x01402000, 0x24003800, 0x20a01818, 0x00000000, 0x00c1601b,
        0x34377800, 0x20b08858, 0x00000000, 0x00c1801b, 0x34379800,
        0x20b04858, 0x00000000, 0x00c1a01b, 0x3437b800, 0x20b02858,
        0x00000000, 0x00c1c01b, 0x3437d800, 0x20b01858, 0x00000000,
        0x0040001a, 0xc4aa2800, 0x20b0e801, 0x00000000,
    };
    static const unsigned int VShader3[77] = {
        0x00132078, 0x00000000, 0x0062401a, 0x08001468, 0xbeb00000,
        0x00000000, 0x0020001a, 0x08001000, 0x2e300000, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x00e1401b,
        0x08375800, 0x20a01800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0a88800, 0x00000000, 0x06e1001b, 0x08371bff, 0x10a44800,
        0x00000000, 0x00822000, 0x14016d54, 0x78000000, 0x00000000,
        0x00e1201b, 0x08373800, 0x20b02800, 0x00000000, 0x02422400,
        0x05543854, 0xa800c854, 0x00000000, 0x0041e8aa, 0x4801f800,
        0x28200000, 0x00000000, 0x00822000, 0x05fe3800, 0x78000000,
        0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828, 0x00000000,
        0x02000600, 0x08001068, 0xe0a0e81c, 0x00000000, 0x01402000,
        0x24003800, 0x20a01818, 0x00000000, 0x00c1601b, 0x34377800,
        0x20b08858, 0x00000000, 0x00c1801b, 0x34379800, 0x20b04858,
        0x00000000, 0x00c1a01b, 0x3437b800, 0x20b02858, 0x00000000,
        0x00c1c01b, 0x3437d800, 0x20b01858, 0x00000000, 0x0040001a,
        0xc4aa2800, 0x20b0e801,
    };
    unsigned long VS[4][2] = {};
    unsigned int const* VShaderTable[4] = { VShader0, VShader1, VShader2, VShader3 };
}
namespace cdWorldProjectedRender {
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
namespace cdWorldPixel {
    static const unsigned int PShader0[60] = {
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
    static const unsigned int PShader1[60] = {
        0x00000000, 0xd8301010, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8200000,
        0xccc40000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00000001,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader2[60] = {
        0xd9d41010, 0xd8301010, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000d0, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xc83d0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader3[60] = {
        0xd9d41010, 0x00000000, 0xd8301010, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000d0, 0x00000000, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xc83d0000, 0xccc40000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011103, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader4[60] = {
        0xd8301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c90000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader5[60] = {
        0x00000000, 0xd8301010, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c90000,
        0xccc40000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader6[60] = {
        0x00000000, 0xdad41010, 0xd8301010, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000d0, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c90000,
        0x00000000, 0xcc3d0000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011103, 0x00000421,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader7[60] = {
        0x00000000, 0xdad41010, 0x00000000, 0xd8301010, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000d0, 0x00000000, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c90000,
        0x00000000, 0xcc3d0000, 0xccc40000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x000000c0, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011104, 0x00000421,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2][2][2] = {};
    unsigned int const* PShaderTable[2][2][2] = {
        { { PShader0, PShader1 }, { PShader2, PShader3 } },
        { { PShader4, PShader5 }, { PShader6, PShader7 } },
    };
}
namespace cdWorldProjectedPixel {
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
namespace cdWorldSolidColorPixel {
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
// cdWorldRender::RegisterShader — register the 4 world vertex shaders.
// ea: 0x7DFFE0 (inline COMDAT)
// ============================================================================
inline void cdWorldRender_RegisterShader() {
    for (int v0 = 0; v0 < 4; ++v0) {
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(
                                 &cdWorldRender::VS[v0 / 2][v0 & 1]),
                             cdWorldRender::VShaderTable[v0]);
    }
}

// ============================================================================
// cdWorldProjectedRender::RegisterShader — register the 2 projected VShaders.
// ea: 0x7E0040 (inline COMDAT)
// ============================================================================
inline void cdWorldProjectedRender_RegisterShader() {
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldProjectedRender::VS[v0], cdWorldProjectedRender::VShaderTable, v0);
    }
}

// ============================================================================
// cdWorldPixel::RegisterShader — register the 8 world pixel shaders.
// ea: 0x7E0090 (inline COMDAT)
// ============================================================================
inline void cdWorldPixel_RegisterShader() {
    for (int v0 = 0, i = 8; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldPixel::PS[0][0][v0], cdWorldPixel::PShaderTable[0][0], v0);
    }
}

// ============================================================================
// cdWorldProjectedPixel::RegisterShader — register the 2 projected PShaders.
// ea: 0x7E00F0 (inline COMDAT)
// ============================================================================
inline void cdWorldProjectedPixel_RegisterShader() {
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldProjectedPixel::PS[v0], cdWorldProjectedPixel::PShaderTable, v0);
    }
}

// ============================================================================
// InitCDWorldShader — allocate the shader and link into the init list.
// ea: 0x7DF0D0
// ============================================================================
void InitCDWorldShader() {
    cdWorldShader* result = (cdWorldShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdWorldShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~2;
        gCDWorldShader = result;
    } else {
        gCDWorldShader = NULL;

    }

}

// ============================================================================
// ToggleCDWorldShader — toggle the world-shader enable bit (bit 1).
// ea: 0x7DF120
// ============================================================================
void ToggleCDWorldShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (2 * ~(byte >> 1))) & 2) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;

}

// ============================================================================
// cdWorldShader::Register — register all world vertex/pixel shaders.
// ea: 0x7DF140
// ============================================================================
tlFixedString cdWorldShader::GetName() { return tlFixedString("cdWorld"); }

void cdWorldShader::Register() {
    nglShader::Register();
    cdWorldRender_RegisterShader();
    cdWorldProjectedRender_RegisterShader();
    cdWorldPixel_RegisterShader();
    cdWorldProjectedPixel_RegisterShader();
    nglDxRegisterPShaderSafe((unsigned int**)cdWorldSolidColorPixel::PS, cdWorldSolidColorPixel::PShaderTable, 0);
    cdWorldSolidColorPixel::Shader = cdWorldSolidColorPixel::PS != nullptr ? cdWorldSolidColorPixel::PS[0] : 0;
}

// ============================================================================
// cdWorldShader::AddNode — add a clipped world node to the opaque list.
// ea: 0x7DF180
// ============================================================================
void cdWorldShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 2) == 0) {
        int ClipResult = cdGetClipResult(iSection, iMeshNode, nglBuildScene);
        if (ClipResult != -1) {
            cdWorldShaderNode* node = (cdWorldShaderNode*)nglListAlloc(0x20, 0x10);
            if (node != NULL) {
                ::new (node) cdWorldShaderNode;
                node->MeshNode = iMeshNode;
                node->Section = iSection;
                // vftable = cdWorldShaderNode
                node->mMaterial = (cdWorldShaderMat*)iMat;
                node->hasColorVerts = false;
            } else {
                node = NULL;
            }
            node->Clip = ClipResult;
            node->SortHash = gCDWorldShader->ID;
            node->Next = nglBuildScene->OpaqueRenderList;
            nglBuildScene->OpaqueRenderList = node;
            ++nglBuildScene->OpaqueListCount;
        }
    }
}

// ============================================================================
// cdWorldShaderNode::Render — ea: 0x7DF220
// The constant layout and state order follow the IDA disassembly.  The
// projected/dynamic-light passes are selected only when their native shader
// tables are available; the base world pass is always emitted.
// ============================================================================
void cdWorldShaderNode::Render() {
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

    cdWorldShaderMat* material = this->mMaterial;
    nglDxSetTexture(0, material != nullptr ? material->mDiffuse : nullptr, 1u, 3u);
    if (nglDxTexCache.Prev[0].WrapU != 1) {
        nglDxTexCache.Prev[0].WrapU = 1;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 1) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSU] = 1;
        }
    }
    if (nglDxTexCache.Prev[0].WrapV != 1) {
        nglDxTexCache.Prev[0].WrapV = 1;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 1) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSV] = 1;
        }
    }
    if (material != nullptr && material->mLightmap != nullptr) {
        nglDxSetTexture(1, material->mLightmap, 1u, 3u);
        if (nglDxTexCache.Prev[1].WrapU != 3) {
            nglDxTexCache.Prev[1].WrapU = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(1, D3DTSS_ADDRESSU, 3) == 0) {
                D3D__DirtyFlags |= 2u;
                D3D__TextureState[1][D3DTSS_ADDRESSU] = 3;
            }
        }
        if (nglDxTexCache.Prev[1].WrapV != 3) {
            nglDxTexCache.Prev[1].WrapV = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(1, D3DTSS_ADDRESSV, 3) == 0) {
                D3D__DirtyFlags |= 2u;
                D3D__TextureState[1][D3DTSS_ADDRESSV] = 3;
            }
        }
    }

    nglDxInitShaders(false);
    const unsigned int lightmap = material != nullptr && material->mLightmap != nullptr;
    // IDA flattens the base-pass indices as VS[dynamicLights][colorVerts] and
    // PS[lightmap][dynamicLights][colorVerts].  This port emits no dynamic-light
    // pass here, so that selector is zero even when a lightmap is bound.
    const unsigned int vertexShader = (unsigned int)cdWorldRender::VS[0][this->hasColorVerts];
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>((uintptr_t)vertexShader), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const unsigned int pixelShader = (unsigned int)(uintptr_t)
        cdWorldPixel::PS[lightmap][0][this->hasColorVerts];
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(reinterpret_cast<const _D3DPixelShaderDef*>(
            (uintptr_t)pixelShader));
    }

    cdWorldRender::cdWorldParams params = {};
    params.mConsts.v = _mm_setr_ps(0.5f, 2.0f, 0.0f, 0.0f);
    params.mLocalToScreen = this->MeshNode->LocalToScreen;
    SetIdentity(&params.mWorldToShadow);
    params.cShadowTint = gProjShadowAlpha;
    params.cFogColor = nglBuildScene->FogColor;
    params.cFog.v = _mm_setr_ps(
        nglBuildScene->FogMin,
        nglBuildScene->FogNear,
        1.0f / (nglBuildScene->FogFar - nglBuildScene->FogNear),
        nglBuildScene->FogMax - nglBuildScene->FogMin);
    math::Mat43 worldToLocalNoScale;
    MeshNode_GetWToLNoScale(this->MeshNode, &worldToLocalNoScale);
    const __m128 eyeLocal = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(nglBuildScene->ViewToWorld.w.v,
                                      nglBuildScene->ViewToWorld.w.v, 0),
                       worldToLocalNoScale.x.v),
            _mm_mul_ps(_mm_shuffle_ps(nglBuildScene->ViewToWorld.w.v,
                                      nglBuildScene->ViewToWorld.w.v, 85),
                       worldToLocalNoScale.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(nglBuildScene->ViewToWorld.w.v,
                                      nglBuildScene->ViewToWorld.w.v, 170),
                       worldToLocalNoScale.z.v),
            worldToLocalNoScale.w.v));
    params.cEyePos.v = _mm_shuffle_ps(
        eyeLocal, _mm_shuffle_ps(_mm_set1_ps(1.0f), eyeLocal, 160), 52);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_FOGCOLOR,
                                                  FogColor(nglBuildScene->FogColor)) == 0)
        D3DDevice_SetRenderState_FogColor(FogColor(nglBuildScene->FogColor));
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1u) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &params, 0x44u);
    nglGpuDrawSection(this->Section);
    nglDxState.PrevBM = (unsigned int)-1;
}
