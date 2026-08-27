// ============================================================================
// apsColorRectangleNode.cpp — color rectangle node Render forwarder (1 func).
// Source: source/apsColorRectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorRectangleNode.o):
//   apsColorRectangleNode::Render @0x815150
// ============================================================================
#include "apsColorRectangleRenderer.h"
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

// cNodeRenderer<ColorRectangleParticle,apsColorRectangleNode>::cNodeRenderer - ea: 0x00815200
template <>
cNodeRenderer<ColorRectangleParticle, apsColorRectangleNode>::cNodeRenderer(
    apsColorRectangleNode* node)
    : mNode(node)
{
}

// cNodeRenderer<ColorRectangleParticle,apsColorRectangleNode>::SetupDefaultShaders - ea: 0x00815210
template <>
void cNodeRenderer<ColorRectangleParticle, apsColorRectangleNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix)
{
    math::Mat43 localToScreen;
    BuildLocalToScreen(localMatrix, nglBuildScene, localToScreen);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &localToScreen, 0x10u);
    nglDxInitShaders(false);

    const unsigned int vertexShader = apsColorRectangleRender::VS[0];
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(apsColorRectangleRender::VS), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(apsColorRectangleRenderPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(
                apsColorRectangleRenderPixel::PS[0]));
    }
}

// cNodeRenderer<ColorRectangleParticle,apsColorRectangleNode>::SetupShaders - ea: 0x00815500
template <>
void cNodeRenderer<ColorRectangleParticle, apsColorRectangleNode>::SetupShaders()
{
    SetupDefaultShaders(mNode->mLocalToWorld);
}

namespace apsRenderSort {

// fncompare<ColorRectangleParticle> - ea: 0x008154C0
template <>
int fncompare<ColorRectangleParticle>(const void* elem1, const void* elem2)
{
    const ColorRectangleParticle* p1 =
        *static_cast<const ColorRectangleParticle* const*>(elem1);
    const ColorRectangleParticle* p2 =
        *static_cast<const ColorRectangleParticle* const*>(elem2);
    if (p2->mAge <= p1->mAge)
        return p1->mAge > p2->mAge;
    return -1;
}

// SortPointers<ColorRectangleParticle> - ea: 0x00815510
template <>
void SortPointers<ColorRectangleParticle>(Buffer* sortBuffer,
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
          &fncompare<ColorRectangleParticle>);
}

}

// apsColorRectangleRender::GetVShader - ea: 0x00815170
unsigned long apsColorRectangleRender::GetVShader()
{
    return VS[0];
}

// apsColorRectangleRenderPixel::GetPShader - ea: 0x00815180
unsigned long* apsColorRectangleRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsColorRectangleNode::Render — forward to the color rectangle renderer.
// ea: 0x815150
// ============================================================================
void apsColorRectangleNode::Render() {
    cNodeRenderer<ColorRectangleParticle, apsColorRectangleNode> renderer(this);
    renderer.Render();
}
