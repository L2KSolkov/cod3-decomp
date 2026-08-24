// ============================================================================
// ngl_aux.cpp - auxiliary NGL helpers + resource directories (40 funcs).
// Source: src/render/ngl_aux.cpp (render_xboxr:ngl_aux.o)
// Verified against IDA (render_xboxr:ngl_aux.o).
// ============================================================================

#include "ngl_aux.h"

#include "ngl/ngl_dx_fsaa.h"
#include "ngl/ngl_scene.h"
#include "filesystem/apk.h"

#include <new>
#include <string.h>

// ============================================================================
// DebugRender singleton (render.o data @ 0xF74D20, 148-byte value).
// The ported call sites reference the artifact `void* DebugRender_sInst`
// (3PAXA); it points at the backing storage.
// ============================================================================
char g_debugRenderStorage[148];
void* DebugRender_sInst = g_debugRenderStorage;  // ?DebugRender_sInst@@3PAXA

// ============================================================================
// Cross-object externs
// ============================================================================
struct nglFrustum;
extern bool nglIsSphereVisible(const nglFrustum* Frustum,
                               const math::Vector4* Center, float Radius);
extern nglMesh* nglCreateScratchMesh(unsigned int Flags, unsigned int NSections);
extern void tlWarning(const char* Format, ...);
extern tlSkipList<nglFont, tlFixedString>     nglFontDirectory;
extern tlSkipList<nglMesh, tlFixedString>     nglMeshDirectory;
extern tlSkipList<nglMaterial, tlFixedString> nglMaterialDirectory;
extern tlSkipList<nglTexture, tlFixedString>  nglTextureDirectory;

// Skip-list key accessors (defined in ngl_mesh.cpp / ngl_internal.cpp /
// ngl_font.cpp).
const tlFixedString* GetKey(const nglMesh* m);
const tlFixedString* GetKey(const nglMaterial* m);
const tlFixedString* GetKey(const nglTexture* t);
const tlFixedString* GetKey(const nglFont* f);

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
extern int gCurrentLine;
extern const char* gCurrentFile;
extern const char* gCurrentExpr;
extern ECoderId gCurrentAuthor;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

// ============================================================================
// Data (render_xboxr:ngl_aux.o)
// ============================================================================
tlResourceDirectory<nglFont>*     auxFontDirectory = NULL;
tlResourceDirectory<nglTexture>*  auxTextureDirectory = NULL;
tlResourceDirectory<nglMesh>*     auxMeshDirectory = NULL;
tlResourceDirectory<nglMaterial>* auxMaterialDirectory = NULL;

// ============================================================================
// ngl_aux_static_init - file-scope static initializer (ea: 0x7C3A50).
// Builds the four delegate_directory objects over the external skip lists.
// ============================================================================
struct ngl_aux_static_init {
    ngl_aux_static_init() {
        delegate_directory<nglFont>* dFont =
            (delegate_directory<nglFont>*)tlMemAlloc(sizeof(delegate_directory<nglFont>), 8u, 0);
        if (dFont != NULL) {
            ::new (dFont) delegate_directory<nglFont>(&nglFontDirectory);
            auxFontDirectory = dFont;
        } else {
            auxFontDirectory = NULL;
        }

        delegate_directory<nglMesh>* dMesh =
            (delegate_directory<nglMesh>*)tlMemAlloc(sizeof(delegate_directory<nglMesh>), 8u, 0);
        if (dMesh != NULL) {
            ::new (dMesh) delegate_directory<nglMesh>(&nglMeshDirectory);
            auxMeshDirectory = dMesh;
        } else {
            auxMeshDirectory = NULL;
        }

        delegate_directory<nglMaterial>* dMaterial =
            (delegate_directory<nglMaterial>*)tlMemAlloc(sizeof(delegate_directory<nglMaterial>), 8u, 0);
        if (dMaterial != NULL) {
            ::new (dMaterial) delegate_directory<nglMaterial>(&nglMaterialDirectory);
            auxMaterialDirectory = dMaterial;
        } else {
            auxMaterialDirectory = NULL;
        }

        delegate_directory<nglTexture>* dTexture =
            (delegate_directory<nglTexture>*)tlMemAlloc(sizeof(delegate_directory<nglTexture>), 8u, 0);
        if (dTexture != NULL) {
            ::new (dTexture) delegate_directory<nglTexture>(&nglTextureDirectory);
            auxTextureDirectory = dTexture;
        } else {
            auxTextureDirectory = NULL;
        }
    }
};

