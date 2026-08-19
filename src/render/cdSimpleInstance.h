// ============================================================================
// cdSimpleInstance.h — simple instance (7 non-inline funcs).
// Source: source/cdSimpleInstance.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleInstance.o):
//   InitCDSimpleInstanceShader @0x7C4C40
//   ToggleCDSimpleInstanceShader @0x7C4C90
//   cdSimpleInstance::Create @0x7C4CB0
//   cdSimpleInstance::Destroy @0x7C4D00
//   cdSimpleInstance::Add @0x7C4D20
//   cdSimpleInstance::Finalize @0x7C4FD0
//   cdSimpleInstance::Render @0x7C50F0
// ============================================================================
#ifndef COD3_RENDER_CDSIMPLEINSTANCE_H
#define COD3_RENDER_CDSIMPLEINSTANCE_H

#include "cdSimpleColorShader.h"
#include "core/math_types.h"
#include "core/ae_array.h"

// ============================================================================
// cdSimpleInstance::XForm — per-instance transform (144 bytes)
// ============================================================================
struct nglMesh;
struct nglMeshSection;
struct cdSimpleShaderMat;

struct cdSimpleInstance {
    struct XForm {
        math::Mat43    Mat;         // +0x00
        float          Scale;       // +0x40
        // +0x44..0x4F: padding
        math::Dir3     LightDir;    // +0x50
        math::Vector4  LightColor;  // +0x60
        math::Vector4  Ambient;     // +0x70
        int            renderFlag;  // +0x80
    };
    static_assert(sizeof(XForm) == 0x90, "XForm size mismatch");

    nglMesh*          Mesh;          // +0x00
    nglMeshSection*   Section;       // +0x04
    cdSimpleShaderMat* Material;     // +0x08
    int               NInstances;    // +0x0C
    float             mMinLodDist2;  // +0x10
    float             mMaxLodDist2;  // +0x14
    XForm*            Insts;         // +0x18
    int*              cells;         // +0x1C

    void Create(nglMesh* mesh, nglMeshSection* section, int numInstances);  // @0x7C4CB0
    void Destroy();                                                          // @0x7C4D00
    void Add(const math::Mat43& mat, float scale, const math::Mat44& dir,
             const math::Mat44& color, ae_sized_array<int*, 384>& flags,
             int cellNum);                                                   // @0x7C4D20
    void Finalize(ae_sized_array<int*, 384>& flags);                         // @0x7C4FD0
    void Render();                                                           // @0x7C50F0
};
static_assert(sizeof(cdSimpleInstance) == 0x20, "cdSimpleInstance size mismatch");

// ============================================================================
// cdSimpleInstanceShader — simple instance shader (16 bytes)
// ============================================================================
class cdSimpleInstanceShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7C4C00
};
static_assert(sizeof(cdSimpleInstanceShader) == 0x10, "cdSimpleInstanceShader size mismatch");

// ============================================================================
// Externs
// ============================================================================
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);  // ?tlMemAlloc@@YAPAXIII@Z
extern void tlMemFree(void* Ptr);                                                    // ?tlMemFree@@YAXPAX@Z

extern cdSimpleInstanceShader* gCDSimpleInstanceShader;  // @0x10DE008 (defined in cdSimpleInstanceShader.cpp)

void InitCDSimpleInstanceShader();   // @0x7C4C40
void ToggleCDSimpleInstanceShader(); // @0x7C4C90

#endif // COD3_RENDER_CDSIMPLEINSTANCE_H
