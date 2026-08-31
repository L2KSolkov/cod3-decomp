// ============================================================================
// cdDynamicDecalShader.cpp — dynamic decal shader (6 non-inline funcs).
// Source: source/cdDynamicDecalShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDynamicDecalShader.o):
//   cdDynamicDecalShaderMat::ctor @0x7CBD80
//   InitCDDynamicDecalShader  @0x7CBE10
//   ToggleCDDynamicDecalShader @0x7CBE60
//   cdDynamicDecalShader::Register @0x7CBE80
//   cdDynamicDecalShader::AddNode @0x7CBE90
// ============================================================================
#include "cdDynamicDecalShader.h"
#include "ngl/ngl_lighting.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "render/ShaderCommon.h"
#include "render/ngl_aux.h"

extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);

#include <intrin.h>
#include <cstring>

// Shader global pointer definitions
cdDynamicDecalShader* gCDDynamicDecalShader = nullptr;  // ?gCDDynamicDecalShader@@3PAVcdDynamicDecalShader@@A

extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_40358;
extern unsigned int dword_4035C;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D10;
extern unsigned int dword_BC2D1C;
extern unsigned int dword_BC2D80;
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;
extern float gProjShadowSize;
extern float gProjShadowAlpha;
bool gProjShadowNoCulling = false; // ?gProjShadowNoCulling@@3_NA (render.o)

static math::Mat43 DynamicGetWToLNoScale(const nglMeshNode* node)
{
    math::Mat43 local;
    if (node->MeshParams != nullptr && (node->MeshParams->Flags & 2) != 0) {
        const __m128 scale = node->MeshParams->Scale.v;
        const __m128 reciprocal = _mm_rcp_ps(scale);
        const __m128 inverse = _mm_mul_ps(
            _mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(reciprocal, scale)), reciprocal);
        local.x.v = _mm_mul_ps(node->LocalToWorld.x.v, _mm_shuffle_ps(inverse, inverse, 0));
        local.y.v = _mm_mul_ps(node->LocalToWorld.y.v, _mm_shuffle_ps(inverse, inverse, 85));
        local.z.v = _mm_mul_ps(node->LocalToWorld.z.v, _mm_shuffle_ps(inverse, inverse, 170));
        local.w.v = node->LocalToWorld.w.v;
    } else {
        local = node->LocalToWorld;
    }
    const __m128 xy = _mm_shuffle_ps(local.x.v, local.y.v, 0x44);
    const __m128 xz = _mm_shuffle_ps(xy, local.z.v, 0xDD);
    const __m128 yz = _mm_shuffle_ps(_mm_shuffle_ps(local.x.v, local.y.v, 0xEE), local.z.v, 0xA8);
    const __m128 xx = _mm_shuffle_ps(xy, local.z.v, 0x88);
    math::Mat43 result;
    result.x.v = _mm_shuffle_ps(xx, _mm_setzero_ps(), 0xE4);
    result.y.v = xz;
    result.z.v = yz;
    result.w.v = _mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
        _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(local.w.v, local.w.v, 0), xx),
                              _mm_mul_ps(_mm_shuffle_ps(local.w.v, local.w.v, 85), xz)),
                   _mm_mul_ps(_mm_shuffle_ps(local.w.v, local.w.v, 170), yz)));
    return result;
}

static math::Mat43 DynamicGetWToL(const nglMeshNode* node)
{
    const __m128 x = node->LocalToWorld.x.v;
    const __m128 y = node->LocalToWorld.y.v;
    const __m128 z = node->LocalToWorld.z.v;
    const __m128 xy = _mm_shuffle_ps(x, y, 68);
    const __m128 axisZ = _mm_shuffle_ps(_mm_shuffle_ps(x, y, 238), z, 168);
    const __m128 axisX = _mm_shuffle_ps(xy, z, 136);
    const __m128 axisY = _mm_shuffle_ps(xy, z, 221);
    __m128 scaleSquared = _mm_set1_ps(1.0f);
    if (node->MeshParams != nullptr && (node->MeshParams->Flags & 2) != 0) {
        const __m128 reciprocal = _mm_rcp_ps(node->MeshParams->Scale.v);
        const __m128 inverse = _mm_mul_ps(
            _mm_sub_ps(_mm_set1_ps(2.0f),
                       _mm_mul_ps(reciprocal, node->MeshParams->Scale.v)), reciprocal);
        scaleSquared = _mm_mul_ps(inverse, inverse);
    }
    math::Mat43 result;
    result.x.v = _mm_mul_ps(axisX, scaleSquared);
    result.y.v = _mm_mul_ps(axisY, scaleSquared);
    result.z.v = _mm_mul_ps(axisZ, scaleSquared);
    const __m128 translation = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(node->LocalToWorld.w.v, node->LocalToWorld.w.v, 0), axisX),
                   _mm_mul_ps(_mm_shuffle_ps(node->LocalToWorld.w.v, node->LocalToWorld.w.v, 85), axisY)),
        _mm_mul_ps(_mm_shuffle_ps(node->LocalToWorld.w.v, node->LocalToWorld.w.v, 170), axisZ));
    result.w.v = _mm_mul_ps(
        _mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)), translation), scaleSquared);
    return result;
}

