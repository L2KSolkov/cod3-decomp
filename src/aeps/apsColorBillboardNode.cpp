// ============================================================================
// apsColorBillboardNode.cpp — color billboard node Render forwarder (1 func).
// Source: source/apsColorBillboardNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorBillboardNode.o):
//   apsColorBillboardNode::Render @0x81C2A0
// ============================================================================
#include "apsColorBillboardRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"
#include "apsVertexBuffer.h"
#include "apsInternal.h"
#include "ngl/ngl_gpu_debug.h"

extern void nglDxInitShaders(bool registerShaders);
extern unsigned int dword_40358;
extern unsigned int dword_4035C;
extern unsigned int dword_BC2D10;
extern unsigned int dword_BC2D1C;

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

static __m128 Cross3(const __m128 a, const __m128 b)
{
    return _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)),
                   _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2))),
        _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2)),
                   _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1))));
}

static __m128 Normalize3(const __m128 v)
{
    const __m128 sq = _mm_mul_ps(v, v);
    const float len2 = sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
    if (len2 <= 1.0e-12f)
        return _mm_setzero_ps();
    return _mm_mul_ps(v, _mm_set1_ps(1.0f / sqrtf(len2)));
}

}

// cNodeRenderer<ColorBillboardParticle,apsColorBillboardNode>::cNodeRenderer - ea: 0x0081C350
template <>
cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode>::cNodeRenderer(
    apsColorBillboardNode* node)
    : mNode(node)
{
}

// cNodeRenderer<ColorBillboardParticle,apsColorBillboardNode>::SetupDefaultShaders - ea: 0x0081C360
template <>
void cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix)
{
    math::Mat43 localToScreen;
    BuildLocalToScreen(localMatrix, nglBuildScene, localToScreen);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, &localToScreen, 0x10u);
    nglDxInitShaders(false);

    const unsigned int vertexShader = apsColorBillboardRender::VS[0];
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(apsColorBillboardRender::VS), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const unsigned int pixelShader = static_cast<unsigned int>(
        reinterpret_cast<uintptr_t>(apsColorBillboardRenderPixel::PS[0]));
    if (pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = pixelShader;
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(
                apsColorBillboardRenderPixel::PS[0]));
    }
}

// cNodeRenderer<ColorBillboardParticle,apsColorBillboardNode>::SetupShaders - ea: 0x0081C650
template <>
void cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode>::SetupShaders()
{
    SetupDefaultShaders(mNode->mLocalToWorld);
}

namespace apsRenderSort {

// fncompare<ColorBillboardParticle> - ea: 0x0081C610
template <>
int fncompare<ColorBillboardParticle>(const void* elem1, const void* elem2)
{
    const ColorBillboardParticle* p1 =
        *static_cast<const ColorBillboardParticle* const*>(elem1);
    const ColorBillboardParticle* p2 =
        *static_cast<const ColorBillboardParticle* const*>(elem2);
    if (p2->mAge <= p1->mAge)
        return p1->mAge > p2->mAge;
    return -1;
}

// SortPointers<ColorBillboardParticle> - ea: 0x0081C660
template <>
void SortPointers<ColorBillboardParticle>(Buffer* sortBuffer,
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
          &fncompare<ColorBillboardParticle>);
}

}

