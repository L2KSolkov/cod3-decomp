// ============================================================================
// apsUVANode.cpp — UVA node Render forwarder (1 func).
// Source: source/apsUVANode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsUVANode.o):
//   apsUVANode::Render @0x816350
// ============================================================================
#include "apsUVARenderer.h"
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

// cNodeRenderer<UVAParticle,apsUVANode>::cNodeRenderer - ea: 0x00816490
template <>
cNodeRenderer<UVAParticle, apsUVANode>::cNodeRenderer(apsUVANode* node)
    : mNode(node)
{
}

// cNodeRenderer<UVAParticle,apsUVANode>::SetupDefaultShaders - ea: 0x008164A0
template <>
void cNodeRenderer<UVAParticle, apsUVANode>::SetupDefaultShaders(
    const math::Mat43& localMatrix)
{
    math::Mat43 localToScreen;
    BuildLocalToScreen(localMatrix, nglBuildScene, localToScreen);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &localToScreen, 0x10u);
    nglDxInitShaders(false);
    if (apsUVARender::VS[0] != gpuHashVertexShader) {
        gpuHashVertexShader = apsUVARender::VS[0];
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(apsUVARender::VS), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(apsUVARenderPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(apsUVARenderPixel::PS[0]));
    }
}

// cNodeRenderer<UVAParticle,apsUVANode>::SetupShaders - ea: 0x00816790
template <>
void cNodeRenderer<UVAParticle, apsUVANode>::SetupShaders()
{
    SetupDefaultShaders(mNode->mLocalToWorld);
}

namespace apsRenderSort {

// fncompare<UVAParticle> - ea: 0x00816750
template <>
int fncompare<UVAParticle>(const void* elem1, const void* elem2)
{
    const UVAParticle* p1 = *static_cast<const UVAParticle* const*>(elem1);
    const UVAParticle* p2 = *static_cast<const UVAParticle* const*>(elem2);
    if (p2->mAge <= p1->mAge)
        return p1->mAge > p2->mAge;
    return -1;
}

// SortPointers<UVAParticle> - ea: 0x008167A0
template <>
void SortPointers<UVAParticle>(Buffer* sortBuffer, unsigned char* firstParticle,
                               unsigned int stride, unsigned int numParticles)
{
    unsigned int capacity = numParticles;
    if (sortBuffer->capacity <= numParticles)
        capacity = sortBuffer->capacity;
    for (unsigned int i = 0; i < capacity; ++i) {
        sortBuffer->buffer[i] = firstParticle;
        firstParticle += stride;
    }
    qsort(sortBuffer->buffer, capacity, sizeof(unsigned char*),
          &fncompare<UVAParticle>);
}

}

// apsUVANode::GetRenderSortBuffer - ea: 0x00816370
apsRenderSort::Buffer* apsUVANode::GetRenderSortBuffer()
{
    return mRenderSortBuffer;
}

// apsUVARenderer::WidthFrames - ea: 0x00816380
float apsUVARenderer::WidthFrames() const
{
    return mWidthFrames;
}

// apsUVARenderer::MaxFrame - ea: 0x00816390
float apsUVARenderer::MaxFrame() const
{
    return mMaxFrame;
}

// apsUVARenderer::InvWidthFrames - ea: 0x008163A0
float apsUVARenderer::InvWidthFrames() const
{
    return mInvWidthFrames;
}

// apsUVARenderer::InvHeightFrames - ea: 0x008163B0
float apsUVARenderer::InvHeightFrames() const
{
    return mInvHeightFrames;
}

// apsUVARender::GetVShader - ea: 0x008163C0
unsigned long apsUVARender::GetVShader()
{
    return VS[0];
}

// apsUVARenderPixel::GetPShader - ea: 0x008163D0
unsigned long* apsUVARenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsUVANode::Render — forward to the UVA node renderer.
// ea: 0x816350
// ============================================================================
void apsUVANode::Render() {
    cNodeRenderer<UVAParticle, apsUVANode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
