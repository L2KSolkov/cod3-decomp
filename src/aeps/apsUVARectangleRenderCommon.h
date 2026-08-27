// Shared reconstructed body for the UVA rectangle node renderers.
#ifndef COD3_AEPS_APSUVARECTANGLERENDERCOMMON_H
#define COD3_AEPS_APSUVARECTANGLERENDERCOMMON_H

#include "apsInternal.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"
#include "apsVertexBuffer.h"
#include "ngl/ngl_gpu_debug.h"

#include <cmath>

extern unsigned int dword_40358;
extern unsigned int dword_4035C;
extern unsigned int dword_BC2D10;
extern unsigned int dword_BC2D1C;

namespace apsUVARectangleRenderCommon {

inline __m128 Cross3(const __m128 a, const __m128 b)
{
    return _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)),
                   _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2))),
        _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2)),
                   _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1))));
}

inline __m128 Normalize3(const __m128 v)
{
    const __m128 sq = _mm_mul_ps(v, v);
    const float len2 = sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
    if (len2 <= 1.0e-12f)
        return _mm_setzero_ps();
    return _mm_mul_ps(v, _mm_set1_ps(1.0f / sqrtf(len2)));
}

template <typename Particle, typename Node>
inline void Render(cNodeRenderer<Particle, Node>* self)
{
    Node* node = self->mNode;
    if (node == NULL || node->mNumParticles <= 0)
        return;

    apsUVARenderer* renderer = static_cast<apsUVARenderer*>(node->mRenderer);
    if (renderer == NULL)
        return;

    apsVertexBuffer::SpriteVertex* out = apsVertexBuffer::GetBufferPtr(
        static_cast<unsigned int>(node->mNumParticles));
    if (out == NULL)
        return;

    const unsigned int oldZWrite = dword_BC2D10;
    const bool fogEnabled = (node->mFlags & 8u) != 0;
    const unsigned int shimmerWriteMask = 0x01000000u;
    if (renderer->mIsShimmer != 0) {
        if (D3DDevice_SetRenderState_ParameterCheck(
                D3DRS_COLORWRITEENABLE, shimmerWriteMask) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40358, shimmerWriteMask);
            dword_BC2D1C = shimmerWriteMask;
        }
    }
    apsInternal::SetupBlendAndTexture(
        renderer->mTexture,
        renderer->mIsShimmer ? static_cast<apsEBlendMode>(0) : renderer->mBlendMode,
        fogEnabled, 0);
    self->SetupDefaultShaders(node->mLocalToWorld);
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
        apsInternal::GetUserCoordinateSystem(renderer->mNormal, forward, left,
                                              up);

    math::Vector4 frameScale;
    frameScale.v = _mm_setr_ps(renderer->mInvWidthFrames,
                               renderer->mInvHeightFrames, 0.0f, 1.0f);
    D3DDevice_SetVertexShaderConstant1Fast(23, &frameScale);

    apsRenderSort::SortedParticleIterator sorted;
    apsRenderSort::UnsortedParticleIterator unsorted;
    apsRenderSort::ParticleIterator* iterator = &unsorted;
    if (renderer->UseSortedRendering() && node->mRenderSortBuffer != NULL &&
        node->mRenderSortBuffer->capacity >=
            static_cast<unsigned int>(node->mNumParticles)) {
        apsRenderSort::SortPointers<Particle>(
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
         bytes != NULL && written < static_cast<unsigned int>(node->mNumParticles);
         bytes = iterator->GetNextParticle(), ++written) {
        Particle* particle = reinterpret_cast<Particle*>(bytes);
        const __m128 center = _mm_setr_ps(particle->mPos.x, particle->mPos.y,
                                          particle->mPos.z, 0.0f);
        __m128 halfLeft;
        __m128 halfUp;
        if (velocityFacing) {
            const __m128 along = Normalize3(particle->mVelocity.v);
            const __m128 toView = _mm_sub_ps(center, forward.v);
            __m128 side = Normalize3(Cross3(toView, along));
            if (side.m128_f32[0] == 0.0f && side.m128_f32[1] == 0.0f &&
                side.m128_f32[2] == 0.0f)
                side = left.v;
            halfLeft = _mm_mul_ps(
                side, _mm_set1_ps(particle->mWidth * apsCommon::mCamera.mXFlip));
            halfUp = _mm_mul_ps(along, _mm_set1_ps(particle->mHeight));
        } else {
            const float cosine = cosf(particle->mAngle);
            const float sine = sinf(particle->mAngle);
            const __m128 c = _mm_set1_ps(cosine);
            const __m128 s = _mm_set1_ps(sine);
            halfLeft = _mm_mul_ps(
                _mm_add_ps(_mm_mul_ps(left.v, c), _mm_mul_ps(up.v, s)),
                _mm_set1_ps(particle->mWidth));
            halfUp = _mm_mul_ps(
                _mm_add_ps(_mm_mul_ps(left.v, _mm_set1_ps(-sine)),
                           _mm_mul_ps(up.v, c)),
                _mm_set1_ps(particle->mHeight));
        }

        float frame = particle->mFrame;
        if (frame < 0.0f)
            frame = 0.0f;
        if (frame > renderer->mMaxFrame)
            frame = renderer->mMaxFrame;
        const float row = floorf(frame * renderer->mInvWidthFrames);
        const float column = frame - row * renderer->mWidthFrames;
        const unsigned int color = apsInternal::ClampToColor32(
            particle->GetColor());
        math::Dir3 p0(_mm_add_ps(_mm_add_ps(center, halfLeft), halfUp));
        math::Dir3 p1(_mm_add_ps(_mm_sub_ps(center, halfLeft), halfUp));
        math::Dir3 p2(_mm_sub_ps(_mm_sub_ps(center, halfLeft), halfUp));
        math::Dir3 p3(_mm_add_ps(_mm_sub_ps(center, halfUp), halfLeft));
        out[0].Set(p0, color, column, row);
        out[1].Set(p1, color, column, row + 1.0f);
        out[2].Set(p2, color, column + 1.0f, row + 1.0f);
        out[3].Set(p3, color, column + 1.0f, row);
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

}

#endif
