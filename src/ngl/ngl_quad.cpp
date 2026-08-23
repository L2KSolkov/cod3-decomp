// ============================================================================
// ngl_quad.cpp â€” quad helpers (17 funcs).
// Source: source/ngl_quad.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_quad.o).
// ============================================================================

#include "ngl_dx_quad.h"

#include <math.h>
#include <new>
#include <string.h>

// ============================================================================
// nglInitQuad â€” ea: 0x83F6E0
// ============================================================================
void nglInitQuad(nglQuad* Quad) {
    memset(Quad, 0, sizeof(nglQuad));
    Quad->Verts[0].Color = 0xFFFFFFFF;
    Quad->Verts[1].Color = 0xFFFFFFFF;
    Quad->Verts[2].Color = 0xFFFFFFFF;
    Quad->Verts[3].Color = 0xFFFFFFFF;
    Quad->Verts[0].U = 0.0f;
    Quad->Verts[1].U = 1.0f;
    Quad->Verts[2].U = 0.0f;
    Quad->Verts[3].U = 1.0f;
    Quad->Verts[0].V = 0.0f;
    Quad->Verts[1].V = 0.0f;
    Quad->Verts[2].V = 1.0f;
    Quad->Verts[3].V = 1.0f;
    Quad->MapFlags = 193;
    Quad->BlendMode = 1691321856u;
}

// ============================================================================
// nglSetQuadTex â€” ea: 0x83F750
// ============================================================================
void nglSetQuadTex(nglQuad* Quad, nglTexture* Tex) {
    Quad->Tex = Tex;
}

// ============================================================================
// nglSetQuadMapFlags â€” ea: 0x83F760
// ============================================================================
void nglSetQuadMapFlags(nglQuad* Quad, unsigned int MapFlags) {
    Quad->MapFlags = MapFlags;
}

// ============================================================================
// nglSetQuadBlend â€” ea: 0x83F770
// ============================================================================
void nglSetQuadBlend(nglQuad* Quad, unsigned int Blend) {
    Quad->BlendMode = Blend;
}

// ============================================================================
// nglSetQuadUV â€” ea: 0x83F780
// ============================================================================
void nglSetQuadUV(nglQuad* Quad, float u1, float v1, float u2, float v2) {
    Quad->Verts[0].U = u1;
    Quad->Verts[2].U = u1;
    Quad->Verts[0].V = v1;
    Quad->Verts[1].U = u2;
    Quad->Verts[1].V = v1;
    Quad->Verts[2].V = v2;
    Quad->Verts[3].U = u2;
    Quad->Verts[3].V = v2;
}

// ============================================================================
// nglSetQuadColor â€” ea: 0x83F7D0
// ============================================================================
void nglSetQuadColor(nglQuad* Quad, unsigned int c) {
    Quad->Verts[0].Color = c;
    Quad->Verts[1].Color = c;
    Quad->Verts[2].Color = c;
    Quad->Verts[3].Color = c;
}

// ============================================================================
// nglSetQuadRect â€” ea: 0x83F7F0
// ============================================================================
void nglSetQuadRect(nglQuad* Quad, float x1, float y1, float x2, float y2) {
    Quad->Verts[0].X = x1;
    Quad->Verts[2].X = x1;
    Quad->Verts[0].Y = y1;
    Quad->Verts[1].X = x2;
    Quad->Verts[1].Y = y1;
    Quad->Verts[2].Y = y2;
    Quad->Verts[3].X = x2;
    Quad->Verts[3].Y = y2;
}

// ============================================================================
// nglSetQuadZ â€” ea: 0x83F840
// ============================================================================
void nglSetQuadZ(nglQuad* Quad, float z) {
    Quad->Z = z;
}

// ============================================================================
// nglSetQuadVPos â€” ea: 0x83F860
// ============================================================================
void nglSetQuadVPos(nglQuad* Quad, int VertIdx, float x, float y) {
    Quad->Verts[VertIdx].X = x;
    Quad->Verts[VertIdx].Y = y;
}

// ============================================================================
// nglSetQuadVUV â€” ea: 0x83F890
// ============================================================================
void nglSetQuadVUV(nglQuad* Quad, int VertIdx, float u, float v) {
    Quad->Verts[VertIdx].U = u;
    Quad->Verts[VertIdx].V = v;
}

// ============================================================================
// nglSetQuadVColor â€” ea: 0x83F8C0
// ============================================================================
void nglSetQuadVColor(nglQuad* Quad, int VertIdx, unsigned int Color) {
    Quad->Verts[VertIdx].Color = Color;
}

// ============================================================================
// nglRotateQuad â€” ea: 0x83F8E0
// ============================================================================
void nglRotateQuad(nglQuad* Quad, float cx, float cy, float theta) {
    float s = sinf(theta);
    float c = cosf(theta);
    for (int i = 0; i < 4; ++i) {
        float x = Quad->Verts[i].X - cx;
        float y = Quad->Verts[i].Y - cy;
        Quad->Verts[i].X = ((c * x) - (s * y)) + cx;
        Quad->Verts[i].Y = ((s * x) + (c * y)) + cy;
    }
}

