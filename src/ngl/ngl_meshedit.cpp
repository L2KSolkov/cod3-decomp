// ============================================================================
// ngl_meshedit.cpp - mesh container management (9 funcs).
// Source: src/ngl_meshedit.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_meshedit.o).
// ============================================================================

#include "ngl/ngl_dx_gpu.h"

#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* msg);
extern void tlWarning(const char* fmt, ...);
extern void tlMemFree(void* Ptr);
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern void* nglListAlloc(unsigned int Bytes, unsigned int Alignment);

// The scratch mesh name (nglCreateMesh vs nglCreateScratchMesh).
tlFixedString nglMeshCreatedName;

static const unsigned char s_infiniteSphereRadius[4] = { 0xAE, 0xC5, 0x9D, 0x74 };

// ============================================================================
// nglSetMeshSphere - ea: 0x844780
// ============================================================================
void nglSetMeshSphere(nglMesh* Mesh, const math::Position3* Center, float Radius) {
    Mesh->Sphere[0] = Center->v.m128_f32[0];
    Mesh->Sphere[1] = Center->v.m128_f32[1];
    Mesh->Sphere[2] = Center->v.m128_f32[2];
    Mesh->Sphere[3] = Radius;
}

// ============================================================================
// nglAddMeshSection - ea: 0x844820
// ============================================================================
void nglAddMeshSection(nglMesh* Mesh, nglMeshSection* Section,
                       nglMaterial* Material, int Flags) {
    unsigned int NSections = Mesh->NSections;
    unsigned int i = 0;
    if (NSections != 0) {
        for (i = 0; i < NSections; i++) {
            if (Mesh->Sections[i].Section == NULL)
                break;
        }
        if (i < NSections) {
            Mesh->Sections[i].Section = Section;
            Mesh->Sections[i].Section->Material = Material;
            Mesh->Sections[i].Flags = Flags;
        }
    }
    if (i >= NSections) {
        _tlAssert("src/ngl_meshedit.cpp", 74, "false",
                  "Tried to add a section to a mesh with no free slots.");
        __debugbreak();
    }
}

// ============================================================================
// nglDestroyMesh - ea: 0x844890
// ============================================================================
void nglDestroyMesh(nglMesh* Mesh) {
    if (Mesh != NULL) {
        if (!nglCanReleaseMesh(Mesh)) {
            tlWarning("NGL: Mesh %s destroyed while still referenced by the async renderer.\n",
                      Mesh->Name->str);
            ngliWaitForResource();
        }
        for (unsigned int i = 0; i < Mesh->NSections; i++) {
            nglMeshSectionTableEntry* entry = &Mesh->Sections[i];
            if (entry->Flags != 0)
                nglDestroySection(entry->Section);
        }
        tlMemFree(Mesh->Sections);
        tlMemFree(Mesh);
    }
}

// ============================================================================
// nglCreateMeshClone - ea: 0x844910
// ============================================================================
nglMesh* nglCreateMeshClone(nglMesh* SrcMesh) {
    if (SrcMesh == NULL)
        return NULL;

    nglMesh* clone = (nglMesh*)tlMemAlloc(0x40u, 0x10u, 0x1000000);
    memset(clone, 0, 0x40u);
    clone->Flags = SrcMesh->Flags;
    clone->NSections = SrcMesh->NSections;

    nglMeshSectionTableEntry* sections =
        (nglMeshSectionTableEntry*)tlMemAlloc(8 * SrcMesh->NSections, 8u, 0x1000000);
    clone->Sections = sections;
    for (unsigned int i = 0; i < clone->NSections; i++) {
        sections[i].Section = SrcMesh->Sections[i].Section;
        sections[i].Flags = 0;
    }

    clone->Skeleton = SrcMesh->Skeleton;
    clone->NLODs = SrcMesh->NLODs;
    if (SrcMesh->NLODs != 0) {
        nglMeshLOD* lods = (nglMeshLOD*)tlMemAlloc(8 * SrcMesh->NLODs, 8u, 0x1000000);
        clone->LODs = lods;
        memcpy(lods, SrcMesh->LODs, 8 * SrcMesh->NLODs);
    } else {
        clone->LODs = NULL;
    }
    clone->NPolys = SrcMesh->NPolys;
    clone->Sphere[0] = SrcMesh->Sphere[0];
    clone->Sphere[1] = SrcMesh->Sphere[1];
    clone->Sphere[2] = SrcMesh->Sphere[2];
    clone->Sphere[3] = SrcMesh->Sphere[3];
    return clone;
}

