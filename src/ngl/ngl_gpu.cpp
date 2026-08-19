// ============================================================================
// ngl_gpu.cpp â€” GPU index/section helpers (subset of ngl_gpu.o).
// Source: src/gpu/ngl_gpu.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_gpu.o).
// ============================================================================

#include "ngl_dx_gpu.h"
#include "ngl_dx_quad.h"
#include "ngl_dx_fsaa.h"
#include "ngl_gpu_debug.h"
#include "nglDebug.h"

#include <string.h>

// ngl_gpu.o data (shader/buffer hash registers, init 0xDEADBEEF)
unsigned int gpuHashVertexBuffer = 0xDEADBEEF;   // ?gpuHashVertexBuffer@@3IA
unsigned int gpuHashPixelShader = 0xDEADBEEF;    // ?gpuHashPixelShader@@3IA
unsigned int gpuHashVertexShader = 0xDEADBEEF;   // ?gpuHashVertexShader@@3IA
unsigned int gpuHashVertexFormat = 0xDEADBEEF;   // ?gpuHashVertexFormat@@3IA
gpuVertexFormat nglGpuPCUVVertexFmt;             // ?nglGpuPCUVVertexFmt@@3UgpuVertexFormat@@A (BSS)

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* msg);
extern void nglSceneDumpEnd(void);
extern void nglRenderDebug(void);
struct jqBatch;
extern void nglListSendBatch(jqBatch* pBatch);
extern void tlFatal(const char* Format, ...);

extern nglDebugStruct nglDebug;   // ngl_debug.o (data)

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
// _nglGpuUnpackTriangleStrip / Fan - ea: 0x84DCD0-0x84DE60
// The four instantiations below are emitted by ngl_gpu.o and are selected by
// nglGpuUnpackIndexBuffer according to the source index width.
// ============================================================================
template <typename T>
unsigned int _nglGpuUnpackTriangleStrip(D3DIndexBuffer* idx, T* buf,
                                         unsigned int nindices) {
    T* Data = (T*)(size_t)idx->Data;
    unsigned int result = 0;
    unsigned int ntris = 0;
    if (nindices != 0) {
        T* v6 = (T*)((unsigned char*)Data - sizeof(T) * 2);
        do {
            if (result >= 2) {
                T v7 = v6[2];
                T v8 = v6[1];
                if (v7 != v8 && v7 != *v6 && v8 != *v6) {
                    if ((result & 1) != 0) {
                        *buf = v7;
                        buf[1] = v6[1];
                        buf[2] = *v6;
                    } else {
                        buf[2] = v7;
                        buf[1] = v6[1];
                        *buf = *v6;
                    }
                    buf += 3;
                    ++ntris;
                }
            }
            ++result;
            ++v6;
        } while (result < nindices);
        return ntris;
    }
    return result;
}

template <typename T>
unsigned int _nglGpuUnpackTriangleFan(D3DIndexBuffer* idx, T* buf,
                                      unsigned int nindices) {
    T* Data = (T*)(size_t)idx->Data;
    unsigned int result = 0;
    T* indices = Data;
    unsigned int ntris = 0;
    if (nindices == 0)
        return 0;
    T* v6 = (T*)((unsigned char*)Data - sizeof(T));
    do {
        if (result >= 2) {
            T v7 = v6[1];
            if (v7 != *v6) {
                T v8 = *(v6 - 1);
                if (v7 != v8 && *v6 != v8) {
                    buf[2] = v7;
                    buf[1] = *v6;
                    *buf = *indices;
                    buf += 3;
                    ++ntris;
                }
            }
        }
        ++result;
        ++v6;
    } while (result < nindices);
    return ntris;
}

// ============================================================================
// nglGpuPackIndexBuffer â€” ea: 0x84C670
// ============================================================================
void nglGpuPackIndexBuffer(D3DIndexBuffer* idx, gpuPrimType primtype,
                           gpuIndexType idxformat, unsigned int* buf,
                           unsigned int nindices) {
    if (primtype == GPU_PRIM_TRIANGLELIST && idxformat == GPU_INDEX_16) {
        _nglGpuPackTriangleList(idx, (unsigned short*)buf, nindices);
    } else if (_tlAssert("src/gpu/ngl_gpu.cpp", 1037, "false",
                         "nglGpuPackIndexBuffer() : unsupported mode")) {
        __debugbreak();
    }
}

