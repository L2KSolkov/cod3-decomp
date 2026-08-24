// ============================================================================
// ngl_mesh.cpp - mesh/material resources + mesh list-add (17 funcs).
// Source: src/ngl_mesh.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_mesh.o).
// Data: nglMeshDirectory/nglMaterialDirectory/nglShaderBank/
// nglGeometryShaderBank/nglVertexDefBank/nglGetMeshFunc/nglGetMaterialFunc
// + file statics (ScaledLocalToWorld_0, _S23_8, gEmptyShader,
//   gEmptyMaterial, nglEmptyMeshParams).
// ============================================================================

#include "ngl/ngl_mesh.h"
#include "ngl/nglDebug.h"
#include "ngl/ngl_dx_quad.h"
#include "filesystem/apk.h"

#include <intrin.h>
#include <math.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern int nglFrame;                              // ngl_internal.o
extern nglScene* nglBuildScene;                   // ngl_scene.o
extern nglDebugStruct nglSyncDebug;               // ngl_debug.o
extern nglPerfInfoStruct nglPerfInfo;             // ngl_debug.o
extern bool nglProfileEvalShader(nglShader* Shader);  // ngl_debug.o
extern void nglValidateMatrices(nglScene* Scene);     // ngl_scene.o
extern void nglSceneDumpMesh(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                             const nglMeshParams* Params);  // ngl_scenedump.o
extern int ngliListAddMesh_GetClipResult(const math::Position3& Center,
                                         float Radius,
                                         int ParamFlags);  // ngl_dx_mesh.o
extern void ngliWaitForResource(void);            // ngl_dx_core.o
extern void nglMorphInit();                        // ngl_morph.o
extern void* nglListAlloc(unsigned int Bytes, unsigned int Alignment);  // nglRenderNode.h
extern void* nglEmptyParamSet;                    // ngl_params.o
extern void tlWarning(const char* fmt, ...);
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data (ngl_mesh.o)
// ============================================================================
tlSkipList<nglMesh, tlFixedString> nglMeshDirectory;
tlSkipList<nglMaterial, tlFixedString> nglMaterialDirectory;
tlInstanceBank nglShaderBank;
tlInstanceBank nglGeometryShaderBank;
tlInstanceBank nglVertexDefBank;
void* (*nglGetMeshFunc)(const tlFixedString&, unsigned int) = NULL;
void* (*nglGetMaterialFunc)(const tlFixedString&, unsigned int) = NULL;

static math::Mat43 ScaledLocalToWorld_0;
static unsigned char nglListAddMesh_GetScaledMatrix_InitFlag = 0;  // _S23_8
static nglShader gEmptyShader;
static nglMaterial gEmptyMaterial;
static nglMeshParams nglEmptyMeshParams;

// Skip-list key accessors (template free functions).
const tlFixedString* GetKey(const nglMesh* m) { return m->Name; }
const tlFixedString* GetKey(const nglMaterial* m) { return m->Name; }

// ============================================================================
// nglCanReleaseMesh - ea: 0x843610
// ============================================================================
bool nglCanReleaseMesh(nglMesh* Mesh) {
    return Mesh->LastFrameRef + 1 < nglFrame;
}

// ============================================================================
// nglProcessMaterial - ea: 0x843630
// ============================================================================
void nglProcessMaterial(nglMaterial* Material) {
    tlFixedString ShaderName = *(tlFixedString*)Material->Shader;
    tlInstanceBank::Instance* v2 = nglShaderBank.Search(ShaderName);
    if (v2 != NULL) {
        nglShader* Value = (nglShader*)v2->Value;
        if (Value->CheckMaterialVersion(Material)) {
            Material->Shader = Value;
            Value->BindMaterial(Material);
            return;
        }
        tlWarning("Material %s binary version (%d) is not compatible with shader %s.\n",
                  Material->Name->str, Material->BinaryVersion, ShaderName.str);
    } else {
        tlWarning("NGL: Unable to find shader %s, used by material %s.\n",
                  ShaderName.str, Material->Name->str);
    }
    Material->Shader = &gEmptyShader;
    Material->Shader->BindMaterial(Material);
}

