// ============================================================================
// ngl_dx_quad.h — quad rendering (1 non-inline func + inline COMDAT helpers).
// Source: source/ngl_dx_quad.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_dx_quad.o):
//   nglRenderQuad         @0x847480
//   nglQuadNode::Render   @0x8479B0
//   nglDxViewToScreenZ    @0x847B70 (inline COMDAT)
// ============================================================================
#ifndef COD3_NGL_NGL_DX_QUAD_H
#define COD3_NGL_NGL_DX_QUAD_H

#include "d3d8.h"
#include "ngl/nglScene.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_xb_internal.h"  // nglXbPushQuad
#include "ngl/nglTexture.h"
#include "aeps/apsRenderNode.h"    // nglRenderNode

#include <intrin.h>

// ============================================================================
// nglQuadVertex — quad corner vertex (20 bytes, verified against IDA)
// ============================================================================
struct nglQuadVertex {
    float        X;       // +0x00
    float        Y;       // +0x04
    float        U;       // +0x08
    float        V;       // +0x0C
    unsigned int Color;   // +0x10
};
static_assert(sizeof(nglQuadVertex) == 0x14, "nglQuadVertex size mismatch");

// ============================================================================
// nglQuad — quad descriptor (96 bytes, verified against IDA)
// ============================================================================
struct nglQuad {
    nglQuadVertex Verts[4];  // +0x00 (80 bytes)
    float         Z;         // +0x50
    unsigned int  MapFlags;  // +0x54
    unsigned int  BlendMode; // +0x58
    nglTexture*   Tex;       // +0x5C
};
static_assert(sizeof(nglQuad) == 0x60, "nglQuad size mismatch");

// ============================================================================
// nglQuadNode — quad render node (108 bytes)
// ============================================================================
class nglQuadNode : public nglRenderNode {
public:
    nglQuad Quad;  // +0x0C

    virtual void Render();  // @0x8479B0
};
static_assert(sizeof(nglQuadNode) == 0x6C, "nglQuadNode size mismatch");

// ============================================================================
// nglDxTexCache — cached texture-state (128 bytes, owned by ngl_dx_tex_cache.o)
// ============================================================================
struct nglDxTexCacheClass {
    struct StageCache {
        unsigned int WrapU;  // +0x00
        unsigned int WrapV;  // +0x04
    };
    StageCache Prev[4];  // +0x00
};
extern nglDxTexCacheClass nglDxTexCache;

// nglSyncDebug (data, owned by ngl_xboxr)
struct nglSyncDebugStruct {
    int DisableQuads;  // +0x00
    int DumpSceneFile; // +0x04
    int DumpFrameLog;  // +0x08
    int DumpTextures;  // +0x0C
};
extern nglSyncDebugStruct nglSyncDebug;

// ============================================================================
// Supporting types
// ============================================================================
struct nglDisplayModeType {
    int          Width;           // +0x00
    int          Height;          // +0x04
    bool         Progressive;     // +0x08
    bool         PAL;             // +0x09
    bool         Widescreen;      // +0x0A
    unsigned int ImpersonateMode; // +0x0C
};
static_assert(sizeof(nglDisplayModeType) == 0x10, "nglDisplayModeType size mismatch");

struct nglDxRenderState {
    unsigned int PrevBM;  // +0x00

    void SetBlendMode(unsigned int BM);  // ngl_dx_state.o
};

