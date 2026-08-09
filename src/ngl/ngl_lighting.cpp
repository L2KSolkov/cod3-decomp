// ============================================================================
// ngl_lighting.cpp - light context + light list building (22 funcs).
// Source: src/ngl_lighting.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_lighting.o).
// Data: nglDefaultLightContext/nglBuildLightContext/nglSendLightContext/
// nglLightContextParam/nglLightContextParamID live here.
// ============================================================================

#include "ngl/ngl_lighting.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/nglDebug.h"
#include "ngl/nglTexture.h"

#include <intrin.h>
#include <math.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern nglScene* nglBuildScene;                       // ngl_scene.o
extern nglDebugStruct nglSyncDebug;                   // ngl_debug.o
extern void* nglListAlloc(unsigned int Bytes, unsigned int Alignment);  // nglRenderNode.h
extern void nglSceneDumpDirLight(unsigned int LightCat, const math::Dir3* Dir,
                                 const math::Vector4* Color);  // ngl_scenedump.o
extern void nglSceneDumpPointLight(nglLightType Type, unsigned int LightCat,
                                   const math::Position3* Pos, float Near, float Far,
                                   const math::Vector4* Color);  // ngl_scenedump.o
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data (ngl_lighting.o)
// ============================================================================
nglLightContext* nglDefaultLightContext = NULL;
nglLightContext* nglBuildLightContext = NULL;
nglLightContext* nglSendLightContext = NULL;

// Param-set descriptor for the light-context shader param.
struct nglLightContextParamType {
    nglLightContext* Value;  // +0x00
};
nglLightContextParamType nglLightContextParam;
unsigned int nglLightContextParamID = 0;

// ============================================================================
// nglSetLightContext - ea: 0x845930
// ============================================================================
void nglSetLightContext(nglLightContext* Context, nglScene* Scene) {
    Scene->LightContext = Context;
}

// ============================================================================
// nglSelectBuildLightContext - ea: 0x845950
// ============================================================================
nglLightContext* nglSelectBuildLightContext(nglLightContext* Context) {
    nglLightContext* result = nglBuildLightContext;
    nglBuildLightContext = Context;
    return result;
}

// ============================================================================
// nglSetAmbientLight - ea: 0x845970
// ============================================================================
void nglSetAmbientLight(float r, float g, float b) {
    __m128 v2 = _mm_setr_ps(r, g, b, 1.0f);
    __m128 shuffled = _mm_shuffle_ps(v2, _mm_shuffle_ps(_mm_set1_ps(1.0f), v2, 160), 52);
    nglBuildLightContext->Ambient.v = _mm_min_ps(_mm_max_ps(shuffled, _mm_setzero_ps()),
                                                 _mm_set1_ps(1.0f));
}

// ============================================================================
// nglDetermineGunLights - ea: 0x845A10
// ============================================================================
void nglDetermineGunLights(const math::Position3* WorldPos) {
    nglSendLightContext = nglBuildScene->LightContext;
    nglSendLightContext->Head.LocalNext = &nglSendLightContext->Head;
    for (nglLightNode* i = nglSendLightContext->Head.Next[6];
         i != (nglLightNode*)nglSendLightContext;
         i = i->Next[6]) {
        switch (i->Type) {
        case NGLLIGHT_POINT: {
            __m128* NodeData = (__m128*)i->NodeData;
            __m128 v4 = _mm_sub_ps(NodeData[0], WorldPos->v);
            __m128 v5 = _mm_mul_ps(v4, v4);
            float Far = NodeData[2].m128_f32[1] + 500.0f;
            if (Far * Far >= v5.m128_f32[0] + (v5.m128_f32[1] + v5.m128_f32[2]))
                goto add;
            break;
        }
        case NGLLIGHT_DIRECTIONAL:
        case NGLLIGHT_POINTGUN:
        add:
            i->LocalNext = nglSendLightContext->Head.LocalNext;
            nglSendLightContext->Head.LocalNext = i;
            break;
        case NGLLIGHT_PROJECTED_SPOT:
        case NGLLIGHT_PROJECTED_PARALLEL:
            break;
        default:
            if (_tlAssert("src/ngl_lighting.cpp", 284, "false", "Invalid light type."))
                __debugbreak();
            break;
        }
    }
}