// ============================================================================
// nglGpuUnpackIndexBuffer â€” ea: 0x84D2A0
// ============================================================================
unsigned int nglGpuUnpackIndexBuffer(D3DIndexBuffer* idx, gpuPrimType primtype,
                                     gpuIndexType idxformat, unsigned int* buf,
                                     unsigned int nindices) {
    switch (primtype) {
    case GPU_PRIM_TRIANGLELIST:
        if (idxformat == GPU_INDEX_16)
            return _nglGpuUnpackTriangleList(idx, (unsigned short*)buf, nindices);
        memcpy(buf, (const void*)(size_t)idx->Data, 4 * nindices);
        return nindices / 3;
    case GPU_PRIM_TRIANGLESTRIP:
        if (idxformat == GPU_INDEX_16)
            return _nglGpuUnpackTriangleStrip<unsigned short>(idx, (unsigned short*)buf, nindices);
        return _nglGpuUnpackTriangleStrip<unsigned int>(idx, buf, nindices);
    case GPU_PRIM_TRIANGLEFAN:
        if (idxformat == GPU_INDEX_16)
            return _nglGpuUnpackTriangleFan<unsigned short>(idx, (unsigned short*)buf, nindices);
        return _nglGpuUnpackTriangleFan<unsigned int>(idx, buf, nindices);
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
        D3DDevice_DrawVertices((_D3DPRIMITIVETYPE)Section->PrimitiveType,
                               Section->VertexOffset / Section->VertexFormat->VertexSize,
                               Section->NVertices);
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

    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(nglGpuDebugVertexShader::VS),
                         nglGpuDebugVertexShader::VShaderTable[0]);
    nglGpuDebugVertexShader::Shader = nglGpuDebugVertexShader::VS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(nglGpuDebugPixelShader::PS), nglGpuDebugPixelShader::PShaderTable[0]);
    nglGpuDebugPixelShader::Shader = nglGpuDebugPixelShader::PS[0];
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(nglGpuQuadPCVertexShader::VS), nglGpuQuadPCVertexShader::VShaderTable[0]);
    nglGpuQuadPCVertexShader::Shader = nglGpuQuadPCVertexShader::VS[0];
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(nglGpuQuadPUVVertexShader::VS), nglGpuQuadPUVVertexShader::VShaderTable[0]);
    nglGpuQuadPUVVertexShader::Shader = nglGpuQuadPUVVertexShader::VS[0];
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(nglGpuQuadPCUVVertexShader::VS), nglGpuQuadPCUVVertexShader::VShaderTable[0]);
    nglGpuQuadPCUVVertexShader::Shader = nglGpuQuadPCUVVertexShader::VS[0];
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(nglGpuQuadPUV4VertexShader::VS), nglGpuQuadPUV4VertexShader::VShaderTable[0]);
    nglGpuQuadPUV4VertexShader::Shader = nglGpuQuadPUV4VertexShader::VS[0];
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(nglGpuQuadPUVMatColVertexShader::VS),
                         nglGpuQuadPUVMatColVertexShader::VShaderTable[0]);
    nglGpuQuadPUVMatColVertexShader::Shader = nglGpuQuadPUVMatColVertexShader::VS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(nglGpuTexColPixelShader::PS), nglGpuTexColPixelShader::PShaderTable[0]);
    nglGpuTexColPixelShader::Shader = nglGpuTexColPixelShader::PS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(nglGpuTexPixelShader::PS), nglGpuTexPixelShader::PShaderTable[0]);
    nglGpuTexPixelShader::Shader = nglGpuTexPixelShader::PS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(nglGpuColPixelShader::PS), nglGpuColPixelShader::PShaderTable[0]);
    nglGpuColPixelShader::Shader = nglGpuColPixelShader::PS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(nglGpuFilterPixelShader::PS), nglGpuFilterPixelShader::PShaderTable[0]);
    nglGpuFilterPixelShader::Shader = nglGpuFilterPixelShader::PS[0];
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(nglGpuZFogPixelShader::PS), nglGpuZFogPixelShader::PShaderTable[0]);
    nglGpuZFogPixelShader::Shader = nglGpuZFogPixelShader::PS[0];
}

