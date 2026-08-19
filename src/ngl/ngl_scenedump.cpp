// ============================================================================
// ngl_scenedump.cpp - scene file dump helpers (7 funcs).
// Source: src/ngl_scenedump.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_scenedump.o).
// ============================================================================

#include "ngl/nglDebug.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "core/math_types.h"

enum nglLightType {
    NGLLIGHT_POINT = 0x0,
    NGLLIGHT_DIRECTIONAL = 0x1,
    NGLLIGHT_PROJECTED_DIRECTIONAL = 0x2,
    NGLLIGHT_PROJECTED_SPOT = 0x3,
    NGLLIGHT_PROJECTED_PARALLEL = 0x4,
    NGLLIGHT_POINTGUN = 0x5,
    NGLLIGHT_SUN = 0x6,
    NGLLIGHT_SPECIAL = 0x7,
    NGLLIGHT_USER_FIRST = 0x8,
};

// ============================================================================
// Globals (data)
// ============================================================================
void* nglSceneDumpFile = NULL;

// ============================================================================
// nglSceneDumpStart - ea: 0x852980
// ============================================================================
void nglSceneDumpStart() {
    if (nglSyncDebug.DumpSceneFile != 0) {
        nglSceneDumpFile = nglHostOpen("scenedump.scene");
        nglHostPrintf(nglSceneDumpFile, "//\n");
        nglHostPrintf(nglSceneDumpFile, "// Midnight scene file dump.\n");
        nglHostPrintf(nglSceneDumpFile, "//\n");
        nglHostPrintf(nglSceneDumpFile, "\n");
        nglHostPrintf(nglSceneDumpFile, "\n");
    }
}

// ============================================================================
// nglSceneDumpEnd - ea: 0x8529F0
// ============================================================================
void nglSceneDumpEnd() {
    if (nglSyncDebug.DumpSceneFile != 0) {
        nglHostPrintf(nglSceneDumpFile, "\n");
        nglHostPrintf(nglSceneDumpFile, "ENDSCENE\n");
        nglHostClose(nglSceneDumpFile);
    }
}

// ============================================================================
// nglSceneDumpCamera - ea: 0x852A30
// ============================================================================
void nglSceneDumpCamera(const math::Mat43& WorldToView) {
    nglHostPrintf(nglSceneDumpFile, "CAMERA\n");
    nglHostPrintf(nglSceneDumpFile, "  ROW1 %f %f %f %f\n",
                  WorldToView.x.v.m128_f32[0], WorldToView.x.v.m128_f32[1],
                  WorldToView.x.v.m128_f32[2], WorldToView.x.v.m128_f32[3]);
    nglHostPrintf(nglSceneDumpFile, "  ROW2 %f %f %f %f\n",
                  WorldToView.y.v.m128_f32[0], WorldToView.y.v.m128_f32[1],
                  WorldToView.y.v.m128_f32[2], WorldToView.y.v.m128_f32[3]);
    nglHostPrintf(nglSceneDumpFile, "  ROW3 %f %f %f %f\n",
                  WorldToView.z.v.m128_f32[0], WorldToView.z.v.m128_f32[1],
                  WorldToView.z.v.m128_f32[2], WorldToView.z.v.m128_f32[3]);
    nglHostPrintf(nglSceneDumpFile, "  ROW4 %f %f %f %f\n",
                  WorldToView.w.v.m128_f32[0], WorldToView.w.v.m128_f32[1],
                  WorldToView.w.v.m128_f32[2], WorldToView.w.v.m128_f32[3]);
    nglHostPrintf(nglSceneDumpFile, "ENDCAMERA\n");
}