struct nglPerfInfoStruct {
    unsigned int ListWorkUsage;       // +0x00
    unsigned int ScratchWorkUsage;    // +0x04
    unsigned int PhysListWorkUsage;   // +0x08
    float        QuadMS;              // +0x0C
    float        FontMS;              // +0x10
    unsigned char _pad14[4];          // +0x14
    unsigned __int64 CPUStart;        // +0x18
    volatile unsigned __int64 RenderStart;   // +0x20
    volatile unsigned __int64 RenderFinish;  // +0x28
    unsigned __int64 ListSubmitCycles;       // +0x30
    unsigned __int64 ListSendCycles;         // +0x38
    unsigned __int64 QuadCycles;             // +0x40
    unsigned __int64 FontCycles;             // +0x48
    float        FPS;                // +0x50
    float        TotalMS;            // +0x54
    float        TotalSeconds;       // +0x58
    float        RenderMS;           // +0x5C
    float        CPUMS;              // +0x60
    float        FrameMS;            // +0x64
    float        ListSendMS;         // +0x68
    float        ListSubmitMS;       // +0x6C
    unsigned int TotalPolys;         // +0x70
    unsigned int TotalVerts;         // +0x74
    unsigned int NodeCount;          // +0x78
};

// ============================================================================
// Shader handles (data owned by ngl_xboxr:ngl_gpu_common.o)
// ============================================================================
struct nglGpuQuadPCUVVertexShader {
    static unsigned int Shader;   // ?Shader@nglGpuQuadPCUVVertexShader@@3KA
    static unsigned int* VS;
    static const unsigned int** VShaderTable;
};
struct nglGpuTexColPixelShader {
    static unsigned int* Shader;  // ?Shader@nglGpuTexColPixelShader@@3PAKA
    static unsigned int** PS;
    static const unsigned int** PShaderTable;
};
struct nglGpuQuadPCVertexShader {
    static unsigned int Shader;   // ?Shader@nglGpuQuadPCVertexShader@@3KA
    static unsigned int* VS;
    static const unsigned int** VShaderTable;
};
struct nglGpuColPixelShader {
    static unsigned int* Shader;  // ?Shader@nglGpuColPixelShader@@3PAKA
    static unsigned int** PS;
    static const unsigned int** PShaderTable;
};
struct nglGpuTexPixelShader {
    static unsigned int*  Shader;     // ?Shader@nglGpuTexPixelShader@@3PAKA
    static unsigned int** PS;
    static const unsigned int** PShaderTable;
};
struct nglGpuFilterPixelShader {
    static unsigned int*  Shader;     // ?Shader@nglGpuFilterPixelShader@@3PAKA
    static unsigned int** PS;
    static const unsigned int** PShaderTable;
};
struct nglGpuZFogPixelShader {
    static unsigned int*  Shader;     // ?Shader@nglGpuZFogPixelShader@@3PAKA
    static unsigned int** PS;
    static const unsigned int** PShaderTable;
};
struct nglGpuQuadPUVVertexShader {
    static unsigned int  Shader;      // ?Shader@nglGpuQuadPUVVertexShader@@3KA
    static unsigned int* VS;
    static const unsigned int** VShaderTable;
};
struct nglGpuQuadPUV4VertexShader {
    static unsigned int  Shader;      // ?Shader@nglGpuQuadPUV4VertexShader@@3KA
    static unsigned int* VS;
    static const unsigned int** VShaderTable;
};
struct nglGpuQuadPUVMatColVertexShader {
    static unsigned int  Shader;      // ?Shader@nglGpuQuadPUVMatColVertexShader@@3KA
    static unsigned int* VS;
    static const unsigned int** VShaderTable;
};

extern const _D3DVERTEXSHADERINPUT* nglGpuPCVertexElements;
extern const _D3DVERTEXSHADERINPUT* nglGpuPUVVertexElements;
extern const _D3DVERTEXSHADERINPUT* nglGpuPCUVVertexElements;
extern const _D3DVERTEXSHADERINPUT* nglGpuPUV4VertexElements;
extern gpuVertexFormat nglGpuPUV4VertexFmt;

// ============================================================================
// Externs
// ============================================================================
extern nglScene* nglBuildScene;
extern nglDisplayModeType nglDisplayMode;
extern nglDxRenderState nglDxState;
extern int nglTextureAnimFrame;
extern nglPerfInfoStruct nglPerfInfo;
extern gpuVertexFormat nglGpuPCUVVertexFmt;
extern gpuVertexFormat nglGpuPCVertexFmt;
extern gpuVertexFormat nglGpuPUVVertexFmt;
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern unsigned int gpuHashVertexBuffer;
extern unsigned int gpuHashVertexFormat;

