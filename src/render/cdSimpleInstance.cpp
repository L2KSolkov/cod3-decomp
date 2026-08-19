// ============================================================================
// cdSimpleInstance.cpp — simple instance (7 non-inline funcs).
// Source: source/cdSimpleInstance.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleInstance.o):
//   InitCDSimpleInstanceShader @0x7C4C40
//   ToggleCDSimpleInstanceShader @0x7C4C90
//   cdSimpleInstance::Create @0x7C4CB0
//   cdSimpleInstance::Destroy @0x7C4D00
// ============================================================================
#include "cdSimpleInstance.h"

#include "ngl/ngl_lighting.h"
#include "ngl/ngl_dx_gpu.h"

#include <math.h>
#include <string.h>

extern void auxSetScale(nglMeshParams* dest, float x, float y, float z);
extern nglMeshNode* nglListAddMesh_Setup(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                                         nglMeshParams* MeshParams,
                                         nglShaderParamSet* ShaderParams,
                                         void (*fn)(nglMeshNode*));
extern nglMeshNode* nglListAddMesh_Sections(nglMesh* Mesh, nglMeshNode* MeshNode);

cdSimpleInstanceShader* gCDSimpleInstanceShader = nullptr;  // ?gCDSimpleInstanceShader (render_xboxr @ 0x10DE008)

#include <intrin.h>

// ============================================================================
// InitCDSimpleInstanceShader — allocate the shader and link into the init list.
// ea: 0x7C4C40
// ============================================================================
void InitCDSimpleInstanceShader() {
    cdSimpleInstanceShader* result = (cdSimpleInstanceShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSimpleInstanceShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~0x10;
        gCDSimpleInstanceShader = result;
    } else {
        gCDSimpleInstanceShader = NULL;
    }
}

// ============================================================================
// ToggleCDSimpleInstanceShader — toggle instance enable bit (bit 4).
// ea: 0x7C4C90
// ============================================================================
void ToggleCDSimpleInstanceShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ (16 * ~(byte >> 4))) & 0x10) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;
}

// ============================================================================
// cdSimpleInstance::Create — allocate the instance buffers.
// ea: 0x7C4CB0
// ============================================================================
void cdSimpleInstance::Create(nglMesh* mesh, nglMeshSection* section, int numInstances) {
    this->Mesh = mesh;
    this->Section = section;
    this->Material = NULL;
    this->Insts = (XForm*)tlMemAlloc(0xD800, 0x20, 0);
    this->cells = (int*)tlMemAlloc(0x600, 0x20, 0);
    this->NInstances = 0;
}

// ============================================================================
// cdSimpleInstance::Destroy — free the instance buffers.
// ea: 0x7C4D00
// ============================================================================
void cdSimpleInstance::Destroy() {
    tlMemFree(this->Insts);
    tlMemFree(this->cells);
}

namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
    extern ECoderId gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
}

static void cdSimpleInstanceAssertCapacity(int count, int line) {
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdSimpleInstance.cpp";
    AeAssert::gCurrentLine = line;
    AeAssert::gCurrentExpr = "NInstances < 384";
    if (!AeAssert::IsIgnored() &&
        AeAssert::Assert("more instances than expected, %d", count))
        __debugbreak();
}

// cdSimpleInstance::Add - ea: 0x7C4D20
void cdSimpleInstance::Add(const math::Mat43& mat, float scale,
                           const math::Mat44& dir, const math::Mat44& color,
                           ae_sized_array<int*, 384>& flags, int cellNum) {
    (void)flags;
    if (NInstances >= 384)
        cdSimpleInstanceAssertCapacity(NInstances, 337);
    if (NInstances < 384) {
        XForm& instance = Insts[NInstances];
        instance.Mat = mat;
        instance.Mat.z.v = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(mat.x.v, mat.x.v, 9),
                       _mm_shuffle_ps(mat.y.v, mat.y.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(mat.x.v, mat.x.v, 18),
                       _mm_shuffle_ps(mat.y.v, mat.y.v, 9)));
        instance.Scale = scale;
        __m128 dirSquared = _mm_mul_ps(dir.x.v, dir.x.v);
        float length = sqrtf(dirSquared.m128_f32[0] +
                             _mm_shuffle_ps(dirSquared, dirSquared, 85).m128_f32[0] +
                             _mm_shuffle_ps(dirSquared, dirSquared, 170).m128_f32[0]);
        instance.LightDir.v = _mm_div_ps(dir.x.v, _mm_set1_ps(length));
        instance.LightColor = color.x;
        instance.Ambient = color.w;
        instance.renderFlag = 1;
        instance.Ambient.v = _mm_add_ps(instance.Ambient.v,
                                        _mm_mul_ps(color.z.v, _mm_set1_ps(0.25f)));
        instance.Ambient.v = _mm_add_ps(instance.Ambient.v,
                                        _mm_mul_ps(color.y.v, _mm_set1_ps(0.25f)));
        cells[NInstances++] = cellNum;
    }
}

// cdSimpleInstance::Finalize - ea: 0x7C4FD0
void cdSimpleInstance::Finalize(ae_sized_array<int*, 384>& renderFlagList) {
    if (NInstances >= 384)
        cdSimpleInstanceAssertCapacity(NInstances, 357);

    XForm* newInsts = (XForm*)tlMemAlloc(144 * NInstances, 0x20, 0);
    int* newCells = (int*)tlMemAlloc(4 * NInstances, 0x20, 0);
    memcpy(newInsts, Insts, 144 * NInstances);
    memcpy(newCells, cells, 4 * NInstances);
    tlMemFree(Insts);
    tlMemFree(cells);
    Insts = newInsts;
    cells = newCells;
    for (int i = 0; i < NInstances; ++i)
        renderFlagList.push_back(&Insts[i].renderFlag);
}

// cdSimpleInstance::Render - ea: 0x7C50F0
void cdSimpleInstance::Render() {
    if ((ShaderCommon::ShaderSwitching.__s0[1] & 0x10) != 0)
        return;

    for (int i = 0; i < NInstances; ++i) {
        XForm& instance = Insts[i];
        if (instance.renderFlag != 0)
            continue;

        nglMeshParams meshParams = {};
        auxSetScale(&meshParams, instance.Scale, instance.Scale, instance.Scale);
        nglMeshNode* meshNode = nglListAddMesh_Setup(
            Mesh, instance.Mat, &meshParams, nullptr, nullptr);
        if (meshNode == nullptr)
            continue;

        nglLightContext* lightContext = nglCreateLightContext();
        unsigned int* shaderParams = (unsigned int*)nglListAlloc(
            4 * nglShaderParamSet::NumParams + 8, 8);
        shaderParams[0] = 0;
        shaderParams[1] = 0;
        const unsigned int id = nglLightContextParamID;
        const unsigned long long mask = 1ull << id;
        shaderParams[0] |= (unsigned int)mask;
        shaderParams[1] |= (unsigned int)(mask >> 32);
        shaderParams[id + 2] = (unsigned int)lightContext;
        meshNode->ShaderParams.Array = shaderParams;
        nglListAddDirLight(0xFFFFFFFFu, instance.LightDir, instance.LightColor);
        nglSetAmbientLight(instance.Ambient.v.m128_f32[0],
                           instance.Ambient.v.m128_f32[1],
                           instance.Ambient.v.m128_f32[2]);
        nglListAddMesh_Sections(Mesh, meshNode);
    }
}