// ============================================================================
// nglGetFakePointLight - ea: 0x845B10
// ============================================================================
bool nglGetFakePointLight(nglDirLightInfo* DirLight, nglPointLightInfo* Light,
                          const math::Position3* Pos) {
    __m128 v3 = _mm_sub_ps(Pos->v, Light->Pos.v);
    __m128 v4 = _mm_mul_ps(v3, v3);
    float dist2 = v4.m128_f32[0] + (v4.m128_f32[1] + v4.m128_f32[2]);
    if (dist2 >= 0.000001f) {
        float invDist = 1.0f / sqrtf(dist2);
        float t = 1.0f - ((dist2 * invDist) - Light->Near) / (Light->Far - Light->Near);
        bool result = true;
        if (t >= 0.0f) {
            if (t > 1.0f)
                t = 1.0f;
        } else {
            t = 0.0f;
            result = false;
        }
        DirLight->Color.v = _mm_mul_ps(Light->Color.v, _mm_set1_ps(t));
        DirLight->Dir.v = _mm_mul_ps(v3, _mm_set1_ps(invDist));
        return result;
    } else {
        DirLight->Color = Light->Color;
        DirLight->Dir.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        return false;
    }
}

// ============================================================================
// nglGetLightAsPointLight - ea: 0x845C40
// ============================================================================
nglPointLightInfo* nglGetLightAsPointLight(nglPointLightInfo* Out, nglLightNode* Node,
                                           const math::Position3* Pos) {
    switch (Node->Type) {
    case NGLLIGHT_POINT:
    case NGLLIGHT_POINTGUN:
    case NGLLIGHT_SUN:
    case NGLLIGHT_SPECIAL:
        return (nglPointLightInfo*)Node->NodeData;
    case NGLLIGHT_DIRECTIONAL: {
        __m128* NodeData = (__m128*)Node->NodeData;
        Out->Pos.v = _mm_sub_ps(Pos->v, _mm_mul_ps(NodeData[0], _mm_set1_ps(1.0e12f)));
        Out->Far = 1.0e24f;
        Out->Near = 1.0e24f;
        Out->Color.v = NodeData[1];
        return Out;
    }
    case NGLLIGHT_PROJECTED_SPOT:
    case NGLLIGHT_PROJECTED_PARALLEL:
    default:
        if (Node->Type != NGLLIGHT_PROJECTED_SPOT && Node->Type != NGLLIGHT_PROJECTED_PARALLEL)
            _tlAssert("src/ngl_lighting.cpp", 429, "false", "Unsupported light type.");
        return NULL;
    }
}

// ============================================================================
// nglGetLightAsDirLight - ea: 0x845D20
// ============================================================================
nglDirLightInfo* nglGetLightAsDirLight(nglDirLightInfo* Out, nglLightNode* Node,
                                       const math::Position3* Pos) {
    switch (Node->Type) {
    case NGLLIGHT_POINT:
    case NGLLIGHT_POINTGUN:
    case NGLLIGHT_SUN:
    case NGLLIGHT_SPECIAL:
        nglGetFakePointLight(Out, (nglPointLightInfo*)Node->NodeData, Pos);
        return Out;
    case NGLLIGHT_DIRECTIONAL:
        return (nglDirLightInfo*)Node->NodeData;
    case NGLLIGHT_PROJECTED_SPOT:
    case NGLLIGHT_PROJECTED_PARALLEL:
    case NGLLIGHT_USER_FIRST:
    default:
        _tlAssert("src/ngl_lighting.cpp", 467, "false", "Unsupported light type.");
        return NULL;
    }
}

// ============================================================================
// nglGetSinglePointLight - ea: 0x845DA0
// ============================================================================
nglPointLightInfo* nglGetSinglePointLight(nglPointLightInfo* Out, const math::Position3* Pos) {
    nglLightNode* LocalNext = nglSendLightContext->Head.LocalNext;
    if (LocalNext != (nglLightNode*)nglSendLightContext)
        return nglGetLightAsPointLight(Out, LocalNext, Pos);
    Out->Color.v = _mm_setzero_ps();
    Out->Near = 0.0f;
    Out->Far = 0.0f;
    Out->Pos.v = _mm_setzero_ps();
    return Out;
}

// ============================================================================
// nglGetSingleDirLight - ea: 0x845E20
// ============================================================================
nglDirLightInfo* nglGetSingleDirLight(nglDirLightInfo* Out, const math::Position3* Pos) {
    nglLightNode* LocalNext = nglSendLightContext->Head.LocalNext;
    if (LocalNext == (nglLightNode*)nglSendLightContext) {
        Out->Color.v = _mm_setzero_ps();
        Out->Dir.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        return Out;
    }
    nglDirLightInfo* result = nglGetLightAsDirLight(Out, LocalNext, Pos);
    if (result == NULL) {
        Out->Color.v = _mm_setzero_ps();
        Out->Dir.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        return Out;
    }
    return result;
}

