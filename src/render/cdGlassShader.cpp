// ============================================================================
// cdGlassShader.cpp — glass shader (4 non-inline funcs).
// Source: source/cdGlassShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGlassShader.o):
//   InitCDGlassShader  @0x7CFF90
//   ToggleCDGlassShader @0x7CFFE0
//   cdGlassShader::Register @0x7D0000
//   cdGlassShader::AddNode @0x7D0970 (inline COMDAT)
// ============================================================================
#include "cdGlassShader.h"

nglTexture* gProjShadowTex = nullptr;  // ?gProjShadowTex@@3PAUnglTexture@@A (render.o @ 0x1363930)

#include <intrin.h>

// Shader global pointer definitions
cdGlassShader* gCDGlassShader = nullptr;  // ?gCDGlassShader@@3PAVcdGlassShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdGlassRender {
    unsigned long VS[2][2] = {};
    unsigned int const* VShaderTable[2][2] = {};
}
namespace cdGlassPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
}
namespace cdGlassSolidColorPixel {
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = {};
}

// ea: 0x7D0810
void cdGlassRender::RegisterVShader() {
    for (int index = 0; index < 4; ++index)
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(VS) + index,
                             VShaderTable[0][index]);
}

// ea: 0x7D0840
unsigned int cdGlassRender::GetVShader(unsigned int param0,
                                       unsigned int param1) {
    const unsigned int* shaders = reinterpret_cast<const unsigned int*>(VS);
    return shaders[2 * param0 + param1];
}

// ea: 0x7D0860
void cdGlassPixel::RegisterPShader() {
    for (int index = 0; index < 2; ++index)
        nglDxRegisterPShader(&PS[index], PShaderTable[index]);
}

// ea: 0x7D0890
unsigned int* cdGlassPixel::GetPShader(unsigned int index) {
    return reinterpret_cast<unsigned int*>(PS[index]);
}

// ea: 0x7D08A0
void cdGlassSolidColorPixel::RegisterPShader() {
    nglDxRegisterPShader(PS, PShaderTable[0]);
}

// ea: 0x7D0930
cdGlassRender::Params::Params() {}

// ea: 0x7D0940
nglDirLightInfo::nglDirLightInfo() {}

// ea: 0x7D0950
template <typename T>
void cdGlassRender::SetConstants(const T& params) {
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &params, 0x28u);
}

template void cdGlassRender::SetConstants<cdGlassRender::Params>(
    const cdGlassRender::Params& params);

// ea: 0x7D0600
const math::Mat44& math::Mat44::operator=(const math::Mat43& matrix) {
    x.v = _mm_shuffle_ps(matrix.x.v, _mm_shuffle_ps(_mm_setzero_ps(), matrix.x.v, 160), 52);
    y.v = _mm_shuffle_ps(matrix.y.v, _mm_shuffle_ps(_mm_setzero_ps(), matrix.y.v, 160), 52);
    z.v = _mm_shuffle_ps(matrix.z.v, _mm_shuffle_ps(_mm_setzero_ps(), matrix.z.v, 160), 52);
    w.v = _mm_shuffle_ps(matrix.w.v, _mm_shuffle_ps(_mm_set_ss(1.0f), matrix.w.v, 160), 52);
    return *this;
}

// ea: 0x7D0680
void gpuSetPixelConstant(unsigned int idx, math::Vector4* data,
                         unsigned int nelements) {
    D3DDevice_SetPixelShaderConstant(idx, data, nelements);
}

// ea: 0x7D06A0
math::Mat43 nglMeshNode::GetLToV(const math::Mat43& WorldToView) const {
    math::Mat43 result;
    const __m128 viewX = WorldToView.x.v;
    const __m128 viewY = WorldToView.y.v;
    const __m128 viewZ = WorldToView.z.v;
    const __m128 localY = LocalToWorld.y.v;
    const __m128 localZ = LocalToWorld.z.v;
    const __m128 localX = LocalToWorld.x.v;
    const __m128 translation = LocalToWorld.w.v;
    const __m128 localYView = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(localY, localY, 0), viewX),
                   _mm_mul_ps(_mm_shuffle_ps(localY, localY, 85), viewY)),
        _mm_mul_ps(_mm_shuffle_ps(localY, localY, 170), viewZ));
    const __m128 localZView = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(localZ, localZ, 0), viewX),
                   _mm_mul_ps(_mm_shuffle_ps(localZ, localZ, 85), viewY)),
        _mm_mul_ps(_mm_shuffle_ps(localZ, localZ, 170), viewZ));
    result.x.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(localX, localX, 0), viewX),
                   _mm_mul_ps(_mm_shuffle_ps(localX, localX, 85), viewY)),
        _mm_mul_ps(_mm_shuffle_ps(localX, localX, 170), viewZ));
    result.y.v = localYView;
    result.z.v = localZView;
    result.w.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(translation, translation, 0), viewX),
                   _mm_mul_ps(_mm_shuffle_ps(translation, translation, 85), viewY)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(translation, translation, 170), viewZ),
                   WorldToView.w.v));
    return result;
}

// ============================================================================
// InitCDGlassShader — allocate the shader and link into the init list.
// ea: 0x7CFF90
// ============================================================================
void InitCDGlassShader() {
    cdGlassShader* result = (cdGlassShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdGlassShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdGlassShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~0x40;
        gCDGlassShader = result;
    } else {
        gCDGlassShader = NULL;
    }
}

// ============================================================================
// ToggleCDGlassShader — toggle glass-shader enable bit (bit 6).
// ea: 0x7CFFE0
// ============================================================================
void ToggleCDGlassShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (~(byte >> 6) << 6)) & 0x40) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;
}

tlFixedString cdGlassShader::GetName() { return tlFixedString("cdGlass"); }

// ea: 0x7D0910
void cdGlassShader::BindMaterial(nglMaterial* material) {
    cdGlassShaderMat* glass = reinterpret_cast<cdGlassShaderMat*>(material);
    if (glass->mEnvironment != nullptr && glass->mEnvironment != nglDefaultTex)
        glass->mFlags |= 2;
}

// ============================================================================
// cdGlassShader::Register — register the glass vertex/pixel shaders.
// ea: 0x7D0000
// ============================================================================
void cdGlassShader::Register() {
    nglShader::Register();
    cdGlassRender::RegisterVShader();
    cdGlassPixel::RegisterPShader();
    cdGlassSolidColorPixel::RegisterPShader();
}

// ============================================================================
// cdGlassShader::AddNode — add a glass node to the transparent list.
// ea: 0x7D0970 (inline COMDAT)
// ============================================================================
void cdGlassShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if (nglBuildScene->RenderTarget != gProjShadowTex &&
        (ShaderCommon::ShaderSwitching.__s0[3] & 0x40) == 0) {
        cdGlassShaderNode* node = (cdGlassShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdGlassShaderNode
            node->mMaterial = (cdGlassShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortDist = node->GetDist(nglBuildScene->WorldToView);
        node->Next = nglBuildScene->TransRenderList;
        nglBuildScene->TransRenderList = node;
        ++nglBuildScene->TransListCount;
    }
}
