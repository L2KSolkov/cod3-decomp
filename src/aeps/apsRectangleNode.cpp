// ============================================================================
// apsRectangleNode.cpp — rectangle node Render forwarder (1 func).
// Source: source/apsRectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsRectangleNode.o):
//   apsRectangleNode::Render @0x819D30
// ============================================================================
#include "apsRectangleRenderer.h"
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

// cNodeRenderer<RectangleParticle,apsRectangleNode>::cNodeRenderer - ea: 0x00819E20
template <>
cNodeRenderer<RectangleParticle, apsRectangleNode>::cNodeRenderer(
    apsRectangleNode* node)
    : mNode(node)
{
}

// cNodeRenderer<RectangleParticle,apsRectangleNode>::SetupDefaultShaders - ea: 0x00819E30
template <>
void cNodeRenderer<RectangleParticle, apsRectangleNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix)
{
    math::Mat43 localToScreen;
    BuildLocalToScreen(localMatrix, nglBuildScene, localToScreen);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &localToScreen, 0x10u);
    nglDxInitShaders(false);

    const unsigned int vertexShader = apsRectangleRender::VS[0];
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(apsRectangleRender::VS), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(apsRectangleRenderPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(
                apsRectangleRenderPixel::PS[0]));
    }
}

// cNodeRenderer<RectangleParticle,apsRectangleNode>::SetupShaders - ea: 0x0081A120
template <>
void cNodeRenderer<RectangleParticle, apsRectangleNode>::SetupShaders()
{
    SetupDefaultShaders(mNode->mLocalToWorld);
}

namespace apsRenderSort {

// fncompare<RectangleParticle> - ea: 0x0081A0E0
template <>
int fncompare<RectangleParticle>(const void* elem1, const void* elem2)
{
    const RectangleParticle* p1 =
        *static_cast<const RectangleParticle* const*>(elem1);
    const RectangleParticle* p2 =
        *static_cast<const RectangleParticle* const*>(elem2);
    if (p2->mAge <= p1->mAge)
        return p1->mAge > p2->mAge;
    return -1;
}

// SortPointers<RectangleParticle> - ea: 0x0081A130
template <>
void SortPointers<RectangleParticle>(Buffer* sortBuffer,
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
          &fncompare<RectangleParticle>);
}

}

apsRectangleNode::apsRectangleNode()
    : apsBillboardNode()
{
    mFlags = 0;
}

// apsRectangleRender::GetVShader - ea: 0x00819D50
unsigned long apsRectangleRender::GetVShader()
{
    return apsRectangleRender::VS[0];
}

// apsRectangleRenderPixel::GetPShader - ea: 0x00819D60
unsigned long* apsRectangleRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(apsRectangleRenderPixel::PS[0]);
}

// ============================================================================
// apsRectangleNode::Render — forward to the rectangle node renderer.
// ea: 0x819D30
// ============================================================================
void apsRectangleNode::Render() {
    cNodeRenderer<RectangleParticle, apsRectangleNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