// ============================================================================
// nglScaleQuad â€” ea: 0x83F970
// ============================================================================
void nglScaleQuad(nglQuad* Quad, float cx, float cy, float sx, float sy) {
    Quad->Verts[0].X = ((Quad->Verts[0].X - cx) * sx) + cx;
    Quad->Verts[0].Y = ((Quad->Verts[0].Y - cy) * sy) + cy;
    Quad->Verts[1].X = ((Quad->Verts[1].X - cx) * sx) + cx;
    Quad->Verts[1].Y = ((Quad->Verts[1].Y - cy) * sy) + cy;
    Quad->Verts[2].X = ((Quad->Verts[2].X - cx) * sx) + cx;
    Quad->Verts[2].Y = ((Quad->Verts[2].Y - cy) * sy) + cy;
    Quad->Verts[3].X = ((Quad->Verts[3].X - cx) * sx) + cx;
    Quad->Verts[3].Y = ((Quad->Verts[3].Y - cy) * sy) + cy;
}

// ============================================================================
// nglRotateQuadUV â€” ea: 0x83FA40
// ============================================================================
void nglRotateQuadUV(nglQuad* Quad, float cx, float cy, float theta) {
    float s = sinf(theta);
    float c = cosf(theta);
    for (int i = 0; i < 4; ++i) {
        float u = Quad->Verts[i].U - cx;
        float v = Quad->Verts[i].V - cy;
        Quad->Verts[i].U = ((c * u) - (s * v)) + cx;
        Quad->Verts[i].V = ((s * u) + (c * v)) + cy;
    }
}

// ============================================================================
// nglScaleQuadUV â€” ea: 0x83FAD0
// ============================================================================
void nglScaleQuadUV(nglQuad* Quad, float cx, float cy, float sx, float sy) {
    Quad->Verts[0].U = ((Quad->Verts[0].U - cx) * sx) + cx;
    Quad->Verts[0].V = ((Quad->Verts[0].V - cy) * sy) + cy;
    Quad->Verts[1].U = ((Quad->Verts[1].U - cx) * sx) + cx;
    Quad->Verts[1].V = ((Quad->Verts[1].V - cy) * sy) + cy;
    Quad->Verts[2].U = ((Quad->Verts[2].U - cx) * sx) + cx;
    Quad->Verts[2].V = ((Quad->Verts[2].V - cy) * sy) + cy;
    Quad->Verts[3].U = ((Quad->Verts[3].U - cx) * sx) + cx;
    Quad->Verts[3].V = ((Quad->Verts[3].V - cy) * sy) + cy;
}

// ============================================================================
// nglSetQuadPos â€” ea: 0x83FBA0
// ============================================================================
void nglSetQuadPos(nglQuad* Quad, float x, float y) {
    float Width;
    float Height;
    if (Quad->Tex != NULL) {
        Width = Quad->Tex->Width;
        Height = Quad->Tex->Height;
    } else {
        Width = 50.0f;
        Height = 50.0f;
    }
    Quad->Verts[0].X = x;
    Quad->Verts[0].Y = y;
    Quad->Verts[1].X = Width + x;
    Quad->Verts[1].Y = y;
    Quad->Verts[2].X = x;
    Quad->Verts[2].Y = Height + y;
    Quad->Verts[3].X = Width + x;
    Quad->Verts[3].Y = Height + y;
}

// ============================================================================
// nglListAddQuad â€” ea: 0x83FC00
// ============================================================================
void nglListAddQuad(nglQuad* Quad) {
    if (Quad != NULL) {
        nglQuadNode* v1 = (nglQuadNode*)nglListAlloc(0x6Cu, 0x10u);
        if (v1 != NULL) {
            new (v1) nglQuadNode();
            nglValidateMatrices(nglBuildScene);
            memcpy(&v1->Quad, Quad, 0x60u);
            if ((Quad->BlendMode & 0x20000) != 0) {
                // LODWORD in the reference: the sort key is the raw float
                // bit pattern, not an arithmetic conversion.  Positive
                // IEEE-754 floats order correctly when compared as
                // unsigned ints, which is how nglListAddString stores z
                // via the SortDist arm of the same union.
                v1->SortHash = *(const unsigned int*)&Quad->Z;
                v1->Next = nglBuildScene->TransRenderList;
                nglBuildScene->TransRenderList = v1;
                ++nglBuildScene->TransListCount;
            } else {
                nglListAddNode_Opaque(v1, (unsigned int)Quad->Tex);
            }
            if (nglSyncDebug.DumpSceneFile != 0)
                nglSceneDumpQuad(Quad);
        }
    } else {
        tlWarning("NULL mesh passed to nglListAddMesh !\n");
    }
}
