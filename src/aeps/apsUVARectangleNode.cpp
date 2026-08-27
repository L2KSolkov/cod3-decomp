// ============================================================================
// apsUVARectangleNode.cpp — UVA rectangle node Render forwarder (1 func).
// Source: source/apsUVARectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsUVARectangleNode.o):
//   apsUVARectangleNode::Render @0x8189C0
// ============================================================================
#include "apsUVARectangleRenderer.h"
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

// cNodeRenderer<UVARectangleParticle,apsUVARectangleNode>::cNodeRenderer - ea: 0x00818AB0
template <>
cNodeRenderer<UVARectangleParticle, apsUVARectangleNode>::cNodeRenderer(
    apsUVARectangleNode* node)
    : mNode(node)
{
}

// cNodeRenderer<UVARectangleParticle,apsUVARectangleNode>::SetupDefaultShaders - ea: 0x00818AC0
template <>
void cNodeRenderer<UVARectangleParticle, apsUVARectangleNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix)
{
    math::Mat43 localToScreen;
    BuildLocalToScreen(localMatrix, nglBuildScene, localToScreen);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &localToScreen, 0x10u);
    nglDxInitShaders(false);
    if (apsUVARectangleRender::VS[0] != gpuHashVertexShader) {
        gpuHashVertexShader = apsUVARectangleRender::VS[0];
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(apsUVARectangleRender::VS), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(apsUVARectangleRenderPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(apsUVARectangleRenderPixel::PS[0]));
    }
}

// cNodeRenderer<UVARectangleParticle,apsUVARectangleNode>::SetupShaders - ea: 0x00818DB0
template <>
void cNodeRenderer<UVARectangleParticle, apsUVARectangleNode>::SetupShaders()
{
    SetupDefaultShaders(mNode->mLocalToWorld);
}

namespace apsRenderSort {

// fncompare<UVARectangleParticle> - ea: 0x00818D70
template <>
int fncompare<UVARectangleParticle>(const void* elem1, const void* elem2)
{
    const UVARectangleParticle* p1 =
        *static_cast<const UVARectangleParticle* const*>(elem1);
    const UVARectangleParticle* p2 =
        *static_cast<const UVARectangleParticle* const*>(elem2);
    if (p2->mAge <= p1->mAge)
        return p1->mAge > p2->mAge;
    return -1;
}

// SortPointers<UVARectangleParticle> - ea: 0x00818DC0
template <>
void SortPointers<UVARectangleParticle>(Buffer* sortBuffer,
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
          &fncompare<UVARectangleParticle>);
}

}

// apsUVARectangleRender::GetVShader - ea: 0x008189E0
unsigned long apsUVARectangleRender::GetVShader()
{
    return VS[0];
}

// apsUVARectangleRenderPixel::GetPShader - ea: 0x008189F0
unsigned long* apsUVARectangleRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsUVARectangleNode::Render — forward to the UVA rectangle renderer.
// ea: 0x8189C0
// ============================================================================
void apsUVARectangleNode::Render() {
    cNodeRenderer<UVARectangleParticle, apsUVARectangleNode> renderer(this);
    renderer.Render();
}