// ============================================================================
// nglMeshNode::GetLToWNoScale / GetWToLNoScale (inline COMDATs,
// render_xboxr:cdDynamicDecalShader.o) - reconstructed for nglGetDirLightMatrix.
// ============================================================================
static math::Mat43* nglMeshNode_GetLToWNoScale(const nglMeshNode* This, math::Mat43* result) {
    if ((This->MeshParams->Flags & 2) != 0) {
        __m128 Scale = This->MeshParams->Scale.v;
        __m128 rcp = _mm_rcp_ps(Scale);
        __m128 inv = _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(rcp, Scale)), rcp);
        result->x.v = _mm_mul_ps(This->LocalToWorld.x.v, _mm_shuffle_ps(inv, inv, 0));
        result->y.v = _mm_mul_ps(This->LocalToWorld.y.v, _mm_shuffle_ps(inv, inv, 85));
        result->z.v = _mm_mul_ps(This->LocalToWorld.z.v, _mm_shuffle_ps(inv, inv, 170));
        result->w.v = This->LocalToWorld.w.v;
    } else {
        *result = This->LocalToWorld;
    }
    return result;
}

static math::Mat43* nglMeshNode_GetWToLNoScale(const nglMeshNode* This, math::Mat43* result) {
    math::Mat43 LToWNoScale;
    nglMeshNode_GetLToWNoScale(This, &LToWNoScale);
    // Transpose the 3x3 rotation (rows x/y/z -> columns) and negate translation.
    __m128 y = LToWNoScale.y.v;
    __m128 z = LToWNoScale.z.v;
    __m128 w = LToWNoScale.w.v;
    __m128 v7 = _mm_shuffle_ps(LToWNoScale.x.v, y, 0x44);
    __m128 v8 = _mm_shuffle_ps(v7, z, 0xDD);
    __m128 v16 = _mm_shuffle_ps(_mm_shuffle_ps(LToWNoScale.x.v, y, 0xEE), z, 0xA8);
    __m128 v9 = _mm_shuffle_ps(v7, z, 0x88);
    result->x.v = _mm_shuffle_ps(v9, _mm_setzero_ps(), 0xE4);
    result->y.v = v8;
    result->z.v = v16;
    __m128 v12 = _mm_mul_ps(_mm_shuffle_ps(w, w, 170), v16);
    result->w.v = _mm_xor_ps(
        _mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(w, w, 0), v9),
                       _mm_mul_ps(_mm_shuffle_ps(w, w, 85), v8)),
            v12));
    return result;
}

// ============================================================================
// nglGetDirLightMatrix - ea: 0x845ED0
// ============================================================================
void nglGetDirLightMatrix(nglMeshNode* MeshNode, math::Mat44* Dir, math::Mat44* Color) {
    math::Vector4 Sphere = MeshNode->Mesh->Sphere;
    math::Position3 Center;
    Center.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Sphere.v, Sphere.v, 0), MeshNode->LocalToWorld.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(Sphere.v, Sphere.v, 85), MeshNode->LocalToWorld.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Sphere.v, Sphere.v, 170), MeshNode->LocalToWorld.z.v),
                   MeshNode->LocalToWorld.w.v));
    nglLightNode* LightNode = nglSendLightContext->Head.LocalNext;
    int v6 = 0;
    if (LightNode != NULL) {
        float* pColor = &Color->z.v.m128_f32[0];
        math::Mat44* pDir = Dir;
        while (LightNode != (nglLightNode*)nglSendLightContext) {
            nglDirLightInfo Local;
            nglDirLightInfo* LightAsDirLight = nglGetLightAsDirLight(&Local, LightNode,
                                                                     &Center);
            if (LightAsDirLight != NULL) {
                __m128 Storage = _mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
                                            LightAsDirLight->Dir.v);
                math::Mat43 WToLNoScale;
                nglMeshNode_GetWToLNoScale(MeshNode, &WToLNoScale);
                __m128 v13 = _mm_add_ps(
                    _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Storage, Storage, 0), WToLNoScale.x.v),
                               _mm_mul_ps(_mm_shuffle_ps(Storage, Storage, 85), WToLNoScale.y.v)),
                    _mm_mul_ps(_mm_shuffle_ps(Storage, Storage, 170), WToLNoScale.z.v));
                pDir->x.v = _mm_shuffle_ps(v13, _mm_setzero_ps(), 0xE4);
                pColor[0] = LightAsDirLight->Color.v.m128_f32[0];
                pColor[4] = LightAsDirLight->Color.v.m128_f32[1];
                pColor[8] = LightAsDirLight->Color.v.m128_f32[2];
                pDir++;
                pColor += 4;
                v6++;
                if (v6 == 4)
                    break;
            }
            LightNode = LightNode->LocalNext;
            if (LightNode == NULL)
                break;
        }
    }
    if (v6 < 4) {
        float* v14 = &Dir->x.v.m128_f32[4 * v6];
        float* v15 = &Color->z.v.m128_f32[v6];
        for (int i = 4 - v6; i != 0; --i) {
            v14[0] = 0.0f;
            v14[1] = 0.0f;
            v14[2] = 0.0f;
            v14[3] = 0.0f;
            v15[-8] = 0.0f;
            v15[-4] = 0.0f;
            v15[0] = 0.0f;
            v14 += 4;
            ++v15;
        }
    }
    Color->w = nglSendLightContext->Ambient;
}

