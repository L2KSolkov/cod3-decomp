// ============================================================================
// ngl_morph.cpp - NGL morph target directory + blending (7 funcs).
// Source: src/ngl_morph.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_morph.o). Data: nglMorphDirectory
// (0x14d5f08) lives here.
// ============================================================================

#include "core/tlSkipList.h"
#include "core/tlFixedString.h"
#include "ngl/nglMorph.h"

// ============================================================================
// Cross-object externs
// ============================================================================
extern void* nglGetResource(const tlFixedString& FileName, unsigned int FourCC);
extern void tlWarning(const char* fmt, ...);
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern unsigned char* nglListWorkPos;
extern void* nglListAlloc(unsigned int Bytes, unsigned int Alignment);
extern void nglMakeSectionUnique(nglMesh* Mesh, int SectionIdx);
extern void nglCopyMesh(nglMesh* Dst, nglMesh* Src);

// ============================================================================
// Globals (data)
// ============================================================================
tlSkipList<nglMorphSet, tlFixedString> nglMorphDirectory;

// Skip-list key accessor (free function, used by tlSkipList<nglMorphSet>).
const tlFixedString* GetKey(const nglMorphSet* m) {
    return m->Name;
}

// ============================================================================
// nglGetMorph - ea: 0x8533E0
// ============================================================================
nglMorphSet* nglGetMorph(const tlFixedString& Name, bool Warn) {
    void* Resource = nglGetResource(Name, 0x48524F4D);  // 'MORH'
    if (Resource == NULL && Warn)
        tlWarning("nglGetMorph: Unable to find morph %s.\n", Name.str);
    return (nglMorphSet*)Resource;
}

// ============================================================================
// nglMorphFrame::Apply - ea: 0x853420
// ============================================================================
void nglMorphFrame::Apply(nglMeshSection* Section, int SectionIdx, float Weight,
                          unsigned int Flags) const {
    (void)Section; (void)SectionIdx; (void)Weight; (void)Flags;
}

// ============================================================================
// nglMeshMorph::Apply - ea: 0x853430
// ============================================================================
void nglMeshMorph::Apply(nglMeshSection* Section, int SectionIdx, float Weight,
                         unsigned int Flags) const {
    (void)Section; (void)SectionIdx; (void)Weight; (void)Flags;
}

// ============================================================================
// nglMakeMorphSectionsUnique - ea: 0x853440
// ============================================================================
void nglMakeMorphSectionsUnique(nglMesh* Mesh, nglMorphSet* Morph) {
    unsigned int i = 0;
    while (i < Morph->NFrames) {
        nglMorphSetFrame* Frames = Morph->Frames;
        unsigned int v5 = 0;
        if (Frames[i].NSections != 0) {
            unsigned int v6 = 0;
            do {
                if (Frames[i].Sections[v6].ComponentMask != 0)
                    nglMakeSectionUnique(Mesh, (int)v5);
                Frames = Morph->Frames;
                ++v5;
                ++v6;
            } while (v5 < Frames[i].NSections);
        }
        ++i;
    }
}

// ============================================================================
// nglBlendMorphs - ea: 0x8534B0
// ============================================================================
void nglBlendMorphs(nglMesh* Mesh, unsigned int NMorphs, nglMorphEntry* Morphs) {
    if (NMorphs == 0)
        return;

    if (Morphs->Morph->IsMeshMorph() && ((nglMeshMorph*)Morphs->Morph)->Mesh != Mesh)
        nglCopyMesh(Mesh, ((nglMeshMorph*)Morphs->Morph)->Mesh);

    unsigned int sz = 4 * NMorphs * Mesh->NSections;
    unsigned int* v7 = (unsigned int*)nglListAlloc(sz, 0x10u);
    unsigned int i = 0;
    if (Mesh->NSections != 0) {
        unsigned int* v12 = v7;
        do {
            unsigned int v3 = NMorphs;
            if (v3 != 0) {
                unsigned int* v8 = v12;
                const nglMorph** v9 = &Morphs->Morph;
                do {
                    *v8++ = (*v9)->GetComponentMask(i);
                    v9 += 2;
                    --v3;
                } while (v3 != 0);
                v12 += NMorphs;
            }
            ++i;
        } while (i < Mesh->NSections);
    }
    nglListWorkPos -= sz;
}

// ============================================================================
// nglMorphInit - ea: 0x853570
// ============================================================================
void nglMorphInit() {
    nglMorphDirectory.Level = 0;
    nglMorphDirectory.Head =
        (tlSkipList<nglMorphSet, tlFixedString>::Instance*)tlMemAlloc(0x44u, 8u, 0x1000000);
    for (int result = 0; result <= 15; ++result)
        nglMorphDirectory.Head->Forward[result] = NULL;
}

// ============================================================================
// nglProcessMorph - ea: 0x8535B0
// ============================================================================
void nglProcessMorph(nglMorphSet* Morph) {
    if (nglMorphDirectory.Add(Morph) != NULL)
        tlWarning("Duplicate morph %s found.\n", Morph->Name->str);
    Morph->Frames->Flags = 2;
}
