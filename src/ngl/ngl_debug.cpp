// ============================================================================
// ngl_debug.cpp - debug flags, perf info, debug shape drawing (36 funcs).
// Source: src/ngl_debug.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_debug.o).
// Data: nglDebug/nglPerfInfo/nglAvgPerfInfo/nglSyncDebug/nglSyncPerfInfo/
// nglShaderProfiler::ShaderProfiler/nglCurDebugLineNode/nglCurDebugTriNode/
// nglPerfBarNumVB live here.
// ============================================================================

#include "ngl/nglDebug.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_gpu_debug.h"
#include "ngl/ngl_dx_debug.h"
#include "core/tlFixedString.h"

#include <intrin.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
struct nglShader;

extern nglPerfInfoStruct nglPerfInfo;      // ngl_debug.o (this file)
extern nglPerfInfoStruct nglSyncPerfInfo;  // ngl_debug.o (this file)
extern nglPerfInfoStruct nglAvgPerfInfo;   // ngl_debug.o (this file)
extern nglDebugStruct nglDebug;            // ngl_debug.o (this file)

extern nglScene* nglBuildScene;                              // ngl_scene.o
extern void nglValidateMatrices(nglScene* Scene);            // ngl_scene.o
extern float nglGetVBlankMS();                               // ngl_internal.o
extern int nglGetScreenWidth();                              // ngl_internal.o
extern int nglGetScreenHeight();                             // ngl_internal.o
extern math::Position3 nglProjectPoint(const math::Position3& In, nglScene* Scene);
extern math::Position3 nglUnprojectPoint(const math::Position3& In, nglScene* Scene);

struct nglFont;
extern nglFont* nglSysFont;                                  // ngl_font.o
extern void nglGetStringDimensions(nglFont* Font, const char* Text, unsigned int* Width,
                                   unsigned int* Height, float ScaleX, float ScaleY);
extern void nglGetStringDimensions(nglFont* Font, unsigned int* Width, unsigned int* Height,
                                   const char* Fmt, ...);
extern void nglListAddString(nglFont* Font, const char* Text, float x, float y, float z,
                             unsigned int Color, float ScaleX, float ScaleY);
extern void nglListAddString(nglFont* Font, float x, float y, float z, unsigned int Color,
                             const char* Fmt, ...);

extern void* nglListAlloc(unsigned int Bytes, unsigned int Alignment);  // nglRenderNode.h inline
extern void tlPrintf(const char* fmt, ...);
extern void tlFatal(const char* fmt, ...);
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern void tlMemFree(void* Ptr);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data (ngl_debug.o)
// ============================================================================
nglDebugStruct nglDebug;
nglPerfInfoStruct nglPerfInfo;
nglPerfInfoStruct nglAvgPerfInfo;
nglPerfInfoStruct nglSyncPerfInfo;
nglDebugStruct nglSyncDebug;
int nglPerfBarNumVB = 0;
nglDebugLineNode* nglCurDebugLineNode = NULL;
nglDebugTriNode* nglCurDebugTriNode = NULL;

// ============================================================================
// Fast sin/cos (math::Sin inline, ea: 0x835800 / 0x835720). The polynomial is
// the XDK fast-sin the compiler inlined into the ellipse/label code below.
// ============================================================================
static const __m128 kSignMask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
static const __m128 kFloorMagic = _mm_set1_ps(12582912.0f);
static const __m128 kInv2PI = _mm_set1_ps(0.15915494f);
static const __m128 kSin3PiOver2 = _mm_set1_ps(4.712389f);
static const __m128 kHalf = _mm_set1_ps(0.5f);
static const __m128 kQuarter = _mm_set1_ps(0.25f);

// sin(radians) - matches math::Sin(float) exactly (ea: 0x835800).
static float FastSin(float radians) {
    __m128 v = _mm_set1_ps(radians);
    __m128 v1 = _mm_mul_ps(_mm_xor_ps(kSignMask, _mm_andnot_ps(kSignMask, _mm_add_ps(v, kSin3PiOver2))), kInv2PI);
    float v2 = _mm_andnot_ps(kSignMask, _mm_sub_ps(_mm_sub_ps(_mm_add_ps(_mm_sub_ps(v1, kFloorMagic), kFloorMagic), v1), kHalf)).m128_f32[0] - 0.25f;
    float v3 = v2 * v2;
    return (((((((v2 * (v3 * v3)) * (v3 * v3)) * 39.710659f)
             + (((v2 * v3) * (v3 * v3)) * -76.574959f))
            + ((v2 * (v3 * v3)) * 81.602226f))
           + ((v2 * v3) * -41.341675f))
          + (v2 * 6.283185f));
}

// cos(radians) - same polynomial, no phase offset (used by nglDebugAddEllipse).
static float FastCos(float radians) {
    __m128 v = _mm_set1_ps(radians);
    __m128 v1 = _mm_mul_ps(_mm_xor_ps(kSignMask, _mm_andnot_ps(kSignMask, v)), kInv2PI);
    float v2 = _mm_andnot_ps(kSignMask, _mm_sub_ps(_mm_sub_ps(_mm_add_ps(_mm_sub_ps(v1, kFloorMagic), kFloorMagic), v1), kHalf)).m128_f32[0] - 0.25f;
    float v3 = v2 * v2;
    return (((((((v2 * (v3 * v3)) * (v3 * v3)) * 39.710659f)
             + (((v2 * v3) * (v3 * v3)) * -76.574959f))
            + ((v2 * (v3 * v3)) * 81.602226f))
           + ((v2 * v3) * -41.341675f))
          + (v2 * 6.283185f));
}

// ============================================================================
// Debug flag API
// ============================================================================
// nglGetDebugFlagPtr - ea: 0x835910
unsigned char* nglGetDebugFlagPtr(const char* Flag) {
    if (_stricmp(Flag, "ShowPerfInfo") == 0)
        return &nglDebug.ShowPerfInfo;
    if (_stricmp(Flag, "ShowPerfBar") == 0)
        return &nglDebug.ShowPerfBar;
    if (_stricmp(Flag, "ScreenShot") == 0)
        return &nglDebug.ScreenShot;
    if (_stricmp(Flag, "DisableQuads") == 0)
        return &nglDebug.DisableQuads;
    if (_stricmp(Flag, "DisableVSync") == 0)
        return &nglDebug.DisableVSync;
    if (_stricmp(Flag, "DisableScratch") == 0)
        return &nglDebug.DisableScratch;
    if (_stricmp(Flag, "DebugPrints") == 0)
        return &nglDebug.DebugPrints;
    if (_stricmp(Flag, "DumpFrameLog") == 0)
        return &nglDebug.DumpFrameLog;
    if (_stricmp(Flag, "DumpSceneFile") == 0)
        return &nglDebug.DumpSceneFile;
    if (_stricmp(Flag, "DumpTextures") == 0)
        return &nglDebug.DumpTextures;
    if (_stricmp(Flag, "DrawLightSpheres") == 0)
        return &nglDebug.DrawLightSpheres;
    if (_stricmp(Flag, "DrawMeshSpheres") == 0)
        return &nglDebug.DrawMeshSpheres;
    if (_stricmp(Flag, "DisableDuplicateMaterialWarning") == 0)
        return &nglDebug.DisableDuplicateMaterialWarning;
    if (_stricmp(Flag, "DisableMissingTextureWarning") == 0)
        return &nglDebug.DisableMissingTextureWarning;
    return ngliGetDebugFlagPtr(Flag);
}

