// ============================================================================
// ngl_dx_gpu.h — GPU texture/format wrapper layer.
// Source: c:\cod\code\tl\ngl\include\dx/ngl_dx_gpu.h
//
// All functions here are inline COMDATs emitted into ngl_xboxr:ngl_gpu_texture.o
// (and other ngl objects). The map-mangled names are listed per function.
// ============================================================================
#ifndef COD3_NGL_NGL_DX_GPU_H
#define COD3_NGL_NGL_DX_GPU_H

#include "d3d8.h"
#include "ngl/nglScene.h"

#include <cstddef>
#include <intrin.h>

// tl_system.o (tl_xboxr, ported)
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern bool  _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void  tlFatal(const char* fmt, ...);

// ============================================================================
// gpuVertexFormat — GPU vertex format descriptor (12 bytes, verified against IDA)
// ============================================================================
struct gpuVertexFormat {
    int                          VertexSize;         // +0x00
    const _D3DVERTEXSHADERINPUT* Elements;           // +0x04
    _D3DVERTEXATTRIBUTEFORMAT*   VertexDeclaration;  // +0x08
};
static_assert(sizeof(gpuVertexFormat) == 0x0C, "gpuVertexFormat size mismatch");

// ============================================================================
// nglMeshSection â€” GPU mesh section (112 bytes, verified against IDA)
// ============================================================================
struct nglMeshSection {
    uint8_t        _pad0[0x10];             // +0x00 (link prefix)
    int            _pad10;                  // +0x10
    int            _pad14;                  // +0x14
    int            _pad18;                  // +0x18
    int            _pad1C;                  // +0x1C
    int            _pad20;                  // +0x20
    int            _pad24;                  // +0x24
    int            _pad28;                  // +0x28
    int            _pad2C;                  // +0x2C
    int            _pad30;                  // +0x30
    int            _pad34;                  // +0x34
    int            _pad38;                  // +0x38
    int            _pad3C;                  // +0x3C
    void*          VertexBuffer;            // +0x40
    int            VertexOffset;            // +0x44
    int            VertexSize;              // +0x48
    int            NVertices;               // +0x4C
    void*          IndexBuffer;             // +0x50
    int            IndexOffset;             // +0x54
    int            NIndices;                // +0x58
    int            IndexSize;               // +0x5C
    gpuVertexFormat* VertexFormat;          // +0x60
    int            PrimitiveType;           // +0x64
    int            LOD;                     // +0x68
    void*          Sphere;                  // +0x6C
    void*          BoxMin;                  // +0x70
    void*          BoxMax;                  // +0x74
    float          SqrtAreaEstimate;        // +0x78
    void*          Material;                // +0x7C
};
static_assert(sizeof(nglMeshSection) == 0x80, "nglMeshSection size mismatch");

// ============================================================================
// Scratch buffer state (ngl_gpu_meshedit.o)
// ============================================================================
extern int             nglScratchIndexBufferSize;
extern int             nglScratchVertexBufferSize;
extern int             nglScratchIndexOffset;
extern int             nglScratchVertexOffset;
extern D3DIndexBuffer* nglScratchIndexBufferA;
extern D3DVertexBuffer* nglScratchVertexBufferA;

extern int   nglScratchIndexAlloc(int Size);
extern int   nglScratchVertexAlloc(int Size, int Align);
extern nglMeshSection* nglCreateSection(int Prim, int NIndices, int NVertices,
                                        gpuVertexFormat* VertexFormat);
extern void  nglDestroySection(nglMeshSection* Section);
extern void* nglLockSectionIndices(nglMeshSection* Section);
extern void  nglUnlockSectionIndices(void);
extern unsigned char* nglLockSectionVertices(nglMeshSection* Section);
extern void  nglUnlockSectionVertices(void);
extern nglMeshSection* nglCreateScratchSection(int Prim, int NIndices, int NVertices,
                                               gpuVertexFormat* VertexFormat);