static void DynamicSetTextureWrap(unsigned int stage, unsigned int wrap,
                                   unsigned int dirtyBit, unsigned int* cacheU,
                                   unsigned int* cacheV)
{
    if (nglDxTexCache.Prev[stage].WrapU != wrap) {
        nglDxTexCache.Prev[stage].WrapU = wrap;
        if (D3DDevice_SetTextureState_ParameterCheck(stage, D3DTSS_ADDRESSU, wrap) == 0) {
            D3D__DirtyFlags |= dirtyBit;
            *cacheU = wrap;
        }
    }
    if (nglDxTexCache.Prev[stage].WrapV != wrap) {
        nglDxTexCache.Prev[stage].WrapV = wrap;
        if (D3DDevice_SetTextureState_ParameterCheck(stage, D3DTSS_ADDRESSV, wrap) == 0) {
            D3D__DirtyFlags |= dirtyBit;
            *cacheV = wrap;
        }
    }
}

void cdDynamicDecalRender::RegisterShader()
{
    for (int index = 0; index != 2; ++index) {
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(&cdDynamicDecalRender::VS[index]),
                             cdDynamicDecalRender::VShaderTable[index]);
    }
}

// ea: 0x007CD2E0
void cdDynamicDecalRender::RegisterVShader()
{
    cdDynamicDecalRender::RegisterShader();
}

unsigned int cdDynamicDecalRender::GetVShader(unsigned int index)
{
    return static_cast<unsigned int>(cdDynamicDecalRender::VS[index]);
}

cdDynamicDecalRender::cdDynamicDecalParams::cdDynamicDecalParams() {}

void cdDynamicDecalRender::SetConstants(
    const cdDynamicDecalRender::cdDynamicDecalParams& Params)
{
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &Params, 0x6Cu);
}

void cdDynamicDecalPixel::RegisterShader()
{
    for (int index = 0; index != 2; ++index) {
        nglDxRegisterPShader(&cdDynamicDecalPixel::PS[index],
                             cdDynamicDecalPixel::PShaderTable[index]);
    }
}

void cdDynamicDecalPixel::RegisterPShader()
{
    cdDynamicDecalPixel::RegisterShader();
}