// ============================================================================
// nglCreateLightContext - ea: 0x8460C0
// ============================================================================
nglLightContext* nglCreateLightContext() {
    nglLightContext* result = (nglLightContext*)nglListAlloc(0x70, 0x10);
    if (result != NULL) {
        for (int i = 0; i < 8; ++i) {
            result->ProjHead.Next[i] = &result->ProjHead;
            result->Head.Next[i] = &result->Head;
        }
        result->Ambient.v = _mm_set1_ps(1.0f);
        nglBuildLightContext = result;
    }
    return result;
}

// ============================================================================
// nglListAddLight - ea: 0x846160
// ============================================================================
void nglListAddLight(nglLightType Type, void* NodeData, int LightCat) {
    nglLightNode* v3 = (nglLightNode*)nglListAlloc(0x30, 0x10);
    if (v3 != NULL) {
        v3->Type = Type;
        v3->NodeData = NodeData;
        v3->LightCat = LightCat;
        unsigned int Cat = (unsigned int)LightCat;
        for (int i = 0; i < 8; ++i) {
            unsigned int Bit = 0x1000000u << i;
            if ((Cat & Bit) != 0) {
                v3->Next[i] = nglBuildLightContext->Head.Next[i];
                nglBuildLightContext->Head.Next[i] = v3;
            }
        }
    }
}

// ============================================================================
// nglListAddProjLightNode - ea: 0x846A10
// ============================================================================
void nglListAddProjLightNode(nglLightType Type, void* NodeData, int LightCat) {
    nglLightNode* v3 = (nglLightNode*)nglListAlloc(0x30, 0x10);
    if (v3 != NULL) {
        v3->Type = Type;
        v3->NodeData = NodeData;
        v3->LightCat = LightCat;
        unsigned int Cat = (unsigned int)LightCat;
        for (int i = 0; i < 8; ++i) {
            unsigned int Bit = 0x1000000u << i;
            if ((Cat & Bit) != 0) {
                v3->Next[i] = nglBuildLightContext->ProjHead.Next[i];
                nglBuildLightContext->ProjHead.Next[i] = v3;
            }
        }
    }
}

// ============================================================================
// nglListAddDirLight - ea: 0x846270
// ============================================================================
void nglListAddDirLight(unsigned int LightCat, const math::Dir3* Dir, const math::Vector4* Color) {
    nglDirLightInfo* v3 = (nglDirLightInfo*)nglListAlloc(0x20, 0x10);
    if (v3 != NULL) {
        v3->Dir = *Dir;
        v3->Color.v = _mm_shuffle_ps(Color->v, _mm_shuffle_ps(_mm_set1_ps(1.0f), Color->v, 160), 52);
        nglListAddLight(NGLLIGHT_DIRECTIONAL, v3, (int)LightCat);
        if (nglSyncDebug.DumpSceneFile != 0)
            nglSceneDumpDirLight(LightCat, Dir, Color);
    }
}

// ============================================================================
// nglListAddPointLight / nglListAddPointLightGun
// ============================================================================
static void nglListAddPointLightCommon(nglLightType Type, unsigned int LightCat,
                                       const math::Position3* Pos, float Near, float Far,
                                       const math::Vector4* Color, bool isVertexPointLight) {
    if (nglIsSphereVisible((const nglFrustum*)&nglBuildScene->ClipPlanes,
                           (const math::Vector4*)Pos, Far)) {
        __m128* v6 = (__m128*)nglListAlloc(0x30, 0x10);
        if (v6 != NULL) {
            v6[0] = Pos->v;
            v6[1] = _mm_shuffle_ps(Color->v, _mm_shuffle_ps(_mm_set1_ps(1.0f), Color->v, 160), 52);
            v6[2].m128_f32[0] = Near;
            v6[2].m128_f32[1] = Far;
            v6[2].m128_f32[2] = isVertexPointLight ? 1.0f : 0.0f;
            nglListAddLight(Type, v6, (int)LightCat);
            if (nglSyncDebug.DumpSceneFile != 0)
                nglSceneDumpPointLight(Type, LightCat, Pos, Near, Far, Color);
        }
    }
}