// nglSetDebugFlag - ea: 0x835A90
void nglSetDebugFlag(const char* Flag, unsigned char Set) {
    unsigned char* DebugFlagPtr = nglGetDebugFlagPtr(Flag);
    if (DebugFlagPtr != NULL)
        *DebugFlagPtr = Set;
    nglSyncDebug = nglDebug;
}

// nglGetDebugFlag - ea: 0x835AF0
int nglGetDebugFlag(const char* Flag) {
    unsigned char* DebugFlagPtr = nglGetDebugFlagPtr(Flag);
    if (DebugFlagPtr != NULL)
        return *DebugFlagPtr;
    return 0;
}

// nglDebugInit - ea: 0x835B10
void nglDebugInit() {
    memset(&nglDebug, 0, sizeof(nglDebug));
    memset(&nglPerfInfo, 0, sizeof(nglPerfInfo));
    nglSyncPerfInfo = nglPerfInfo;
    nglAvgPerfInfo = nglPerfInfo;
    nglSyncDebug = nglDebug;
}

// nglAveragePerfInfo - ea: 0x835B90
void nglAveragePerfInfo(unsigned int Frames) {
    float T = 1.0f / (float)Frames;
    nglAvgPerfInfo.FPS = (nglSyncPerfInfo.FPS * T) + (nglAvgPerfInfo.FPS * (1.0f - T));
    nglAvgPerfInfo.CPUMS = (nglAvgPerfInfo.CPUMS * (1.0f - T)) + (nglSyncPerfInfo.CPUMS * T);
    nglAvgPerfInfo.RenderMS = (nglAvgPerfInfo.RenderMS * (1.0f - T)) + (nglSyncPerfInfo.RenderMS * T);
    nglAvgPerfInfo.ListSubmitMS = (nglAvgPerfInfo.ListSubmitMS * (1.0f - T)) + (nglSyncPerfInfo.ListSubmitMS * T);
    nglAvgPerfInfo.ListSendMS = (nglAvgPerfInfo.ListSendMS * (1.0f - T)) + (nglSyncPerfInfo.ListSendMS * T);
    nglAvgPerfInfo.TotalPolys = (unsigned int)((float)nglAvgPerfInfo.TotalPolys * (1.0f - T) + (float)nglSyncPerfInfo.TotalPolys * T);
    nglAvgPerfInfo.NodeCount = (unsigned int)((float)nglAvgPerfInfo.NodeCount * (1.0f - T) + (float)nglSyncPerfInfo.NodeCount * T);
    nglAvgPerfInfo.ListWorkUsage = (unsigned int)((float)nglAvgPerfInfo.ListWorkUsage * (1.0f - T) + (float)nglSyncPerfInfo.ListWorkUsage * T);
}

// ============================================================================
// Shader profiler
// ============================================================================
// tlInstanceBank (tl_xboxr:tl_instbank.o, size 0x14; Instance = 0x2C).
struct tlInstanceBank {
    struct Instance {
        unsigned char pad[0x20];   // +0x00 (Key/skip-list links)
        void*        Value;        // +0x20
        unsigned int _pad24;       // +0x24
        Instance*    Forward[1];   // +0x28 (level-0 next)
    };
    static_assert(sizeof(Instance) == 0x2C, "tlInstanceBank::Instance size mismatch");

    Instance* NIL;      // +0x00
    Instance* Head;     // +0x04
    int       RandomsLeft;  // +0x08
    int       RandomBits;   // +0x0C
    int       Level;        // +0x10
};
static_assert(sizeof(tlInstanceBank) == 0x14, "tlInstanceBank size mismatch");

struct nglShaderProfiler {
    int CurrentShaderIndex;     // +0x00
    nglShader* CurrentShader;   // +0x04
    int ShaderCount;            // +0x08
    ngliShaderProfile* ProfileData;  // +0x0C

    nglShaderProfiler();        // ea: 0x8390B0 (inline COMDAT)
    void Destroy();             // ea: 0x839110 (inline COMDAT)
    void PrintResults();        // ea: 0x839150 (inline COMDAT)
    void ProfileFrame();        // ea: 0x839210 (inline COMDAT)

    static nglShaderProfiler* ShaderProfiler;  // 0x14D2358
};

// Function-local static of ProfileFrame (0x14D2368).
static int nglShaderProfileFrameCounter = 0;

// Minimal nglShader surface used by ProfileFrame (GetName is vtable slot 2).
struct nglShader {
    virtual ~nglShader() {}
    virtual void Dummy() {}
    virtual tlFixedString& GetName(tlFixedString& Result) { return Result; }
    static int NextID;  // ngl_internal.o
};

nglShaderProfiler* nglShaderProfiler::ShaderProfiler = NULL;
int nglShader::NextID = 0;
extern tlInstanceBank nglShaderBank;  // ngl_mesh.o

// ngliShaderProfile forward-declared helpers (ngl_dx_debug.o).

nglShaderProfiler::nglShaderProfiler() {
    CurrentShaderIndex = -1;
    CurrentShader = NULL;
    ShaderCount = nglShader::NextID + 1;
    ProfileData = (ngliShaderProfile*)tlMemAlloc(52 * (ShaderCount + 2), 8, 0x1000000);
    memset(ProfileData, 0, 52 * (ShaderCount + 2));
    ShaderProfiler = this;
}

void nglShaderProfiler::Destroy() {
    tlMemFree(ProfileData);
    ShaderProfiler = NULL;
}

void nglShaderProfiler::PrintResults() {
    tlPrintf("\n\n------------------------------------------------\n");
    ngliShaderProfile::PrintHeader();
    for (int i = 0; i < ShaderCount + 1; ++i)
        ProfileData[i].Print();
    tlPrintf("------------------------------------------------\n\n\n");
}