unsigned long* cdDynamicDecalPixel::GetPShader(unsigned int index)
{
    return cdDynamicDecalPixel::PS[index];
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
// cdDynamicDecalShaderMat::cdDynamicDecalShaderMat — default material.
// ea: 0x7CBD80
// ============================================================================
cdDynamicDecalShaderMat::cdDynamicDecalShaderMat() {
    this->mTexture = NULL;
    this->mZbias = 0.0020000001f;
    this->mAlphaBlend = false;
    cdDynamicDecalShader* v2 = gCDDynamicDecalShader;
    if (gCDDynamicDecalShader != NULL) {
        this->Shader = v2;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdDynamicDecalShader.cpp";
    AeAssert::gCurrentLine = 18;
    AeAssert::gCurrentExpr = "gCDDynamicDecalShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDDynamicDecalShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
        __debugbreak();
        this->Shader = gCDDynamicDecalShader;
        return;
    }
    this->Shader = gCDDynamicDecalShader;
}

// ============================================================================
// InitCDDynamicDecalShader — allocate the shader and link into the init list.
// ea: 0x7CBE10
// ============================================================================
// ea: 0x007CD260
cdDynamicDecalShader::cdDynamicDecalShader() {
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    ShaderCommon::ShaderSwitching.__s0[0] &= ~2;
}

void InitCDDynamicDecalShader() {
    cdDynamicDecalShader* result = (cdDynamicDecalShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdDynamicDecalShader;
        gCDDynamicDecalShader = result;
    } else {
        gCDDynamicDecalShader = NULL;
    }
}

// ============================================================================
// ToggleCDDynamicDecalShader — toggle dynamic-decal enable bit (bit 2).
// ea: 0x7CBE60
// ============================================================================
void ToggleCDDynamicDecalShader() {
    unsigned char byte = *((unsigned char*)&gShaderSwitchingFlags);
    byte = (unsigned char)(((byte ^ (4 * ~(byte >> 2))) & 4) ^ byte);
    *((unsigned char*)&gShaderSwitchingFlags) = byte;
}

tlFixedString cdDynamicDecalShader::GetName() { return tlFixedString("cdDynamicDecal"); }

// ============================================================================
// cdDynamicDecalShader::Register — register the dynamic-decal shaders.
// ea: 0x7CBE80
// ============================================================================
void cdDynamicDecalShader::Register() {
    nglShader::Register();
    cdDynamicDecalRender::RegisterShader();
    cdDynamicDecalPixel::RegisterShader();
}

// ============================================================================
// cdDynamicDecalShader::AddNode — add a dynamic-decal node to the opaque list.
// ea: 0x7CBE90
// ============================================================================
void cdDynamicDecalShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                   nglMaterial* iMat) {
    if ((gShaderSwitchingFlags & 4) == 0) {
        cdDynamicDecalShaderNode* node = (cdDynamicDecalShaderNode*)nglListAlloc(0x1C, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // Release writes the node vtable directly here; the constructor
            // symbol exists in the object but is not called by AddNode.
            node->mMaterial = (cdDynamicDecalShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->Clip = 0;
        node->SortHash = gCDDynamicDecalShader->ID | 0x80000000;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

// ea: 0x007CBF10
void cdDynamicDecalShaderNode::Render()
{
    nglMeshNode* meshNode = this->MeshNode;
    nglMesh* mesh = meshNode->Mesh;
    alignas(16) unsigned char work[544] = {};
    math::Mat43 worldToLocal = DynamicGetWToL(meshNode);
    memcpy(work, &worldToLocal, sizeof(worldToLocal));
    math::Vector4 sphere;
    auxGetSphere(&sphere, mesh);
    math::Position3 sphereCenter;
    auxGetSphereCenter(&sphereCenter, mesh);
    const float sphereRadius = auxGetSphereRadius(mesh);
    mesh->Flags |= 0x07000000u;
    nglDetermineLights(meshNode);

    math::Position3 worldCenter;
    worldCenter.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 0), meshNode->LocalToWorld.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 85), meshNode->LocalToWorld.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 170), meshNode->LocalToWorld.z.v),
                   meshNode->LocalToWorld.w.v));

    unsigned int lightCount = 0;
    for (nglLightNode* light = nglSendLightContext->Head.LocalNext;
         light != reinterpret_cast<nglLightNode*>(nglSendLightContext) && lightCount < 4;
         light = light->LocalNext) {
        nglPointLightInfo info;
        nglPointLightInfo* point = nglGetLightAsPointLight(&info, light, worldCenter);
        if (point == nullptr)
            continue;
        math::Position3 localPos;
        localPos.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(point->Pos.v, point->Pos.v, 0), worldToLocal.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(point->Pos.v, point->Pos.v, 85), worldToLocal.y.v)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(point->Pos.v, point->Pos.v, 170), worldToLocal.z.v),
                       worldToLocal.w.v));
        *reinterpret_cast<__m128*>(work + 368 + lightCount * 16) = _mm_setr_ps(
            localPos.v.m128_f32[0], localPos.v.m128_f32[1],
            localPos.v.m128_f32[2], point->Far);
        *reinterpret_cast<__m128*>(work + 432 + lightCount * 16) = _mm_setr_ps(
            point->Color.v.m128_f32[0], point->Color.v.m128_f32[1],
            point->Color.v.m128_f32[2], 0.0f);
        reinterpret_cast<float*>(work + 512)[lightCount] =
            1.0f / (point->Far - point->Near);
        reinterpret_cast<float*>(work + 496)[lightCount] =
            reinterpret_cast<float*>(work + 512)[lightCount] * point->Far;
        ++lightCount;
    }
    while (lightCount < 4) {
        memset(work + 368 + lightCount * 16, 0, 16);
        memset(work + 432 + lightCount * 16, 0, 16);
        reinterpret_cast<float*>(work + 512)[lightCount] = 1.0f;
        reinterpret_cast<float*>(work + 496)[lightCount] = 0.0f;
        ++lightCount;
    }

    // The release transposes the four collected light colors into the
    // register-major layout consumed by the dynamic-decal pixel shader.
    const __m128 color0 = *reinterpret_cast<__m128*>(work + 432);
    const __m128 color1 = *reinterpret_cast<__m128*>(work + 448);
    const __m128 color2 = *reinterpret_cast<__m128*>(work + 464);
    const __m128 color3 = *reinterpret_cast<__m128*>(work + 480);
    const __m128 colorX = _mm_shuffle_ps(color0, color1, 68);
    const __m128 colorY = _mm_shuffle_ps(color0, color1, 238);
    const __m128 colorZ = _mm_shuffle_ps(color2, color3, 68);
    const __m128 colorW = _mm_shuffle_ps(color2, color3, 238);
    *reinterpret_cast<__m128*>(work + 432) = _mm_shuffle_ps(colorX, colorZ, 136);
    *reinterpret_cast<__m128*>(work + 448) = _mm_shuffle_ps(colorX, colorZ, 221);
    *reinterpret_cast<__m128*>(work + 464) = _mm_shuffle_ps(colorY, colorW, 136);
    *reinterpret_cast<__m128*>(work + 480) = _mm_shuffle_ps(colorY, colorW, 221);

    nglTexture* projectedTexture = nullptr;
    if (nglSendLightContext->ProjHead.Next[0] != &nglSendLightContext->ProjHead) {
        nglLightNode* projected = nglSendLightContext->ProjHead.Next[0];
        unsigned char* data = reinterpret_cast<unsigned char*>(projected->NodeData);
        __m128 projectedCenter = *reinterpret_cast<__m128*>(data + 40);
        __m128 delta = _mm_sub_ps(
            _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 0), meshNode->LocalToWorld.x.v),
                                  _mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 85), meshNode->LocalToWorld.y.v)),
                       _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 170), meshNode->LocalToWorld.z.v),
                                  meshNode->LocalToWorld.w.v)), projectedCenter);
        const float limit = gProjShadowSize + sphereRadius;
        const __m128 sq = _mm_mul_ps(delta, delta);
        const float distanceSquared = sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
        if (gProjShadowNoCulling || distanceSquared <= limit * limit) {
            const __m128 projectorX = *reinterpret_cast<__m128*>(data + 0);
            const __m128 projectorY = *reinterpret_cast<__m128*>(data + 16);
            const __m128 projectorZ = *reinterpret_cast<__m128*>(data + 32);
            const __m128 projectorW = *reinterpret_cast<__m128*>(data + 48);
            const __m128 projectorXY = _mm_shuffle_ps(projectorX, projectorY, 68);
            const __m128 projectorZW = _mm_shuffle_ps(projectorZ, projectorW, 68);
            *reinterpret_cast<__m128*>(work + 176) = _mm_shuffle_ps(projectorXY, projectorZW, 136);
            *reinterpret_cast<__m128*>(work + 192) = _mm_shuffle_ps(projectorXY, projectorZW, 221);
            *reinterpret_cast<__m128*>(work + 208) = _mm_shuffle_ps(
                _mm_shuffle_ps(projectorX, projectorY, 238),
                _mm_shuffle_ps(projectorZ, projectorW, 238), 136);
            *reinterpret_cast<__m128*>(work + 224) = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
            projectedTexture = *reinterpret_cast<nglTexture**>(data + 132);
        }
    }

    const unsigned int oldZWrite = dword_BC2D10;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 0);
        dword_BC2D00 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0);
        dword_BC2D04 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x204u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, 0x204u);
        dword_BC2CF8 = 0x204u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_COLORWRITEENABLE, 0x1010101u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40358, 0x1010101u);
        dword_BC2D1C = 0x1010101u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZBIAS, 0xCu) == 0)
        D3DDevice_SetRenderState_ZBias(0xCu);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }
    if (this->mMaterial->mAlphaBlend)
        nglDxState.SetBlendMode(0x64CF8600u);
    else if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 0);
        dword_BC2CFC = 0;
    }

    nglDxSetTexture(0, this->mMaterial->mTexture, 1u, 3u);
    DynamicSetTextureWrap(0, 1, 1u, &D3D__TextureState[0][D3DTSS_ADDRESSU], &D3D__TextureState[0][D3DTSS_ADDRESSV]);
    if (projectedTexture != nullptr) {
        nglDxSetTexture(1, projectedTexture, 1u, 3u);
        DynamicSetTextureWrap(1, 4, 2u,
                              &D3D__TextureState[1][D3DTSS_ADDRESSU],
                              &D3D__TextureState[1][D3DTSS_ADDRESSV]);
        if (D3DDevice_SetTextureState_ParameterCheck(1, D3DTSS_BORDERCOLOR, 0) == 0) {
            D3D__DirtyFlags |= 2u;
            D3D__TextureState[1][D3DTSS_BORDERCOLOR] = 0;
        }
    }

    const unsigned int shaderIndex = projectedTexture != nullptr ? 1u : 0u;
    nglDxInitShaders(false);
    const unsigned int vertexShader = static_cast<unsigned int>(cdDynamicDecalRender::VS[shaderIndex]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const _D3DPixelShaderDef* pixelShader = reinterpret_cast<const _D3DPixelShaderDef*>(
        cdDynamicDecalPixel::PS[shaderIndex]);
    if (pixelShader != reinterpret_cast<const _D3DPixelShaderDef*>(static_cast<uintptr_t>(gpuHashPixelShader))) {
        gpuHashPixelShader = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pixelShader));
        D3DDevice_SetPixelShaderProgram(pixelShader);
    }

    memcpy(work + 112, &meshNode->LocalToScreen, sizeof(math::Mat44));
    *reinterpret_cast<float*>(work + 240) = gProjShadowAlpha;
    float* fog = reinterpret_cast<float*>(work + 256);
    fog[0] = nglBuildScene->FogColor.v.m128_f32[0];
    fog[1] = nglBuildScene->FogColor.v.m128_f32[1];
    fog[2] = nglBuildScene->FogColor.v.m128_f32[2];
    fog[3] = nglBuildScene->FogColor.v.m128_f32[3];
    float* fogParams = reinterpret_cast<float*>(work + 272);
    fogParams[0] = nglBuildScene->FogMin;
    fogParams[1] = nglBuildScene->FogNear;
    fogParams[2] = 1.0f / (nglBuildScene->FogFar - nglBuildScene->FogNear);
    fogParams[3] = nglBuildScene->FogMax - nglBuildScene->FogMin;
    math::Mat43 worldToLocalNoScale = DynamicGetWToLNoScale(meshNode);
    __m128 eye = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(nglBuildScene->ViewToWorld.w.v, nglBuildScene->ViewToWorld.w.v, 0), worldToLocalNoScale.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(nglBuildScene->ViewToWorld.w.v, nglBuildScene->ViewToWorld.w.v, 85), worldToLocalNoScale.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(nglBuildScene->ViewToWorld.w.v, nglBuildScene->ViewToWorld.w.v, 170), worldToLocalNoScale.z.v),
                   worldToLocalNoScale.w.v));
    eye.m128_f32[3] = 1.0f;
    *reinterpret_cast<__m128*>(work + 288) = eye;
    *reinterpret_cast<__m128*>(work + 96) = _mm_setr_ps(0.5f, 2.0f, 0.0f, 0.0f);
    const __m128 fogScaled = _mm_mul_ps(nglBuildScene->FogColor.v, _mm_set1_ps(127.0f));
    const unsigned int fogColor = static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[2])) |
        (static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[1])) << 8) |
        (static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[0])) << 16) |
        (static_cast<unsigned int>(static_cast<int>(fogScaled.m128_f32[3])) << 24);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_FOGCOLOR, fogColor) == 0)
        D3DDevice_SetRenderState_FogColor(fogColor);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 1u) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 1;
    }
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, work + 96, 0x6Cu);
    nglGpuDrawSection(this->Section);
    const unsigned int fbWriteMask = nglBuildScene->FBWriteMask;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_COLORWRITEENABLE, fbWriteMask) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40358, fbWriteMask);
        dword_BC2D1C = fbWriteMask;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZBIAS, 0) == 0)
        D3DDevice_SetRenderState_ZBias(0);
    nglDxState.PrevBM = static_cast<unsigned int>(-1);
    const unsigned int zWrite = oldZWrite != 0;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, zWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, zWrite);
        dword_BC2D10 = zWrite;
    }
}

cdDynamicDecalShader::~cdDynamicDecalShader() = default;