// ea: 0x846310
void nglListAddPointLight(unsigned int LightCat, const math::Position3* Pos,
                          float Near, float Far, const math::Vector4* Color,
                          bool isVertexPointLight) {
    nglListAddPointLightCommon(NGLLIGHT_POINT, LightCat, Pos, Near, Far, Color,
                               isVertexPointLight);
}

// ea: 0x8463F0
void nglListAddPointLightGun(unsigned int LightCat, const math::Position3* Pos,
                             float Near, float Far, const math::Vector4* Color,
                             bool isVertexPointLight) {
    nglListAddPointLightCommon(NGLLIGHT_POINTGUN, LightCat, Pos, Near, Far, Color,
                               isVertexPointLight);
}

// ============================================================================
// nglDetermineProjLights - ea: 0x8464D0
// ============================================================================
void nglDetermineProjLights(nglMeshNode* MeshNode) {
    nglLightContext* LightContext;
    if ((MeshNode->ShaderParams.Array[nglLightContextParamID >> 5]
         & (1u << (nglLightContextParamID & 0x1F))) != 0)
        LightContext = nglLightContextParam.Value;
    else
        LightContext = nglBuildScene->LightContext;
    nglSendLightContext = LightContext;
    LightContext->ProjHead.LocalNext = &LightContext->ProjHead;
    nglMesh* Mesh = MeshNode->Mesh;
    int v3 = (Mesh->Flags >> 24) - 1;
    if ((Mesh->Flags >> 24) != 0) {
        nglLightNode* v5 = nglSendLightContext->ProjHead.Next[v3];
        math::Position3 Center;
        Center.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 0),
                                  MeshNode->LocalToWorld.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 85),
                                  MeshNode->LocalToWorld.y.v)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 170),
                                  MeshNode->LocalToWorld.z.v),
                       MeshNode->LocalToWorld.w.v));
        if (v5 != NULL) {
            while (v5 != (nglLightNode*)((char*)nglSendLightContext + 0x30)) {
                if (v5->Type == NGLLIGHT_PROJECTED_DIRECTIONAL) {
                    __m128* NodeData = (__m128*)v5->NodeData;
                    __m128 one = _mm_set1_ps(1.0f);
                    __m128 v8 = _mm_shuffle_ps(
                        _mm_shuffle_ps(Center.v, one, 160), one, 52);
                    float negRadius = -Mesh->Sphere.v.m128_f32[3];
                    bool visible = true;
                    // Frustum plane distance tests (planes at NodeData+16..21).
                    for (int p = 0; p < 6; ++p) {
                        __m128 v9 = _mm_mul_ps(NodeData[16 + p], v8);
                        if (negRadius > v9.m128_f32[0] + (v9.m128_f32[1] + v9.m128_f32[2]
                                                          + v9.m128_f32[3])) {
                            visible = false;
                            break;
                        }
                    }
                    if (visible) {
                        v5->LocalNext = nglSendLightContext->ProjHead.LocalNext;
                        nglSendLightContext->ProjHead.LocalNext = v5;
                    }
                } else {
                    _tlAssert("src/ngl_lighting.cpp", 203, "false", "Invalid light type.");
                }
                v5 = v5->Next[v3];
                if (v5 == NULL)
                    break;
            }
        }
    }
}

// ============================================================================
// nglCheckAvailableLights - ea: 0x8467A0
// ============================================================================
unsigned int nglCheckAvailableLights(nglMeshNode* MeshNode) {
    nglLightContext* LightContext;
    if ((MeshNode->ShaderParams.Array[nglLightContextParamID >> 5]
         & (1u << (nglLightContextParamID & 0x1F))) != 0)
        LightContext = nglLightContextParam.Value;
    else
        LightContext = nglBuildScene->LightContext;
    nglSendLightContext = LightContext;
    nglMesh* Mesh = MeshNode->Mesh;
    int v4 = (Mesh->Flags >> 24) - 1;
    if ((Mesh->Flags >> 24) == 0)
        return 0;
    int v1 = 0;
    for (nglLightNode* i = LightContext->Head.Next[v4]; i != NULL; ++v1) {
        if (i == (nglLightNode*)LightContext)
            break;
        i = i->Next[v4];
    }
    return v1;
}

