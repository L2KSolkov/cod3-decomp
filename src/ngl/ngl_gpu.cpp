// ============================================================================
// ngl_gpu.cpp â€” GPU index/section helpers (subset of ngl_gpu.o).
// Source: src/gpu/ngl_gpu.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_gpu.o).
// ============================================================================

#include "ngl_dx_gpu.h"
#include "ngl_dx_quad.h"
#include "ngl_gpu_debug.h"

#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* msg);
extern void nglSceneDumpEnd(void);
extern void nglRenderDebug(void);
extern void nglListSendBatch(void* pBatch);
extern void tlFatal(const char* Format, ...);

struct nglDebugStruct {
    int DumpFrameLog;
    int DumpSceneFile;
    int DumpTextures;
};
extern nglDebugStruct nglDebug;

extern nglScene* nglRootBuildScene;
int nglSceneRecursion = 0;

// ============================================================================
// _nglGpuUnpackTriangleList â€” ea: 0x84C5A0
// ============================================================================
unsigned int _nglGpuUnpackTriangleList(D3DIndexBuffer* idx, unsigned short* buf,
                                       unsigned int nindices) {
    unsigned short* Data = (unsigned short*)idx->Data;
    unsigned int ntris = 0;
    if (nindices > 2) {
        unsigned int v6 = (nindices - 3) / 3 + 1;
        do {
            unsigned short v7 = Data[2];
            unsigned short v8 = Data[1];
            if (v7 != v8 && v7 != Data[0] && v8 != Data[0]) {
                buf[2] = v7;
                buf[1] = Data[1];
                buf[0] = Data[0];
                buf += 3;
                ++ntris;
            }
            Data += 3;
            --v6;
        } while (v6 != 0);
        return ntris;
    }
    return 0;
}

// ============================================================================
// _nglGpuPackTriangleList â€” ea: 0x84C620
// ============================================================================
void _nglGpuPackTriangleList(D3DIndexBuffer* idx, unsigned short* buf,
                             unsigned int nindices) {
    unsigned short* Data = (unsigned short*)idx->Data;
    if (nindices > 2) {
        unsigned int v5 = (nindices - 3) / 3 + 1;
        do {
            Data[2] = buf[2];
            Data[1] = buf[1];
            Data[0] = buf[0];
            Data += 3;
            buf += 3;
            --v5;
        } while (v5 != 0);
    }
}

// ============================================================================
// nglGpuPackIndexBuffer â€” ea: 0x84C670
// ============================================================================
void nglGpuPackIndexBuffer(D3DIndexBuffer* idx, gpuPrimType primtype,
                           gpuIndexType idxformat, unsigned short* buf,
                           unsigned int nindices) {
    if (primtype == GPU_PRIM_TRIANGLELIST && idxformat == GPU_INDEX_16) {
        _nglGpuPackTriangleList(idx, buf, nindices);
    } else if (_tlAssert("src/gpu/ngl_gpu.cpp", 1037, "false",
                         "nglGpuPackIndexBuffer() : unsupported mode")) {
        __debugbreak();
    }
}

// ============================================================================
// nglGpuUnpackIndexBuffer â€” ea: 0x84D2A0
// ============================================================================
unsigned int nglGpuUnpackIndexBuffer(D3DIndexBuffer* idx, gpuPrimType primtype,
                                     gpuIndexType idxformat, unsigned short* buf,
                                     unsigned int nindices) {
    switch (primtype) {
    case GPU_PRIM_TRIANGLELIST:
        if (idxformat == GPU_INDEX_16)
            return _nglGpuUnpackTriangleList(idx, buf, nindices);
        memcpy(buf, (const void*)(size_t)idx->Data, 4 * nindices);
        return nindices / 3;
    default:
        if (_tlAssert("src/gpu/ngl_gpu.cpp", 1020, "false", "Invalid primitive type."))
            __debugbreak();
        return 0;
    }
}

// ============================================================================
// nglGpuDrawSection â€” ea: 0x84C6C0
// ============================================================================
void nglGpuDrawSection(nglMeshSection* Section) {
    if (Section->NIndices) {
        int IndexOffset = Section->IndexOffset;
        D3DIndexBuffer* IndexBuffer = (D3DIndexBuffer*)Section->IndexBuffer;
        gpuSetVertexBuffer((D3DVertexBuffer*)Section->VertexBuffer, Section->VertexFormat,
                           Section->VertexOffset, 0);
        D3DDevice_DrawIndexedVertices((_D3DPRIMITIVETYPE)Section->PrimitiveType,
                                      Section->NIndices,
                                      (const unsigned short*)((unsigned char*)IndexBuffer->Data + IndexOffset));
    } else {
        gpuSetVertexBuffer((D3DVertexBuffer*)Section->VertexBuffer, Section->VertexFormat,
                           0, 0);
        D3DDevice_DrawVerticesUP((_D3DPRIMITIVETYPE)Section->PrimitiveType,
                                 Section->NVertices,
                                 (unsigned char*)Section->VertexBuffer + Section->VertexOffset,
                                 Section->VertexFormat->VertexSize);
    }
}