// ============================================================================
// nglProcessSection - ea: 0x8436F0
// ============================================================================
extern void ngliProcessSection(nglMesh* Mesh, nglMeshSection* Section,
                               apk::apkFileEntry* Entry);  // ngl_dx_mesh.o

void nglProcessSection(nglMesh* Mesh, nglMeshSection* Section, apk::apkFileEntry* Entry) {
    if (Section->Material == NULL) {
        tlWarning("NGL: Section missing material.\n");
        Section->Material = &gEmptyMaterial;
    }
    nglMaterial* Material = Section->Material;
    if (Material != NULL && !Material->Shader->CheckVertexDefVersion(Section)) {
        tlFixedString Name = Material->Shader->GetName();
        tlWarning("Section VertexDef Binary version (%d) is incompatible with shader %s\n.",
                  Section->BinaryVersion, Name.str);
        Section->Material->Shader = &gEmptyShader;
    }
    ngliProcessSection(Mesh, Section, Entry);
    Section->Material->Shader->BindSection(Section, Mesh);
}

// ============================================================================
// nglProcessMesh - ea: 0x843780
// ============================================================================
void nglProcessMesh(nglMesh* Mesh, apk::apkFileEntry* Entry) {
    unsigned int Flags = Mesh->Flags;
    if ((Flags & 0x10000) == 0) {
        Mesh->Flags = Flags | 0x10000;
        for (unsigned int i = 0; i < Mesh->NSections; ++i) {
            Mesh->Sections[i].Flags = 1;
            nglMeshSection* Section = Mesh->Sections[i].Section;
            if (Section != NULL)
                nglProcessSection(Mesh, Section, Entry);
        }
    }
}

// ============================================================================
// nglUnloadMesh - ea: 0x8437E0
// ============================================================================
extern void ngliUnloadSection(nglMeshSection* Section);  // ngl_dx_mesh.o (nullsub_43)

void nglUnloadMesh(nglMesh* Mesh) {
    if ((Mesh->Flags & 0x10000) != 0) {
        for (unsigned int i = 0; i < Mesh->NSections; ++i) {
            if (Mesh->Sections[i].Section != NULL)
                ngliUnloadSection(Mesh->Sections[i].Section);
        }
        Mesh->Flags &= ~0x10000u;
    }
}

// ============================================================================
// nglGetLOD - ea: 0x843830
// ============================================================================
unsigned int nglGetLOD(nglMesh* Mesh, const math::Mat43& LocalToWorld, nglScene* Scene) {
    if (Mesh == NULL)
        return 0;
    nglValidateMatrices(Scene);
    __m128 v3 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 0), LocalToWorld.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 85), LocalToWorld.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 170), LocalToWorld.z.v),
                   LocalToWorld.w.v));
    __m128 v4 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v3, v3, 0), Scene->WorldToView.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(v3, v3, 85), Scene->WorldToView.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v3, v3, 170), Scene->WorldToView.z.v),
                   Scene->WorldToView.w.v));
    float viewZ = _mm_shuffle_ps(v4, v4, 170).m128_f32[0];
    int v5 = (int)Mesh->NLODs - 1;
    if (v5 < 0)
        return 0;
    for (float* i = &Mesh->LODs[v5].Range; viewZ <= *i; i -= 2) {
        if (--v5 < 0)
            return 0;
    }
    return (unsigned int)(v5 + 1);
}

// ============================================================================
// nglListAddMesh_GetLOD (file-static) - ea: 0x843930
// ============================================================================
static nglMesh* nglListAddMesh_GetLOD(nglMesh* Mesh, char LOD,
                                      nglMeshParams* MeshParams,
                                      const math::Position3* Center) {
    nglMesh* v6;
    if (LOD >= 0) {
        __m128 v7 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Center->v, Center->v, 0), nglBuildScene->WorldToView.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(Center->v, Center->v, 85), nglBuildScene->WorldToView.y.v)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Center->v, Center->v, 170), nglBuildScene->WorldToView.z.v),
                       nglBuildScene->WorldToView.w.v));
        float v11 = _mm_shuffle_ps(v7, v7, 170).m128_f32[0];
        int v8 = (int)Mesh->NLODs - 1;
        if (v8 < 0)
            return Mesh;
        nglMeshLOD* LODs = Mesh->LODs;
        for (float* i = &LODs[v8].Range; v11 <= *i; i -= 2) {
            if (--v8 < 0)
                return Mesh;
        }
        v6 = LODs[v8].Mesh;
    } else {
        unsigned int v4 = MeshParams->LOD;
        if (v4 == 0)
            return Mesh;
        unsigned int NLODs = Mesh->NLODs;
        if (v4 - 1 <= NLODs)
            v6 = Mesh->LODs[v4 - 1].Mesh;
        else
            v6 = Mesh->LODs[NLODs - 1].Mesh;
    }
    if (v6 != NULL)
        return v6;
    return Mesh;
}

