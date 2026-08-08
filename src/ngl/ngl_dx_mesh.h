// ============================================================================
// ngl_dx_mesh.h - D3D mesh-section processing (ngl_xboxr:ngl_dx_mesh.o).
// ============================================================================
#ifndef COD3_NGL_NGL_DX_MESH_H
#define COD3_NGL_NGL_DX_MESH_H

#include "core/math_types.h"

struct nglMesh;
struct nglMeshSection;
struct _D3DVERTEXATTRIBUTEFORMAT;
namespace apk { class apkFileEntry; }

// ngl_dx_mesh.o (functions, defined in ngl_dx_mesh.cpp)
_D3DVERTEXATTRIBUTEFORMAT* nglGetVertexDeclaration(_D3DVERTEXATTRIBUTEFORMAT* Src);
void ngliProcessSection(nglMesh* Mesh, nglMeshSection* Section, apk::apkFileEntry* Entry);
void ngliReleaseSection(nglMeshSection* Section);
int ngliListAddMesh_GetClipResult(const math::Position3* Center, float Radius,
                                  unsigned char ParamFlags);

#endif // COD3_NGL_NGL_DX_MESH_H
