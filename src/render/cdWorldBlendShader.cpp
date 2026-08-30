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

extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);
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

// Shader static data definitions are restored in cdWorldBlendShaderData.cpp.
void cdWorldBlendRender::RegisterShader()
{
    for (int index = 0; index != 2; ++index) {
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(&cdWorldBlendRender::VS[index]),
                             cdWorldBlendRender::VShaderTable[index]);
    }
}

// ea: 0x007DE200
void cdWorldBlendRender::RegisterVShader()
{
    cdWorldBlendRender::RegisterShader();
}

void cdWorldBlendProjectedRender::RegisterShader()
{
    for (int index = 0; index != 2; ++index) {
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(&cdWorldBlendProjectedRender::VS[index]),
                             cdWorldBlendProjectedRender::VShaderTable[index]);
    }
}

// ea: 0x007DE250
void cdWorldBlendProjectedRender::RegisterVShader()
{
    cdWorldBlendProjectedRender::RegisterShader();
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

tlFixedString cdWorldBlendShader::GetName() { return tlFixedString("cdWorldBlend"); }

// ============================================================================
// cdWorldBlendShader::Register — register the world-blend shaders.
// ea: 0x7DD310
// ============================================================================
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
            ::new (node) cdWorldBlendShaderNode;
            node->MeshNode = iMeshNode;
            node->Section = iSection;
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