// ============================================================================
// Bone-matrix constants (ngl_gpu.o data)
// ============================================================================
static __m128 BonesArray[3 * 48 + 1];    // 0x10E3920
static __m128 BonesArray_0[3 * 48 + 1];  // 0x10E4230

// Row-vector matrix helpers (reconstructed from the IDA SSE chains).
static __m128 RowMul(const __m128 r, const math::Mat43* m) {
    return _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(r, r, 0), m->x.v),
                   _mm_mul_ps(_mm_shuffle_ps(r, r, 85), m->y.v)),
        _mm_mul_ps(_mm_shuffle_ps(r, r, 170), m->z.v));
}

// Affine product A*B: rows = RowMul(A.row, B), w = RowMul(A.w, B) + B.w.
static math::Mat43 MulMat43World(const math::Mat43& M, const math::Mat43& W) {
    math::Mat43 r;
    r.x.v = RowMul(M.x.v, &W);
    r.y.v = RowMul(M.y.v, &W);
    r.z.v = RowMul(M.z.v, &W);
    r.w.v = _mm_add_ps(RowMul(M.w.v, &W), W.w.v);
    return r;
}

// Transpose a Mat43 into 3 constant rows (3x3 transposed + w handled by caller).
static void Mat43ToConstants(const math::Mat43& M, __m128* out) {
    __m128 a = _mm_shuffle_ps(M.x.v, M.y.v, 0x44);
    __m128 b = _mm_shuffle_ps(M.z.v, M.w.v, 0x44);
    out[0] = _mm_shuffle_ps(a, b, 0x88);
    out[1] = _mm_shuffle_ps(a, b, 0xDD);
    out[2] = _mm_shuffle_ps(_mm_shuffle_ps(M.x.v, M.y.v, 0xEE),
                            _mm_shuffle_ps(M.z.v, M.w.v, 0xEE), 0x88);
}

static const __m128 kIdentityX = { 1.0f, 0.0f, 0.0f, 0.0f };
static const __m128 kIdentityY = { 0.0f, 1.0f, 0.0f, 0.0f };
static const __m128 kIdentityZ = { 0.0f, 0.0f, 1.0f, 0.0f };

// nglMeshNode::GetWToLNoScale (inline COMDAT, cdDynamicDecalShader.o).
static math::Mat43* MeshNode_GetWToLNoScale(const nglMeshNode* This, math::Mat43* result) {
    math::Mat43 LToWNoScale;
    if ((This->MeshParams->Flags & 2) != 0) {
        __m128 Scale = This->MeshParams->Scale.v;
        __m128 rcp = _mm_rcp_ps(Scale);
        __m128 inv = _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(rcp, Scale)), rcp);
        LToWNoScale.x.v = _mm_mul_ps(This->LocalToWorld.x.v, _mm_shuffle_ps(inv, inv, 0));
        LToWNoScale.y.v = _mm_mul_ps(This->LocalToWorld.y.v, _mm_shuffle_ps(inv, inv, 85));
        LToWNoScale.z.v = _mm_mul_ps(This->LocalToWorld.z.v, _mm_shuffle_ps(inv, inv, 170));
        LToWNoScale.w.v = This->LocalToWorld.w.v;
    } else {
        LToWNoScale = This->LocalToWorld;
    }
    math::Mat43 T;
    __m128 y = LToWNoScale.y.v;
    __m128 z = LToWNoScale.z.v;
    __m128 w = LToWNoScale.w.v;
    __m128 v7 = _mm_shuffle_ps(LToWNoScale.x.v, y, 0x44);
    __m128 v8 = _mm_shuffle_ps(v7, z, 0xDD);
    __m128 v16 = _mm_shuffle_ps(_mm_shuffle_ps(LToWNoScale.x.v, y, 0xEE), z, 0xA8);
    __m128 v9 = _mm_shuffle_ps(v7, z, 0x88);
    T.x.v = _mm_shuffle_ps(v9, _mm_setzero_ps(), 0xE4);
    T.y.v = v8;
    T.z.v = v16;
    T.w.v = _mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
                       _mm_add_ps(
                           _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(w, w, 0), v9),
                                      _mm_mul_ps(_mm_shuffle_ps(w, w, 85), v8)),
                           _mm_mul_ps(_mm_shuffle_ps(w, w, 170), v16)));
    *result = T;
    return result;
}