void nglShaderProfiler::ProfileFrame() {
    tlFixedString Name;
    if (CurrentShaderIndex == -1) {
        Name = "Total";
        ProfileData[ShaderCount].Name = Name;
        ProfileData[ShaderCount].Record();
        CurrentShader = NULL;
        CurrentShaderIndex = 0;
        nglShaderProfileFrameCounter = 3;
    } else if (nglShaderProfileFrameCounter != 0) {
        --nglShaderProfileFrameCounter;
    } else {
        nglShaderProfileFrameCounter = 3;
        if (CurrentShaderIndex != 0) {
            ProfileData[CurrentShaderIndex].Record();
            ProfileData[CurrentShaderIndex].Compare(ProfileData);
        } else {
            Name = "Leftover";
            ProfileData->Name = Name;
            ProfileData->Record();
        }
        int ShaderCount_ = ShaderCount;
        int NextIndex = CurrentShaderIndex + 1;
        CurrentShaderIndex = NextIndex;
        if (NextIndex == ShaderCount_) {
            PrintResults();
            tlMemFree(ProfileData);
            ShaderProfiler = NULL;
            delete this;
        } else {
            CurrentShader = NULL;
            int v12 = 0;
            tlInstanceBank::Instance* v13 = nglShaderBank.Head->Forward[0];
            if (v13 != nglShaderBank.NIL) {
                while (++v12 != NextIndex) {
                    v13 = v13->Forward[0];
                    if (v13 == nglShaderBank.NIL)
                        return;
                }
                nglShader* Value = (nglShader*)v13->Value;
                CurrentShader = Value;
                tlFixedString& ShaderName = Value->GetName(Name);
                ProfileData[CurrentShaderIndex].Name = ShaderName;
            }
        }
    }
}

// ============================================================================
// Debug line/triangle nodes
// ============================================================================
// nglDebugLineNode::nglDebugLineNode - ea: 0x836180
nglDebugLineNode::nglDebugLineNode() {
    NVerts = 0;
    Verts = (nglDebugLineVertex*)nglListAlloc(0x640, 0x10);
}

// nglDebugLineNode::~nglDebugLineNode - ea: 0x8391E0
nglDebugLineNode::~nglDebugLineNode() {
}

// nglDebugLineNode::GetDesc - ea: 0x836150
void nglDebugLineNode::GetDesc(char* Desc) {
}

// nglDebugLineNode::GetSortInfo - base nglRenderNode::GetSortInfo (empty)
void nglDebugLineNode::GetSortInfo(nglSortInfo& Info) {
}

// nglDebugTriNode::nglDebugTriNode - ea: 0x8393C0 (inline COMDAT)
nglDebugTriNode::nglDebugTriNode() {
    NVerts = 0;
    Verts = (nglDebugLineVertex*)nglListAlloc(0x640, 0x10);
}

// ============================================================================
// Line/triangle batch adders
// ============================================================================
// nglFlushLinesBatch - ea: 0x836160
void nglFlushLinesBatch() {
    nglCurDebugLineNode = NULL;
    nglCurDebugTriNode = NULL;
}

// nglDebugAddLine - ea: 0x8361B0
void nglDebugAddLine(const math::Position3* pt1, const math::Position3* pt2,
                     unsigned int color) {
    nglDebugLineNode* v3 = nglCurDebugLineNode;
    if (nglCurDebugLineNode == NULL || nglCurDebugLineNode->NVerts == 100) {
        nglValidateMatrices(nglBuildScene);
        nglDebugLineNode* v4 = (nglDebugLineNode*)nglListAlloc(0x14, 0x10);
        nglDebugLineNode* v5 = v4;
        if (v4 != NULL)
            v5 = new (v4) nglDebugLineNode();
        else
            v5 = NULL;
        nglCurDebugLineNode = v5;
        v5->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = v5;
        ++nglBuildScene->OpaqueListCount;
        v3 = nglCurDebugLineNode;
    }
    nglDebugLineVertex* v6 = &v3->Verts[v3->NVerts];
    v3->NVerts += 2;
    v6->x = pt1->v.m128_f32[0];
    v6->y = pt1->v.m128_f32[1];
    v6->z = pt1->v.m128_f32[2];
    v6->color = color;
    v6[1].x = pt2->v.m128_f32[0];
    v6[1].y = pt2->v.m128_f32[1];
    v6[1].z = pt2->v.m128_f32[2];
    v6[1].color = color;
}

// nglDebugAddRay - ea: 0x836310
void nglDebugAddRay(const math::Position3* pt1, const math::Position3* pt2,
                    unsigned int Color) {
    nglDebugAddLine(pt1, pt2, Color);
    __m128 v4 = _mm_sub_ps(pt2->v, pt1->v);
    __m128 v5 = _mm_mul_ps(v4, v4);
    float len = sqrtf(v5.m128_f32[0] + (v5.m128_f32[1] + v5.m128_f32[2]));
    if (len > 0.001f) {
        __m128 v7 = _mm_div_ps(v4, _mm_set1_ps(len));
        // Pick a perpendicular axis: cross(YAxis, dir), or XAxis if nearly parallel.
        __m128 v8 = _mm_mul_ps(v7, _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f));
        float dotY = v8.m128_f32[0] + (v8.m128_f32[1] + v8.m128_f32[2]);
        __m128 v9 = _mm_shuffle_ps(v7, v7, 18);  // {z, x, y, w}
        __m128 v10 = _mm_shuffle_ps(v7, v7, 9);  // {y, z, x, w}
        __m128 v11;
        if (dotY <= 0.89999998f) {
            __m128 yz = _mm_shuffle_ps(_mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f), _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f), 9);
            __m128 yy = _mm_shuffle_ps(_mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f), _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f), 18);
            v11 = _mm_sub_ps(_mm_mul_ps(yz, v9), _mm_mul_ps(yy, v10));
        } else {
            __m128 xz = _mm_shuffle_ps(_mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f), _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f), 9);
            __m128 xy = _mm_shuffle_ps(_mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f), _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f), 18);
            v11 = _mm_sub_ps(_mm_mul_ps(xz, v9), _mm_mul_ps(xy, v10));
        }
        __m128 v12 = _mm_mul_ps(v11, v11);
        float perpLen = sqrtf(v12.m128_f32[0] + (v12.m128_f32[1] + v12.m128_f32[2]));
        __m128 v13 = _mm_div_ps(v11, _mm_set1_ps(perpLen));
        __m128 perp2 = _mm_sub_ps(_mm_mul_ps(v10, _mm_shuffle_ps(v13, v13, 18)),
                                  _mm_mul_ps(v9, _mm_shuffle_ps(v13, v13, 9)));
        math::Position3 base = *pt2;
        float L = 0.04f;
        // Four offset "girdle" lines from pt2 back toward pt1.
        math::Position3 p;
        p.v = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_set1_ps(L * 0.5f), v13),
                                    _mm_mul_ps(_mm_set1_ps(0.0f), perp2)),
                         _mm_add_ps(_mm_mul_ps(_mm_set1_ps(-L), v7), base.v));
        nglDebugAddLine(&p, pt2, Color);
        p.v = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_set1_ps(-L * 0.5f), v13),
                                    _mm_mul_ps(_mm_set1_ps(0.0f), perp2)),
                         _mm_add_ps(_mm_mul_ps(_mm_set1_ps(-L), v7), base.v));
        nglDebugAddLine(&p, pt2, Color);
        p.v = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_set1_ps(0.0f), v13),
                                    _mm_mul_ps(_mm_set1_ps(L * 0.5f), perp2)),
                         _mm_add_ps(_mm_mul_ps(_mm_set1_ps(-L), v7), base.v));
        nglDebugAddLine(&p, pt2, Color);
        p.v = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_set1_ps(0.0f), v13),
                                    _mm_mul_ps(_mm_set1_ps(-L * 0.5f), perp2)),
                         _mm_add_ps(_mm_mul_ps(_mm_set1_ps(-L), v7), base.v));
        nglDebugAddLine(&p, pt2, Color);
    }
}

