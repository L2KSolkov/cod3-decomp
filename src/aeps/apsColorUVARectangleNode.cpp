// ============================================================================
// apsColorUVARectangleNode.cpp — color UVA rect node Render forwarder (1).
// Source: source/apsColorUVARectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorUVARectangleNode.o):
//   apsColorUVARectangleNode::Render @0x817710
// ============================================================================
#include "apsColorUVARectangleRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"
#include "apsUVARectangleRenderCommon.h"
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

// cNodeRenderer<ColorUVARectangleParticle,apsColorUVARectangleNode>::cNodeRenderer - ea: 0x008177C0
template <>
cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode>::cNodeRenderer(
    apsColorUVARectangleNode* node)
    : mNode(node)
{
}

// cNodeRenderer<ColorUVARectangleParticle,apsColorUVARectangleNode>::SetupDefaultShaders - ea: 0x008177D0
template <>
void cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix)
{
    math::Mat43 localToScreen;
    BuildLocalToScreen(localMatrix, nglBuildScene, localToScreen);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &localToScreen, 0x10u);
    nglDxInitShaders(false);
    if (apsColorUVARectangleRender::VS[0] != gpuHashVertexShader) {
        gpuHashVertexShader = apsColorUVARectangleRender::VS[0];
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(apsColorUVARectangleRender::VS), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(apsColorUVARectangleRenderPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(apsColorUVARectangleRenderPixel::PS[0]));
    }
}

// cNodeRenderer<ColorUVARectangleParticle,apsColorUVARectangleNode>::SetupShaders - ea: 0x00817AC0
template <>
void cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode>::SetupShaders()
{
    SetupDefaultShaders(mNode->mLocalToWorld);
}

namespace apsRenderSort {

// fncompare<ColorUVARectangleParticle> - ea: 0x00817A80
template <>
int fncompare<ColorUVARectangleParticle>(const void* elem1, const void* elem2)
{
    const ColorUVARectangleParticle* p1 =
        *static_cast<const ColorUVARectangleParticle* const*>(elem1);
    const ColorUVARectangleParticle* p2 =
        *static_cast<const ColorUVARectangleParticle* const*>(elem2);
    if (p2->mFrame <= p1->mFrame)
        return p1->mFrame > p2->mFrame;
    return -1;
}

// SortPointers<ColorUVARectangleParticle> - ea: 0x00817AD0
template <>
void SortPointers<ColorUVARectangleParticle>(Buffer* sortBuffer,
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
          &fncompare<ColorUVARectangleParticle>);
}

}

// apsColorUVARectangleRender::GetVShader - ea: 0x00817730
unsigned long apsColorUVARectangleRender::GetVShader()
{
    return VS[0];
}

// apsColorUVARectangleRenderPixel::GetPShader - ea: 0x00817740
unsigned long* apsColorUVARectangleRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsColorUVARectangleNode::Render — forward to the color UVA rect renderer.
// ea: 0x817710
// ============================================================================
void apsColorUVARectangleNode::Render() {
    cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode> renderer(this);
    renderer.Render();
}

// cNodeRenderer<ColorUVARectangleParticle,apsColorUVARectangleNode>::Render - ea: 0x00817B10
template <>
void cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode>::Render()
{
    apsUVARectangleRenderCommon::Render(this);
}