// ============================================================================
// nglDetermineLights - ea: 0x846830
// ============================================================================
void nglDetermineLights(nglMeshNode* MeshNode) {
    nglLightContext* LightContext;
    if ((MeshNode->ShaderParams.Array[nglLightContextParamID >> 5]
         & (1u << (nglLightContextParamID & 0x1F))) != 0)
        LightContext = nglLightContextParam.Value;
    else
        LightContext = nglBuildScene->LightContext;
    nglSendLightContext = LightContext;
    LightContext->Head.LocalNext = &LightContext->Head;
    nglMesh* Mesh = MeshNode->Mesh;
    int v3 = (Mesh->Flags >> 24) - 1;
    if ((Mesh->Flags >> 24) != 0) {
        if (v3 >= 8
            && _tlAssert("src/ngl_lighting.cpp", 316, "MeshCat < 8",
                         "Invalid mesh lighting category."))
            __debugbreak();
        nglLightNode* v6 = nglSendLightContext->Head.Next[v3];
        math::Position3 Center;
        Center.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 0),
                                  MeshNode->LocalToWorld.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 85),
                                  MeshNode->LocalToWorld.y.v)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(Mesh->Sphere.v, Mesh->Sphere.v, 170),
                                  MeshNode->LocalToWorld.z.v),
                       MeshNode->LocalToWorld.w.v));
        if (v6 != (nglLightNode*)nglSendLightContext) {
            while (1) {
                switch (v6->Type) {
                case NGLLIGHT_POINT: {
                    __m128* NodeData = (__m128*)v6->NodeData;
                    if (NodeData != NULL) {
                        __m128 v9 = _mm_sub_ps(NodeData[0], Center.v);
                        __m128 v10 = _mm_mul_ps(v9, v9);
                        float Far = NodeData[2].m128_f32[1] + Mesh->Sphere.v.m128_f32[3];
                        if (Far * Far >= v10.m128_f32[0] + (v10.m128_f32[1] + v10.m128_f32[2]))
                            goto add;
                    }
                    break;
                }
                case NGLLIGHT_DIRECTIONAL:
                case NGLLIGHT_PROJECTED_SPOT:
                case NGLLIGHT_PROJECTED_PARALLEL:
                case NGLLIGHT_POINTGUN:
                case NGLLIGHT_USER_FIRST:
                add:
                    v6->LocalNext = nglSendLightContext->Head.LocalNext;
                    nglSendLightContext->Head.LocalNext = v6;
                    break;
                default:
                    _tlAssert("src/ngl_lighting.cpp", 352, "false", "Invalid light type.");
                    break;
                }
                v6 = v6->Next[v3];
                if (v6 == (nglLightNode*)nglSendLightContext)
                    break;
            }
        }
    }
}

// ============================================================================
// nglListAddProjectorLight - ea: 0x846B20
// ============================================================================
void nglListAddProjectorLight(void* NodeData, unsigned int LightCat) {
    nglListAddProjLightNode(NGLLIGHT_PROJECTED_SPOT, NodeData, (int)LightCat);
}