// ============================================================================
// nglCopyMesh - ea: 0x844A10
// ============================================================================
void nglCopyMesh(nglMesh* Dst, nglMesh* Src) {
    if (Dst->NSections != Src->NSections) {
        _tlAssert("src/ngl_meshedit.cpp", 150, "Dst->NSections == Src->NSections",
                  "Different number of sections.");
        __debugbreak();
    }
    for (unsigned int i = 0; i < Dst->NSections; i++) {
        if ((Dst->Sections[i].Flags & 1) != 0)
            nglCopySection(Dst, i, Src, i);
    }
}

// ============================================================================
// nglMakeSectionUnique - ea: 0x844A80
// ============================================================================
void nglMakeSectionUnique(nglMesh* Mesh, int SectionIdx) {
    nglMeshSectionTableEntry* entry = &Mesh->Sections[SectionIdx];
    if ((entry->Flags & 1) == 0) {
        entry->Section = nglCreateSectionCopy(entry->Section);
        entry->Flags |= 1u;
    }
}

// ============================================================================
// nglCreateScratchMesh - ea: 0x844AC0
// ============================================================================
nglMesh* nglCreateScratchMesh(unsigned int Flags, unsigned int NSections) {
    nglMesh* mesh = (nglMesh*)nglListAlloc(0x40u, 0x10u);
    memset(mesh, 0, 0x40u);
    mesh->Flags = Flags | NGL_MESH_SCRATCH_ALLOC;
    mesh->NSections = NSections;
    mesh->Sections = (nglMeshSectionTableEntry*)nglListAlloc(8 * NSections, 0x10u);
    for (unsigned int i = 0; i < NSections; i++) {
        mesh->Sections[i].Section = NULL;
        mesh->Sections[i].Flags = 0;
    }
    mesh->Skeleton = NULL;
    mesh->Sphere[0] = 0.0f;
    mesh->Sphere[1] = 0.0f;
    mesh->Sphere[2] = 0.0f;
    memcpy(&mesh->Sphere[3], s_infiniteSphereRadius, 4);
    return mesh;
}

// ============================================================================
// nglCreateMesh - ea: 0x844B90
// ============================================================================
nglMesh* nglCreateMesh(unsigned int Flags, unsigned int NSections) {
    if ((Flags & NGL_MESH_SCRATCH) != 0)
        return nglCreateScratchMesh(Flags, NSections);

    nglMesh* mesh = (nglMesh*)tlMemAlloc(0x40u, 0x10u, 0);
    memset(mesh, 0, 0x40u);
    mesh->Flags = Flags;
    mesh->Name = &nglMeshCreatedName;
    mesh->NSections = NSections;
    mesh->Sections = (nglMeshSectionTableEntry*)tlMemAlloc(8 * NSections, 0x10u, 0);
    for (unsigned int i = 0; i < NSections; i++) {
        mesh->Sections[i].Section = NULL;
        mesh->Sections[i].Flags = 0;
    }
    mesh->Skeleton = NULL;
    mesh->Sphere[0] = 0.0f;
    mesh->Sphere[1] = 0.0f;
    mesh->Sphere[2] = 0.0f;
    memcpy(&mesh->Sphere[3], s_infiniteSphereRadius, 4);
    return mesh;
}

// ============================================================================
// nglCreateMeshCopy - ea: 0x844C80
// ============================================================================
nglMesh* nglCreateMeshCopy(nglMesh* SrcMesh) {
    nglMesh* clone = nglCreateMeshClone(SrcMesh);
    for (unsigned int i = 0; i < clone->NSections; i++) {
        if ((SrcMesh->Sections[i].Flags & 1) != 0 && (clone->Sections[i].Flags & 1) == 0) {
            clone->Sections[i].Section = nglCreateSectionCopy(clone->Sections[i].Section);
            clone->Sections[i].Flags |= 1u;
        }
    }
    return clone;
}
