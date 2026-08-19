// ============================================================================
// apsShrimpNode.cpp — shrimp particle render node.
// Source: source/apsShrimpNode.cpp (aeps_xboxr).
// Render @ 0x812D90 is reconstructed from the IDA release decompile and
// verified local types in codmp_xboxr.xbe.h.
// ============================================================================
#include "apsShrimpRenderer.h"
#include "apsParticleTypes.h"
#include "apsVertexBuffer.h"
#include "apsInternal.h"
#include "ngl/ngl_dx_quad.h"

#include <cmath>

extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

static const __m128 Float4_XAxis_130 = { 1.0f, 0.0f, 0.0f, 0.0f };
static const __m128 Float4_ZAxis_130 = { 0.0f, 0.0f, 1.0f, 0.0f };

void apsShrimpNode::Render() {
    if (mRenderer == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsShrimpNode.h", 14,
                  "mRenderer", "null renderer"))
        __debugbreak();

    apsShrimpRenderer* renderer = mRenderer;
    nglTexture* texture = renderer->mTexture;
    apsInternal::SetupBlendAndTexture(texture, renderer->mBlendMode, false, 127);

    __m128 wtsY = nglBuildScene->WorldToScreen.y.v;
    __m128 wtsZ = nglBuildScene->WorldToScreen.z.v;
    __m128 wtsW = nglBuildScene->WorldToScreen.w.v;
    __m128 wtsX = nglBuildScene->WorldToScreen.x.v;
    __m128 v6 = _mm_shuffle_ps(wtsX, wtsY, 68);
    __m128 v7 = _mm_shuffle_ps(wtsX, wtsY, 238);
    __m128 v8 = _mm_shuffle_ps(wtsZ, wtsW, 68);
    __m128 v9 = _mm_shuffle_ps(wtsZ, wtsW, 238);
    __m128 worldToScreen[4];
    worldToScreen[0] = _mm_shuffle_ps(v6, v8, 136);
    worldToScreen[1] = _mm_shuffle_ps(v6, v8, 221);
    worldToScreen[2] = _mm_shuffle_ps(v7, v9, 136);
    worldToScreen[3] = _mm_shuffle_ps(v7, v9, 221);
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, worldToScreen, 0x10);

    nglDxInitShaders(false);
    if (apsShrimpRender::VS != nullptr) {
        const unsigned int vertexShader = apsShrimpRender::VS[0];
        if (vertexShader != gpuHashVertexShader) {
            gpuHashVertexShader = vertexShader;
            D3DDevice_LoadVertexShaderProgram(apsShrimpRender::VS, 0);
            D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
        }
    }
    if (apsShrimpRenderPixel::PS != nullptr) {
        const unsigned int* pixelShader = apsShrimpRenderPixel::PS[0];
        if (pixelShader != reinterpret_cast<const unsigned int*>(gpuHashPixelShader)) {
            gpuHashPixelShader = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pixelShader));
            D3DDevice_SetPixelShaderProgram(
                reinterpret_cast<const _D3DPixelShaderDef*>(pixelShader));
        }
    }

    const int count = mNumParticles;
    if (count == 0)
        return;

    apsVertexBuffer::SpriteVertex* vertices = apsVertexBuffer::GetBufferPtr(count);
    if (vertices == nullptr)
        return;

    const int frames = renderer->mNumFrames;
    const float numRotations = static_cast<float>(renderer->mNumRotations);
    const float spriteUScale = static_cast<float>(renderer->mSpriteWidth) /
                               static_cast<float>(renderer->mTextureWidth);
    const float spriteVScale = static_cast<float>(renderer->mSpriteHeight) /
                               static_cast<float>(renderer->mTextureHeight);
    const int numRows = renderer->mNumRows;
    const int framesPerRow = frames / numRows;
    const float invFramesPerRow = 1.0f / static_cast<float>(framesPerRow);

    __m128 view = nglBuildScene->ViewDir.v;
    __m128 zero = _mm_setzero_ps();
    __m128 forward = _mm_shuffle_ps(view, _mm_shuffle_ps(zero, view, 240), 196);
    __m128 lengthSquared = _mm_mul_ps(forward, forward);
    float length = lengthSquared.m128_f32[0]
                 + _mm_shuffle_ps(lengthSquared, lengthSquared, 85).m128_f32[0]
                 + _mm_shuffle_ps(lengthSquared, lengthSquared, 170).m128_f32[0];
    if (length < 0.000001f)
        forward = Float4_XAxis_130;

    lengthSquared = _mm_mul_ps(forward, forward);
    length = lengthSquared.m128_f32[0]
           + _mm_shuffle_ps(lengthSquared, lengthSquared, 85).m128_f32[0]
           + _mm_shuffle_ps(lengthSquared, lengthSquared, 170).m128_f32[0];
    forward = _mm_div_ps(forward, _mm_set1_ps(std::sqrt(length)));

    math::Dir3 left;
    left.v = _mm_setr_ps(
        _mm_shuffle_ps(forward, forward, 85).m128_f32[0],
        0.0f - forward.m128_f32[0],
        0.0f,
        0.0f);

    math::Dir3 screenFacing;
    screenFacing.v = _mm_mul_ps(left.v, _mm_set1_ps(apsCommon::mCamera.mXFlip));
    float angleOffset = apsMath::ACos(forward.m128_f32[0]);
    if (_mm_shuffle_ps(forward, forward, 85).m128_f32[0] < 0.0f)
        angleOffset = 6.2831855f - angleOffset;
    angleOffset = angleOffset * 0.15915494f * apsCommon::mCamera.mXFlip;
    if (angleOffset < 0.0f)
        angleOffset += 1.0f;
    if (angleOffset >= 1.0f)
        angleOffset -= 1.0f;

    const float halfRotation = 0.5f / numRotations;
    const unsigned char* particleBytes = mParticles;
    for (int i = 0; i < count; ++i, particleBytes += mStride, vertices += 4) {
        const ShrimpParticle* particle =
            reinterpret_cast<const ShrimpParticle*>(particleBytes);

        math::Dir3 position;
        const __m128 packed = _mm_setr_ps(particle->mPos.x, particle->mPos.y,
                                          particle->mPos.z, 0.0f);
        position.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(packed, packed, 0), mLocalToWorld.x.v),
                _mm_mul_ps(_mm_shuffle_ps(packed, packed, 85), mLocalToWorld.y.v)),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(packed, packed, 170), mLocalToWorld.z.v),
                mLocalToWorld.w.v));

        math::Vector4 particleColor;
        particleColor.v = _mm_setr_ps(1.0f, 1.0f, 1.0f, particle->mAlpha);
        const unsigned int color =
            apsInternal::ClampToColor32(mBlendColor, particleColor);

        float rotation = particle->mAngle * 0.15915494f + halfRotation;
        if (rotation > 1.0f)
            rotation -= 1.0f;
        rotation -= angleOffset;
        if (rotation < 0.0f)
            rotation += 1.0f;

        const int frame = static_cast<int>(particle->mFrame);
        const int frameRow = static_cast<int>(particle->mFrame * invFramesPerRow);
        const float u0 = static_cast<float>(frame - framesPerRow * frameRow) * spriteUScale;
        const float v0 = (static_cast<float>(frameRow)
                        + static_cast<float>(numRows) * (rotation * numRotations)) * spriteVScale;
        const float u1 = u0 + spriteUScale;
        const float v1 = v0 + spriteVScale;

        math::Dir3 top;
        top.v = _mm_add_ps(position.v,
                           _mm_mul_ps(Float4_ZAxis_130, _mm_set1_ps(particle->mHeight)));
        const math::Dir3 halfLeft(_mm_mul_ps(screenFacing.v,
                                              _mm_set1_ps(particle->mWidth * 0.5f)));
        math::Dir3 topRight;
        topRight.v = _mm_add_ps(top.v, halfLeft.v);
        math::Dir3 bottomRight;
        bottomRight.v = _mm_add_ps(position.v, halfLeft.v);
        math::Dir3 topLeft;
        topLeft.v = _mm_sub_ps(top.v, halfLeft.v);
        math::Dir3 bottomLeft;
        bottomLeft.v = _mm_sub_ps(position.v, halfLeft.v);

        vertices[0].Set(topRight, color, u0, v0);
        vertices[1].Set(bottomRight, color, u0, v1);
        vertices[2].Set(topLeft, color, u1, v0);
        vertices[3].Set(bottomLeft, color, u1, v1);
    }

    apsVertexBuffer::ReleaseAndDraw();
}