extern void  nglCopySection(nglMeshSection* Dst, nglMeshSection* Src);
extern nglMeshSection* nglCreateSectionCopy(nglMeshSection* Section);

extern void tlFatal(const char* Format, ...);
extern void nglSceneDumpEnd(void);
extern void nglRenderDebug(void);
extern void nglListSendBatch(void* pBatch);
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);
extern nglScene* nglRootBuildScene;
extern int nglSceneRecursion;

// ============================================================================
// nglMesh â€” mesh container (minimal view for nglCopySection overload)
// ============================================================================
struct nglMeshSections {
    nglMeshSection* Section;  // +0x00
};

struct nglMesh {
    uint8_t _pad0[0x10];              // +0x00
    nglMeshSections* Sections;        // +0x10
};

// ============================================================================
// gpuCreateVertexFormat — build a gpuVertexFormat from a D3D8 vertex element
// list (terminated by Format == D3DVSDT_END). Inline COMDAT in cdGlowShader.o.
// ea: 0x7C2670
// ============================================================================
inline gpuVertexFormat* gpuCreateVertexFormat(gpuVertexFormat* result, unsigned int size,
                                              const _D3DVERTEXSHADERINPUT* elements) {
    _D3DVERTEXATTRIBUTEFORMAT* decl = (_D3DVERTEXATTRIBUTEFORMAT*)tlMemAlloc(0x100, 8, 0);
    const _D3DVERTEXSHADERINPUT* src = elements;

    unsigned int n = 0;
    if (elements->Format != 2) {
        do {
            if (n >= 0x10 && _tlAssert("c:\\cod\\code\\tl\\ngl\\include\\dx/ngl_dx_gpu.h", 211,
                                       "n < 16", "Too many vertex elements."))
                __debugbreak();
            decl->Input[n].StreamIndex = src->StreamIndex;
            decl->Input[n].Offset = src->Offset;
            decl->Input[n].Format = src->Format;
            decl->Input[n].TessType = src->TessType;
            ++src;
            ++n;
        } while (src->Format != 2);
    }

    if (n < 0x10) {
        for (unsigned int i = n; i < 0x10; ++i) {
            decl->Input[i].StreamIndex = 0;
            decl->Input[i].Offset = 0;
            decl->Input[i].Format = 2;
            decl->Input[i].TessType = 0;
            decl->Input[i].TessSource = 0;
        }
    }

    result->VertexSize = size;
    result->Elements = elements;
    result->VertexDeclaration = decl;
    return result;
}