// ============================================================================
// nglGpuSetBonesWorld - ea: 0x84C740
// ============================================================================
void nglGpuSetBonesWorld(int p, nglMeshNode* MeshNode, nglMeshSection* Section) {
    int NBones = Section->NBones;
    nglMeshParams* MeshParams = MeshNode->MeshParams;
    math::Mat43* MeshBones = MeshParams->Bones;
    if (NBones > 48 &&
        _tlAssert("src/gpu/ngl_gpu.cpp", 1224, "NBones <= MAX_BONES",
                  "Too many bones in mesh section for vertex shader constants!"))
        __debugbreak();
    if (NBones != 0) {
        unsigned int Flags = MeshParams->Flags;
        nglSkeleton* Skeleton = (nglSkeleton*)MeshNode->Mesh->Skeleton;
        __m128* dst = BonesArray;
        if ((Flags & 4) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                const math::Mat43* Rel = &MeshBones[Section->BoneIndices[i]];
                const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
                math::Mat43 M = MulMat43World(Bone->InvRest, *Rel);
                Mat43ToConstants(M, dst);
            }
        } else if ((Flags & 8) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                const math::Mat43* Rel = &MeshBones[Section->BoneIndices[i]];
                const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
                math::Mat43 M = MulMat43World(MulMat43World(Bone->InvRest, *Rel),
                                              MeshNode->LocalToWorld);
                Mat43ToConstants(M, dst);
            }
        } else if ((Flags & 0x10) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                math::Mat43 M = MulMat43World(MeshBones[Section->BoneIndices[i]],
                                              MeshNode->LocalToWorld);
                Mat43ToConstants(M, dst);
            }
        } else {
            for (int i = 0; i < NBones; ++i, dst += 3)
                Mat43ToConstants(MeshNode->LocalToWorld, dst);
        }
    } else {
        Mat43ToConstants(MeshNode->LocalToWorld, BonesArray);
        Mat43ToConstants(MeshNode->LocalToWorld, BonesArray + 3);
        NBones = 2;
    }
    int v71 = 3 * NBones;
    int v72 = p + 96;
    if (v71 == 1)
        D3DDevice_SetVertexShaderConstant1Fast(v72, BonesArray);
    else
        D3DDevice_SetVertexShaderConstantNotInlineFast(v72, BonesArray, 4 * v71);
}

