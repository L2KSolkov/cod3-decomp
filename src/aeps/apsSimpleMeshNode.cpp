// ============================================================================
// apsSimpleMeshNode.cpp — mesh particle render node.
// Source: source/apsSimpleMeshNode.cpp (aeps_xboxr).
// Render @ 0x812530 is reconstructed from the IDA release decompile and
// verified local types in codmp_xboxr.xbe.h.
// ============================================================================
#include "apsSimpleMeshRenderer.h"
#include "apsParticleTypes.h"
#include "apsInternal.h"
#include "ngl/ngl_dx_quad.h"

extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// apsSimpleMeshNode.o inline/COMDAT entries, matched to the IDA C dump.
apsSphere::apsSphere() = default;

void apsSphere::Set(const math::Dir3::Packed& center, float radius)
{
    mSphere.v = _mm_setr_ps(center.x, center.y, center.z, radius);
}

apsSphere::apsSphere(const math::Dir3::Packed& center, float radius)
{
    Set(center, radius);
}

const math::Mat43& apsRenderNode::Matrix() const
{
    return mLocalToWorld;
}

apsSimpleMeshRenderer& apsSimpleMeshNode::Renderer()
{
    if (mRenderer == nullptr &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsSimpleMeshNode.h", 16,
                  "mRenderer", "null renderer"))
        __debugbreak();
    return *mRenderer;
}

nglTexture* apsSimpleMeshRenderer::Texture() const
{
    return mTexture;
}

nglMesh* apsSimpleMeshRenderer::Mesh() const
{
    return mMesh;
}

unsigned long apsSimpleMeshRender::GetVShader()
{
    return VS[0];
}

unsigned long* apsSimpleMeshRenderPixel::GetPShader()
{
    return PS[0];
}

MeshParticleContext::MeshParticleContext()
{
}

extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_40344;
extern unsigned int dword_40348;
extern unsigned int dword_40350;
extern unsigned int dword_4035C;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D08;
extern unsigned int dword_BC2D0C;
extern unsigned int dword_BC2D38;
extern unsigned int dword_BC2D80;

static __m128 TransformRowToScreen(const __m128 row, const nglScene* scene) {
    return _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(row, row, 0), scene->WorldToScreen.x.v),
            _mm_mul_ps(_mm_shuffle_ps(row, row, 85), scene->WorldToScreen.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(row, row, 170), scene->WorldToScreen.z.v),
            _mm_mul_ps(_mm_shuffle_ps(row, row, 255), scene->WorldToScreen.w.v)));
}

static void BuildLocalToScreen(const math::Mat43& localToWorld,
                               const nglScene* scene, math::Mat43& out) {
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

void apsSimpleMeshNode::Render() {
    if (mRenderer == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsSimpleMeshNode.h", 16,
                  "mRenderer", "null renderer"))
        __debugbreak();

    apsSimpleMeshRenderer* renderer = mRenderer;
    nglDxSetTexture(0, renderer->mTexture, 1u, 3u);

    if (nglDxTexCache.Prev[0].WrapU != 1u) {
        nglDxTexCache.Prev[0].WrapU = 1u;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 1u) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSU] = 1u;
        }
    }
    if (nglDxTexCache.Prev[0].WrapV != 1u) {
        nglDxTexCache.Prev[0].WrapV = 1u;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 1u) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSV] = 1u;
        }
    }

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 1u);
        dword_BC2CFC = 1u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 1u);
        dword_BC2D00 = 1u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x204u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, 0x204u);
        dword_BC2CF8 = 0x204u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0u);
        dword_BC2D04 = 0u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0u) == 0)
        D3DDevice_SetRenderState_CullMode(0u);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, 0x8006u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40350, 0x8006u);
        dword_BC2D38 = 0x8006u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, 0x302u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40344, 0x302u);
        dword_BC2D08 = 0x302u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, 0x303u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40348, 0x303u);
        dword_BC2D0C = 0x303u;
    }

    nglDxState.PrevBM = static_cast<unsigned int>(-1);
    nglDxInitShaders(false);
    if (apsSimpleMeshRender::VS != nullptr) {
        const unsigned int vertexShader = apsSimpleMeshRender::VS[0];
        if (vertexShader != gpuHashVertexShader) {
            gpuHashVertexShader = vertexShader;
            D3DDevice_LoadVertexShaderProgram(
                reinterpret_cast<const unsigned int*>(apsSimpleMeshRender::VS), 0);
            D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
        }
    }
    if (apsSimpleMeshRenderPixel::PS != nullptr) {
        const unsigned int* pixelShader = reinterpret_cast<const unsigned int*>(
            apsSimpleMeshRenderPixel::PS[0]);
        const unsigned int pixelShaderHash =
            static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pixelShader));
        if (pixelShaderHash != gpuHashPixelShader) {
            gpuHashPixelShader = pixelShaderHash;
            D3DDevice_SetPixelShaderProgram(
                reinterpret_cast<const _D3DPixelShaderDef*>(pixelShader));
        }
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 0u) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 0u;
    }

    if (mRenderer == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsSimpleMeshNode.h", 16,
                  "mRenderer", "null renderer"))
        __debugbreak();
    nglMesh* mesh = renderer->mMesh;
    if (mesh == nullptr &&
        _tlAssert("source/apsSimpleMeshNode.cpp", 270, "pMesh", "NULL Mesh"))
        __debugbreak();
    if (mesh->NSections != 1u &&
        _tlAssert("source/apsSimpleMeshNode.cpp", 271,
                  "pMesh->NSections == 1",
                  "apsSimpleMeshNode only knows how to render ngl meshes with one section (material)"))
        __debugbreak();

    const unsigned int oldZWrite = dword_BC2D10;
    int count = mNumParticles;
    const unsigned char* particleBytes = mParticles;
    while (count != 0) {
        const MeshParticle* particle =
            reinterpret_cast<const MeshParticle*>(particleBytes);
        math::Dir3 position;
        position.v = _mm_setr_ps(particle->mPos.x, particle->mPos.y,
                                 particle->mPos.z, 0.0f);

        math::Mat43 particleToWorld;
        apsMath::GetQuaternionMatrix(particleToWorld, particle->mOrientation, position);
        math::Mat43 worldMatrix;
        apsMath::Multiply43(worldMatrix, particleToWorld, mLocalToWorld);

        MeshParticleContext context;
        BuildLocalToScreen(worldMatrix, nglBuildScene, context.mLToS);

        const float alpha = particle->mAlpha;
        const unsigned int zWrite = alpha >= 0.4f ? 1u : 0u;
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, zWrite) == 0) {
            D3DDevice_SetRenderState_Simple(dword_4035C, zWrite);
            dword_BC2D10 = zWrite;
        }

        apsSphere sphere;
        sphere.mSphere.v = _mm_setr_ps(particle->mPos.x, particle->mPos.y,
                                       particle->mPos.z,
                                       _mm_shuffle_ps(mesh->Sphere.v, mesh->Sphere.v, 255).m128_f32[0]);
        const math::Mat43 inverseWorld = apsMath::InverseOrtho(worldMatrix);
        apsInternal::GetLightMatrices(context.mLightDir, context.mLightColor,
                                      inverseWorld, mLightContext, sphere);
        context.mAlpha.v = _mm_setr_ps(1.0f, 1.0f, 1.0f, alpha);

        D3DDevice_SetVertexShaderConstantNotInlineFast(6, &context, 0x34u);
        nglGpuDrawSection(mesh->Sections->Section);

        particleBytes += mStride;
        --count;
    }

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, oldZWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, oldZWrite);
        dword_BC2D10 = oldZWrite;
    }
}
