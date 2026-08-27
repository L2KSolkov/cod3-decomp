// ============================================================================
// apsColorUVANode.cpp — color UVA node Render forwarder (1 func).
// Source: source/apsColorUVANode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorUVANode.o):
//   apsColorUVANode::Render @0x81AFF0
// ============================================================================
#include "apsColorUVARenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"
#include "ngl/ngl_gpu_debug.h"

extern void nglDxInitShaders(bool registerShaders);

namespace {

static __m128 TransformRowToScreen(const __m128 row, const nglScene* scene)
{
    return _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(row, row, 0), scene->WorldToScreen.x.v),
            _mm_mul_ps(_mm_shuffle_ps(row, row, 85), scene->WorldToScreen.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(row, row, 170), scene->WorldToScreen.z.v),
            _mm_mul_ps(_mm_shuffle_ps(row, row, 255), scene->WorldToScreen.w.v)));
}

static void BuildLocalToScreen(const math::Mat43& localToWorld,
                               const nglScene* scene, math::Mat43& out)
{
    const __m128 row0 = TransformRowToScreen(localToWorld.x.v, scene);
    const __m128 row1 = TransformRowToScreen(localToWorld.y.v, scene);
    const __m128 row2 = TransformRowToScreen(localToWorld.z.v, scene);
    const __m128 row3 = TransformRowToScreen(localToWorld.w.v, scene);
    const __m128 x01 = _mm_shuffle_ps(row0, row1, 68);
    const __m128 x23 = _mm_shuffle_ps(row0, row1, 238);
    const __m128 y01 = _mm_shuffle_ps(row2, row3, 68);
    const __m128 y23 = _mm_shuffle_ps(row2, row3, 238);
    out.x.v = _mm_shuffle_ps(x01, y01, 136);
    out.y.v = _mm_shuffle_ps(x01, y01, 221);
    out.z.v = _mm_shuffle_ps(x23, y23, 136);
    out.w.v = _mm_shuffle_ps(x23, y23, 221);
}

}

// cNodeRenderer<ColorUVAParticle,apsColorUVANode>::cNodeRenderer - ea: 0x0081B0A0
template <>
cNodeRenderer<ColorUVAParticle, apsColorUVANode>::cNodeRenderer(
    apsColorUVANode* node)
    : mNode(node)
{
}

// cNodeRenderer<ColorUVAParticle,apsColorUVANode>::SetupDefaultShaders - ea: 0x0081B0B0
template <>
void cNodeRenderer<ColorUVAParticle, apsColorUVANode>::SetupDefaultShaders(
    const math::Mat43& localMatrix)
{
    math::Mat43 localToScreen;
    BuildLocalToScreen(localMatrix, nglBuildScene, localToScreen);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &localToScreen, 0x10u);
    nglDxInitShaders(false);
    if (apsColorUVARender::VS[0] != gpuHashVertexShader) {
        gpuHashVertexShader = apsColorUVARender::VS[0];
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(apsColorUVARender::VS), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(apsColorUVARenderPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(apsColorUVARenderPixel::PS[0]));
    }
}

// cNodeRenderer<ColorUVAParticle,apsColorUVANode>::SetupShaders - ea: 0x0081B3A0
template <>
void cNodeRenderer<ColorUVAParticle, apsColorUVANode>::SetupShaders()
{
    SetupDefaultShaders(mNode->mLocalToWorld);
}

namespace apsRenderSort {

// fncompare<ColorUVAParticle> - ea: 0x0081B360
template <>
int fncompare<ColorUVAParticle>(const void* elem1, const void* elem2)
{
    const ColorUVAParticle* p1 =
        *static_cast<const ColorUVAParticle* const*>(elem1);
    const ColorUVAParticle* p2 =
        *static_cast<const ColorUVAParticle* const*>(elem2);
    if (p2->mAge <= p1->mAge)
        return p1->mAge > p2->mAge;
    return -1;
}

// SortPointers<ColorUVAParticle> - ea: 0x0081B3B0
template <>
void SortPointers<ColorUVAParticle>(Buffer* sortBuffer,
                                    unsigned char* firstParticle,
                                    unsigned int stride,
                                    unsigned int numParticles)
{
    unsigned int capacity = numParticles;
    if (sortBuffer->capacity <= numParticles)
        capacity = sortBuffer->capacity;
    for (unsigned int i = 0; i < capacity; ++i) {
        sortBuffer->buffer[i] = firstParticle;
        firstParticle += stride;
    }
    qsort(sortBuffer->buffer, capacity, sizeof(unsigned char*),
          &fncompare<ColorUVAParticle>);
}

}

// apsColorUVARender::GetVShader - ea: 0x0081B010
unsigned long apsColorUVARender::GetVShader()
{
    return VS[0];
}

// apsColorUVARenderPixel::GetPShader - ea: 0x0081B020
unsigned long* apsColorUVARenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsColorUVANode::Render — forward to the color UVA renderer.
// ea: 0x81AFF0
// ============================================================================
void apsColorUVANode::Render() {
    cNodeRenderer<ColorUVAParticle, apsColorUVANode> renderer(this);
    renderer.Render();
}
