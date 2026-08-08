// ============================================================================
// ngl_gpu_debug.h — GPU debug line/triangle rendering (2 non-inline funcs).
// Source: source/ngl_gpu_debug.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_gpu_debug.o):
//   nglDebugLineNode::Render @0x851460
//   nglDebugTriNode::Render  @0x851620
//   gpuEnableVertexAttributes / gpuDisableVertexAttributes (inline stubs)
// ============================================================================
#ifndef COD3_NGL_NGL_GPU_DEBUG_H
#define COD3_NGL_NGL_GPU_DEBUG_H

#include "d3d8.h"
#include "ngl/nglScene.h"
#include "ngl/ngl_dx_gpu.h"
#include "aeps/apsRenderNode.h"  // nglRenderNode

#include <intrin.h>

class nglSortInfo;

// ============================================================================
// nglDebugLineVertex — debug line vertex (16 bytes, verified against IDA)
// ============================================================================
struct nglDebugLineVertex {
    float        x;       // +0x00
    float        y;       // +0x04
    float        z;       // +0x08
    unsigned int color;   // +0x0C
};
static_assert(sizeof(nglDebugLineVertex) == 0x10, "nglDebugLineVertex size mismatch");

// ============================================================================
// nglDebugLineNode — debug line render node (20 bytes)
// ============================================================================
class nglDebugLineNode : public nglRenderNode {
public:
    int                  NVerts;  // +0x0C
    nglDebugLineVertex*  Verts;   // +0x10

    nglDebugLineNode();                 // @0x836180 (ngl_debug.o)
    virtual ~nglDebugLineNode();        // @0x8391E0 (ngl_debug.o)
    virtual void Render();        // @0x851460
    virtual void GetDesc(char* Desc);   // @0x836150 (ngl_debug.o)
    virtual void GetSortInfo(class nglSortInfo& Info);  // (ngl_debug.o)
};
static_assert(sizeof(nglDebugLineNode) == 0x14, "nglDebugLineNode size mismatch");

// ============================================================================
// nglDebugTriNode — debug triangle render node (20 bytes)
// ============================================================================
class nglDebugTriNode : public nglDebugLineNode {
public:
    nglDebugTriNode();                  // @0x8393C0 (inline, ngl_debug.o)
    virtual void Render();        // @0x851620
};
static_assert(sizeof(nglDebugTriNode) == 0x14, "nglDebugTriNode size mismatch");

// ============================================================================
// Debug shader handles (data owned by ngl_xboxr:ngl_gpu_common.o)
// ============================================================================
struct nglGpuDebugVertexShader {
    static unsigned int Shader;                 // ?Shader@nglGpuDebugVertexShader@@3KA
    static const unsigned int** VShaderTable;   // ?VShaderTable@nglGpuDebugVertexShader@@3PAPBIA
    static unsigned int* VS;                    // ?VS@nglGpuDebugVertexShader@@3PAKA
};

struct nglGpuDebugPixelShader {
    static unsigned int* Shader;                // ?Shader@nglGpuDebugPixelShader@@3PAKA
    static const unsigned int** PShaderTable;   // ?PShaderTable@nglGpuDebugPixelShader@@3PAPBIA
    static unsigned int** PS;                   // ?PS@nglGpuDebugPixelShader@@3PAPAKA
};

// nglGpuPCVertexFmt (data, owned by ngl_xboxr:ngl_gpu.o)
extern gpuVertexFormat nglGpuPCVertexFmt;

// gpuHash* globals (data, owned by ngl_xboxr:ngl_gpu.o)
extern unsigned int gpuHashVertexBuffer;
extern unsigned int gpuHashVertexFormat;
extern unsigned int gpuHashPixelShader;
extern unsigned int gpuHashVertexShader;

// gpuSetVertexShader::Inputs (data, owned by render_xboxr:cdGlowShader.o)
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;

// ============================================================================
// gpuSetVertexShader — load + select a vertex shader program. (inline COMDAT)
// ea: 0x7C2750
// ============================================================================
inline void gpuSetVertexShader(const unsigned int* shader) {
    if ((unsigned int)shader != gpuHashVertexShader) {
        gpuHashVertexShader = (unsigned int)shader;
        D3DDevice_LoadVertexShaderProgram(shader, 0);
        D3DDevice_SelectVertexShaderDirect((_D3DVERTEXATTRIBUTEFORMAT*)&gpuSetVertexShaderInputs, 0);
    }
}

// ============================================================================
// gpuEnableVertexAttributes / gpuDisableVertexAttributes (inline stubs)
// ea: 0x8517F0 / 0x851800
// ============================================================================
inline void gpuEnableVertexAttributes(unsigned int mask, gpuVertexFormat* format) {
}
inline void gpuDisableVertexAttributes(unsigned int mask, gpuVertexFormat* format) {
}

#endif // COD3_NGL_NGL_GPU_DEBUG_H
