// ============================================================================
// ngl_gpu_debug.cpp — GPU debug line/triangle rendering (2 non-inline funcs).
// Source: source/ngl_gpu_debug.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_gpu_debug.o):
//   nglDebugLineNode::Render @0x851460
//   nglDebugTriNode::Render  @0x851620
// ============================================================================
#include "ngl_gpu_debug.h"

#include <intrin.h>

// d3d8d data (owned by d3d8d:state.obj)
extern unsigned int dword_40304;
extern unsigned int dword_BC2CFC;

// ngl_scene.o (data)
extern nglScene* nglBuildScene;

// ============================================================================
// nglDebugLineNode::Render — draw the debug lines.
// ea: 0x851460
// ============================================================================
void nglDebugLineNode::Render() {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 0);
        dword_BC2CFC = 0;
    }

    if (nglGpuDebugVertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuDebugVertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram((const unsigned int*)nglGpuDebugVertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect((_D3DVERTEXATTRIBUTEFORMAT*)&gpuSetVertexShaderInputs, 0);
    }

    // transpose WorldToScreen into shader constants 6..21
    __m128 wtsY = nglBuildScene->WorldToScreen.y.v;
    __m128 wtsZ = nglBuildScene->WorldToScreen.z.v;
    __m128 wtsW = nglBuildScene->WorldToScreen.w.v;
    __m128 wtsX = nglBuildScene->WorldToScreen.x.v;

    __m128 v6 = _mm_shuffle_ps(wtsX, wtsY, 68);
    __m128 v7 = _mm_shuffle_ps(wtsX, wtsY, 238);
    __m128 v8 = _mm_shuffle_ps(wtsZ, wtsW, 68);
    __m128 v9 = _mm_shuffle_ps(wtsZ, wtsW, 238);
    __m128 m[4];
    m[0] = _mm_shuffle_ps(v6, v8, 136);
    m[1] = _mm_shuffle_ps(v6, v8, 221);
    m[2] = _mm_shuffle_ps(v7, v9, 136);
    m[3] = _mm_shuffle_ps(v7, v9, 221);

    D3DDevice_SetVertexShaderConstantNotInlineFast(6, m, 0x10);

    if ((unsigned int)nglGpuDebugPixelShader::Shader != gpuHashPixelShader) {
        gpuHashPixelShader = (unsigned int)nglGpuDebugPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGpuDebugPixelShader::Shader);
    }

    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SelectVertexShaderDirect(nglGpuPCVertexFmt.VertexDeclaration, 0);
    D3DDevice_DrawVerticesUP(D3DPT_LINELIST, 2 * (NVerts >> 1), Verts,
                             (unsigned int)nglGpuPCVertexFmt.VertexSize);

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0x900) == 0)
        D3DDevice_SetRenderState_CullMode(0x900);
}

// ============================================================================
// nglDebugTriNode::Render — draw the debug triangles.
// ea: 0x851620
// ============================================================================
void nglDebugTriNode::Render() {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 0);
        dword_BC2CFC = 0;
    }

    if (nglGpuDebugVertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuDebugVertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram((const unsigned int*)nglGpuDebugVertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect((_D3DVERTEXATTRIBUTEFORMAT*)&gpuSetVertexShaderInputs, 0);
    }

    __m128 wtsY = nglBuildScene->WorldToScreen.y.v;
    __m128 wtsZ = nglBuildScene->WorldToScreen.z.v;
    __m128 wtsW = nglBuildScene->WorldToScreen.w.v;
    __m128 wtsX = nglBuildScene->WorldToScreen.x.v;

    __m128 v6 = _mm_shuffle_ps(wtsX, wtsY, 68);
    __m128 v7 = _mm_shuffle_ps(wtsX, wtsY, 238);
    __m128 v8 = _mm_shuffle_ps(wtsZ, wtsW, 68);
    __m128 v9 = _mm_shuffle_ps(wtsZ, wtsW, 238);
    __m128 m[4];
    m[0] = _mm_shuffle_ps(v6, v8, 136);
    m[1] = _mm_shuffle_ps(v6, v8, 221);
    m[2] = _mm_shuffle_ps(v7, v9, 136);
    m[3] = _mm_shuffle_ps(v7, v9, 221);

    D3DDevice_SetVertexShaderConstantNotInlineFast(6, m, 0x10);

    if ((unsigned int)nglGpuDebugPixelShader::Shader != gpuHashPixelShader) {
        gpuHashPixelShader = (unsigned int)nglGpuDebugPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGpuDebugPixelShader::Shader);
    }

    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SelectVertexShaderDirect(nglGpuPCVertexFmt.VertexDeclaration, 0);
    D3DDevice_DrawVerticesUP(D3DPT_TRIANGLELIST, 3 * (NVerts / 3), Verts,
                             (unsigned int)nglGpuPCVertexFmt.VertexSize);

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0x900) == 0)
        D3DDevice_SetRenderState_CullMode(0x900);
}
