// ============================================================================
// ngl_gpu_meshedit.cpp â€” GPU mesh-section management (12 funcs).
// Source: src/gpu/ngl_gpu_meshedit.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_gpu_meshedit.o).
// ============================================================================

#include "ngl_dx_gpu.h"

#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* msg);
extern void tlMemFree(void* Ptr);
extern void* nglListAlloc(unsigned int Bytes, unsigned int Alignment);

// Scratch state
int             nglScratchIndexBufferSize = 0;
int             nglScratchVertexBufferSize = 0;
int             nglScratchIndexOffset = 0;
int             nglScratchVertexOffset = 0;
D3DIndexBuffer* nglScratchIndexBufferA = NULL;
D3DVertexBuffer* nglScratchVertexBufferA = NULL;

// ============================================================================
// nglScratchIndexAlloc â€” ea: 0x843110
// ============================================================================
int nglScratchIndexAlloc(int Size) {
    if (Size >= nglScratchIndexBufferSize &&
        _tlAssert("src/gpu/ngl_gpu_meshedit.cpp", 16,
                  "Size < nglScratchIndexBufferSize",
                  "Scratch index buffer too small to fit allocation request."))
        __debugbreak();
    int v1 = nglScratchIndexOffset;
    if (nglScratchIndexOffset + Size > nglScratchIndexBufferSize)
        v1 = 0;
    nglScratchIndexOffset = Size + v1;
    return v1;
}

// ============================================================================
// nglScratchVertexAlloc â€” ea: 0x843160
// ============================================================================
int nglScratchVertexAlloc(int Size, int Align) {
    if (Size >= nglScratchVertexBufferSize &&
        _tlAssert("src/gpu/ngl_gpu_meshedit.cpp", 26,
                  "Size < nglScratchVertexBufferSize",
                  "Scratch vertex buffer too small to fit allocation request."))
        __debugbreak();
    int result = nglScratchVertexOffset;
    if (Align)
        result = Align * ((nglScratchVertexOffset + Align - 1) / Align);
    if (result + Size > nglScratchVertexBufferSize)
        result = 0;
    nglScratchVertexOffset = Size + result;
    return result;
}

// ============================================================================
// nglCreateSection â€” ea: 0x8431D0
// ============================================================================
nglMeshSection* nglCreateSection(int Prim, int NIndices, int NVertices,
                                 gpuVertexFormat* VertexFormat) {
    nglMeshSection* v4 = (nglMeshSection*)tlMemAlloc(0x70u, 0x10u, 0);
    v4->VertexFormat = VertexFormat;
    memset(&v4->_pad0, 0, 0x0C);
    *(unsigned long long*)&v4->_pad0[4] = 0x749DC5AE00000000LL;
    v4->IndexSize = 2 * (NVertices >= 0xFFFF) + 2;
    v4->PrimitiveType = Prim;
    v4->NIndices = NIndices;
    if (NIndices != 0)
        v4->IndexBuffer = D3DDevice_CreateIndexBuffer2(4 * NIndices);
    else
        v4->IndexBuffer = NULL;
    v4->IndexOffset = 0;
    v4->NVertices = NVertices;
    v4->VertexBuffer = D3DDevice_CreateVertexBuffer2(NVertices * VertexFormat->VertexSize);
    v4->VertexOffset = 0;
    v4->LOD = -1;
    return v4;
}

// ============================================================================
// nglDestroySection â€” ea: 0x8432B0
// ============================================================================
void nglDestroySection(nglMeshSection* Section) {
    D3DResource_Release((D3DResource*)Section->IndexBuffer);
    D3DResource_Release((D3DResource*)Section->VertexBuffer);
    tlMemFree(Section);
}

// ============================================================================
// nglLockSectionIndices â€” ea: 0x8432E0
// ============================================================================
void* nglLockSectionIndices(nglMeshSection* Section) {
    return (unsigned char*)Section->IndexBuffer + Section->IndexOffset;
}

// ============================================================================
// nglUnlockSectionIndices â€” ea: 0x843300
// ============================================================================
void nglUnlockSectionIndices() {
}

// ============================================================================
// nglLockSectionVertices â€” ea: 0x843310
// ============================================================================
unsigned char* nglLockSectionVertices(nglMeshSection* Section) {
    int VertexOffset = Section->VertexOffset;
    return &((unsigned char*)D3DVertexBuffer_Lock2((D3DVertexBuffer*)Section->VertexBuffer, 0x20u))[VertexOffset];
}

