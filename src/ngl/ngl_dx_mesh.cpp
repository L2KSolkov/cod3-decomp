// ============================================================================
// ngl_dx_mesh.cpp - vertex-decl bank + mesh-section D3D processing (4 funcs).
// Source: src/dx/ngl_dx_mesh.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_dx_mesh.o).
// ============================================================================

#include "ngl/ngl_dx_mesh.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_lighting.h"
#include "ngl/ngl_scene.h"
#include "d3d8.h"

#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern nglScene* nglBuildScene;                        // ngl_scene.o
extern bool nglIsSphereVisible(const math::Position3& Center, float Radius,
                               const math::Vector4* Clip); // render.o inline
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);

// ============================================================================
// nglVertexDeclBankEntry - 8 bytes
// ============================================================================
struct nglVertexDeclBankEntry {
    nglVertexDeclBankEntry* Next;              // +0x00
    _D3DVERTEXATTRIBUTEFORMAT* VertexDeclaration;  // +0x04
};
static_assert(sizeof(nglVertexDeclBankEntry) == 8, "nglVertexDeclBankEntry size mismatch");

// ngl_dx_mesh.o (data)
static nglVertexDeclBankEntry* nglVertexDeclarationBank = NULL;

// ============================================================================
// nglGetVertexDeclaration - ea: 0x853F50
// ============================================================================
_D3DVERTEXATTRIBUTEFORMAT* nglGetVertexDeclaration(_D3DVERTEXATTRIBUTEFORMAT* Src) {
    nglVertexDeclBankEntry* Entry = nglVertexDeclarationBank;
    if (nglVertexDeclarationBank != NULL) {
        while (1) {
            _D3DVERTEXATTRIBUTEFORMAT* VertexDeclaration = Entry->VertexDeclaration;
            unsigned int v3 = 0;
            const char* v4 = (const char*)(Src - VertexDeclaration);
            for (const char* i = v4; ; v4 = i) {
                const char* v5 = (const char*)VertexDeclaration + (size_t)v4;
                const unsigned short* v7 = (const unsigned short*)VertexDeclaration;
                bool v8 = true;
                for (int v6 = 7; v6 != 0; --v6) {
                    if (*v7++ != *(const unsigned short*)v5) {
                        v8 = false;
                        break;
                    }
                    v5 += 2;
                }
                if (!v8)
                    break;
                ++v3;
                VertexDeclaration = (_D3DVERTEXATTRIBUTEFORMAT*)((char*)VertexDeclaration + 16);
                if (v3 >= 0x10)
                    break;
            }
            if (v3 == 16)
                return Entry->VertexDeclaration;
            Entry = Entry->Next;
            if (Entry == NULL)
                break;
        }
    }
    nglVertexDeclBankEntry* v9 = (nglVertexDeclBankEntry*)tlMemAlloc(8, 8, 0x1000000);
    v9->Next = nglVertexDeclarationBank;
    nglVertexDeclarationBank = v9;
    _D3DVERTEXATTRIBUTEFORMAT* v10 =
        (_D3DVERTEXATTRIBUTEFORMAT*)tlMemAlloc(0x100, 8, 0x1000000);
    v9->VertexDeclaration = v10;
    memcpy(v10, Src, sizeof(_D3DVERTEXATTRIBUTEFORMAT));
    return v9->VertexDeclaration;
}

// ============================================================================
// ngliProcessSection - ea: 0x854000
// ============================================================================
void ngliProcessSection(nglMesh* Mesh, nglMeshSection* Section, apk::apkFileEntry* Entry) {
    Section->VertexBuffer->Data -= Section->VertexOffset;
    Section->VertexBuffer->Data &= ~0x80000000u;
    if (Section->IndexBuffer != NULL)
        Section->IndexBuffer->Data -= Section->IndexOffset;
    Section->VertexFormat->VertexDeclaration =
        nglGetVertexDeclaration(Section->VertexFormat->VertexDeclaration);
}

// ============================================================================
// ngliReleaseSection - ea: 0x854050
// ============================================================================
void ngliReleaseSection(nglMeshSection* Section) {
}

// ============================================================================
// ngliListAddMesh_GetClipResult - ea: 0x854060
// ============================================================================
int ngliListAddMesh_GetClipResult(const math::Position3& Center, float Radius,
                                  int ParamFlags) {
    if ((ParamFlags & 0x40) != 0
        || nglIsSphereVisible(Center, Radius, nglBuildScene->ClipPlanes))
        return 0;
    return -1;
}
