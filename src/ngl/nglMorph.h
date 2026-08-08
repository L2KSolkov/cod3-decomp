// ============================================================================
// nglMorph - NGL morph targets for mesh skinning.
// Source: c:\cod\code\tl\ngl\include\nglMorph.h
// Layouts from IDA local types (verified):
//   nglMorph            4 bytes  (vftable)
//   nglMorphFrame       8 bytes  (Morph + Frame)
//   nglMeshMorph        8 bytes  (Morph + Mesh)
//   nglMorphEntry       8 bytes  {Weight, Morph*}
//   nglMorphSet         12 bytes {Name, NFrames, Frames*}
//   nglMorphSetFrame    12 bytes {Flags, NSections, Sections*}
//   nglMorphSetSection  136 bytes {NVerts, ComponentMask, Data[32]}
// Virtual slots (from nglBlendMorphs disasm ea 0x8534B0): slot 1 = IsMeshMorph,
// slot 3 = GetComponentMask. Apply slots are pure-virtual; the two overrides in
// ngl_morph.o (Apply@nglMorphFrame/nglMeshMorph) are empty.
// ============================================================================

#ifndef COD3_NGL_NGL_MORPH_H
#define COD3_NGL_NGL_MORPH_H

#include "core/tlFixedString.h"
#include "core/tlSkipList.h"
#include "ngl/ngl_dx_gpu.h"   // nglMesh, nglMeshSection

// ============================================================================
// nglMorph - abstract morph base (4 bytes)
// ============================================================================
class nglMorph {
public:
    virtual ~nglMorph() {}
    virtual bool IsMeshMorph() const { return false; }
    virtual void Apply(nglMeshSection* Section, int SectionIdx, float Weight,
                       unsigned int Flags) const = 0;
    virtual unsigned int GetComponentMask(unsigned int SectionIdx) const { return 0; }
};

// ============================================================================
// nglMorphFrame - 8 bytes (frame-based morph)
// ============================================================================
class nglMorphFrame : public nglMorph {
public:
    class nglMorphSetFrame* Frame;   // +0x04

    virtual void Apply(nglMeshSection* Section, int SectionIdx, float Weight,
                       unsigned int Flags) const;  // ea: 0x853420 (empty)
};
static_assert(sizeof(nglMorphFrame) == 8, "nglMorphFrame size mismatch");

// ============================================================================
// nglMeshMorph - 8 bytes (whole-mesh morph)
// ============================================================================
class nglMeshMorph : public nglMorph {
public:
    nglMesh* Mesh;   // +0x04

    virtual bool IsMeshMorph() const { return true; }
    virtual void Apply(nglMeshSection* Section, int SectionIdx, float Weight,
                       unsigned int Flags) const;  // ea: 0x853430 (empty)
};
static_assert(sizeof(nglMeshMorph) == 8, "nglMeshMorph size mismatch");

// ============================================================================
// nglMorphEntry - 8 bytes
// ============================================================================
struct nglMorphEntry {
    float Weight;          // +0x00
    const nglMorph* Morph; // +0x04
};
static_assert(sizeof(nglMorphEntry) == 8, "nglMorphEntry size mismatch");

// ============================================================================
// nglMorphSetSection - 136 bytes
// ============================================================================
struct nglMorphSetSection {
    unsigned int NVerts;        // +0x00
    unsigned int ComponentMask; // +0x04
    void* Data[32];             // +0x08
};
static_assert(sizeof(nglMorphSetSection) == 0x88, "nglMorphSetSection size mismatch");

// ============================================================================
// nglMorphSetFrame - 12 bytes
// ============================================================================
struct nglMorphSetFrame {
    unsigned int Flags;            // +0x00
    unsigned int NSections;        // +0x04
    nglMorphSetSection* Sections;  // +0x08
};
static_assert(sizeof(nglMorphSetFrame) == 0xC, "nglMorphSetFrame size mismatch");

// ============================================================================
// nglMorphSet - 12 bytes
// ============================================================================
struct nglMorphSet {
    tlFixedString* Name;          // +0x00
    unsigned int NFrames;         // +0x04
    nglMorphSetFrame* Frames;     // +0x08
};
static_assert(sizeof(nglMorphSet) == 0xC, "nglMorphSet size mismatch");

// Skip-list key accessor (free function, defined in ngl_morph.cpp)
const tlFixedString* GetKey(const nglMorphSet* m);

// ngl_morph.o (data, defined in ngl_morph.cpp)
extern tlSkipList<nglMorphSet, tlFixedString> nglMorphDirectory;

// ngl_morph.o (functions, defined in ngl_morph.cpp)
nglMorphSet* nglGetMorph(const tlFixedString& Name, bool Warn);
void nglMakeMorphSectionsUnique(nglMesh* Mesh, nglMorphSet* Morph);
void nglBlendMorphs(nglMesh* Mesh, unsigned int NMorphs, nglMorphEntry* Morphs);
void nglMorphInit();
void nglProcessMorph(nglMorphSet* Morph);

#endif // COD3_NGL_NGL_MORPH_H