// ============================================================================
// nglListAddMesh_GetScaledMatrix - ea: 0x843A20
// ============================================================================
math::Mat43* nglListAddMesh_GetScaledMatrix(const math::Mat43& LocalToWorld,
                                            nglMeshParams* MeshParams, float* MaxScale) {
    __m128 v3 = MeshParams->Scale.v;
    __m128 v4 = _mm_andnot_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)), v3);
    __m128 v5 = _mm_shuffle_ps(v4, _mm_shuffle_ps(_mm_set1_ps(1.0f), v4, 160), 52);
    float v6 = _mm_shuffle_ps(v5, v5, 170).m128_f32[0];
    float v7 = _mm_shuffle_ps(v5, v5, 85).m128_f32[0];
    if (v7 <= v6)
        v7 = v6;
    if (v5.m128_f32[0] > v7)
        v7 = v5.m128_f32[0];
    *MaxScale = v7;
    if ((nglListAddMesh_GetScaledMatrix_InitFlag & 1) == 0)
        nglListAddMesh_GetScaledMatrix_InitFlag |= 1u;
    ScaledLocalToWorld_0.x.v = _mm_mul_ps(LocalToWorld.x.v, _mm_shuffle_ps(v3, v3, 0));
    ScaledLocalToWorld_0.y.v = _mm_mul_ps(LocalToWorld.y.v, _mm_shuffle_ps(v3, v3, 85));
    ScaledLocalToWorld_0.z.v = _mm_mul_ps(LocalToWorld.z.v, _mm_shuffle_ps(v3, v3, 170));
    ScaledLocalToWorld_0.w.v = LocalToWorld.w.v;
    return &ScaledLocalToWorld_0;
}

// ============================================================================
// Transpose four 4-vectors (matrix transpose for LocalToScreen).
// ============================================================================
static void TransposeFour(__m128 c0, __m128 c1, __m128 c2, __m128 c3, __m128 out[4]) {
    __m128 t0 = _mm_shuffle_ps(c0, c1, 0x44);
    __m128 t1 = _mm_shuffle_ps(c0, c1, 0xEE);
    __m128 t2 = _mm_shuffle_ps(c2, c3, 0x44);
    __m128 t3 = _mm_shuffle_ps(c2, c3, 0xEE);
    out[0] = _mm_shuffle_ps(t0, t2, 0x88);
    out[1] = _mm_shuffle_ps(t0, t2, 0xDD);
    out[2] = _mm_shuffle_ps(t1, t3, 0x88);
    out[3] = _mm_shuffle_ps(t1, t3, 0xDD);
}