// ============================================================================
// ngliListSend â€” ea: 0x84D230
// ============================================================================
void ngliListSend() {
    if (nglBuildScene != nglRootBuildScene)
        tlFatal("n`glListSend called while one or more scenes were still active (need to call nglListEndScene).\n");
    if (nglSyncDebug.DumpSceneFile != 0)
        nglSceneDumpEnd();
    if (nglSyncDebug.DumpFrameLog != 0)
        nglDebug.DumpFrameLog = 0;
    if (nglSyncDebug.DumpSceneFile != 0)
        nglDebug.DumpSceneFile = 0;
    if (nglSyncDebug.DumpTextures != 0)
        nglDebug.DumpTextures = 0;
    nglRenderDebug();
    nglListSendBatch(NULL);
}

// ============================================================================
// nglGpuAcquireDevice â€” ea: 0x84D210
// ============================================================================
void nglGpuAcquireDevice() {
}

// ============================================================================
// nglGpuReleaseDevice â€” ea: 0x84D220
// ============================================================================
void nglGpuReleaseDevice() {
}

// ============================================================================
// nglGpuInitShaders â€” ea: 0x84D000
// ============================================================================
void nglGpuInitShaders() {
    gpuVertexFormat result;
    nglGpuPCVertexFmt = *gpuCreateVertexFormat(&result, 0x10u, nglGpuPCVertexElements);
    nglGpuPUVVertexFmt = *gpuCreateVertexFormat(&result, 0x14u, nglGpuPUVVertexElements);
    nglGpuPCUVVertexFmt = *gpuCreateVertexFormat(&result, 0x18u, nglGpuPCUVVertexElements);
    nglGpuPUV4VertexFmt = *gpuCreateVertexFormat(&result, 0x2Cu, nglGpuPUV4VertexElements);

    nglDxRegisterVShader(nglGpuDebugVertexShader::VS, nglGpuDebugVertexShader::VShaderTable[0]);
    nglGpuDebugVertexShader::Shader = nglGpuDebugVertexShader::VS[0];
    nglDxRegisterPShader(nglGpuDebugPixelShader::PS, nglGpuDebugPixelShader::PShaderTable[0]);
    nglGpuDebugPixelShader::Shader = nglGpuDebugPixelShader::PS[0];
    nglDxRegisterVShader(nglGpuQuadPCVertexShader::VS, nglGpuQuadPCVertexShader::VShaderTable[0]);
    nglGpuQuadPCVertexShader::Shader = nglGpuQuadPCVertexShader::VS[0];
    nglDxRegisterVShader(nglGpuQuadPUVVertexShader::VS, nglGpuQuadPUVVertexShader::VShaderTable[0]);
    nglGpuQuadPUVVertexShader::Shader = nglGpuQuadPUVVertexShader::VS[0];
    nglDxRegisterVShader(nglGpuQuadPCUVVertexShader::VS, nglGpuQuadPCUVVertexShader::VShaderTable[0]);
    nglGpuQuadPCUVVertexShader::Shader = nglGpuQuadPCUVVertexShader::VS[0];
    nglDxRegisterVShader(nglGpuQuadPUV4VertexShader::VS, nglGpuQuadPUV4VertexShader::VShaderTable[0]);
    nglGpuQuadPUV4VertexShader::Shader = nglGpuQuadPUV4VertexShader::VS[0];
    nglDxRegisterVShader(nglGpuQuadPUVMatColVertexShader::VS,
                         nglGpuQuadPUVMatColVertexShader::VShaderTable[0]);
    nglGpuQuadPUVMatColVertexShader::Shader = nglGpuQuadPUVMatColVertexShader::VS[0];
    nglDxRegisterPShader(nglGpuTexColPixelShader::PS, nglGpuTexColPixelShader::PShaderTable[0]);
    nglGpuTexColPixelShader::Shader = nglGpuTexColPixelShader::PS[0];
    nglDxRegisterPShader(nglGpuTexPixelShader::PS, nglGpuTexPixelShader::PShaderTable[0]);
    nglGpuTexPixelShader::Shader = nglGpuTexPixelShader::PS[0];
    nglDxRegisterPShader(nglGpuColPixelShader::PS, nglGpuColPixelShader::PShaderTable[0]);
    nglGpuColPixelShader::Shader = nglGpuColPixelShader::PS[0];
    nglDxRegisterPShader(nglGpuFilterPixelShader::PS, nglGpuFilterPixelShader::PShaderTable[0]);
    nglGpuFilterPixelShader::Shader = nglGpuFilterPixelShader::PS[0];
    nglDxRegisterPShader(nglGpuZFogPixelShader::PS, nglGpuZFogPixelShader::PShaderTable[0]);
    nglGpuZFogPixelShader::Shader = nglGpuZFogPixelShader::PS[0];
}