// ============================================================================
// gpuTextureFormat — Xbox GPU texture format enum (values from IDA)
// ============================================================================
enum gpuTextureFormat {
    GPU_TEXTURE_L8 = 0,
    GPU_TEXTURE_AL8 = 1,
    GPU_TEXTURE_A1R5G5B5 = 2,
    GPU_TEXTURE_X1R5G5B5 = 3,
    GPU_TEXTURE_A4R4G4B4 = 4,
    GPU_TEXTURE_R5G6B5 = 5,
    GPU_TEXTURE_A8R8G8B8 = 6,
    GPU_TEXTURE_X8R8G8B8 = 7,
    GPU_TEXTURE_P8 = 11,
    GPU_TEXTURE_DXT1 = 12,
    GPU_TEXTURE_DXT2 = 14,
    GPU_TEXTURE_DXT3 = 14,
    GPU_TEXTURE_DXT4 = 15,
    GPU_TEXTURE_DXT5 = 15,
    GPU_TEXTURE_LIN_A1R5G5B5 = 16,
    GPU_TEXTURE_LIN_R5G6B5 = 17,
    GPU_TEXTURE_LIN_A8R8G8B8 = 18,
    GPU_TEXTURE_LIN_Q8W8V8U8 = 18,
    GPU_TEXTURE_LIN_L8 = 19,
    GPU_TEXTURE_LIN_R8B8 = 22,
    GPU_TEXTURE_LIN_G8B8 = 23,
    GPU_TEXTURE_LIN_V8U8 = 23,
    GPU_TEXTURE_LIN_AL8 = 27,
    GPU_TEXTURE_LIN_X1R5G5B5 = 28,
    GPU_TEXTURE_LIN_A4R4G4B4 = 29,
    GPU_TEXTURE_LIN_X8R8G8B8 = 30,
    GPU_TEXTURE_LIN_X8L8V8U8 = 30,
    GPU_TEXTURE_LIN_A8 = 31,
    GPU_TEXTURE_LIN_A8L8 = 32,
    GPU_TEXTURE_YUY2 = 36,
    GPU_TEXTURE_UYVY = 37,
    GPU_TEXTURE_R6G5B5 = 39,
    GPU_TEXTURE_L6V5U5 = 39,
    GPU_TEXTURE_G8B8 = 40,
    GPU_TEXTURE_V8U8 = 40,
    GPU_TEXTURE_R8B8 = 41,
    GPU_TEXTURE_D24S8 = 42,
    GPU_TEXTURE_F24S8 = 43,
    GPU_TEXTURE_D16 = 44,
    GPU_TEXTURE_D16_LOCKABLE = 44,
    GPU_TEXTURE_F16 = 45,
    GPU_TEXTURE_LIN_D24S8 = 46,
    GPU_TEXTURE_LIN_F24S8 = 47,
    GPU_TEXTURE_LIN_D16 = 48,
    GPU_TEXTURE_LIN_F16 = 49,
    GPU_TEXTURE_L16 = 50,
    GPU_TEXTURE_V16U16 = 51,
    GPU_TEXTURE_LIN_L16 = 53,
    GPU_TEXTURE_LIN_V16U16 = 54,
    GPU_TEXTURE_LIN_R6G5B5 = 55,
    GPU_TEXTURE_LIN_L6V5U5 = 55,
    GPU_TEXTURE_R5G5B5A1 = 56,
    GPU_TEXTURE_R4G4B4A4 = 57,
    GPU_TEXTURE_A8B8G8R8 = 58,
    GPU_TEXTURE_Q8W8V8U8 = 58,
    GPU_TEXTURE_B8G8R8A8 = 59,
    GPU_TEXTURE_R8G8B8A8 = 60,
    GPU_TEXTURE_LIN_R5G5B5A1 = 61,
    GPU_TEXTURE_LIN_R4G4B4A4 = 62,
    GPU_TEXTURE_LIN_A8B8G8R8 = 63,
    GPU_TEXTURE_LIN_B8G8R8A8 = 64,
    GPU_TEXTURE_LIN_R8G8B8A8 = 65,
};

// ============================================================================
// gpuMultiSampleType — GPU multisample mode enum (values from IDA)
// ============================================================================
enum gpuMultiSampleType {
    NGL_GPU_MULTISAMPLE_NONE = 17,
    NGL_GPU_MULTISAMPLE_2X = 4385,
    NGL_GPU_MULTISAMPLE_4X = 4642,
};

// ============================================================================
// D3DResource type selectors (D3DRTYPE_*)
// ============================================================================
enum _D3DRESOURCETYPE {
    D3DRTYPE_TEXTURE = 3,
    D3DRTYPE_VOLUMETEXTURE = 4,
    D3DRTYPE_CUBETEXTURE = 5,
};