// nglDebugAddTriData - ea: 0x837FE0
void nglDebugAddTriData(unsigned int NTris, const math::Position3* Points,
                        const unsigned char* Indices, unsigned int Color,
                        unsigned int Skip, unsigned int Mapping0,
                        unsigned int Mapping1, unsigned int Mapping2) {
    unsigned int NIndices = 3 * NTris;
    nglDebugTriNode* v9 = nglCurDebugTriNode;
    if (nglCurDebugTriNode == NULL || NIndices + nglCurDebugTriNode->NVerts >= 100) {
        if (NIndices > 100
            && _tlAssert("src/ngl_debug.cpp", 654, "NIndices <= NGL_DEBUG_LINE_BATCH_SIZE",
                         "Your polygons smell of old men."))
            __debugbreak();
        nglValidateMatrices(nglBuildScene);
        nglDebugTriNode* v10 = (nglDebugTriNode*)nglListAlloc(0x14, 0x10);
        nglDebugTriNode* v11 = v10;
        if (v10 != NULL)
            v11 = new (v10) nglDebugTriNode();
        else
            v11 = NULL;
        nglCurDebugTriNode = v11;
        v11->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = v11;
        ++nglBuildScene->OpaqueListCount;
        v9 = nglCurDebugTriNode;
    }
    int NVerts = v9->NVerts;
    nglDebugLineVertex* Verts = v9->Verts;
    __m128 Light = _mm_setr_ps(-0.27f, -0.53f, -0.8f, 0.0f);
    v9->NVerts = NIndices + NVerts;
    nglDebugLineVertex* v14 = &Verts[NVerts];
    if (NTris != 0) {
        const unsigned char* v15 = &Indices[Mapping1];
        int v16 = (int)Mapping0 - (int)Mapping1;
        int v17 = (int)Mapping2 - (int)Mapping1;
        const unsigned char* v34 = &Indices[Mapping1];
        unsigned int v33 = NTris;
        while (1) {
            int i0 = v15[v16];
            const math::Position3* v20 = &Points[i0];
            int i1 = *v15;
            const math::Position3* v22 = &Points[v15[v17]];
            const math::Position3* v23 = &Points[i1];
            v34 += Skip;
            v14->x = v20->v.m128_f32[0];
            v14->y = v20->v.m128_f32[1];
            v14->z = v20->v.m128_f32[2];
            v14[1].x = v23->v.m128_f32[0];
            v14[1].y = v23->v.m128_f32[1];
            v14[1].z = v23->v.m128_f32[2];
            v14[2].x = v22->v.m128_f32[0];
            v14[2].y = v22->v.m128_f32[1];
            v14[2].z = v22->v.m128_f32[2];
            __m128 v24 = _mm_sub_ps(v23->v, v20->v);
            __m128 v25 = _mm_sub_ps(v22->v, v20->v);
            __m128 v26 = _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(v24, v24, 9), _mm_shuffle_ps(v25, v25, 18)),
                _mm_mul_ps(_mm_shuffle_ps(v24, v24, 18), _mm_shuffle_ps(v25, v25, 9)));
            __m128 v27 = _mm_mul_ps(v26, v26);
            float nLen = sqrtf(v27.m128_f32[0] + (v27.m128_f32[1] + v27.m128_f32[2]));
            __m128 v28 = _mm_mul_ps(_mm_div_ps(v26, _mm_set1_ps(nLen)), Light);
            float shade = (v28.m128_f32[0] + (v28.m128_f32[1] + v28.m128_f32[2]))
                          * 0x1.7DF3B6p-4f + 0x1.6p-1f;
            unsigned int v30 = ((unsigned int)((Color & 0xFF) * shade))
                             | (((unsigned int)(((Color >> 8) & 0xFF) * shade)) << 8)
                             | (((unsigned int)(((Color >> 16) & 0xFF) * shade)) << 16)
                             | ((Color >> 24) << 24);
            v14->color = v30;
            v14[1].color = v30;
            v14[2].color = v30;
            v14 += 3;
            if (--v33 == 0)
                break;
            v16 = (int)Mapping0 - (int)Mapping1;
            v15 = v34;
            v17 = (int)Mapping2 - (int)Mapping1;
        }
    }
}

// nglDebugAddTriList / nglDebugAddQuadList / nglDebugAddTriStrip
void nglDebugAddTriList(unsigned int NTris, const math::Position3* Points,
                        const unsigned char* Indices, unsigned int Color) {
    nglDebugAddTriData(NTris, Points, Indices, Color, 3, 0, 1, 2);
}

void nglDebugAddQuadList(unsigned int NQuads, const math::Position3* Points,
                         const unsigned char* Indices, unsigned int Color) {
    nglDebugAddTriData(NQuads, Points, Indices, Color, 4, 0, 1, 2);
    nglDebugAddTriData(NQuads, Points, Indices, Color, 4, 2, 3, 0);
}

void nglDebugAddTriStrip(unsigned int NTris, const math::Position3* Points,
                         const unsigned char* Indices, unsigned int Color) {
    nglDebugAddTriData(NTris >> 1, Points, Indices, Color, 2, 0, 1, 2);
    nglDebugAddTriData((NTris + 1) >> 1, Points, Indices, Color, 2, 1, 3, 2);
}

// nglDebugAddTri / nglDebugAddQuad
void nglDebugAddTri(const math::Position3* pt1, const math::Position3* pt2,
                    const math::Position3* pt3, unsigned int Color) {
    static const unsigned char Indices[3] = { 0, 1, 2 };
    math::Position3 Points[3];
    Points[0] = *pt1;
    Points[1] = *pt2;
    Points[2] = *pt3;
    nglDebugAddTriData(1, Points, Indices, Color, 3, 0, 1, 2);
}

void nglDebugAddQuad(const math::Position3* pt1, const math::Position3* pt2,
                     const math::Position3* pt3, const math::Position3* pt4,
                     unsigned int Color) {
    static const unsigned char Indices[6] = { 0, 1, 2, 2, 3, 0 };
    math::Position3 Points[4];
    Points[0] = *pt1;
    Points[1] = *pt2;
    Points[2] = *pt3;
    Points[3] = *pt4;
    nglDebugAddTriData(2, Points, Indices, Color, 3, 0, 1, 2);
}