extern int nglGetScreenWidth();
extern int nglGetScreenHeight();
extern void nglDxInitShaders(bool RegisterShaders);
extern void nglDxSetTexture(unsigned int Stage, nglTexture* Tex, unsigned int FilterFlags,
                            unsigned int MaxAnisotropy);
extern void nglDxSetTextureU(unsigned int Stage, unsigned int Mode);
extern void nglDxSetTextureV(unsigned int Stage, unsigned int Mode);
extern void nglDxResetDevice();

// ============================================================================
// Quad helper API (ngl_quad.o)
// ============================================================================
extern void nglInitQuad(nglQuad* Quad);
extern void nglSetQuadTex(nglQuad* Quad, nglTexture* Tex);
extern void nglSetQuadMapFlags(nglQuad* Quad, unsigned int MapFlags);
extern void nglSetQuadBlend(nglQuad* Quad, unsigned int Blend);
extern void nglSetQuadUV(nglQuad* Quad, float u1, float v1, float u2, float v2);
extern void nglSetQuadColor(nglQuad* Quad, unsigned int c);
extern void nglSetQuadRect(nglQuad* Quad, float x1, float y1, float x2, float y2);
extern void nglSetQuadZ(nglQuad* Quad, float z);
extern void nglSetQuadVPos(nglQuad* Quad, int VertIdx, float x, float y);
extern void nglSetQuadVUV(nglQuad* Quad, int VertIdx, float u, float v);
extern void nglSetQuadVColor(nglQuad* Quad, int VertIdx, unsigned int Color);
extern void nglRotateQuad(nglQuad* Quad, float cx, float cy, float theta);
extern void nglScaleQuad(nglQuad* Quad, float cx, float cy, float sx, float sy);
extern void nglRotateQuadUV(nglQuad* Quad, float cx, float cy, float theta);
extern void nglScaleQuadUV(nglQuad* Quad, float cx, float cy, float sx, float sy);
extern void nglSetQuadPos(nglQuad* Quad, float x, float y);
extern void nglListAddQuad(nglQuad* Quad);

extern void* nglListAlloc(unsigned int Bytes, unsigned int Alignment);
extern void  nglValidateMatrices(nglScene* Scene);
extern void  nglListAddNode_Opaque(nglRenderNode* Node, unsigned int Hash);
extern void  nglSceneDumpQuad(nglQuad* Quad);
extern void  tlWarning(const char* Format, ...);

// ============================================================================
// nglDxViewToScreenZ — project a view-space Z to screen Z (inline COMDAT)
// ea: 0x847B70
// ============================================================================
inline float nglDxViewToScreenZ(float z, nglScene* Scene) {
    float zc = z;
    if (Scene->NearZ <= z) {
        if (z > Scene->FarZ)
            zc = Scene->FarZ;
    } else {
        zc = Scene->NearZ;
    }
    __m128 v3 = _mm_add_ps(_mm_mul_ps(Scene->ViewToScreen.z.v, _mm_set1_ps(zc)),
                           Scene->ViewToScreen.w.v);
    float v4 = _mm_shuffle_ps(v3, v3, 170).m128_f32[0]
             / _mm_shuffle_ps(v3, v3, 255).m128_f32[0];
    if (v4 < 0.0f)
        return 0.0f;
    return v4 <= 16777215.0f ? v4 : 16777215.0f;
}

// gpuSetVertexShader::Inputs (data, owned by render_xboxr:cdGlowShader.o)
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;
extern gpuVertexFormat nglGpuPCUVVertexFmt;
extern gpuVertexFormat nglGpuPCVertexFmt;
extern unsigned int gpuHashIndexBuffer;

#endif // COD3_NGL_NGL_DX_QUAD_H
