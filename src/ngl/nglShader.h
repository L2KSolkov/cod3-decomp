// ============================================================================
// nglShader.h - canonical NGL shader/material types (ngl_xboxr).
// nglShader vtable layout verified against the gEmptyShader vtable (0xD44EA0):
//   0 dtor, 1 Register, 2 GetName, 3 AddNode, 4 BindMaterial,
//   5 AddNodeFlags, 6 DisableNode, 7 CheckMaterialVersion,
//   8 CheckVertexDefVersion, 9 BindSection.
// ============================================================================
#ifndef COD3_NGL_NGL_SHADER_H
#define COD3_NGL_NGL_SHADER_H

#include "core/tlFixedString.h"
#include "render/cdDebugVertexDef.h"  // tlInitList

struct nglMesh;
class nglMeshNode;
struct nglMeshSection;
struct nglMaterial;

// ============================================================================
// nglShader - base shader (16 bytes: tlInitList vptr+next, Disabled, ID).
// ============================================================================
struct nglShader : tlInitList {
    bool Disabled;  // +0x08
    int  ID;        // +0x0C

    virtual ~nglShader() {}                                  // slot 0
    virtual void Register() {}                               // slot 1
    virtual tlFixedString GetName() { return tlFixedString(); }  // slot 2
    virtual void AddNode(nglMeshNode* MeshNode, nglMeshSection* Section,
                         nglMaterial* Material) {}           // slot 3
    virtual void BindMaterial(nglMaterial* Material) {}      // slot 4
    virtual void AddNodeFlags(unsigned int Flags) {}         // slot 5
    virtual void DisableNode() {}                            // slot 6
    virtual bool CheckMaterialVersion(nglMaterial* Material) { return true; }  // slot 7
    virtual bool CheckVertexDefVersion(nglMeshSection* Section) { return true; }  // slot 8
    virtual void BindSection(nglMeshSection* Section, nglMesh* Mesh) {}  // slot 9

    static int NextID;  // ngl_internal.o (0x14D2538)
};
static_assert(sizeof(nglShader) == 0x10, "nglShader size mismatch");

// ============================================================================
// nglMaterial - material base (16 bytes, verified against IDA).
// ============================================================================
struct nglMaterial {
    tlFixedString* Name;          // +0x00
    nglShader*     Shader;        // +0x04
    int            BinaryVersion; // +0x08
    void*          RuntimeData;   // +0x0C
};
static_assert(sizeof(nglMaterial) == 0x10, "nglMaterial size mismatch");

#endif // COD3_NGL_NGL_SHADER_H