// ============================================================================
// Shape drawing
// ============================================================================
// nglDebugAddAxes - ea: 0x836680
void nglDebugAddAxes(const math::Mat43* LToW, unsigned int Length,
                     unsigned int ColX, unsigned int ColY, unsigned int ColZ) {
    math::Position3 p;
    p.v = _mm_add_ps(LToW->w.v, _mm_mul_ps(LToW->x.v, _mm_set1_ps((float)Length)));
    nglDebugAddRay(&LToW->w, &p, ColX);
    p.v = _mm_add_ps(LToW->w.v, _mm_mul_ps(LToW->y.v, _mm_set1_ps((float)Length)));
    nglDebugAddRay(&LToW->w, &p, ColY);
    p.v = _mm_add_ps(LToW->w.v, _mm_mul_ps(LToW->z.v, _mm_set1_ps((float)Length)));
    nglDebugAddRay(&LToW->w, &p, ColZ);
}

// nglDebugAddGrid - ea: 0x837190
void nglDebugAddGrid(const math::Mat43* mat, int w, int h, float xstep,
                     float zstep, unsigned int color) {
    for (int i = -w; i <= w; ++i) {
        math::Position3 a = mat->w, b = mat->w;
        a.v = _mm_add_ps(a.v, _mm_mul_ps(mat->x.v, _mm_set1_ps((float)i * xstep)));
        a.v = _mm_add_ps(a.v, _mm_mul_ps(mat->z.v, _mm_set1_ps((float)h * zstep)));
        b.v = _mm_add_ps(b.v, _mm_mul_ps(mat->x.v, _mm_set1_ps((float)i * xstep)));
        b.v = _mm_sub_ps(b.v, _mm_mul_ps(mat->z.v, _mm_set1_ps((float)h * zstep)));
        nglDebugAddLine(&a, &b, color);
    }
    for (int i = -h; i <= h; ++i) {
        math::Position3 a = mat->w, b = mat->w;
        a.v = _mm_add_ps(a.v, _mm_mul_ps(mat->z.v, _mm_set1_ps((float)i * zstep)));
        a.v = _mm_add_ps(a.v, _mm_mul_ps(mat->x.v, _mm_set1_ps((float)w * xstep)));
        b.v = _mm_add_ps(b.v, _mm_mul_ps(mat->z.v, _mm_set1_ps((float)i * zstep)));
        b.v = _mm_sub_ps(b.v, _mm_mul_ps(mat->x.v, _mm_set1_ps((float)w * xstep)));
        nglDebugAddLine(&a, &b, color);
    }
}

// nglDebugAddEllipse - ea: 0x8372F0
void nglDebugAddEllipse(const math::Mat43* LToW, float RadiusX, float RadiusZ,
                        unsigned int Color) {
    nglValidateMatrices(nglBuildScene);
    // Screen-space z of the ellipse origin decides the segment count.
    math::Position3 origin = LToW->w;
    __m128 wz = _mm_add_ps(_mm_add_ps(_mm_mul_ps(origin.v, nglBuildScene->WorldToView.x.v),
                                      _mm_mul_ps(_mm_shuffle_ps(origin.v, origin.v, 85), nglBuildScene->WorldToView.y.v)),
                           _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(origin.v, origin.v, 170), nglBuildScene->WorldToView.z.v),
                                      nglBuildScene->WorldToView.w.v));
    float viewZ = wz.m128_f32[2];
    float step = 1.0f;
    if (viewZ > RadiusX * RadiusZ)
        step = (RadiusX * RadiusZ) / viewZ;
    int count = (int)(step * 1000.0f);
    if (count < 8)
        count = 8;
    if (count > 64)
        count = 64;
    float delta = 6.2831855f / (float)count;
    math::Position3 prev;
    prev.v = _mm_add_ps(LToW->w.v, _mm_mul_ps(LToW->x.v, _mm_set1_ps(RadiusX)));
    float a = 0.0f;
    for (int i = count; i != 0; --i) {
        a += delta;
        math::Position3 cur;
        cur.v = _mm_add_ps(
            _mm_add_ps(LToW->w.v, _mm_mul_ps(LToW->x.v, _mm_set1_ps(FastCos(a) * RadiusX))),
            _mm_mul_ps(LToW->z.v, _mm_set1_ps(FastSin(a) * RadiusZ)));
        nglDebugAddLine(&prev, &cur, Color);
        prev = cur;
    }
}

// nglDebugAddCircle - ea: 0x837620
void nglDebugAddCircle(const math::Mat43* LToW, float Radius, unsigned int Color) {
    nglDebugAddEllipse(LToW, Radius, Radius, Color);
}

// nglDebugAddCylinder - ea: 0x837640
void nglDebugAddCylinder(const math::Mat43* LToW, float Radius, float Height,
                         unsigned int Color) {
    nglDebugAddEllipse(LToW, Radius, Radius, Color);
    math::Mat43 top = *LToW;
    top.w.v = _mm_add_ps(top.w.v, _mm_mul_ps(top.y.v, _mm_set1_ps(Height)));
    nglDebugAddEllipse(&top, Radius, Radius, Color);
    math::Position3 a = LToW->w, b = top.w;
    a.v = _mm_add_ps(a.v, _mm_mul_ps(LToW->x.v, _mm_set1_ps(Radius)));
    b.v = _mm_add_ps(b.v, _mm_mul_ps(top.x.v, _mm_set1_ps(Radius)));
    nglDebugAddLine(&a, &b, Color);
    a = LToW->w; b = top.w;
    a.v = _mm_sub_ps(a.v, _mm_mul_ps(LToW->x.v, _mm_set1_ps(Radius)));
    b.v = _mm_sub_ps(b.v, _mm_mul_ps(top.x.v, _mm_set1_ps(Radius)));
    nglDebugAddLine(&a, &b, Color);
    a = LToW->w; b = top.w;
    a.v = _mm_add_ps(a.v, _mm_mul_ps(LToW->z.v, _mm_set1_ps(Radius)));
    b.v = _mm_add_ps(b.v, _mm_mul_ps(top.z.v, _mm_set1_ps(Radius)));
    nglDebugAddLine(&a, &b, Color);
    a = LToW->w; b = top.w;
    a.v = _mm_sub_ps(a.v, _mm_mul_ps(LToW->z.v, _mm_set1_ps(Radius)));
    b.v = _mm_sub_ps(b.v, _mm_mul_ps(top.z.v, _mm_set1_ps(Radius)));
    nglDebugAddLine(&a, &b, Color);
    nglValidateMatrices(nglBuildScene);
    a = LToW->w; b = top.w;
    a.v = _mm_add_ps(a.v, _mm_mul_ps(nglBuildScene->ViewToWorld.x.v, _mm_set1_ps(Radius)));
    b.v = _mm_add_ps(b.v, _mm_mul_ps(nglBuildScene->ViewToWorld.x.v, _mm_set1_ps(Radius)));
    nglDebugAddLine(&a, &b, Color);
    a = LToW->w; b = top.w;
    a.v = _mm_sub_ps(a.v, _mm_mul_ps(nglBuildScene->ViewToWorld.x.v, _mm_set1_ps(Radius)));
    b.v = _mm_sub_ps(b.v, _mm_mul_ps(nglBuildScene->ViewToWorld.x.v, _mm_set1_ps(Radius)));
    nglDebugAddLine(&a, &b, Color);
}

