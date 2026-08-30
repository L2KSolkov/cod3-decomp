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

// Shader static data definitions are restored in cdWorldShaderData.cpp.
// ============================================================================
// cdWorldRender::RegisterShader — register the 4 world vertex shaders.
// ea: 0x7DFFE0 (inline COMDAT)
// ============================================================================
void cdWorldRender::RegisterShader() {
    for (int index = 0; index != 4; ++index)
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(&cdWorldRender::VS[0][index]),
                             cdWorldRender::VShaderTable[index]);
}

void cdWorldRender_RegisterShader() {
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
void cdWorldProjectedRender::RegisterShader() {
    for (int index = 0; index != 2; ++index)
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(&cdWorldProjectedRender::VS[index]),
                             cdWorldProjectedRender::VShaderTable[index]);
}

void cdWorldProjectedRender_RegisterShader() {
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldProjectedRender::VS[v0], cdWorldProjectedRender::VShaderTable, v0);
    }
}

// ea: 0x007E0010
void cdWorldRender::RegisterVShader()
{
    cdWorldRender::RegisterShader();
}

// ea: 0x007E0070
void cdWorldProjectedRender::RegisterVShader()
{
    cdWorldProjectedRender::RegisterShader();
}

// ea: 0x007E0020
unsigned long cdWorldRender::GetVShader(unsigned int dynamicLights, unsigned int colorVerts)
{
    return cdWorldRender::VS[dynamicLights][colorVerts];
}

// ea: 0x007E0080
unsigned long cdWorldProjectedRender::GetVShader(unsigned int index)
{
    return cdWorldProjectedRender::VS[index];
}

// ============================================================================
// cdWorldPixel::RegisterShader — register the 8 world pixel shaders.
// ea: 0x7E0090 (inline COMDAT)
// ============================================================================
// ea: 0x007E0090
void cdWorldPixel::RegisterShader() {
    for (int v0 = 0, i = 8; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldPixel::PS[0][0][v0], cdWorldPixel::PShaderTable[0][0], v0);
    }
}

// ea: 0x007E00C0
void cdWorldPixel::RegisterPShader() {
    cdWorldPixel::RegisterShader();
}

// ea: 0x007E00D0
unsigned long* cdWorldPixel::GetPShader(unsigned int a, unsigned int b, unsigned int c) {
    return cdWorldPixel::PS[a][b][c];
}

// ============================================================================
// cdWorldProjectedPixel::RegisterShader — register the 2 projected PShaders.
// ea: 0x7E00F0 (inline COMDAT)
// ============================================================================
// ea: 0x007E00F0
void cdWorldProjectedPixel::RegisterShader() {
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldProjectedPixel::PS[v0], cdWorldProjectedPixel::PShaderTable, v0);
    }
}

// ea: 0x007E0120
void cdWorldProjectedPixel::RegisterPShader() {
    cdWorldProjectedPixel::RegisterShader();
}

// ea: 0x007E0130
unsigned long* cdWorldProjectedPixel::GetPShader(unsigned int index) {
    return cdWorldProjectedPixel::PS[index];
}

// ea: 0x007E0140
void cdWorldSolidColorPixel::RegisterShader() {
    nglDxRegisterPShader(cdWorldSolidColorPixel::PS,
                         cdWorldSolidColorPixel::PShaderTable[0]);
    cdWorldSolidColorPixel::Shader = cdWorldSolidColorPixel::PS[0];
}

// ea: 0x007E0160
void cdWorldSolidColorPixel::RegisterPShader() {
    cdWorldSolidColorPixel::RegisterShader();
}

// ea: 0x007E0180
unsigned long* cdWorldSolidColorPixel::GetPShader() {
    return cdWorldSolidColorPixel::PS[0];
}

// ea: 0x007E0190
cdWorldShader::cdWorldShader()
{
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    ShaderCommon::ShaderSwitching.__s0[0] &= ~2;
}

// ea: 0x007E0240
cdWorldShader::~cdWorldShader() = default;

cdWorldRender::cdWorldParams::cdWorldParams() {}

// ea: 0x007E01F0
template <typename T>
void cdWorldRender::SetConstants(const T& params)
{
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &params, 0x44u);
}

template void cdWorldRender::SetConstants<cdWorldRender::cdWorldParams>(
    const cdWorldRender::cdWorldParams& params);

// ============================================================================
// InitCDWorldShader — allocate the shader and link into the init list.
// ea: 0x7DF0D0
// ============================================================================
void InitCDWorldShader() {
    cdWorldShader* result = (cdWorldShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdWorldShader;
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

// ea: 0x007E01C0
tlFixedString cdWorldShader::GetName() { return tlFixedString("cdWorld"); }

// ============================================================================
// cdWorldShader::Register — register all world vertex/pixel shaders.
// ea: 0x7DF140
// ============================================================================
void cdWorldShader::Register() {
    nglShader::Register();
    cdWorldRender_RegisterShader();
    cdWorldProjectedRender_RegisterShader();
    cdWorldPixel::RegisterShader();
    cdWorldProjectedPixel::RegisterShader();
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
                ::new (node) cdWorldShaderNode(iMeshNode, iSection,
                                                (cdWorldShaderMat*)iMat, false);
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
