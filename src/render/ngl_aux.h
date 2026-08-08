// ============================================================================
// ngl_aux.h - auxiliary NGL helpers + resource directories.
// Source: src/render/ngl_aux.cpp (render_xboxr:ngl_aux.o, 40 non-inline funcs)
// Verified against IDA (render_xboxr:ngl_aux.o).
// ============================================================================

#ifndef COD3_RENDER_NGL_AUX_H
#define COD3_RENDER_NGL_AUX_H

#include "core/math_types.h"
#include "core/tlResourceDirectory.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/nglFont.h"
#include "ngl/nglShader.h"
#include "ngl/nglTexture.h"

struct nglScene;

// Resource directory globals (render_xboxr:ngl_aux.o data).
extern tlResourceDirectory<nglFont>*     auxFontDirectory;
extern tlResourceDirectory<nglTexture>*  auxTextureDirectory;
extern tlResourceDirectory<nglMesh>*     auxMeshDirectory;
extern tlResourceDirectory<nglMaterial>* auxMaterialDirectory;

// Key accessor (free function, inline COMDAT ea: 0x7C2C70).
const tlFixedString* GetKeyPtr(const nglMaterial* Material);
const tlFixedString* GetKeyPtr(const nglTexture* Tex);
const tlFixedString* GetKeyPtr(const nglMesh* Mesh);
const tlFixedString* GetKeyPtr(const nglFont* Font);

// Sphere helpers (ngl_aux.o).
math::Position3* auxGetSphereCenter(math::Position3* result, nglMesh* Mesh);
math::Position3* auxGetSphereCenter(math::Position3* result, nglMeshSection* Section);
float            auxGetSphereRadius(nglMesh* Mesh);
float            auxGetSphereRadius(nglMeshSection* Section);
math::Vector4*   auxGetSphere(math::Vector4* result, nglMesh* Mesh);
math::Vector4*   auxGetSphere(math::Vector4* result, nglMeshSection* Section);

// Clip test (used by cd* shaders).
int cdGetClipResult(const nglMeshSection* Section, const nglMeshNode* MeshNode,
                    nglScene* Scene);

// Misc helpers.
unsigned int auxGetHash(const nglTexture* Tex);
void         auxSetScale(nglMeshParams* dest, float x, float y, float z);
int          auxGetNBones(nglMesh* m);
nglMesh*     auxCreateScratchMesh(unsigned int flags, unsigned int num);
nglMesh*     auxCloseScratchMesh(nglMesh* m);
int          auxIsTextureAnimated(nglTexture* Tex);

// Mesh iterator over the APK "image" section.
class auxMeshIterator {
public:
    apk::apkFile*      Parent;
    apk::apkFileEntry* Curr;

    auxMeshIterator(apk::apkFile* parent, apk::apkFileEntry* curr)
        : Parent(parent), Curr(curr) {}

    nglMesh* operator*();
    auxMeshIterator operator++(int);
};

// Directory accessors.
void auxSetFontDirectory(tlResourceDirectory<nglFont>* d);
tlResourceDirectory<nglFont>* auxGetFontDirectory();
void auxSetMaterialDirectory(tlResourceDirectory<nglMaterial>* d);
tlResourceDirectory<nglMaterial>* auxGetMaterialDirectory();
void auxSetMeshDirectory(tlResourceDirectory<nglMesh>* d);
tlResourceDirectory<nglMesh>* auxGetMeshDirectory();
void auxSetTextureDirectory(tlResourceDirectory<nglTexture>* d);
tlResourceDirectory<nglTexture>* auxGetTextureDirectory();

void auxTextureDirectoryAdd(nglTexture* Texture);
void auxFontDirectoryAdd(nglFont* Font);
void auxMaterialDirectoryAdd(nglMaterial* Material);
void auxMeshDirectoryAdd(nglMesh* Mesh);
void auxFontDirectoryDelete(nglFont* Font);
void auxMaterialDirectoryDelete(nglMaterial* Material);
void auxMeshDirectoryDelete(nglMesh* Mesh);
void auxTextureDirectoryDelete(nglTexture* Tex);

// Alloc stub (nglMeshAllocFnStub).
void* nglMeshAllocFnStub(unsigned int a, unsigned int b, unsigned int c);

#endif // COD3_RENDER_NGL_AUX_H