// ============================================================================
// D3DDevice — static D3D8 device entry-point wrappers.
// All inline COMDATs (ea: 0x84A790-0x84A880 in ngl_gpu_texture.o).
// ============================================================================
class D3DDevice {
public:
    static unsigned int __stdcall CreateTexture(unsigned int Width, unsigned int Height,
                                                unsigned int Levels, unsigned int Usage,
                                                _D3DFORMAT Format, unsigned int UnusedPool,
                                                D3DTexture** ppTexture) {
        D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(Width, Height, 1, Levels, Usage, Format,
                                                        D3DRTYPE_TEXTURE);
        *ppTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    static unsigned int __stdcall CreateVolumeTexture(unsigned int Width, unsigned int Height,
                                                      unsigned int Depth, unsigned int Levels,
                                                      unsigned int Usage, _D3DFORMAT Format,
                                                      unsigned int UnusedPool,
                                                      D3DVolumeTexture** ppVolumeTexture) {
        D3DVolumeTexture* Texture2 = (D3DVolumeTexture*)D3DDevice_CreateTexture2(Width, Height, Depth, Levels, Usage,
                                                              Format, D3DRTYPE_VOLUMETEXTURE);
        *ppVolumeTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    static unsigned int __stdcall CreateCubeTexture(unsigned int EdgeLength, unsigned int Levels,
                                                    unsigned int Usage, _D3DFORMAT Format,
                                                    unsigned int UnusedPool,
                                                    D3DCubeTexture** ppCubeTexture) {
        D3DCubeTexture* Texture2 = (D3DCubeTexture*)D3DDevice_CreateTexture2(EdgeLength, EdgeLength, 1, Levels,
                                                            Usage, Format, D3DRTYPE_CUBETEXTURE);
        *ppCubeTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    static unsigned int __stdcall CreateRenderTarget(unsigned int Width, unsigned int Height,
                                                     _D3DFORMAT Format, unsigned int UnusedMultiSample,
                                                     int UnusedLockable, D3DSurface** ppSurface) {
        D3DSurface* Surface2 = (D3DSurface*)D3DDevice_CreateSurface2(Width, Height, 1, Format);
        *ppSurface = Surface2;
        return Surface2 != NULL ? 0 : 0x8007000E;
    }

    static unsigned int __stdcall CreateDepthStencilSurface(unsigned int Width, unsigned int Height,
                                                            _D3DFORMAT Format,
                                                            unsigned int UnusedMultiSample,
                                                            D3DSurface** ppSurface) {
        D3DSurface* Surface2 = (D3DSurface*)D3DDevice_CreateSurface2(Width, Height, 2, Format);
        *ppSurface = Surface2;
        return Surface2 != NULL ? 0 : 0x8007000E;
    }
};

// ============================================================================
// gpuD3DDevice — D3D8 device wrapper (DX9-style member interface).
// All inline COMDATs (ea: 0x84A8E0-0x84A9E0 in ngl_gpu_texture.o).
// ============================================================================
class gpuD3DDevice {
public:
    unsigned int CreateTexture(unsigned int Width, unsigned int Height, unsigned int Levels,
                               unsigned int Usage, _D3DFORMAT Format, unsigned int Pool,
                               D3DTexture** ppTexture, void** pSharedHandle) {
        D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(Width, Height, 1, Levels, Usage, Format,
                                                        D3DRTYPE_TEXTURE);
        *ppTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    unsigned int CreateCubeTexture(unsigned int EdgeLength, unsigned int Levels, unsigned int Usage,
                                   _D3DFORMAT Format, unsigned int Pool,
                                   D3DCubeTexture** ppCubeTexture, void** pSharedHandle) {
        D3DCubeTexture* Texture2 = (D3DCubeTexture*)D3DDevice_CreateTexture2(EdgeLength, EdgeLength, 1, Levels,
                                                            Usage, Format, D3DRTYPE_CUBETEXTURE);
        *ppCubeTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    unsigned int CreateVolumeTexture(unsigned int Width, unsigned int Height, unsigned int Depth,
                                     unsigned int Levels, unsigned int Usage, _D3DFORMAT Format,
                                     unsigned int Pool, D3DVolumeTexture** ppVolumeTexture,
                                     void** pSharedHandle) {
        D3DVolumeTexture* Texture2 = (D3DVolumeTexture*)D3DDevice_CreateTexture2(Width, Height, Depth, Levels, Usage,
                                                              Format, D3DRTYPE_VOLUMETEXTURE);
        *ppVolumeTexture = Texture2;
        return Texture2 != NULL ? 0 : 0x8007000E;
    }

    unsigned int CreateRenderTarget(unsigned int Width, unsigned int Height, _D3DFORMAT Format,
                                    unsigned int MultiSample, unsigned int MultisampleQuality,
                                    int Lockable, D3DSurface** ppSurface, void** pSharedHandle) {
        D3DSurface* Surface2 = (D3DSurface*)D3DDevice_CreateSurface2(Width, Height, 1, Format);
        *ppSurface = Surface2;
        return Surface2 != NULL ? 0 : 0x8007000E;
    }

    unsigned int CreateDepthStencilSurface(unsigned int Width, unsigned int Height, _D3DFORMAT Format,
                                           unsigned int MultiSample, unsigned int MultisampleQuality,
                                           int Discard, D3DSurface** ppSurface,
                                           void** pSharedHandle) {
        D3DSurface* Surface2 = (D3DSurface*)D3DDevice_CreateSurface2(Width, Height, 2, Format);
        *ppSurface = Surface2;
        return Surface2 != NULL ? 0 : 0x8007000E;
    }
};

// ============================================================================
// gpuCreate* — free-function texture creators (inline COMDATs, ngl_gpu_texture.o)
// ============================================================================
inline D3DTexture* gpuCreateTexture(unsigned int width, unsigned int height, unsigned int levels,
                                    gpuTextureFormat format, bool rendertarget,
                                    gpuMultiSampleType multisample) {
    D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(width, height, 1, levels, 0x200, format,
                                                    D3DRTYPE_TEXTURE);
    if (Texture2 == NULL)
        tlFatal("gpu: Out of memory for texture.");
    return Texture2;
}

inline D3DTexture* gpuCreateCubeTexture(unsigned int size, unsigned int levels,
                                        gpuTextureFormat format, bool rendertarget,
                                        gpuMultiSampleType multisample) {
    D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(size, size, 1, levels, 0x200, format,
                                                    D3DRTYPE_CUBETEXTURE);
    return Texture2;
}

inline D3DTexture* gpuCreateVolumeTexture(unsigned int width, unsigned int height,
                                          unsigned int depth, unsigned int levels,
                                          gpuTextureFormat format, bool rendertarget,
                                          gpuMultiSampleType multisample) {
    D3DTexture* Texture2 = (D3DTexture*)D3DDevice_CreateTexture2(width, height, depth, levels, 0x200, format,
                                                    D3DRTYPE_VOLUMETEXTURE);
    return Texture2;
}

inline D3DSurface* gpuCreateRenderSurface(unsigned int w, unsigned int h, gpuTextureFormat format,
                                          gpuMultiSampleType multisample) {
    return D3DDevice_CreateSurface2(w, h, 1, format);
}

inline D3DSurface* gpuCreateDepthStencilSurface(unsigned int w, unsigned int h,
                                                gpuTextureFormat format,
                                                gpuMultiSampleType multisample) {
    return D3DDevice_CreateSurface2(w, h, 2, format);
}

// ============================================================================
// XGIsTiledFormat — is the format a swizzled/tiled format? (inline COMDAT)
// ea: 0x84A8B0
// ============================================================================
inline int XGIsTiledFormat(_D3DFORMAT Format) {
    return XGIsSwizzledFormat(Format);
}

// ============================================================================
// gpuHash* — D3D resource caching hashes (data, owned by ngl_xboxr:ngl_gpu.o)
// ============================================================================
extern unsigned int gpuHashVertexBuffer;
extern unsigned int gpuHashVertexFormat;
extern unsigned int gpuHashIndexBuffer;

// ============================================================================
// gpuSetVertexBuffer — bind a vertex buffer + format (inline COMDAT).
// ea: 0x7C9890
// ============================================================================
inline void gpuSetVertexBuffer(D3DVertexBuffer* vtx, gpuVertexFormat* vertexformat,
                               unsigned int vtxoffset, unsigned int streamidx) {
    unsigned int v4 = vtxoffset + (streamidx << 16) + vtx->Data;
    _D3DVERTEXATTRIBUTEFORMAT* VertexDeclaration = vertexformat->VertexDeclaration;
    if (v4 != gpuHashVertexBuffer || VertexDeclaration != (_D3DVERTEXATTRIBUTEFORMAT*)gpuHashVertexFormat) {
        _D3DSTREAM_INPUT stream;
        stream.VertexBuffer = vtx;
        stream.Offset = vtxoffset;
        stream.Stride = vertexformat->VertexSize;
        D3DDevice_SetVertexShaderInputDirect(VertexDeclaration, 1, &stream);
        gpuHashVertexBuffer = v4;
        gpuHashVertexFormat = (unsigned int)VertexDeclaration;
    }
}

// ============================================================================
// gpuIndexType — index buffer element type enum (values from IDA)
// ============================================================================
enum gpuIndexType {
    GPU_INDEX_16 = 101,
    GPU_INDEX_32 = 101,
};

// ============================================================================
// gpu buffer helpers (inline COMDATs, apsVertexBuffer.o)
// ============================================================================
inline D3DIndexBuffer* gpuCreateIndexBuffer(unsigned int nindices, gpuIndexType indextype) {
    return D3DDevice_CreateIndexBuffer2(nindices * (2 * (indextype == GPU_INDEX_16) + 2));
}

inline void* gpuMapIndexBuffer(D3DIndexBuffer* buf, unsigned int offset) {
    return (void*)(offset + buf->Data);
}

inline void gpuUnmapIndexBuffer(D3DIndexBuffer* buf) {
}

// ============================================================================
// gpuPrimType — primitive type selector (values from IDA)
// ============================================================================
enum gpuPrimType {
    GPU_PRIM_POINTLIST = 1,
    GPU_PRIM_LINELIST = 2,
    GPU_PRIM_LINESTRIP = 4,
    GPU_PRIM_TRIANGLELIST = 5,
    GPU_PRIM_TRIANGLESTRIP = 6,
    GPU_PRIM_TRIANGLEFAN = 7,
    GPU_PRIM_QUADLIST = 8,
};

inline void gpuDrawIndexedPrimitive(gpuPrimType prim, unsigned int nindices, unsigned int idxoffset,
                                    D3DIndexBuffer* idx, gpuIndexType indexformat,
                                    unsigned int nverts, unsigned int vtxoffset,
                                    D3DVertexBuffer* vtx, gpuVertexFormat* vertexformat) {
    gpuSetVertexBuffer(vtx, vertexformat, vtxoffset, 0);
    D3DDevice_DrawIndexedVertices((_D3DPRIMITIVETYPE)prim, nindices,
                                  (const unsigned short*)(idxoffset + idx->Data));
}

// ============================================================================
// GPU index/section helpers (ngl_gpu.o)
// ============================================================================
extern unsigned int _nglGpuUnpackTriangleList(D3DIndexBuffer* idx, unsigned short* buf,
                                              unsigned int nindices);
extern void _nglGpuPackTriangleList(D3DIndexBuffer* idx, unsigned short* buf,
                                    unsigned int nindices);
extern void nglGpuPackIndexBuffer(D3DIndexBuffer* idx, gpuPrimType primtype,
                                  gpuIndexType idxformat, unsigned short* buf,
                                  unsigned int nindices);
extern unsigned int nglGpuUnpackIndexBuffer(D3DIndexBuffer* idx, gpuPrimType primtype,
                                            gpuIndexType idxformat, unsigned short* buf,
                                            unsigned int nindices);
extern void nglGpuDrawSection(nglMeshSection* Section);
extern void ngliListSend(void);
extern void nglGpuAcquireDevice(void);
extern void nglGpuReleaseDevice(void);
extern void nglGpuInitShaders(void);

#endif // COD3_NGL_NGL_DX_GPU_H