// ============================================================================
// nglSceneDumpMesh - ea: 0x852B30
// ============================================================================
void nglSceneDumpMesh(nglMesh* Mesh, const math::Mat43& LocalToWorld,
                      const nglMeshParams* Params) {
    if ((Mesh->Flags & 0x20000) != 0)
        return;

    nglHostPrintf(nglSceneDumpFile, "\n");
    nglHostPrintf(nglSceneDumpFile, "MODEL %s\n", Mesh->Name->str);
    if (Params != NULL && (Params->Flags & 2) != 0) {
        nglHostPrintf(nglSceneDumpFile, "  SCALE %f %f %f\n",
                      Params->Scale.v.m128_f32[0], Params->Scale.v.m128_f32[1],
                      Params->Scale.v.m128_f32[2]);
    }
    nglHostPrintf(nglSceneDumpFile, "  ROW1 %f %f %f %f\n",
                  LocalToWorld.x.v.m128_f32[0], LocalToWorld.x.v.m128_f32[1],
                  LocalToWorld.x.v.m128_f32[2], LocalToWorld.x.v.m128_f32[3]);
    nglHostPrintf(nglSceneDumpFile, "  ROW2 %f %f %f %f\n",
                  LocalToWorld.y.v.m128_f32[0], LocalToWorld.y.v.m128_f32[1],
                  LocalToWorld.y.v.m128_f32[2], LocalToWorld.y.v.m128_f32[3]);
    nglHostPrintf(nglSceneDumpFile, "  ROW3 %f %f %f %f\n",
                  LocalToWorld.z.v.m128_f32[0], LocalToWorld.z.v.m128_f32[1],
                  LocalToWorld.z.v.m128_f32[2], LocalToWorld.z.v.m128_f32[3]);
    nglHostPrintf(nglSceneDumpFile, "  ROW4 %f %f %f %f\n",
                  LocalToWorld.w.v.m128_f32[0], LocalToWorld.w.v.m128_f32[1],
                  LocalToWorld.w.v.m128_f32[2], LocalToWorld.w.v.m128_f32[3]);
    if (Params != NULL && (Params->Flags & 0x3C) != 0) {
        nglHostPrintf(nglSceneDumpFile, "  NBONES %d\n", Params->NBones);
        unsigned int v4 = 0;
        if (Params->NBones != 0) {
            int v5 = 0;
            do {
                const math::Mat43* b = (const math::Mat43*)((const char*)Params->Bones + v5);
                nglHostPrintf(nglSceneDumpFile,
                              "  BONE %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
                              v4++,
                              b->x.v.m128_f32[0], b->x.v.m128_f32[1], b->x.v.m128_f32[2], b->x.v.m128_f32[3],
                              b->y.v.m128_f32[0], b->y.v.m128_f32[1], b->y.v.m128_f32[2], b->y.v.m128_f32[3],
                              b->z.v.m128_f32[0], b->z.v.m128_f32[1], b->z.v.m128_f32[2], b->z.v.m128_f32[3],
                              b->w.v.m128_f32[0], b->w.v.m128_f32[1], b->w.v.m128_f32[2], b->w.v.m128_f32[3]);
                v5 += 64;
            } while (v4 < Params->NBones);
        }
    }
    nglHostPrintf(nglSceneDumpFile, "ENDMODEL\n");
}

// ============================================================================
// nglSceneDumpQuad - ea: 0x852D80
// ============================================================================
void nglSceneDumpQuad(nglQuad* Quad) {
    nglHostPrintf(nglSceneDumpFile, "\n");
    nglHostPrintf(nglSceneDumpFile, "QUAD\n");
    nglTexture* Tex = Quad->Tex;
    const char* TexName;
    if (Tex != NULL)
        TexName = Tex->FileName->str;
    else
        TexName = "[none]";
    nglHostPrintf(nglSceneDumpFile, "  TEXTURE %s\n", TexName);
    nglHostPrintf(nglSceneDumpFile, "  BLEND %d\n", Quad->BlendMode);
    nglHostPrintf(nglSceneDumpFile, "  MAPFLAGS 0x%x\n", Quad->MapFlags);
    nglHostPrintf(nglSceneDumpFile, "  Z %f\n", Quad->Z);
    for (int i = 4; i != 0; --i) {
        const nglQuadVertex* v = &Quad->Verts[4 - i];
        nglHostPrintf(nglSceneDumpFile, "  VERT %f %f 0x%08X %f %f\n",
                      v->X, v->Y, v->Color, v->U, v->V);
    }
    nglHostPrintf(nglSceneDumpFile, "ENDQUAD\n");
}

// ============================================================================
// nglSceneDumpDirLight - ea: 0x852E80
// ============================================================================
void nglSceneDumpDirLight(unsigned int LightCat, const math::Dir3& Dir, const math::Vector4& Color) {
    nglHostPrintf(nglSceneDumpFile, "\n");
    nglHostPrintf(nglSceneDumpFile, "DIRLIGHT\n");
    nglHostPrintf(nglSceneDumpFile, "  LIGHTCAT 0x%8X\n", LightCat);
    nglHostPrintf(nglSceneDumpFile, "  DIR %f %f %f\n",
                  Dir.v.m128_f32[0], Dir.v.m128_f32[1], Dir.v.m128_f32[2]);
    nglHostPrintf(nglSceneDumpFile, "  COLOR %f %f %f %f\n",
                  Color.v.m128_f32[0], Color.v.m128_f32[1],
                  Color.v.m128_f32[2], Color.v.m128_f32[3]);
    nglHostPrintf(nglSceneDumpFile, "ENDLIGHT\n");
}

// ============================================================================
// nglSceneDumpPointLight - ea: 0x852F30
// ============================================================================
void nglSceneDumpPointLight(nglLightType LightType, unsigned int LightCat,
                            const math::Position3& Pos, float Near, float Far,
                            const math::Vector4& Color) {
    (void)LightType;
    nglHostPrintf(nglSceneDumpFile, "\n");
    nglHostPrintf(nglSceneDumpFile, "POINTLIGHT\n");
    nglHostPrintf(nglSceneDumpFile, "  LIGHTCAT 0x%8X\n", LightCat);
    nglHostPrintf(nglSceneDumpFile, "  POS %f %f %f\n",
                  Pos.v.m128_f32[0], Pos.v.m128_f32[1], Pos.v.m128_f32[2]);
    nglHostPrintf(nglSceneDumpFile, "  RANGE %f %f\n", Near, Far);
    nglHostPrintf(nglSceneDumpFile, "  COLOR %f %f %f %f\n",
                  Color.v.m128_f32[0], Color.v.m128_f32[1],
                  Color.v.m128_f32[2], Color.v.m128_f32[3]);
    nglHostPrintf(nglSceneDumpFile, "ENDLIGHT\n");
}