// nglDebugAddBox - ea: 0x836720 (12 edges)
void nglDebugAddBox(const math::Mat43* mat, const math::DiagMat33* size,
                    unsigned int color) {
    math::Position3 p[8];
    for (int i = 0; i < 8; ++i) {
        float sx = (i & 1) ? 0.5f : -0.5f;
        float sy = (i & 2) ? 0.5f : -0.5f;
        float sz = (i & 4) ? 0.5f : -0.5f;
        __m128 v = _mm_mul_ps(_mm_setr_ps(sx, sy, sz, 0.0f), size->v);
        p[i].v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(mat->x.v, _mm_shuffle_ps(v, v, 0)),
                       _mm_mul_ps(mat->y.v, _mm_shuffle_ps(v, v, 85))),
            _mm_add_ps(_mm_mul_ps(mat->z.v, _mm_shuffle_ps(v, v, 170)), mat->w.v));
    }
    // Edges along X.
    nglDebugAddLine(&p[0], &p[1], color);
    nglDebugAddLine(&p[2], &p[3], color);
    nglDebugAddLine(&p[4], &p[5], color);
    nglDebugAddLine(&p[6], &p[7], color);
    // Edges along Y.
    nglDebugAddLine(&p[0], &p[2], color);
    nglDebugAddLine(&p[1], &p[3], color);
    nglDebugAddLine(&p[4], &p[6], color);
    nglDebugAddLine(&p[5], &p[7], color);
    // Edges along Z.
    nglDebugAddLine(&p[0], &p[4], color);
    nglDebugAddLine(&p[1], &p[5], color);
    nglDebugAddLine(&p[2], &p[6], color);
    nglDebugAddLine(&p[3], &p[7], color);
}

// nglDebugAddEllipsoid - ea: 0x837840 (3 ellipse cuts)
void nglDebugAddEllipsoid(const math::Mat43* LToW, const math::Dir3* Radius,
                          unsigned int Color) {
    nglDebugAddEllipse(LToW, Radius->v.m128_f32[0], Radius->v.m128_f32[2], Color);
    math::Mat43 m;
    m.x = LToW->y; m.y = LToW->z; m.z = LToW->x; m.w = LToW->w;
    nglDebugAddEllipse(&m, Radius->v.m128_f32[1], Radius->v.m128_f32[0], Color);
    m.x = LToW->z; m.y = LToW->x; m.z = LToW->y; m.w = LToW->w;
    nglDebugAddEllipse(&m, Radius->v.m128_f32[2], Radius->v.m128_f32[1], Color);
}

// nglDebugAddSphere - ea: 0x837A40
void nglDebugAddSphere(const math::Position3* Pos, float Radius, unsigned int Color) {
    math::Mat43 m;
    m.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    m.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    m.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    m.w = *Pos;
    math::Dir3 r;
    r.v = _mm_setr_ps(Radius, Radius, Radius, 0.0f);
    nglDebugAddEllipsoid(&m, &r, Color);
    nglValidateMatrices(nglBuildScene);
    math::Mat43 s;
    s.x = nglBuildScene->ViewToWorld.x;
    s.y = nglBuildScene->ViewToWorld.z;
    s.z = nglBuildScene->ViewToWorld.y;
    s.w = *Pos;
    nglDebugAddEllipse(&s, Radius, Radius, Color);
}

// nglDebugAddPyramid - ea: 0x837B60 (8 edges)
void nglDebugAddPyramid(const math::Mat43* LToW, const math::Dir3* Size,
                        unsigned int Color) {
    float sx = Size->v.m128_f32[0];
    float sy = Size->v.m128_f32[1];
    float sz = Size->v.m128_f32[2];
    math::Position3 p[5];
    __m128 xa = _mm_mul_ps(LToW->x.v, _mm_set1_ps(sx));
    __m128 ya = _mm_mul_ps(LToW->y.v, _mm_set1_ps(sy));
    __m128 za = _mm_mul_ps(LToW->z.v, _mm_set1_ps(sz));
    p[0].v = _mm_sub_ps(_mm_sub_ps(LToW->w.v, xa), ya);
    p[1].v = _mm_add_ps(_mm_sub_ps(LToW->w.v, xa), ya);
    p[2].v = _mm_add_ps(_mm_add_ps(LToW->w.v, xa), ya);
    p[3].v = _mm_sub_ps(_mm_add_ps(LToW->w.v, xa), ya);
    p[4].v = _mm_add_ps(LToW->w.v, za);
    nglDebugAddLine(&p[3], &p[2], Color);
    nglDebugAddLine(&p[2], &p[1], Color);
    nglDebugAddLine(&p[1], &p[0], Color);
    nglDebugAddLine(&p[0], &p[3], Color);
    nglDebugAddLine(&p[3], &p[4], Color);
    nglDebugAddLine(&p[2], &p[4], Color);
    nglDebugAddLine(&p[1], &p[4], Color);
    nglDebugAddLine(&p[0], &p[4], Color);
}

