// ============================================================================
// cdSkyShader.cpp — sky shader (4 non-inline funcs).
// Source: source/cdSkyShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSkyShader.o):
//   InitCDSkyShader  @0x7E0EA0
//   ToggleCDSkyShader @0x7E0EF0
//   cdSkyShader::Register @0x7E0F10
// ============================================================================
#include "cdSkyShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "render/ShaderCommon.h"

#include <intrin.h>
#include <cmath>
#include <new>

extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_40344;
extern unsigned int dword_40348;
extern unsigned int dword_40350;
extern unsigned int dword_4035C;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D10;
extern unsigned int dword_BC2D08;
extern unsigned int dword_BC2D0C;
extern unsigned int dword_BC2D38;
extern unsigned int dword_BC2D80;
extern unsigned int dword_BC2E4C;
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;

// Shader global pointer definitions
cdSkyShader* gCDSkyShader = nullptr;  // ?gCDSkyShader@@3PAVcdSkyShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdSkyShaderRender {
    static const unsigned int VShaderMicrocode[29] = {
        0x00072078, 0x00000000, 0x00e1201b, 0x08373800, 0x20b01800,
        0x00000000, 0x00e0c01b, 0x0836d800, 0x20b08800, 0x00000000,
        0x06e0e01b, 0x0836fbff, 0x10b84800, 0x00000000, 0x00e1001b,
        0x08371800, 0x20b02800, 0x00000000, 0x00614215, 0x18001056,
        0xb0b0c848, 0x00000000, 0x02000400, 0x0800106c, 0xa0b0f81c,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}
namespace cdSkyShaderPixel {
    static const unsigned int PShaderMicrocode[60] = {
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
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = nullptr;
}
// ============================================================================
// InitCDSkyShader — allocate the shader and link into the init list.
// ea: 0x7E0EA0
// ============================================================================
void InitCDSkyShader() {
    cdSkyShader* result = (cdSkyShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdSkyShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSkyShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~0x10;
        gCDSkyShader = result;
    } else {
        gCDSkyShader = NULL;
    }
}

// ============================================================================
// ToggleCDSkyShader — toggle sky-shader enable bit (bit 4).
// ea: 0x7E0EF0
// ============================================================================
void ToggleCDSkyShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (16 * ~(byte >> 4))) & 0x10) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;
}

// ============================================================================
// cdSkyShader::Register — register the sky vertex/pixel shaders.
// ea: 0x7E0F10
// ============================================================================
tlFixedString cdSkyShader::GetName() { return tlFixedString("cdSky"); }

void cdSkyShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdSkyShaderRender::VS, cdSkyShaderRender::VShaderTable, 0);
    cdSkyShaderRender::Shader = cdSkyShaderRender::VS != nullptr ? cdSkyShaderRender::VS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdSkyShaderPixel::PS, cdSkyShaderPixel::PShaderTable, 0);
    cdSkyShaderPixel::Shader = cdSkyShaderPixel::PS != nullptr ? cdSkyShaderPixel::PS[0] : 0;
}

void cdSkyShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                          nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 0x10) == 0) {
        cdSkyShaderNode* node = (cdSkyShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            ::new (node) cdSkyShaderNode;
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            node->mMaterial = (cdSkyShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortDist = 1000.0f - (float)((cdSkyShaderMat*)iMat)->mDrawOrder;
        node->Next = nglBuildScene->TransRenderList;
        nglBuildScene->TransRenderList = node;
        ++nglBuildScene->TransListCount;
    }
}

// cdSkyShaderNode::Render — ea: 0x7E0F50
void cdSkyShaderNode::Render() {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);

    if (mMaterial->mAlphaBlend != 0) {
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40300, 1u);
            dword_BC2D00 = 1;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x204u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_4033C, 0x204u);
            dword_BC2CF8 = 0x204u;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40340, 0);
            dword_BC2D04 = 0;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40304, 1u);
            dword_BC2CFC = 1;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, 0x8006u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40350, 0x8006u);
            dword_BC2D38 = 0x8006u;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, 0x302u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40344, 0x302u);
            dword_BC2D08 = 0x302u;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, 0x303u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40348, 0x303u);
            dword_BC2D0C = 0x303u;
        }
    } else {
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40300, 0);
            dword_BC2D00 = 0;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40304, 0);
            dword_BC2CFC = 0;
        }
    }

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 0) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 0;
    }

    const unsigned int oldZ = dword_BC2E4C;
    const unsigned int oldZWrite = dword_BC2D10;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 0) == 0)
        D3DDevice_SetRenderState_ZEnable(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }

    const float angle = ShaderCommon::gTime * mMaterial->mZRotate * 0.017453292f;
    const float cosa = cosf(angle);
    const float sina = sinf(angle);
    const math::Mat44& source = MeshNode->LocalToScreen;
    const float src[4][4] = {
        {source.x.v.m128_f32[0], source.y.v.m128_f32[0], source.z.v.m128_f32[0], source.w.v.m128_f32[0]},
        {source.x.v.m128_f32[1], source.y.v.m128_f32[1], source.z.v.m128_f32[1], source.w.v.m128_f32[1]},
        {source.x.v.m128_f32[2], source.y.v.m128_f32[2], source.z.v.m128_f32[2], source.w.v.m128_f32[2]},
        {source.x.v.m128_f32[3], source.y.v.m128_f32[3], source.z.v.m128_f32[3], source.w.v.m128_f32[3]},
    };
    const float rotation[4][4] = {
        {cosa, -sina, 1.0f, 0.0f},
        {sina,  cosa, 1.0f, 0.0f},
        {0.0f,  0.0f, 1.0f, 0.0f},
        {0.0f,  0.0f, 0.0f, 1.0f},
    };
    float transformed[4][4] = {};
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            for (int term = 0; term < 4; ++term)
                transformed[row][column] += rotation[row][term] * src[term][column];

    SkyContext context;
    context.mLToW.x.v = _mm_setr_ps(transformed[0][0], transformed[1][0], transformed[2][0], transformed[3][0]);
    context.mLToW.y.v = _mm_setr_ps(transformed[0][1], transformed[1][1], transformed[2][1], transformed[3][1]);
    context.mLToW.z.v = _mm_setr_ps(transformed[0][2], transformed[1][2], transformed[2][2], transformed[3][2]);
    context.mLToW.w.v = _mm_setr_ps(transformed[0][3], transformed[1][3], transformed[2][3], transformed[3][3]);
    const float uScroll = mMaterial->mUScroll * ShaderCommon::gTime * 0.1f;
    const float vScroll = mMaterial->mVScroll * ShaderCommon::gTime * 0.1f;
    context.mUTranslate = uScroll - static_cast<float>(static_cast<int>(uScroll));
    context.mVTranslate = vScroll - static_cast<float>(static_cast<int>(vScroll));

    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &context, 0x14u);
    nglDxInitShaders(false);

    const unsigned int vertexShader = static_cast<unsigned int>(cdSkyShaderRender::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    nglDxSetTexture(0, mMaterial->mTexture, 1u, 3u);
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

    const unsigned int* pixelShader = reinterpret_cast<const unsigned int*>(cdSkyShaderPixel::PS[0]);
    if (pixelShader != reinterpret_cast<const unsigned int*>(
            static_cast<uintptr_t>(gpuHashPixelShader))) {
        gpuHashPixelShader = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pixelShader));
        D3DDevice_SetPixelShaderProgram(reinterpret_cast<const _D3DPixelShaderDef*>(pixelShader));
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 0) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 0;
    }
    nglGpuDrawSection(Section);

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, oldZ) == 0)
        D3DDevice_SetRenderState_ZEnable(oldZ);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, oldZWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, oldZWrite);
        dword_BC2D10 = oldZWrite;
    }
    nglDxState.PrevBM = static_cast<unsigned int>(-1);
}