// ============================================================================
// nglListAddMesh_Setup - ea: 0x843C50
// ============================================================================
nglMeshNode* nglListAddMesh_Setup(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                                  nglMeshParams* MeshParams,
                                  nglShaderParamSet* ShaderParams,
                                  void (*fn)(nglMeshNode*)) {
    if (Mesh != NULL && Mesh->NSections != 0) {
        if ((Mesh->Flags & 0xB0000) == 0
            && _tlAssert("src/ngl_mesh.cpp", 328,
                         "(Mesh->Flags & NGLMESH_PROCESSED) || (Mesh->Flags & (NGLMESH_SCRATCH_MESH|NGLMESH_STATIC))",
                         "Mesh missing NGLMESH_PROCESSED flag."))
            __debugbreak();
        const math::Mat43* ScaledMatrix = &LocalToWorld;
        __m128 v7 = _mm_mul_ps(ScaledMatrix->x.v, ScaledMatrix->x.v);
        float v54 = v7.m128_f32[0] + (v7.m128_f32[1] + v7.m128_f32[2]);
        __m128 v8 = _mm_mul_ps(ScaledMatrix->y.v, ScaledMatrix->y.v);
        float Radius = v8.m128_f32[0] + (v8.m128_f32[1] + v8.m128_f32[2]);
        __m128 v9 = _mm_mul_ps(ScaledMatrix->z.v, ScaledMatrix->z.v);
        float v57 = fabsf(v54 - 1.0f);
        if (fabsf((v9.m128_f32[0] + (v9.m128_f32[1] + v9.m128_f32[2])) - 1.0f)
                + fabsf(Radius - 1.0f) + v57 >= 0.1f
            && _tlAssert(
                "src/ngl_mesh.cpp", 332,
                "fabsf(math::AbsSquared(LocalToWorld.GetX())-1.0f) + fabsf(math::AbsSquared(LocalToWorld.GetY())-1.0f) + fabsf(math::AbsSquared(LocalToWorld.GetZ())-1.0f) < 0.1f",
                "Invalid scale detected in local to world transform.  If scaling is desired, use MeshParams.\n"))
            __debugbreak();
        if (nglSyncDebug.DisableScratch == 0 || (Mesh->Flags & 0x20000) == 0) {
            if (nglSyncDebug.DumpSceneFile != 0)
                nglSceneDumpMesh(Mesh, LocalToWorld, MeshParams);
            nglValidateMatrices(nglBuildScene);
            unsigned int ParamFlags = MeshParams != NULL ? MeshParams->Flags : 0;
            float v52 = Mesh->Sphere.v.m128_f32[3];
            float MaxScale = 1.0f;
            if ((ParamFlags & 2) != 0) {
                ScaledMatrix = nglListAddMesh_GetScaledMatrix(LocalToWorld, MeshParams, &MaxScale);
                v52 = v52 * MaxScale;
            }
            math::Position3 Center;
            Center.v = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 0), ScaledMatrix->x.v),
                           _mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 85), ScaledMatrix->y.v)),
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 170), ScaledMatrix->z.v),
                           ScaledMatrix->w.v));
            const int clipResult = ngliListAddMesh_GetClipResult(Center, v52,
                                                                  (int)ParamFlags);
            if (clipResult != -1) {
                if (Mesh->NLODs != 0)
                    Mesh = nglListAddMesh_GetLOD(Mesh, (char)ParamFlags, MeshParams, &Center);
                nglMeshNode* node = (nglMeshNode*)nglListAlloc(0x90, 0x40);
                node->LocalToWorld = *ScaledMatrix;
                __m128 rows[4];
                if ((ParamFlags & 1) != 0) {
                    // LocalToScreen = transpose(WorldToScreen).
                    TransposeFour(nglBuildScene->WorldToScreen.x.v,
                                  nglBuildScene->WorldToScreen.y.v,
                                  nglBuildScene->WorldToScreen.z.v,
                                  nglBuildScene->WorldToScreen.w.v, rows);
                } else {
                    // LocalToScreen = transpose(WorldToScreen * M), M = scaled axes + unit w.
                    __m128 ax = _mm_shuffle_ps(ScaledMatrix->x.v, _mm_setzero_ps(), 0xE4);
                    __m128 ay = _mm_shuffle_ps(ScaledMatrix->y.v, _mm_setzero_ps(), 0xE4);
                    __m128 az = _mm_shuffle_ps(ScaledMatrix->z.v, _mm_setzero_ps(), 0xE4);
                    __m128 aw = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
                    __m128 w2s[4] = {
                        nglBuildScene->WorldToScreen.x.v,
                        nglBuildScene->WorldToScreen.y.v,
                        nglBuildScene->WorldToScreen.z.v,
                        nglBuildScene->WorldToScreen.w.v,
                    };
                    __m128 cols[4];
                    for (int c = 0; c < 4; ++c) {
                        __m128 col = _mm_setzero_ps();
                        float mx = ax.m128_f32[c];
                        float my = ay.m128_f32[c];
                        float mz = az.m128_f32[c];
                        float mw = aw.m128_f32[c];
                        for (int r = 0; r < 4; ++r) {
                            float m = r == 0 ? mx : r == 1 ? my : r == 2 ? mz : mw;
                            col = _mm_add_ps(col, _mm_mul_ps(w2s[r], _mm_set1_ps(m)));
                        }
                        cols[c] = col;
                    }
                    TransposeFour(cols[0], cols[1], cols[2], cols[3], rows);
                }
                node->LocalToScreen.x.v = rows[0];
                node->LocalToScreen.y.v = rows[1];
                node->LocalToScreen.z.v = rows[2];
                node->LocalToScreen.w.v = rows[3];
                node->Mesh = Mesh;
                node->MaxScale = MaxScale;
                if (MeshParams == NULL) {
                    node->MeshParams = (nglMeshParams*)nglListAlloc(0x20, 0x10);
                    *node->MeshParams = nglEmptyMeshParams;
                } else if ((int)ParamFlags >= 0) {
                    node->MeshParams = (nglMeshParams*)nglListAlloc(0x20, 0x10);
                    *node->MeshParams = *MeshParams;
                } else {
                    node->MeshParams = MeshParams;
                }
                node->ShaderParams.Array = ShaderParams != NULL
                    ? ShaderParams->Array : (unsigned int*)nglEmptyParamSet;
                if (fn != NULL)
                    fn(node);
                return node;
            }
        }
    }
    return NULL;
}

