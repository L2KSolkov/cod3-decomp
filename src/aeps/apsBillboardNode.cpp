// ============================================================================
// apsBillboardNode.cpp — billboard node Render forwarder (1 func).
// Source: source/apsBillboardNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsBillboardNode.o):
//   apsBillboardNode::Render @0x813C90
// ============================================================================
#include "apsBillboardRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"
#include "apsVertexBuffer.h"
#include "apsInternal.h"
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

// cNodeRenderer<BillboardParticle,apsBillboardNode>::cNodeRenderer - ea: 0x00813F80
template <>
cNodeRenderer<BillboardParticle, apsBillboardNode>::cNodeRenderer(
    apsBillboardNode* node)
    : mNode(node)
{
}

// cNodeRenderer<BillboardParticle,apsBillboardNode>::SetupDefaultShaders - ea: 0x00813F90
template <>
void cNodeRenderer<BillboardParticle, apsBillboardNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix)
{
    math::Mat43 localToScreen;
    BuildLocalToScreen(localMatrix, nglBuildScene, localToScreen);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &localToScreen, 0x10u);
    nglDxInitShaders(false);

    const unsigned int vertexShader = apsBillboardRender::VS[0];
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(apsBillboardRender::VS), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }

    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(apsBillboardRenderPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(
                apsBillboardRenderPixel::PS[0]));
    }
}

// cNodeRenderer<BillboardParticle,apsBillboardNode>::SetupShaders - ea: 0x00814280
template <>
void cNodeRenderer<BillboardParticle, apsBillboardNode>::SetupShaders()
{
    SetupDefaultShaders(mNode->mLocalToWorld);
}

namespace apsRenderSort {

// fncompare<BillboardParticle> - ea: 0x00814240
template <>
int fncompare<BillboardParticle>(const void* elem1, const void* elem2)
{
    const BillboardParticle* p1 = *static_cast<const BillboardParticle* const*>(elem1);
    const BillboardParticle* p2 = *static_cast<const BillboardParticle* const*>(elem2);
    if (p2->mAge <= p1->mAge)
        return p1->mAge > p2->mAge;
    return -1;
}

// SortPointers<BillboardParticle> - ea: 0x00814290
template <>
void SortPointers<BillboardParticle>(Buffer* sortBuffer,
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
          &fncompare<BillboardParticle>);
}

}

