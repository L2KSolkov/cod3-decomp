// ============================================================================
// ngl_dx_quad.cpp — quad rendering (2 non-inline funcs).
// Source: source/ngl_dx_quad.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_dx_quad.o):
//   nglRenderQuad       @0x847480
//   nglQuadNode::Render @0x8479B0
// ============================================================================
#include "ngl_dx_quad.h"

#include <intrin.h>

// d3d8d data
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2D80;
extern unsigned int dword_40304;

// d3d8d data (owned by d3d8d:state.obj)
extern unsigned int D3D__DirtyFlags;
// The XDK cache has 32 DWORDs per texture stage.  Use the shim-owned table
// directly so stages 1-3 update their own ADDRESSU/ADDRESSV slots.
extern unsigned int D3D__TextureState[4][32];

// ============================================================================
// nglDxSetTextureU — set texture wrap-U state (inline COMDAT)
// ea: 0x7C2920
// ============================================================================
void nglDxSetTextureU(unsigned int Stage, unsigned int Mode) {
    if (nglDxTexCache.Prev[Stage].WrapU != Mode) {
        nglDxTexCache.Prev[Stage].WrapU = Mode;
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSU, Mode) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_ADDRESSU] = Mode;
        }
    }
}

// ============================================================================
// nglDxSetTextureV — set texture wrap-V state (inline COMDAT)
// ea: 0x7C2980
// ============================================================================
void nglDxSetTextureV(unsigned int Stage, unsigned int Mode) {
    if (nglDxTexCache.Prev[Stage].WrapV != Mode) {
        nglDxTexCache.Prev[Stage].WrapV = Mode;
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSV, Mode) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_ADDRESSV] = Mode;
        }
    }
}

// ============================================================================
// nglRenderQuad — draw a textured/colored quad via the push buffer.
// ea: 0x847480
// ============================================================================
void nglRenderQuad(nglQuad* Quad) {
    unsigned int IsTexYUY2 = 0;
    unsigned __int64 QuadTick = __rdtsc();

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);

    nglDxState.SetBlendMode(Quad->BlendMode);

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 0) == 0) {
        D3D__DirtyFlags |= 0x2000;
        dword_BC2D80 = 0;
    }

    nglTexture* Tex = Quad->Tex;
    unsigned int* pixelShader;
    if (Tex != NULL) {
        nglDxSetTextureU(0, ((Quad->MapFlags & 0x40) | 0x20) >> 5);
        nglDxSetTextureV(0, ((Quad->MapFlags & 0x80) | 0x40) >> 6);
        nglTextureAnimFrame = nglBuildScene->IFLFrame;
        nglDxSetTexture(0, Tex, Quad->MapFlags, 3);
        if ((Tex->Flags >> 8 & 0x80) != 0) {
            IsTexYUY2 = 1;
            if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_YUVENABLE, 1) == 0)
                D3DDevice_SetRenderState_YuvEnable(1);
        }
        if ((Tex->Flags & 0x4000) == 0) {
            float Width = (float)Tex->Width;
            float Height = (float)Tex->Height;
            Quad->Verts[0].U = Quad->Verts[0].U * Width;
            Quad->Verts[0].V = Quad->Verts[0].V * Height;
            Quad->Verts[1].U = Quad->Verts[1].U * Width;
            Quad->Verts[1].V = Quad->Verts[1].V * Height;
            Quad->Verts[2].U = Quad->Verts[2].U * Width;
            Quad->Verts[2].V = Quad->Verts[2].V * Height;
            Quad->Verts[3].U = Quad->Verts[3].U * Width;
            Quad->Verts[3].V = Quad->Verts[3].V * Height;
        }
        nglDxInitShaders(false);
        if (nglGpuQuadPCUVVertexShader::Shader != gpuHashVertexShader) {
            gpuHashVertexShader = nglGpuQuadPCUVVertexShader::Shader;
            D3DDevice_LoadVertexShaderProgram((const unsigned int*)nglGpuQuadPCUVVertexShader::Shader, 0);
            D3DDevice_SelectVertexShaderDirect((_D3DVERTEXATTRIBUTEFORMAT*)&gpuSetVertexShaderInputs, 0);
        }
        pixelShader = nglGpuTexColPixelShader::Shader;
    } else {
        nglDxInitShaders(false);
        if (nglGpuQuadPCVertexShader::Shader != gpuHashVertexShader) {
            gpuHashVertexShader = nglGpuQuadPCVertexShader::Shader;
            D3DDevice_LoadVertexShaderProgram((const unsigned int*)nglGpuQuadPCVertexShader::Shader, 0);
            D3DDevice_SelectVertexShaderDirect((_D3DVERTEXATTRIBUTEFORMAT*)&gpuSetVertexShaderInputs, 0);
        }
        pixelShader = nglGpuColPixelShader::Shader;
    }

    if ((unsigned int)pixelShader != gpuHashPixelShader) {
        gpuHashPixelShader = (unsigned int)pixelShader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)pixelShader);
    }

    float Z = nglDxViewToScreenZ(Quad->Z, nglBuildScene);
    D3DDevice_SelectVertexShaderDirect(nglGpuPCUVVertexFmt.VertexDeclaration, 0);

    if ((nglBuildScene->RenderTarget->Flags & 0x2000) != 0) {
        for (int i = 0; i < 4; ++i) {
            float w = (float)nglDisplayMode.Width / (float)nglGetScreenWidth();
            Quad->Verts[i].X = w * Quad->Verts[i].X;
            float h = (float)nglDisplayMode.Height / (float)nglGetScreenHeight();
            Quad->Verts[i].Y = h * Quad->Verts[i].Y;
        }
    }

    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SetVertexShaderInputDirect(NULL, 0, NULL);
    gpuHashIndexBuffer = 0;
    D3DDevice_SetIndices(NULL, 0);

    nglXbPushQuad.PB = D3DDevice_BeginPush(0x1D);
    *nglXbPushQuad.PB = 0xFFFFFFFF;  // NOP
    nglXbPushQuad.PB[1] = 8;
    nglXbPushQuad.PB[2] = 1080039448;
    nglXbPushQuad.PB += 3;

    // 4 corners, each 6 dwords (pos xyz, color, uv)
    const int corners[4] = { 0, 1, 3, 2 };
    for (int c = 0; c < 4; ++c) {
        nglQuadVertex& v = Quad->Verts[corners[c]];
        *nglXbPushQuad.PB++ = *(unsigned int*)&v.X;
        *nglXbPushQuad.PB++ = *(unsigned int*)&v.Y;
        *nglXbPushQuad.PB++ = *(unsigned int*)&Z;
        *nglXbPushQuad.PB++ = v.Color;
        *nglXbPushQuad.PB++ = *(unsigned int*)&v.U;
        *nglXbPushQuad.PB++ = *(unsigned int*)&v.V;
    }

    *nglXbPushQuad.PB = 0xFFFFFFFF;  // NOP
    nglXbPushQuad.PB[1] = 0;
    nglXbPushQuad.PB += 2;
    D3DDevice_EndPush(nglXbPushQuad.PB);

    if (IsTexYUY2 != 0 && D3DDevice_SetRenderState_ParameterCheck(D3DRS_YUVENABLE, 0) == 0)
        D3DDevice_SetRenderState_YuvEnable(0);

    nglPerfInfo.QuadCycles += __rdtsc() - QuadTick;
}

// ============================================================================
// nglQuadNode::Render — render the quad if not disabled.
// ea: 0x8479B0
// ============================================================================
void nglQuadNode::Render() {
    if (nglSyncDebug.DisableQuads == 0)
        nglRenderQuad(&this->Quad);
}