// ============================================================================
// nglListAddMesh_Sections - ea: 0x843B90
// ============================================================================
nglMeshNode* nglListAddMesh_Sections(nglMesh* Mesh, nglMeshNode* MeshNode) {
    nglMesh* v2 = MeshNode->Mesh;
    nglMeshSection** Sections = &v2->Sections->Section;
    unsigned int i = v2->NSections;
    unsigned int TotalVerts = nglPerfInfo.TotalVerts;
    do {
        nglMeshSection* v5 = *Sections;
        _mm_prefetch(reinterpret_cast<const char*>(Sections[2]), _MM_HINT_T0);
        Sections += 2;
        if (v5 != NULL) {
            const bool shaderEnabled = v5->Material != nullptr
                && v5->Material->Shader != nullptr
                && nglProfileEvalShader(v5->Material->Shader)
                && !v5->Material->Shader->Disabled;
            if (shaderEnabled)
                v5->Material->Shader->AddNode(MeshNode, v5, v5->Material);
            TotalVerts += v5->NVertices;
        }
        --i;
    } while (i != 0);
    nglPerfInfo.TotalVerts = TotalVerts;
    nglPerfInfo.TotalPolys += v2->NPolys;
    v2->LastFrameRef = nglFrame;
    apk::apkFile* File = (apk::apkFile*)v2->File;
    if (File != NULL)
        File->LastFrameRef = nglFrame;
    return MeshNode;
}

// ============================================================================
// nglListAddMesh - ea: 0x844250
// ============================================================================
nglMeshNode* nglListAddMesh(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                            nglMeshParams* MeshParams, nglShaderParamSet* ShaderParams,
                            void (*fn)(nglMeshNode*)) {
    nglMeshNode* result = nglListAddMesh_Setup(Mesh, LocalToWorld, MeshParams, ShaderParams, fn);
    if (result != NULL)
        return nglListAddMesh_Sections(Mesh, result);
    return result;
}

// ============================================================================
// APK callbacks
// ============================================================================
void nglAPKMeshLoadCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData) {
    tlFixedString name("Image");
    int SectionIndex = File->GetSectionIndex(name);
    nglMesh* Data = (nglMesh*)Entry->GetData(File, SectionIndex, true);
    Data->File = File;
    Data->LastFrameRef = -1;
    nglProcessMesh(Data, Entry);
    if (nglMeshDirectory.Add(Data) != NULL)
        tlWarning("Duplicate mesh %s found.\n", Data->Name->str);
}

void nglAPKMeshDeleteCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData) {
    tlFixedString name("Image");
    int SectionIndex = File->GetSectionIndex(name);
    nglMesh* Data = (nglMesh*)Entry->GetData(File, SectionIndex, true);
    nglMeshDirectory.Del(Data);
    if (Data->LastFrameRef + 1 >= nglFrame) {
        tlWarning("NGL: Mesh %s destroyed while still referenced by the async renderer.\n",
                  Data->Name->str);
        ngliWaitForResource();
    }
    nglUnloadMesh(Data);
}

void nglAPKMaterialLoadCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData) {
    tlFixedString name("Image");
    int SectionIndex = File->GetSectionIndex(name);
    nglMaterial* Data = (nglMaterial*)Entry->GetData(File, SectionIndex, true);
    nglProcessMaterial(Data);
    if (nglMaterialDirectory.Add(Data) != NULL)
        tlWarning("Duplicate material %s found.\n", Data->Name->str);
}

void nglAPKMaterialDeleteCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData) {
    tlFixedString name("Image");
    int SectionIndex = File->GetSectionIndex(name);
    const nglMaterial* Data = (const nglMaterial*)Entry->GetData(File, SectionIndex, true);
    nglMaterialDirectory.Del(Data);
}

// ============================================================================
// nglMeshInit - ea: 0x844420
// ============================================================================
void nglMeshInit() {
    apk::apkRegisterFileType(0x4853454Du, 2, nglAPKMeshLoadCallback,
                             nglAPKMeshDeleteCallback, NULL);
    apk::apkRegisterFileType(0x54414Du, 2, nglAPKMaterialLoadCallback,
                             nglAPKMaterialDeleteCallback, NULL);
    nglMeshDirectory.Level = 0;
    nglMeshDirectory.Head = (tlSkipList<nglMesh, tlFixedString>::Instance*)
        tlMemAlloc(0x44u, 8u, 0x1000000);
    for (int i = 0; i <= 15; ++i)
        nglMeshDirectory.Head->Forward[i] = NULL;
    nglMaterialDirectory.Level = 0;
    nglMaterialDirectory.Head = (tlSkipList<nglMaterial, tlFixedString>::Instance*)
        tlMemAlloc(0x44u, 8u, 0x1000000);
    for (int j = 0; j <= 15; ++j)
        nglMaterialDirectory.Head->Forward[j] = NULL;
    nglVertexDefBank.Init();
    nglShaderBank.Init();
    nglGeometryShaderBank.Init();
    if (nglShaderParamSet::NumParams > 0x40
        && _tlAssert("src/ngl_mesh.cpp", 85, "nglShaderParamSet::NumParams <= 64",
                     "Too many shader parameters registered."))
        __debugbreak();
    if (nglSceneParamSet::NumParams > 0x40
        && _tlAssert("src/ngl_mesh.cpp", 86, "nglSceneParamSet::NumParams <= 64",
                     "Too many scene parameters registered."))
        __debugbreak();
    nglMorphInit();
}

// ============================================================================
// nglGetMesh / nglGetMaterial
// ============================================================================
nglMesh* nglGetMesh(const tlFixedString& Name, bool Warn) {
    if (nglGetMeshFunc == NULL) {
        nglMesh* v3 = nglMeshDirectory.Find(Name);
        if (v3 == NULL && Warn)
            tlWarning("nglGetMesh: Unable to find mesh %s.\n", Name.str);
        return v3;
    }
    nglMesh* result = (nglMesh*)nglGetMeshFunc(Name, 0x4853454Du);
    if (result != NULL)
        return result;
    nglMesh* v3 = nglMeshDirectory.Find(Name);
    if (v3 == NULL && Warn)
        tlWarning("nglGetMesh: Unable to find mesh %s.\n", Name.str);
    return v3;
}

nglMaterial* nglGetMaterial(const tlFixedString& Name, bool Warn) {
    if (nglGetMaterialFunc == NULL) {
        nglMaterial* v3 = nglMaterialDirectory.Find(Name);
        if (v3 == NULL && Warn)
            tlWarning("nglGetMaterial: Unable to find material %s.\n", Name.str);
        return v3;
    }
    nglMaterial* result = (nglMaterial*)nglGetMaterialFunc(Name, 0x4853454Du);
    if (result != NULL)
        return result;
    nglMaterial* v3 = nglMaterialDirectory.Find(Name);
    if (v3 == NULL && Warn)
        tlWarning("nglGetMaterial: Unable to find material %s.\n", Name.str);
    return v3;
}