// cNodeRenderer<BillboardParticle,apsBillboardNode>::Render - ea: 0x008142D0
template <>
void cNodeRenderer<BillboardParticle, apsBillboardNode>::Render()
{
    apsBillboardNode* node = mNode;
    if (node == nullptr || node->mNumParticles <= 0)
        return;

    apsBillboardRenderer* renderer = node->mRenderer;
    if (renderer == nullptr)
        return;

    apsVertexBuffer::SpriteVertex* out =
        apsVertexBuffer::GetBufferPtr(static_cast<unsigned int>(node->mNumParticles));
    if (out == nullptr)
        return;

    const bool fogEnabled = (node->mFlags & 8u) != 0;
    apsInternal::SetupBlendAndTexture(renderer->mTexture,
                                      renderer->mBlendMode,
                                      fogEnabled, 0);
    SetupShaders();
    apsInternal::SetupFog(-77, nglBuildScene->FogNear, nglBuildScene->FogFar,
                          nglBuildScene->FogMin, nglBuildScene->FogMax,
                          fogEnabled);
    D3DDevice_SetVertexShaderConstant1Fast(22, &node->mBlendColor);
    apsInternal::SetupAlphaFade(-76, renderer->mAlphaFadeStart,
                                renderer->mAlphaFadeEnd);

    math::Dir3 forward;
    math::Dir3 left;
    math::Dir3 up;
    if ((node->mFlags & 1u) != 0)
        apsInternal::GetViewCoordinateSystem(forward, left, up);
    else if ((node->mFlags & 2u) != 0)
        forward = math::Dir3(nglBuildScene->ViewPos);
    else
        apsInternal::GetUserCoordinateSystem(renderer->mNormal, forward, left, up);

    math::Vector4 coord = { };
    coord.v = _mm_setr_ps(1.0f, 1.0f, 0.0f, 1.0f);
    D3DDevice_SetVertexShaderConstant1Fast(23, &coord);

    apsRenderSort::SortedParticleIterator sorted;
    apsRenderSort::UnsortedParticleIterator unsorted;
    apsRenderSort::ParticleIterator* iterator = &unsorted;
    if (renderer->UseSortedRendering() && node->mRenderSortBuffer != nullptr &&
        node->mRenderSortBuffer->capacity >=
            static_cast<unsigned int>(node->mNumParticles)) {
        apsRenderSort::SortPointers<BillboardParticle>(
            node->mRenderSortBuffer, node->mParticles,
            static_cast<unsigned int>(node->mStride),
            static_cast<unsigned int>(node->mNumParticles));
        sorted.Init(node->mRenderSortBuffer->buffer,
                    static_cast<unsigned int>(node->mNumParticles));
        iterator = &sorted;
    } else {
        unsorted.Init(node->mParticles,
                      static_cast<unsigned int>(node->mNumParticles),
                      static_cast<unsigned int>(node->mStride));
    }

    unsigned int written = 0;
    for (unsigned char* bytes = iterator->GetNextParticle();
         bytes != nullptr && written < static_cast<unsigned int>(node->mNumParticles);
         bytes = iterator->GetNextParticle(), ++written) {
        BillboardParticle* particle =
            reinterpret_cast<BillboardParticle*>(bytes);
        math::Dir3 position(particle->mPos);
        const float angle = particle->mAngle;
        const float cosine = cosf(angle);
        const float sine = sinf(angle);
        const float radius = particle->mRadius;
        const __m128 halfLeft = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(left.v, _mm_set1_ps(cosine)),
                       _mm_mul_ps(up.v, _mm_set1_ps(sine))),
            _mm_set1_ps(radius));
        const __m128 halfUp = _mm_mul_ps(
            _mm_add_ps(_mm_mul_ps(left.v, _mm_set1_ps(-sine)),
                       _mm_mul_ps(up.v, _mm_set1_ps(cosine))),
            _mm_set1_ps(radius));
        const __m128 center = position.v;
        const unsigned int color = apsInternal::ClampToColor32(particle->GetColor());

        math::Dir3 p0(_mm_add_ps(_mm_add_ps(center, halfLeft), halfUp));
        math::Dir3 p1(_mm_add_ps(_mm_sub_ps(center, halfLeft), halfUp));
        math::Dir3 p2(_mm_sub_ps(_mm_sub_ps(center, halfLeft), halfUp));
        math::Dir3 p3(_mm_add_ps(_mm_sub_ps(center, halfUp), halfLeft));
        out[0].Set(p0, color, 0.0f, 0.0f);
        out[1].Set(p1, color, 1.0f, 0.0f);
        out[2].Set(p2, color, 1.0f, 1.0f);
        out[3].Set(p3, color, 0.0f, 1.0f);
        out += 4;
    }
    apsVertexBuffer::ReleaseAndDraw();
}

// apsRenderNode::TestFlags - ea: 0x00813D00
unsigned int apsRenderNode::TestFlags(unsigned int flag) const
{
    return flag & mFlags;
}

// apsRenderNode::Particles - ea: 0x00813D10
unsigned char* apsRenderNode::Particles() const
{
    return mParticles;
}

// apsRenderNode::NumParticles - ea: 0x00813D20
int apsRenderNode::NumParticles()
{
    return mNumParticles;
}

// apsRenderNode::Stride - ea: 0x00813D30
int apsRenderNode::Stride()
{
    return mStride;
}

// apsBillboardRenderer::Texture - ea: 0x00813D40
nglTexture* apsBillboardRenderer::Texture() const
{
    return mTexture;
}

// apsBillboardRenderer::Normal - ea: 0x00813D50
const math::Dir3& apsBillboardRenderer::Normal() const
{
    return mNormal;
}

// apsBillboardRenderer::WidthFrames - ea: 0x00813D60
float apsBillboardRenderer::WidthFrames() const
{
    return 1.0f;
}

// apsBillboardRenderer::MaxFrame - ea: 0x00813D70
float apsBillboardRenderer::MaxFrame() const
{
    return 0.0f;
}