// ============================================================================
// nglListAddDirProjectorLight - ea: 0x846B40
// ============================================================================
void nglListAddDirProjectorLight(unsigned int LightCat, const math::Mat43* PO,
                                 const math::Position3* _Scale, unsigned int BlendMode,
                                 nglTexture* Tex) {
    struct DirProjNode {
        math::Mat43   m;          // +0x00 (BuildFrustum input)
        math::Mat43   m2;         // +0x40
        unsigned int  BlendMode;  // +0x80
        nglTexture*   Tex;        // +0x84
        uint8_t       _pad88[8];  // +0x88
        math::Mat43   m3;         // +0x90
        uint8_t       _padD0[16]; // +0xD0
        nglFrustum    Frustum;    // +0xE0
    };
    static_assert(sizeof(DirProjNode) == 0x140, "DirProjNode size mismatch");
    char* v5 = (char*)nglListAlloc(0x140, 0x10);
    if (v5 != NULL) {
        DirProjNode* node = (DirProjNode*)v5;
        node->BlendMode = BlendMode;
        if (Tex != NULL)
            node->Tex = Tex;
        math::Dir3 xaxis;
        math::Dir3 yaxis;
        xaxis.v = PO->y.v;
        yaxis.v = PO->w.v;
        math::Dir3 zaxis;
        zaxis.v = _Scale->v;
        __m128 scale_4 = _mm_setr_ps(PO->x.v.m128_f32[0], PO->x.v.m128_f32[1],
                                     PO->x.v.m128_f32[2], PO->x.v.m128_f32[3]);
        __m128 UVTranslationMtx_4 = PO->z.v;
        // Scale matrix (m3): x = scale, y = PO->w, z = PO->x, w = PO->y.
        node->m3.x.v = zaxis.v;
        node->m3.y.v = yaxis.v;
        node->m3.z.v = scale_4;
        node->m3.w.v = xaxis.v;
        __m128 v9 = _mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)), yaxis.v);
        __m128 v10 = _mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
                                _mm_setr_ps(0.5f, 0.5f, 0.5f, 0.5f));
        __m128 v11 = _mm_shuffle_ps(scale_4, xaxis.v, 0x44);
        __m128 v12 = _mm_shuffle_ps(v11, UVTranslationMtx_4, 0x88);
        __m128 v26 = _mm_shuffle_ps(v11, UVTranslationMtx_4, 0xDD);
        __m128 v33 = _mm_shuffle_ps(_mm_shuffle_ps(scale_4, xaxis.v, 0xEE),
                                    UVTranslationMtx_4, 0xA8);
        node->m.x.v = v12;
        node->m.y.v = v26;
        node->m.z.v = v33;
        node->m.w.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v9, v9, 0), v12),
                       _mm_mul_ps(_mm_shuffle_ps(v9, v9, 85), v26)),
            _mm_mul_ps(_mm_shuffle_ps(v9, v9, 170), v33));
        // Normalize the light matrix by the scale (rcp of _Scale).
        __m128 v14 = _mm_mul_ps(
            _mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(_mm_rcp_ps(zaxis.v), zaxis.v)),
            _mm_rcp_ps(zaxis.v));
        node->m.x.v = _mm_mul_ps(node->m.x.v, v14);
        node->m.y.v = _mm_mul_ps(node->m.y.v, v14);
        node->m.z.v = _mm_mul_ps(node->m.z.v, v14);
        node->m.w.v = _mm_mul_ps(node->m.w.v, v14);
        // Second matrix (m2) = axis-scaled world axes.
        __m128 v22 = _mm_mul_ps(scale_4, _mm_shuffle_ps(zaxis.v, zaxis.v, 0));
        __m128 v23 = _mm_mul_ps(xaxis.v, _mm_shuffle_ps(zaxis.v, zaxis.v, 85));
        __m128 v24 = _mm_mul_ps(UVTranslationMtx_4, _mm_shuffle_ps(zaxis.v, zaxis.v, 170));
        __m128 v25 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v10, v10, 0), v22),
                       _mm_mul_ps(_mm_shuffle_ps(v10, v10, 85), v23)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v10, v10, 170), v24),
                       _mm_mul_ps(_mm_shuffle_ps(v10, v10, 255),
                                  _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f))));
        __m128 XAxis = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
        __m128 YAxis = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        __m128 ZAxis = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
        node->m2.x.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v22, v22, 0), XAxis),
                       _mm_mul_ps(_mm_shuffle_ps(v22, v22, 85), YAxis)),
            _mm_mul_ps(_mm_shuffle_ps(v22, v22, 170), ZAxis));
        node->m2.y.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v23, v23, 0), XAxis),
                       _mm_mul_ps(_mm_shuffle_ps(v23, v23, 85), YAxis)),
            _mm_mul_ps(_mm_shuffle_ps(v23, v23, 170), ZAxis));
        node->m2.z.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v24, v24, 0), XAxis),
                       _mm_mul_ps(_mm_shuffle_ps(v24, v24, 85), YAxis)),
            _mm_mul_ps(_mm_shuffle_ps(v24, v24, 170), ZAxis));
        node->m2.w.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v25, v25, 0), XAxis),
                       _mm_mul_ps(_mm_shuffle_ps(v25, v25, 85), YAxis)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v25, v25, 170), ZAxis), yaxis.v));
        nglBuildFrustum(&node->Frustum, &node->m);
        nglListAddProjLightNode(NGLLIGHT_PROJECTED_DIRECTIONAL, v5, (int)LightCat);
    }
}

// ============================================================================
// nglListAddDicLight - ea: 0x8470E0
// ============================================================================
void nglListAddDicLight(unsigned int LightCat, nglDicLightInfo* dic) {
    nglDicLightInfo* v2 = (nglDicLightInfo*)nglListAlloc(0xB0, 0x10);
    if (v2 != NULL) {
        memcpy(v2, dic, sizeof(nglDicLightInfo));
        nglListAddLight(NGLLIGHT_USER_FIRST, v2, (int)LightCat);
    }
}

// ============================================================================
// nglIsSphereVisible (inline COMDAT) - ea: 0x8453B0
// ============================================================================
bool nglIsSphereVisible(const nglFrustum* Frustum, const math::Vector4* Center, float Radius) {
    float negRadius = -Radius;
    for (int i = 0; i < 6; ++i) {
        __m128 v = _mm_mul_ps(Frustum->Planes[i].v,
                              _mm_shuffle_ps(Center->v, _mm_set1_ps(1.0f), 0x34));
        if (negRadius > v.m128_f32[0] + (v.m128_f32[1] + v.m128_f32[2] + v.m128_f32[3]))
            return false;
    }
    return true;
}