// nglDebugAddLabel - ea: 0x837CA0
void nglDebugAddLabel(const math::Position3* Pos, unsigned int Color,
                      const char* Label, ...) {
    char Work[512];
    va_list va;
    va_start(va, Label);
    vsprintf(Work, Label, va);
    va_end(va);

    math::Position3 ScreenPos = nglProjectPoint(*Pos, nglBuildScene);
    float ScreenHeight = (float)nglGetScreenHeight();
    float ScreenWidth = (float)nglGetScreenWidth();

    __m128 w = _mm_setr_ps(ScreenWidth, ScreenHeight, 1.0f, 0.0f);
    __m128 rc = _mm_rcp_ps(w);
    __m128 v6 = _mm_max_ps(
        _mm_min_ps(_mm_mul_ps(ScreenPos.v, _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(rc, w)), rc)),
                   _mm_set1_ps(1.0f)),
        _mm_set1_ps(0.0f));
    // Fast cos/sin of the screen angle (Vector4 math::Sin, ea: 0x835720).
    __m128 v7 = _mm_mul_ps(
        _mm_xor_ps(kSignMask, _mm_andnot_ps(kSignMask, _mm_shuffle_ps(v6, _mm_shuffle_ps(_mm_set1_ps(1.0f), v6, 160), 52))),
        kHalf);
    __m128 v8 = _mm_sub_ps(
        _mm_andnot_ps(kSignMask, _mm_sub_ps(_mm_sub_ps(_mm_add_ps(_mm_sub_ps(v7, kFloorMagic), kFloorMagic), v7), kHalf)),
        kQuarter);
    __m128 v5 = _mm_mul_ps(v8, v8);
    __m128 v6b = _mm_mul_ps(v5, v5);
    __m128 v7b = _mm_mul_ps(v8, v5);
    __m128 v8b = _mm_mul_ps(v8, v6b);
    __m128 res = _mm_add_ps(
        _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_mul_ps(v8b, v6b), _mm_set1_ps(39.710659f)),
                                         _mm_mul_ps(_mm_mul_ps(v7b, v6b), _mm_set1_ps(-76.574959f))),
                              _mm_mul_ps(v8b, _mm_set1_ps(81.602226f))),
                   _mm_mul_ps(v7b, _mm_set1_ps(-41.341675f))),
        _mm_mul_ps(v8, _mm_set1_ps(6.283185f)));
    float rx = res.m128_f32[0];
    float ry = res.m128_f32[1];
    float rl = sqrtf(rx * rx + ry * ry);
    rx /= rl;
    ry /= rl;
    math::Position3 TextPos = ScreenPos;
    TextPos.v = _mm_add_ps(TextPos.v, _mm_mul_ps(_mm_setr_ps(rx, ry, 0.0f, 0.0f), _mm_set1_ps(40.0f)));
    math::Position3 Unproj = nglUnprojectPoint(TextPos, nglBuildScene);
    nglDebugAddRay(&Unproj, Pos, Color);

    unsigned int tw, th;
    nglGetStringDimensions(nglSysFont, &tw, &th, Work);
    // Clamp the screen position to NDC.
    float cx = rx < -1.0f ? -1.0f : (rx > 1.0f ? 1.0f : rx);
    float cy = ry < -1.0f ? -1.0f : (ry > 1.0f ? 1.0f : ry);
    float x = TextPos.v.m128_f32[0] - (0.5f - cx * 0.5f) * (float)tw;
    float y = TextPos.v.m128_f32[1] - (0.5f - cy * 0.5f) * (float)th;
    nglListAddString(nglSysFont, x, y, 0.0f, Color, Work);
}

// ============================================================================
// Solid shapes (dodecagon ring tessellation)
// ============================================================================
static const float kDodec[12] = {
    1.0f, 0.866f, 0.5f, 0.0f, -0.5f, -0.866f,
    -1.0f, -0.866f, -0.5f, 0.0f, 0.5f, 0.866f,
};

// nglDebugAddBoxSolid - ea: 0x838580
void nglDebugAddBoxSolid(const math::Mat43* LToW, const math::DiagMat33* size,
                         unsigned int Color) {
    static const unsigned char Indices[25] = {
        1, 3, 2, 0, 5, 7, 3, 1, 4, 6, 7, 5, 0, 2, 6, 4, 2, 3, 7, 6, 4, 5, 1, 0, 0,
    };
    math::Position3 p[8];
    for (int i = 0; i < 8; ++i) {
        float sx = (i & 1) ? 0.5f : -0.5f;
        float sy = (i & 2) ? 0.5f : -0.5f;
        float sz = (i & 4) ? 0.5f : -0.5f;
        __m128 v = _mm_mul_ps(_mm_setr_ps(sx, sy, sz, 0.0f), size->v);
        p[i].v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(LToW->x.v, _mm_shuffle_ps(v, v, 0)),
                       _mm_mul_ps(LToW->y.v, _mm_shuffle_ps(v, v, 85))),
            _mm_add_ps(_mm_mul_ps(LToW->z.v, _mm_shuffle_ps(v, v, 170)), LToW->w.v));
    }
    nglDebugAddTriData(6, p, Indices, Color, 4, 0, 1, 2);
    nglDebugAddTriData(6, p, Indices, Color, 4, 2, 3, 0);
}

// nglDebugAddPyramidSolid - ea: 0x8388A0
void nglDebugAddPyramidSolid(const math::Mat43* LToW, const math::Dir3* Size,
                             unsigned int Color) {
    static const unsigned char Indices[12] = { 0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4 };
    float sx = Size->v.m128_f32[0];
    float sy = Size->v.m128_f32[1];
    float sz = Size->v.m128_f32[2];
    math::Position3 p[5];
    __m128 xa = _mm_mul_ps(LToW->x.v, _mm_set1_ps(sx));
    __m128 ya = _mm_mul_ps(LToW->y.v, _mm_set1_ps(sy));
    __m128 za = _mm_mul_ps(LToW->z.v, _mm_set1_ps(sz));
    p[0].v = _mm_sub_ps(_mm_sub_ps(LToW->w.v, xa), ya);
    p[1].v = _mm_add_ps(_mm_sub_ps(LToW->w.v, xa), ya);
    p[2].v = _mm_add_ps(_mm_add_ps(LToW->w.v, xa), ya);
    p[3].v = _mm_sub_ps(_mm_add_ps(LToW->w.v, xa), ya);
    p[4].v = _mm_add_ps(LToW->w.v, za);
    nglDebugAddTriData(6, p, Indices, Color, 3, 0, 1, 2);
}

// nglDebugAddCylinderSolid - ea: 0x838D00
void nglDebugAddCylinderSolid(const math::Mat43* LToW, float Radius, float Height,
                              unsigned int Color) {
    static const unsigned char TopIndices[12] = { 0, 1, 11, 2, 10, 3, 9, 4, 8, 5, 7, 6 };
    static const unsigned char BottomIndices[12] = { 12, 23, 13, 22, 14, 21, 15, 20, 16, 19, 17, 18 };
    static const unsigned char SideIndices[25] = {
        0, 12, 1, 13, 2, 14, 3, 15, 4, 16, 5, 17, 6, 18, 7, 19, 8, 20, 9, 21, 10, 22, 11, 23, 0,
    };
    math::Position3 p[24];
    // Ring A (top, z = Height); note the 12th vertex (p[12]) keeps z = 0,
    // matching the compiled binary's ring/z-write split.
    for (int j = 0; j < 12; ++j) {
        p[j].v = _mm_setr_ps(Radius * kDodec[j], 0.0f, j < 11 ? Height : 0.0f, 0.0f);
    }
    // Ring B (bottom, z = 0).
    for (int j = 0; j < 12; ++j) {
        p[13 + j].v = _mm_setr_ps(Radius * kDodec[j], 0.0f, 0.0f, 0.0f);
    }
    for (int k = 0; k < 24; ++k) {
        p[k].v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(LToW->x.v, _mm_shuffle_ps(p[k].v, p[k].v, 0)),
                       _mm_mul_ps(LToW->y.v, _mm_shuffle_ps(p[k].v, p[k].v, 85))),
            _mm_add_ps(_mm_mul_ps(LToW->z.v, _mm_shuffle_ps(p[k].v, p[k].v, 170)), LToW->w.v));
    }
    nglDebugAddTriData(12, p, SideIndices, Color, 2, 0, 1, 2);
    nglDebugAddTriData(12, p, SideIndices, Color, 2, 1, 3, 2);
    nglDebugAddTriData(5, p, TopIndices, Color, 2, 0, 1, 2);
    nglDebugAddTriData(5, p, TopIndices, Color, 2, 1, 3, 2);
    nglDebugAddTriData(5, p, BottomIndices, Color, 2, 0, 1, 2);
    nglDebugAddTriData(5, p, BottomIndices, Color, 2, 1, 3, 2);
}