// ============================================================================
// nglGpuSetBonesLocal - ea: 0x84D370
// ============================================================================
void nglGpuSetBonesLocal(int p, nglMeshNode* MeshNode, nglMeshSection* Section) {
    float Const[4];
    *(unsigned int*)&Const[0] = 0x443FB132;   // 766.90088f
    Const[1] = (float)p;
    Const[2] = nglFSAAParams.v.m128_f32[0];
    Const[3] = nglFSAAParams.v.m128_f32[1];
    D3DDevice_SetVertexShaderConstant1Fast(0, Const);

    nglMeshParams* MeshParams = MeshNode->MeshParams;
    int NBones = Section->NBones;
    math::Mat43* Bones = MeshParams->Bones;
    if (NBones > 48 &&
        _tlAssert("src/gpu/ngl_gpu.cpp", 1165, "NBones <= MAX_BONES",
                  "Too many bones in mesh section for vertex shader constants!"))
        __debugbreak();
    if (NBones != 0) {
        unsigned int Flags = MeshParams->Flags;
        nglSkeleton* Skeleton = (nglSkeleton*)MeshNode->Mesh->Skeleton;
        __m128* dst = BonesArray_0;
        if ((Flags & 4) != 0) {
            math::Mat43 WToLNoScale;
            MeshNode_GetWToLNoScale(MeshNode, &WToLNoScale);
            for (int i = 0; i < NBones; ++i, dst += 3) {
                const math::Mat43* Rel = &Bones[Section->BoneIndices[i]];
                const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
                math::Mat43 M = MulMat43World(MulMat43World(Bone->InvRest, *Rel),
                                              WToLNoScale);
                Mat43ToConstants(M, dst);
            }
        } else if ((Flags & 8) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                const math::Mat43* Rel = &Bones[Section->BoneIndices[i]];
                const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
                math::Mat43 M = MulMat43World(Bone->InvRest, *Rel);
                Mat43ToConstants(M, dst);
            }
        } else if ((Flags & 0x10) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3)
                Mat43ToConstants(Bones[Section->BoneIndices[i]], dst);
        } else {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                dst[0] = kIdentityX;
                dst[1] = kIdentityY;
                dst[2] = kIdentityZ;
            }
        }
    } else {
        NBones = 1;
        BonesArray_0[0] = kIdentityX;
        BonesArray_0[1] = kIdentityY;
        BonesArray_0[2] = kIdentityZ;
    }
    int v93 = 3 * NBones;
    if (v93 == 1)
        D3DDevice_SetVertexShaderConstant1Fast(p + 96, BonesArray_0);
    else
        D3DDevice_SetVertexShaderConstantNotInlineFast(p + 96, BonesArray_0, 4 * v93);
}