// cNodeRenderer<ColorBillboardParticle,apsColorBillboardNode>::Render - ea: 0x0081C6A0
template <>
void cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode>::Render()
{
    apsColorBillboardNode* node = mNode;
    if (node == nullptr || node->mNumParticles <= 0)
        return;

    apsBillboardRenderer* renderer = node->mRenderer;
    if (renderer == nullptr)
        return;

    apsVertexBuffer::SpriteVertex* out = apsVertexBuffer::GetBufferPtr(
        static_cast<unsigned int>(node->mNumParticles));
    if (out == nullptr)
        return;

    const unsigned int oldZWrite = dword_BC2D10;
    const bool fogEnabled = (node->mFlags & 8u) != 0;
    apsInternal::SetupBlendAndTexture(
        renderer->mTexture,
        renderer->mIsShimmer ? apsEBlendMode_Blend : renderer->mBlendMode,
        fogEnabled, 0);
    SetupDefaultShaders(node->mLocalToWorld);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }
    apsInternal::SetupFog(-77, nglBuildScene->FogNear, nglBuildScene->FogFar,
                          nglBuildScene->FogMin, nglBuildScene->FogMax,
                          fogEnabled);
    D3DDevice_SetVertexShaderConstant1Fast(22, &node->mBlendColor);
    apsInternal::SetupAlphaFade(-76, renderer->mAlphaFadeStart,
                                renderer->mAlphaFadeEnd);

    math::Dir3 forward;
    math::Dir3 left;
    math::Dir3 up;
    const bool velocityFacing = (node->mFlags & 2u) != 0;
    if ((node->mFlags & 1u) != 0)
        apsInternal::GetViewCoordinateSystem(forward, left, up);
    else if (velocityFacing)
        forward = math::Dir3(nglBuildScene->ViewPos);
    else
        apsInternal::GetUserCoordinateSystem(renderer->mNormal, forward, left, up);

    math::Vector4 coord;
    coord.v = _mm_setr_ps(1.0f, 1.0f, 0.0f, 1.0f);
    D3DDevice_SetVertexShaderConstant1Fast(23, &coord);

    apsRenderSort::SortedParticleIterator sorted;
    apsRenderSort::UnsortedParticleIterator unsorted;
    apsRenderSort::ParticleIterator* iterator = &unsorted;
    if (renderer->UseSortedRendering() && node->mRenderSortBuffer != nullptr &&
        node->mRenderSortBuffer->capacity >=
            static_cast<unsigned int>(node->mNumParticles)) {
        apsRenderSort::SortPointers<ColorBillboardParticle>(
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
        ColorBillboardParticle* particle =
            reinterpret_cast<ColorBillboardParticle*>(bytes);
        const __m128 center = _mm_setr_ps(particle->mPos.x, particle->mPos.y,
                                          particle->mPos.z, 0.0f);
        __m128 halfLeft;
        __m128 halfUp;
        if (velocityFacing) {
            const __m128 velocity = particle->mVelocity.v;
            const __m128 along = Normalize3(velocity);
            const __m128 toView = _mm_sub_ps(center, forward.v);
            __m128 side = Normalize3(Cross3(toView, along));
            if (side.m128_f32[0] == 0.0f && side.m128_f32[1] == 0.0f &&
                side.m128_f32[2] == 0.0f)
                side = left.v;
            side = _mm_mul_ps(side, _mm_set1_ps(
                particle->mRadius * apsCommon::mCamera.mXFlip));
            halfUp = _mm_mul_ps(along, _mm_set1_ps(particle->mRadius));
            halfLeft = side;
        } else {
            const float cosine = cosf(particle->mAngle);
            const float sine = sinf(particle->mAngle);
            const __m128 c = _mm_set1_ps(cosine);
            const __m128 s = _mm_set1_ps(sine);
            const __m128 radius = _mm_set1_ps(particle->mRadius);
            halfLeft = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(left.v, c),
                                             _mm_mul_ps(up.v, s)), radius);
            halfUp = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(left.v, _mm_set1_ps(-sine)),
                                           _mm_mul_ps(up.v, c)), radius);
        }
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

    const unsigned int oldColorWrite = nglBuildScene->FBWriteMask;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_COLORWRITEENABLE,
                                                 oldColorWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40358, oldColorWrite);
        dword_BC2D1C = oldColorWrite;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE,
                                                oldZWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, oldZWrite);
        dword_BC2D10 = oldZWrite;
    }
}

// apsColorBillboardRender::GetVShader - ea: 0x0081C2C0
unsigned long apsColorBillboardRender::GetVShader()
{
    return VS[0];
}

// apsColorBillboardRenderPixel::GetPShader - ea: 0x0081C2D0
unsigned long* apsColorBillboardRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsColorBillboardNode::Render — forward to the color billboard renderer.
// ea: 0x81C2A0
// ============================================================================
void apsColorBillboardNode::Render() {
    cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode> renderer(this);
    renderer.Render();
}