// nglDebugAddEllipsoidSolid - ea: 0x838980
void nglDebugAddEllipsoidSolid(const math::Mat43* LToW, const math::Dir3* Radius,
                               unsigned int Color) {
    static const float Scale[5] = { 1.0f, 0.866f, 1.0f, 0.866f, 1.0f };
    static const float ScaleY[5] = { 0.866f, 1.0f, 0.0f, -1.0f, -0.866f };
    static const unsigned char TopIndices[36] = {
        0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7,
        0, 7, 8, 0, 8, 9, 0, 9, 10, 0, 10, 11, 0, 11, 12, 0, 12, 1,
    };
    static const unsigned char BottomIndices[36] = {
        61, 50, 49, 61, 51, 50, 61, 52, 51, 61, 53, 52, 61, 54, 53, 61, 55, 54,
        61, 56, 55, 61, 57, 56, 61, 58, 57, 61, 59, 58, 61, 60, 59, 61, 49, 60,
    };
    math::Position3 p[62];
    memset(p, 0, sizeof(p));
    p[0].v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    p[61].v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    for (int k = 0; k < 5; ++k) {
        for (int j = 0; j < 12; ++j)
            p[1 + 12 * k + j].v = _mm_setr_ps(Scale[k] * kDodec[j], 0.0f, ScaleY[k], 0.0f);
    }
    for (int k = 0; k < 62; ++k) {
        p[k].v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(LToW->x.v, _mm_shuffle_ps(p[k].v, p[k].v, 0)),
                       _mm_mul_ps(LToW->y.v, _mm_shuffle_ps(p[k].v, p[k].v, 85))),
            _mm_add_ps(_mm_mul_ps(LToW->z.v, _mm_shuffle_ps(p[k].v, p[k].v, 170)), LToW->w.v));
    }
    unsigned char Band[24];
    for (int b = 1; b < 49; b += 12) {
        for (int m = 0; m < 12; ++m) {
            Band[2 * m] = (unsigned char)(b + m);
            Band[2 * m + 1] = (unsigned char)(b + 12 + m);
        }
        nglDebugAddTriData(12, p, Band, Color, 2, 0, 1, 2);
        nglDebugAddTriData(12, p, Band, Color, 2, 1, 3, 2);
    }
    nglDebugAddTriData(12, p, TopIndices, Color, 3, 0, 1, 2);
    nglDebugAddTriData(12, p, BottomIndices, Color, 3, 0, 1, 2);
}

// nglDebugAddSphereSolid - ea: 0x839020
void nglDebugAddSphereSolid(const math::Position3* Pos, float Radius,
                            unsigned int Color) {
    math::Mat43 m;
    m.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    m.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    m.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    m.w = *Pos;
    math::Dir3 r;
    r.v = _mm_setr_ps(Radius, Radius, Radius, 0.0f);
    nglDebugAddEllipsoidSolid(&m, &r, Color);
}

// ============================================================================
// Profiling API entry points
// ============================================================================
// nglProfileShaders - ea: 0x835D40
void nglProfileShaders() {
    if (nglShaderProfiler::ShaderProfiler == NULL) {
        nglShaderProfiler* result = new nglShaderProfiler();
        nglShaderProfiler::ShaderProfiler = result;
    }
}

// nglDestroyProfiler - ea: 0x835D70
void nglDestroyProfiler() {
    if (nglShaderProfiler::ShaderProfiler != NULL) {
        tlMemFree(nglShaderProfiler::ShaderProfiler->ProfileData);
        nglShaderProfiler::ShaderProfiler = NULL;
    }
}

// nglProfileEvalShader - ea: 0x835D90
bool nglProfileEvalShader(nglShader* Shader) {
    return nglShaderProfiler::ShaderProfiler == NULL
        || nglShaderProfiler::ShaderProfiler->CurrentShaderIndex == -1
        || (nglShaderProfiler::ShaderProfiler->CurrentShaderIndex != 0
            && Shader == nglShaderProfiler::ShaderProfiler->CurrentShader);
}

// nglInitShaderProfiling - ea: 0x836170
void nglInitShaderProfiling() {
    if (nglShaderProfiler::ShaderProfiler != NULL)
        nglShaderProfiler::ShaderProfiler->ProfileFrame();
}

// nglRenderPerfBar - ea: 0x835DC0
void nglRenderPerfBar() {
    float Width = (float)nglGetScreenWidth() - 80.0f;
    float VBWidth = Width / (float)nglPerfBarNumVB;
    nglQuad q;
    nglInitQuad(&q);
    nglSetQuadRect(&q, 37.0f, 37.0f, (Width + 40.0f) + 3.0f, 53.0f);
    nglSetQuadColor(&q, 0xE0001020);
    nglSetQuadZ(&q, -1000.0f);
    nglListAddQuad(&q);
    float Left = nglPerfInfo.RenderMS / nglGetVBlankMS() * VBWidth;
    float v1 = Left;
    if (Width <= Left)
        v1 = Width;
    nglSetQuadRect(&q, 40.0f, 40.0f, v1 + 40.0f, 45.0f);
    nglSetQuadColor(&q, 0xFFFFFF00);
    nglListAddQuad(&q);
    Left = nglPerfInfo.CPUMS / nglGetVBlankMS() * VBWidth;
    v1 = Left;
    if (Width <= Left)
        v1 = Width;
    nglSetQuadRect(&q, 40.0f, 45.0f, v1 + 40.0f, 50.0f);
    nglSetQuadColor(&q, 0xFF0000FF);
    nglListAddQuad(&q);
    Left = 40.0f;
    nglSetQuadColor(&q, 0xFFFFFFFF);
    for (int i = 0; i < nglPerfBarNumVB; ++i) {
        nglSetQuadRect(&q, Left, 40.0f, Left + 1.25f, 50.0f);
        nglListAddQuad(&q);
        nglSetQuadRect(&q, VBWidth * 0.5f + Left, 40.0f, (VBWidth * 0.5f) + 1.25f + Left, 45.0f);
        nglListAddQuad(&q);
        nglSetQuadRect(&q, VBWidth * 0.25f + Left, 40.0f, (VBWidth * 0.25f) + 1.25f + Left, 42.5f);
        nglListAddQuad(&q);
        nglSetQuadRect(&q, VBWidth * 0.75f + Left, 40.0f, (VBWidth * 0.75f) + 1.25f + Left, 42.5f);
        nglListAddQuad(&q);
        Left += VBWidth;
    }
    nglSetQuadRect(&q, Left, 40.0f, Width + 41.25f, 50.0f);
    nglListAddQuad(&q);
}