static ngl_aux_static_init ngl_aux_static_init_instance;

// ============================================================================
// GetKeyPtr - free key accessor for the skip-list directories (inline COMDAT).
// ============================================================================
const tlFixedString* GetKeyPtr(const nglMaterial* Material) { return Material->Name; }
const tlFixedString* GetKeyPtr(const nglTexture* Tex) { return Tex->FileName; }
const tlFixedString* GetKeyPtr(const nglMesh* Mesh) { return Mesh->Name; }
const tlFixedString* GetKeyPtr(const nglFont* Font) { return Font->FileName; }

// ============================================================================
// nglMeshAllocFnStub - ea: 0x7C2C80
// ============================================================================
void* nglMeshAllocFnStub(unsigned int a, unsigned int b, unsigned int c) {
    (void)a;
    (void)b;
    (void)c;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "ngl_aux.cpp";
    AeAssert::gCurrentLine = 7;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("valenzuela@treyarch.com"))
        __debugbreak();
    __debugbreak();
    return NULL;
}

// ============================================================================
// Sphere helpers - ea: 0x7C2CD0..0x7C2DC0
// ============================================================================
math::Position3* auxGetSphereCenter(math::Position3* result, nglMesh* Mesh) {
    result->v = Mesh->Sphere.v;
    return result;
}

math::Position3* auxGetSphereCenter(math::Position3* result, nglMeshSection* Section) {
    result->v = Section->Sphere.v;
    return result;
}

float auxGetSphereRadius(nglMesh* Mesh) {
    return _mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 0xFF).m128_f32[0];
}

float auxGetSphereRadius(nglMeshSection* Section) {
    return _mm_shuffle_ps(Section->Sphere.v, Section->Sphere.v, 0xFF).m128_f32[0];
}

math::Vector4* auxGetSphere(math::Vector4* result, nglMesh* Mesh) {
    *result = Mesh->Sphere;
    return result;
}

math::Vector4* auxGetSphere(math::Vector4* result, nglMeshSection* Section) {
    *result = Section->Sphere;
    return result;
}

// ============================================================================
// cdGetClipResult - ea: 0x7C2DF0 (nglMeshNode is the IDA class-tag type)
// ============================================================================
int cdGetClipResult(const nglMeshSection* Section, const nglMeshNode* MeshNode,
                    nglScene* Scene) {
    math::Vector4 v4 = Section->Sphere;
    math::Dir3 v5 = MeshNode->LocalToWorld.z;
    float v8 = _mm_shuffle_ps(v4.v, v4.v, 0xFF).m128_f32[0];
    math::Position3 v7;
    v7.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v4.v, v4.v, 0), MeshNode->LocalToWorld.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(v4.v, v4.v, 0x55), MeshNode->LocalToWorld.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v4.v, v4.v, 0xAA), v5.v),
                   MeshNode->LocalToWorld.w.v));
    return nglIsSphereVisible((const nglFrustum*)Scene->ClipPlanes,
                              (const math::Vector4*)&v7, v8)
           - 1;
}

// ============================================================================
// auxGetHash - ea: 0x7C2E90
// ============================================================================
unsigned int auxGetHash(const nglTexture* Tex) {
    return Tex->FileName->hash;
}

// ============================================================================
// auxSetScale - ea: 0x7C2EA0
// ============================================================================
void auxSetScale(nglMeshParams* dest, float x, float y, float z) {
    math::DiagMat33 v3;
    v3.v.m128_f32[0] = x;
    v3.v.m128_f32[1] = y;
    v3.v.m128_f32[2] = z;
    dest->Flags |= 2u;
    dest->Scale.v = v3.v;
}