// apsBillboardRenderer::InvWidthFrames - ea: 0x00813D80
float apsBillboardRenderer::InvWidthFrames() const
{
    return 1.0f;
}

// apsBillboardRenderer::InvHeightFrames - ea: 0x00813D90
float apsBillboardRenderer::InvHeightFrames() const
{
    return 1.0f;
}

// apsBillboardRenderer::AlphaFadeStart - ea: 0x00813DA0
float apsBillboardRenderer::AlphaFadeStart() const
{
    return mAlphaFadeStart;
}

// apsBillboardRenderer::AlphaFadeEnd - ea: 0x00813DB0
float apsBillboardRenderer::AlphaFadeEnd() const
{
    return mAlphaFadeEnd;
}

// apsBillboardNode::GetRenderSortBuffer - ea: 0x00813DC0
apsRenderSort::Buffer* apsBillboardNode::GetRenderSortBuffer()
{
    return mRenderSortBuffer;
}

// BillboardParticle accessors (apsBillboardNode.o)
// ea: 0x00813ED0
math::Dir3::Packed& BillboardParticle::GetPos()
{
    return mPos;
}

// ea: 0x00813EE0
math::Vector4 BillboardParticle::GetColor()
{
    math::Vector4 result;
    result.v = _mm_setr_ps(1.0f, 1.0f, 1.0f, mAlpha);
    return result;
}

// ea: 0x00813F30
float BillboardParticle::GetWidth()
{
    return mRadius;
}

// ea: 0x00813F40
float BillboardParticle::GetHeight()
{
    return mRadius;
}

// ea: 0x00813F50
float BillboardParticle::GetAngle()
{
    return mAngle;
}

// ea: 0x00813F60
float BillboardParticle::GetFrame()
{
    return 0.0f;
}

// ea: 0x00813F70
math::Vector4& BillboardParticle::GetVelocity()
{
    return mVelocity;
}

// RectangleParticle accessors (apsRectangleNode.o)
// ea: 0x00819D70
math::Dir3::Packed& RectangleParticle::GetPos() { return mPos; }
// ea: 0x00819D80
math::Vector4 RectangleParticle::GetColor()
{
    math::Vector4 result;
    result.v = _mm_setr_ps(1.0f, 1.0f, 1.0f, mAlpha);
    return result;
}
// ea: 0x00819DD0
float RectangleParticle::GetWidth() { return mWidth; }
// ea: 0x00819DE0
float RectangleParticle::GetHeight() { return mHeight; }
// ea: 0x00819DF0
float RectangleParticle::GetAngle() { return mAngle; }
// ea: 0x00819E00
float RectangleParticle::GetFrame() { return 0.0f; }
// ea: 0x00819E10
math::Vector4& RectangleParticle::GetVelocity() { return mVelocity; }

// ColorRectangleParticle accessors (apsColorRectangleNode.o)
// ea: 0x00815190
math::Dir3::Packed& ColorRectangleParticle::GetPos() { return mPos; }
// ea: 0x008151A0
math::Vector4& ColorRectangleParticle::GetColor() { return mColor; }
// ea: 0x008151B0
float ColorRectangleParticle::GetWidth() { return mWidth; }
// ea: 0x008151C0
float ColorRectangleParticle::GetHeight() { return mHeight; }
// ea: 0x008151D0
float ColorRectangleParticle::GetAngle() { return mAngle; }
// ea: 0x008151E0
float ColorRectangleParticle::GetFrame() { return 0.0f; }
// ea: 0x008151F0
math::Vector4& ColorRectangleParticle::GetVelocity() { return mVelocity; }

// UVAParticle accessors (apsUVANode.o)
// ea: 0x008163E0
math::Dir3::Packed& UVAParticle::GetPos() { return mPos; }
// ea: 0x008163F0
math::Vector4 UVAParticle::GetColor()
{
    math::Vector4 result;
    result.v = _mm_setr_ps(1.0f, 1.0f, 1.0f, mAlpha);
    return result;
}
// ea: 0x00816440
float UVAParticle::GetWidth() { return mRadius; }
// ea: 0x00816450
float UVAParticle::GetHeight() { return mRadius; }
// ea: 0x00816460
float UVAParticle::GetAngle() { return mAngle; }
// ea: 0x00816470
float UVAParticle::GetFrame() { return mFrame; }
// ea: 0x00816480
math::Vector4& UVAParticle::GetVelocity() { return mVelocity; }