// ============================================================================
// nglBuildFrustum (inline COMDAT) - ea: 0x845580
// ============================================================================
void nglBuildFrustum(nglFrustum* frustum, const math::Mat43* m) {
    __m128 v2 = _mm_sub_ps(m->w.v, _mm_setzero_ps());
    __m128 v3 = m->x.v;
    __m128 v4 = m->y.v;
    __m128 v29 = _mm_shuffle_ps(v3, v3, 9);
    __m128 v5 = _mm_shuffle_ps(v4, v4, 9);
    __m128 Bottom = _mm_shuffle_ps(v3, v3, 18);
    __m128 v6 = _mm_shuffle_ps(v4, v4, 18);
    __m128 v7 = _mm_sub_ps(_mm_mul_ps(v29, v6), _mm_mul_ps(Bottom, v5));
    __m128 v8 = _mm_mul_ps(v7, v7);
    __m128 v9 = m->z.v;
    __m128 v10 = _mm_shuffle_ps(v9, v9, 9);
    __m128 v11 = _mm_shuffle_ps(v9, v9, 18);
    __m128 v12 = _mm_sub_ps(_mm_mul_ps(v5, v11), _mm_mul_ps(v6, v10));
    float l1 = sqrtf(v8.m128_f32[0] + (v8.m128_f32[1] + v8.m128_f32[2]));
    __m128 v13 = _mm_div_ps(v7, _mm_set1_ps(l1));
    __m128 v14 = _mm_mul_ps(v12, v12);
    float l2 = sqrtf(v14.m128_f32[0] + (v14.m128_f32[1] + v14.m128_f32[2]));
    __m128 v15 = _mm_div_ps(v12, _mm_set1_ps(l2));
    __m128 v16 = _mm_sub_ps(_mm_mul_ps(v10, Bottom), _mm_mul_ps(v11, v29));
    __m128 v17 = _mm_mul_ps(v16, v16);
    float l3 = sqrtf(v17.m128_f32[0] + (v17.m128_f32[1] + v17.m128_f32[2]));
    __m128 v18 = _mm_div_ps(v16, _mm_set1_ps(l3));
    __m128 WAxis = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    __m128 v19 = _mm_mul_ps(v13, v2);
    float d1 = v19.m128_f32[0] + (v19.m128_f32[1] + v19.m128_f32[2]);
    __m128 Left = _mm_sub_ps(_mm_shuffle_ps(v13, _mm_setzero_ps(), 0xE4),
                             _mm_mul_ps(WAxis, _mm_set1_ps(d1)));
    __m128 v20 = _mm_mul_ps(v13, m->z.v);
    float d2 = v20.m128_f32[0] + (v20.m128_f32[1] + v20.m128_f32[2]);
    __m128 rx = _mm_add_ps(_mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)), Left),
                           _mm_mul_ps(WAxis, _mm_set1_ps(d2)));
    __m128 v21 = _mm_mul_ps(v15, v2);
    float d3 = v21.m128_f32[0] + (v21.m128_f32[1] + v21.m128_f32[2]);
    __m128 v22 = _mm_mul_ps(v15, m->x.v);
    __m128 v23 = _mm_sub_ps(_mm_shuffle_ps(v15, _mm_setzero_ps(), 0xE4),
                            _mm_mul_ps(WAxis, _mm_set1_ps(d3)));
    float d4 = v22.m128_f32[0] + (v22.m128_f32[1] + v22.m128_f32[2]);
    __m128 Top = _mm_add_ps(_mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)), v23),
                            _mm_mul_ps(WAxis, _mm_set1_ps(d4)));
    __m128 v24 = _mm_mul_ps(v18, v2);
    float d5 = v24.m128_f32[0] + (v24.m128_f32[1] + v24.m128_f32[2]);
    __m128 v27 = _mm_mul_ps(v18, m->y.v);
    __m128 v28 = _mm_sub_ps(_mm_shuffle_ps(v18, _mm_setzero_ps(), 0xE4),
                            _mm_mul_ps(WAxis, _mm_set1_ps(d5)));
    float d6 = v27.m128_f32[0] + (v27.m128_f32[1] + v27.m128_f32[2]);
    frustum->Planes[4].v = Left;
    frustum->Planes[5].v = rx;
    frustum->Planes[2].v = v23;
    frustum->Planes[3].v = Top;
    frustum->Planes[0].v = _mm_add_ps(
        _mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)), v28),
        _mm_mul_ps(WAxis, _mm_set1_ps(d6)));
    frustum->Planes[1].v = v28;
}