// ============================================================================
// auxGetNBones - ea: 0x7C2F00
// ============================================================================
int auxGetNBones(nglMesh* m) {
    if (m != NULL && m->Skeleton != NULL)
        return ((nglSkeleton*)m->Skeleton)->NBones;
    return 0;
}

// ============================================================================
// auxCreateScratchMesh / auxCloseScratchMesh - ea: 0x7C2F20 / 0x7C2F30
// ============================================================================
nglMesh* auxCreateScratchMesh(int flags, int num) {
    return nglCreateScratchMesh(flags, num);
}

nglMesh* auxCloseScratchMesh(nglMesh* m) {
    return m;
}

// ============================================================================
// auxIsTextureAnimated - ea: 0x7C2F40
// ============================================================================
int auxIsTextureAnimated(nglTexture* Tex) {
    return (Tex->Flags & 4) != 0;
}

// ============================================================================
// auxMeshIterator - ea: 0x7C2F60 / 0x7C2FA0
// ============================================================================
nglMesh* auxMeshIterator::operator*() {
    tlFixedString name("image");
    int SectionIndex = Parent->GetSectionIndex(name);
    return (nglMesh*)Curr->GetData(Parent, SectionIndex, true);
}

auxMeshIterator auxMeshIterator::operator++(int) {
    apk::apkFileEntry* NextFile =
        Parent->GetNextFile(0x4853454Du, Curr);
    Curr = NextFile;
    auxMeshIterator result(Parent, NextFile);
    return result;
}

// ============================================================================
// Directory accessors - ea: 0x7C3050..0x7C30C0
// ============================================================================
void auxSetFontDirectory(tlResourceDirectory<nglFont>* d) { auxFontDirectory = d; }
tlResourceDirectory<nglFont>* auxGetFontDirectory() { return auxFontDirectory; }
void auxSetMaterialDirectory(tlResourceDirectory<nglMaterial>* d) {
    auxMaterialDirectory = d;
}
tlResourceDirectory<nglMaterial>* auxGetMaterialDirectory() { return auxMaterialDirectory; }
void auxSetMeshDirectory(tlResourceDirectory<nglMesh>* d) { auxMeshDirectory = d; }
tlResourceDirectory<nglMesh>* auxGetMeshDirectory() { return auxMeshDirectory; }
void auxSetTextureDirectory(tlResourceDirectory<nglTexture>* d) {
    auxTextureDirectory = d;
}
tlResourceDirectory<nglTexture>* auxGetTextureDirectory() { return auxTextureDirectory; }

// ============================================================================
// Directory add/delete - ea: 0x7C30D0..0x7C31F0
// ============================================================================
void auxTextureDirectoryAdd(nglTexture* Texture) {
    if (auxTextureDirectory->Find(*Texture->FileName) == NULL)
        auxTextureDirectory->Add(Texture);
}

void auxFontDirectoryAdd(nglFont* Font) {
    if (auxFontDirectory->Find(*Font->FileName) == NULL)
        auxFontDirectory->Add(Font);
}

void auxMaterialDirectoryAdd(nglMaterial* Material) {
    if (auxMaterialDirectory->Find(*Material->Name) == NULL)
        auxMaterialDirectory->Add(Material);
}

void auxMeshDirectoryAdd(nglMesh* Mesh) {
    if (auxMeshDirectory->Find(*Mesh->Name) == NULL)
        auxMeshDirectory->Add(Mesh);
}

void auxFontDirectoryDelete(nglFont* Font) {
    auxFontDirectory->Del(Font);
}

void auxMaterialDirectoryDelete(nglMaterial* Material) {
    auxMaterialDirectory->Del(Material);
}

void auxMeshDirectoryDelete(nglMesh* Mesh) {
    auxMeshDirectory->Del(Mesh);
}

void auxTextureDirectoryDelete(nglTexture* Tex) {
    auxTextureDirectory->Del(Tex);
}