// UVARectangleParticle accessors (apsUVARectangleNode.o)
// ea: 0x00818A00
math::Dir3::Packed& UVARectangleParticle::GetPos() { return mPos; }
// ea: 0x00818A10
math::Vector4 UVARectangleParticle::GetColor()
{
    math::Vector4 result;
    result.v = _mm_setr_ps(1.0f, 1.0f, 1.0f, mAlpha);
    return result;
}
// ea: 0x00818A60
float UVARectangleParticle::GetWidth() { return mWidth; }
// ea: 0x00818A70
float UVARectangleParticle::GetHeight() { return mHeight; }
// ea: 0x00818A80
float UVARectangleParticle::GetAngle() { return mAngle; }
// ea: 0x00818A90
float UVARectangleParticle::GetFrame() { return mFrame; }
// ea: 0x00818AA0
math::Vector4& UVARectangleParticle::GetVelocity() { return mVelocity; }

// ColorUVAParticle accessors (apsColorUVANode.o)
// ea: 0x0081B030
math::Dir3::Packed& ColorUVAParticle::GetPos() { return mPos; }
// ea: 0x0081B040
math::Vector4& ColorUVAParticle::GetColor() { return mColor; }
// ea: 0x0081B050
float ColorUVAParticle::GetWidth() { return mRadius; }
// ea: 0x0081B060
float ColorUVAParticle::GetHeight() { return mRadius; }
// ea: 0x0081B070
float ColorUVAParticle::GetAngle() { return mAngle; }
// ea: 0x0081B080
float ColorUVAParticle::GetFrame() { return mFrame; }
// ea: 0x0081B090
math::Vector4& ColorUVAParticle::GetVelocity() { return mVelocity; }

// ColorUVARectangleParticle accessors (apsColorUVARectangleNode.o)
// ea: 0x00817750
math::Dir3::Packed& ColorUVARectangleParticle::GetPos() { return mPos; }
// ea: 0x00817760
math::Vector4& ColorUVARectangleParticle::GetColor() { return mColor; }
// ea: 0x00817770
float ColorUVARectangleParticle::GetWidth() { return mWidth; }
// ea: 0x00817780
float ColorUVARectangleParticle::GetHeight() { return mHeight; }
// ea: 0x00817790
float ColorUVARectangleParticle::GetAngle() { return mAngle; }
// ea: 0x008177A0
float ColorUVARectangleParticle::GetFrame() { return mFrame; }
// ea: 0x008177B0
math::Vector4& ColorUVARectangleParticle::GetVelocity() { return mVelocity; }

// ColorBillboardParticle accessors (apsColorBillboardNode.o)
// ea: 0x0081C2E0
math::Dir3::Packed& ColorBillboardParticle::GetPos() { return mPos; }
// ea: 0x0081C2F0
math::Vector4& ColorBillboardParticle::GetColor() { return mColor; }
// ea: 0x0081C300
float ColorBillboardParticle::GetWidth() { return mRadius; }
// ea: 0x0081C310
float ColorBillboardParticle::GetHeight() { return mRadius; }
// ea: 0x0081C320
float ColorBillboardParticle::GetAngle() { return mAngle; }
// ea: 0x0081C330
float ColorBillboardParticle::GetFrame() { return 0.0f; }
// ea: 0x0081C340
math::Vector4& ColorBillboardParticle::GetVelocity() { return mVelocity; }

// apsBillboardRender::GetVShader - ea: 0x00813EB0
unsigned long apsBillboardRender::GetVShader()
{
    return VS[0];
}

// apsBillboardRenderPixel::GetPShader - ea: 0x00813EC0
unsigned long* apsBillboardRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsBillboardNode::Render — forward to the billboard node renderer.
// ea: 0x813C90
// ============================================================================
void apsBillboardNode::Render() {
    cNodeRenderer<BillboardParticle, apsBillboardNode> renderer(this);
    renderer.Render();
}