// ============================================================================
// nglUnlockSectionVertices â€” ea: 0x843330
// ============================================================================
void nglUnlockSectionVertices() {
}

// ============================================================================
// nglCreateScratchSection â€” ea: 0x843340
// ============================================================================
nglMeshSection* nglCreateScratchSection(int Prim, int NIndices, int NVertices,
                                        gpuVertexFormat* VertexFormat) {
    nglMeshSection* v4 = (nglMeshSection*)nglListAlloc(0x70u, 0x10u);
    memset(&v4->_pad0, 0, 0x0C);
    *(unsigned long long*)&v4->_pad0[4] = 0x749DC5AE00000000LL;
    v4->PrimitiveType = Prim;
    v4->IndexSize = 2;
    v4->VertexFormat = VertexFormat;
    v4->NIndices = NIndices;
    v4->IndexBuffer = nglScratchIndexBufferA;
    v4->IndexOffset = nglScratchIndexAlloc(2 * NIndices);
    v4->NVertices = NVertices;
    v4->VertexBuffer = nglScratchVertexBufferA;
    v4->VertexOffset = nglScratchVertexAlloc(NVertices * VertexFormat->VertexSize,
                                             VertexFormat->VertexSize);
    v4->VertexSize = VertexFormat->VertexSize;
    v4->LOD = -1;
    return v4;
}

// ============================================================================
// nglCopySection â€” ea: 0x843400
// ============================================================================
void nglCopySection(nglMeshSection* Dst, nglMeshSection* Src) {
    if ((Dst->NIndices != Src->NIndices || Dst->IndexSize != Src->IndexSize) &&
        _tlAssert("src/gpu/ngl_gpu_meshedit.cpp", 113,
                  "Dst->NIndices == Src->NIndices && Dst->IndexSize == Src->IndexSize",
                  "Index buffer sizes do not match."))
        __debugbreak();
    memcpy((unsigned char*)Dst->IndexBuffer + Dst->IndexOffset,
           (unsigned char*)Src->IndexBuffer + Src->IndexOffset,
           Dst->NIndices * Dst->IndexSize);
    if ((Dst->NVertices != Src->NVertices ||
         Dst->VertexFormat->VertexSize != Src->VertexFormat->VertexSize) &&
        _tlAssert("src/gpu/ngl_gpu_meshedit.cpp", 120,
                  "Dst->NVertices == Src->NVertices && Dst->VertexFormat->VertexSize == Src->VertexFormat->VertexSize",
                  "Vertex buffer sizes do not match."))
        __debugbreak();
    int VertexOffset = Src->VertexOffset;
    unsigned char* v3 = &((unsigned char*)D3DVertexBuffer_Lock2((D3DVertexBuffer*)Src->VertexBuffer, 0x20u))[VertexOffset];
    memcpy(&((unsigned char*)D3DVertexBuffer_Lock2((D3DVertexBuffer*)Dst->VertexBuffer, 0x20u))[Dst->VertexOffset],
           v3, Dst->NVertices * Dst->VertexFormat->VertexSize);
    Dst->Sphere = Src->Sphere;
    Dst->BoxMin = Src->BoxMin;
    Dst->BoxMax = Src->BoxMax;
    Dst->SqrtAreaEstimate = Src->SqrtAreaEstimate;
}

// ============================================================================
// nglCopySection (mesh overload) â€” ea: 0x843540
// ============================================================================
void nglCopySection(nglMesh* Mesh, int SectionIdx, nglMesh* SrcMesh, int SrcSectionIdx) {
    nglCopySection(Mesh->Sections[SectionIdx].Section, SrcMesh->Sections[SrcSectionIdx].Section);
}

// ============================================================================
// nglCreateSectionCopy â€” ea: 0x843570
// ============================================================================
nglMeshSection* nglCreateSectionCopy(nglMeshSection* Section) {
    nglMeshSection* v1 = nglCreateSection(Section->PrimitiveType, Section->NIndices,
                                          Section->NVertices, Section->VertexFormat);
    nglCopySection(v1, Section);
    v1->Material = Section->Material;
    return v1;
}
