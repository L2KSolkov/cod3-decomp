// ============================================================================
// ngl_mesh.h - mesh/material resource API (ngl_xboxr:ngl_mesh.o).
// Source: src/ngl_mesh.cpp
// ============================================================================
#ifndef COD3_NGL_NGL_MESH_H
#define COD3_NGL_NGL_MESH_H

#include "core/tlSkipList.h"
#include "core/tlInstanceBank.h"
#include "ngl/nglShader.h"
#include "ngl/ngl_dx_gpu.h"

namespace apk { class apkFile; class apkFileEntry; }

// ngl_mesh.o (data, defined in ngl_mesh.cpp)
extern tlSkipList<nglMesh, tlFixedString> nglMeshDirectory;
extern tlSkipList<nglMaterial, tlFixedString> nglMaterialDirectory;
extern tlInstanceBank nglShaderBank;
extern tlInstanceBank nglGeometryShaderBank;
extern tlInstanceBank nglVertexDefBank;
extern void* (*nglGetMeshFunc)(const tlFixedString&, unsigned int);
extern void* (*nglGetMaterialFunc)(const tlFixedString&, unsigned int);

// Mesh flag bits.
enum {
    NGLMESH_PROCESSED = 0x10000,
    NGLMESH_SCRATCH_MESH = 0x20000,
    NGLMESH_STATIC = 0x80000,
};

// ngl_mesh.o (functions, defined in ngl_mesh.cpp)
bool nglCanReleaseMesh(nglMesh* Mesh);
void nglProcessMaterial(nglMaterial* Material);
void nglProcessSection(nglMesh* Mesh, nglMeshSection* Section, apk::apkFileEntry* Entry);
void nglProcessMesh(nglMesh* Mesh, apk::apkFileEntry* Entry);
void nglUnloadMesh(nglMesh* Mesh);
unsigned int nglGetLOD(nglMesh* Mesh, const math::Mat43* LocalToWorld, nglScene* Scene);
math::Mat43* nglListAddMesh_GetScaledMatrix(const math::Mat43& LocalToWorld,
                                            nglMeshParams* MeshParams,
                                            float* MaxScale);
nglMeshNode* nglListAddMesh_Sections(nglMesh* Mesh, nglMeshNode* MeshNode);
nglMeshNode* nglListAddMesh_Setup(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                                  nglMeshParams* MeshParams,
                                  nglShaderParamSet* ShaderParams,
                                  void (*fn)(nglMeshNode*));
nglMeshNode* nglListAddMesh(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                            nglMeshParams* MeshParams, nglShaderParamSet* ShaderParams,
                            void (*fn)(nglMeshNode*));
void nglAPKMeshLoadCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData);
void nglAPKMeshDeleteCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData);
void nglAPKMaterialLoadCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData);
void nglAPKMaterialDeleteCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData);
void nglMeshInit();
nglMesh* nglGetMesh(const tlFixedString* Name, bool Warn);
nglMaterial* nglGetMaterial(const tlFixedString* Name, bool Warn);

#endif // COD3_NGL_NGL_MESH_H