// ============================================================================
// nglGpuUnpackVertexBuffer - ea: 0x84B760
// ============================================================================
void nglGpuUnpackVertexBuffer(D3DVertexBuffer* vtx, gpuVertexFormat* fmt,
                              gpuVertexElementUsage usage, unsigned int usageindex,
                              math::Vector4* buf, unsigned int start,
                              unsigned int count) {
    const _D3DVERTEXSHADERINPUT* elem = &fmt->Elements[usage];
    unsigned int offset = start * fmt->VertexSize;
    unsigned char* src =
        &((unsigned char*)D3DVertexBuffer_Lock2(vtx, 0x20u))[offset + elem->Offset];
    switch (elem->Format) {
    case 0x11:  // SHORT1N
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[1] = 0.0f;
            v.v.m128_f32[2] = 0.0f;
            v.v.m128_f32[3] = 1.0f;
            do {
                v.v.m128_f32[0] = (float)*(short*)src * 0.000030518509f;
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x12:  // FLOAT1
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[1] = 0.0f;
            v.v.m128_f32[2] = 0.0f;
            v.v.m128_f32[3] = 1.0f;
            do {
                v.v.m128_f32[0] = *(float*)src;
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x15:  // SHORT1
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[1] = 0.0f;
            v.v.m128_f32[2] = 0.0f;
            v.v.m128_f32[3] = 1.0f;
            do {
                v.v.m128_f32[0] = (float)*(short*)src;
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x16:  // packed 565 (signed 10-bit RGB)
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[3] = 1.0f;
            do {
                unsigned int packed = *(unsigned int*)src;
                v.v.m128_f32[0] =
                    (float)((packed & 0x3FF) - 2 * (packed & 0x200)) * 0.0019569471f;
                v.v.m128_f32[1] =
                    (float)(((packed >> 10) & 0x3FF) - 2 * ((packed >> 10) & 0x200)) *
                    0.0019569471f;
                v.v.m128_f32[2] =
                    (float)(((packed >> 20) & 0x3FF) - 2 * ((packed >> 20) & 0x200)) *
                    0.0019569471f;
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x21:  // SHORT2N
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[2] = 0.0f;
            v.v.m128_f32[3] = 1.0f;
            do {
                v.v.m128_f32[0] = (float)*(short*)src * 0.000030518509f;
                v.v.m128_f32[1] = (float)*((short*)src + 1) * 0.000030518509f;
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x22:  // FLOAT2
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[2] = 0.0f;
            v.v.m128_f32[3] = 1.0f;
            do {
                v.v.m128_f32[0] = *(float*)src;
                v.v.m128_f32[1] = *((float*)src + 1);
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x25:  // SHORT2
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[2] = 0.0f;
            v.v.m128_f32[3] = 1.0f;
            do {
                v.v.m128_f32[0] = (float)*(short*)src;
                v.v.m128_f32[1] = (float)*((short*)src + 1);
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x31:  // SHORT3N
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[3] = 1.0f;
            do {
                v.v.m128_f32[0] = (float)*(short*)src * 0.000030518509f;
                v.v.m128_f32[1] = (float)*((short*)src + 1) * 0.000030518509f;
                v.v.m128_f32[2] = (float)*((short*)src + 2) * 0.000030518509f;
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x32:  // FLOAT3
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[3] = 1.0f;
            do {
                v.v.m128_f32[0] = *(float*)src;
                v.v.m128_f32[1] = *((float*)src + 1);
                v.v.m128_f32[2] = *((float*)src + 2);
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x35:  // SHORT3
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            v.v.m128_f32[3] = 1.0f;
            do {
                v.v.m128_f32[0] = (float)*(short*)src;
                v.v.m128_f32[1] = (float)*((short*)src + 1);
                v.v.m128_f32[2] = (float)*((short*)src + 2);
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x40:  // 0xAARRGGBB bytes -> (R, G, B, A)
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            do {
                unsigned int packed = *(unsigned int*)src;
                v.v.m128_f32[0] = (float)((packed >> 16) & 0xFF) * 0.0039215689f;
                v.v.m128_f32[1] = (float)((packed >> 8) & 0xFF) * 0.0039215689f;
                v.v.m128_f32[2] = (float)(packed & 0xFF) * 0.0039215689f;
                v.v.m128_f32[3] = (float)((packed >> 24) & 0xFF) * 0.0039215689f;
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x41:  // SHORT4N
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            do {
                v.v.m128_f32[0] = (float)*(short*)src * 0.000030518509f;
                v.v.m128_f32[1] = (float)*((short*)src + 1) * 0.000030518509f;
                v.v.m128_f32[2] = (float)*((short*)src + 2) * 0.000030518509f;
                v.v.m128_f32[3] = (float)*((short*)src + 3) * 0.000030518509f;
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x42:  // FLOAT4
        if (count != start) {
            unsigned int n = count - start;
            do {
                *buf++ = *(math::Vector4*)src;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    case 0x45:  // SHORT4
        if (count != start) {
            unsigned int n = count - start;
            math::Vector4 v;
            do {
                v.v.m128_f32[0] = (float)*(short*)src;
                v.v.m128_f32[1] = (float)*((short*)src + 1);
                v.v.m128_f32[2] = (float)*((short*)src + 2);
                v.v.m128_f32[3] = (float)*((short*)src + 3);
                *buf++ = v;
                src += fmt->VertexSize;
            } while (--n != 0);
        }
        break;
    default:
        if (_tlAssert("src/gpu/ngl_gpu.cpp", 409, "false", "Unsupported vertex format."))
            __debugbreak();
        break;
    }
}

// ============================================================================
// nglGpuPackVertexBuffer - ea: 0x84BE60
// ============================================================================
void nglGpuPackVertexBuffer(D3DVertexBuffer* vtx, gpuVertexFormat* fmt,
                            gpuVertexElementUsage usage, unsigned int usageindex,
                            const math::Vector4* buf, unsigned int start,
                            unsigned int count) {
    const _D3DVERTEXSHADERINPUT* elem = &fmt->Elements[usage];
    unsigned int offset = start * fmt->VertexSize;
    unsigned char* dst =
        &((unsigned char*)D3DVertexBuffer_Lock2(vtx, 0x20u))[offset + elem->Offset];
    switch (elem->Format) {
    case 0x11:  // SHORT1N
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(short*)dst = (short)(buf->v.m128_f32[0] * 32767.0f);
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x12:  // FLOAT1
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(float*)dst = buf->v.m128_f32[0];
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x15:  // SHORT1
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(short*)dst = (short)buf->v.m128_f32[0];
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x16:  // packed 565 (signed 10-bit RGB)
        if (count != start) {
            unsigned int n = count - start;
            do {
                unsigned int x = (unsigned int)(buf->v.m128_f32[0] * 511.0f);
                unsigned int y = (unsigned int)(buf->v.m128_f32[1] * 511.0f);
                unsigned int z = (unsigned int)(buf->v.m128_f32[2] * 511.0f);
                *(unsigned int*)dst =
                    (x & 0x1FF) | (((y & 0x1FF) | ((z & 0x1FF) << 10)) << 10) |
                    (((z & 0x80000003) | (((y & 0x80000FFF) | ((x >> 10) & 0x200000)) >> 10)) >> 2);
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x21:  // SHORT2N
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(short*)dst = (short)(buf->v.m128_f32[0] * 32767.0f);
                *((short*)dst + 1) = (short)(buf->v.m128_f32[1] * 32767.0f);
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x22:  // FLOAT2
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(float*)dst = buf->v.m128_f32[0];
                *((float*)dst + 1) = buf->v.m128_f32[1];
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x25:  // SHORT2
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(short*)dst = (short)buf->v.m128_f32[0];
                *((short*)dst + 1) = (short)buf->v.m128_f32[1];
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x31:  // SHORT3N
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(short*)dst = (short)(buf->v.m128_f32[0] * 32767.0f);
                *((short*)dst + 1) = (short)(buf->v.m128_f32[1] * 32767.0f);
                *((short*)dst + 2) = (short)(buf->v.m128_f32[2] * 32767.0f);
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x32:  // FLOAT3
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(float*)dst = buf->v.m128_f32[0];
                *((float*)dst + 1) = buf->v.m128_f32[1];
                *((float*)dst + 2) = buf->v.m128_f32[2];
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x35:  // SHORT3
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(short*)dst = (short)buf->v.m128_f32[0];
                *((short*)dst + 1) = (short)buf->v.m128_f32[1];
                *((short*)dst + 2) = (short)buf->v.m128_f32[2];
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x40:  // (R, G, B, A) -> 0xAARRGGBB bytes
        if (count != start) {
            unsigned int n = count - start;
            do {
                unsigned int x = (unsigned int)(buf->v.m128_f32[0] * 255.0f) & 0xFF;
                unsigned int y = (unsigned int)(buf->v.m128_f32[1] * 255.0f) & 0xFF;
                unsigned int z = (unsigned int)(buf->v.m128_f32[2] * 255.0f) & 0xFF;
                unsigned int w = (unsigned int)(buf->v.m128_f32[3] * 255.0f) & 0xFF;
                *(unsigned int*)dst = z | (((y | (((w << 8) | x) << 8)) << 8));
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x41:  // SHORT4N
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(short*)dst = (short)(buf->v.m128_f32[0] * 32767.0f);
                *((short*)dst + 1) = (short)(buf->v.m128_f32[1] * 32767.0f);
                *((short*)dst + 2) = (short)(buf->v.m128_f32[2] * 32767.0f);
                *((short*)dst + 3) = (short)(buf->v.m128_f32[3] * 32767.0f);
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x42:  // FLOAT4
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(math::Vector4*)dst = *buf;
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    case 0x45:  // SHORT4
        if (count != start) {
            unsigned int n = count - start;
            do {
                *(short*)dst = (short)buf->v.m128_f32[0];
                *((short*)dst + 1) = (short)buf->v.m128_f32[1];
                *((short*)dst + 2) = (short)buf->v.m128_f32[2];
                *((short*)dst + 3) = (short)buf->v.m128_f32[3];
                dst += fmt->VertexSize;
                ++buf;
            } while (--n != 0);
        }
        break;
    default:
        if (_tlAssert("src/gpu/ngl_gpu.cpp", 857, "false", "Unsupported vertex format."))
            __debugbreak();
        break;
    }
}
