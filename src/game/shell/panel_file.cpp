// ============================================================================
// panel_file.cpp - PanelFile, PanelAnimObject/PanelQuad virtuals, FE loaders
// (shell.o FEPanel.cpp / FEManager.cpp / FEText.cpp families)
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/sv/sv_stubs.h"
#include "core/ae_fixed_string.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/nglFont.h"

#include <math.h>
#include <string.h>

extern float sNaN;                       // ?sNaN@@3MA @ 0x10F19D0
extern const char defaultFileName[];  // 0xCD67AE
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);            // core.o
extern void tlMemFree(void* ptr);                // core.o

extern FEManager g_femanager;
extern FEMenuColorScheme color_schemes[];  // ?color_schemes@@3PAVFEMenuColorScheme@@A @ 0xDF3AE0

// broc_shell.cpp / fe_util.cpp helpers (already ported)
extern void ReadString(unsigned char* buffer, int& index, Broc::string& ret);
extern void ReadVector3d(Broc::vector& v, unsigned char* buffer, int& index);
extern char* ReadStringPointer(unsigned char* buffer, int& index);
extern color32 ReadColor(unsigned char* buffer, int& index);
extern color32 MultiplyColors(color32 c1, color32 c2);
extern nglTexture* LocalizedGetTexture(const char* name);
extern int CountSharedVertices(short* didxs, int tri1, int tri2, int* ind);
extern void ReadMatrix3x4(math::Mat43& matrix, unsigned char* buffer,
                          int& index);
extern void* tlMemAlloc(unsigned int size, unsigned int align,
                        unsigned int flags);  // core.o

// ae_vector inline COMDAT semantics (shell.o 0x5B3960/0x5B3A40/0x5B5830)
template <typename T>
static void VectorPushBack(ae_vector<T>& v, T el)
{
    if (v.mSize >= v.mCapacity)
    {
        int v4 = v.mSize + 4;
        if (v.mSize <= 3)
            v4 = v.mSize + 1;
        T* v5 = (T*)tlMemAlloc(4 * v4, 8u, 0);
        for (int i = 0; i < v.mSize; ++i)
            v5[i] = v.mElements[i];
        if (v.mElements != nullptr)
        {
            tlMemFree(v.mElements);
            v.mElements = nullptr;
            v.mCapacity = 0;
        }
        v.mElements = v5;
        v.mCapacity = v4;
    }
    v.mElements[v.mSize++] = el;
}

static void VectorResize(ae_vector<PanelQuadSection*>& v, int iNewSize)
{
    if (iNewSize > v.mCapacity)
    {
        PanelQuadSection** v3 =
            (PanelQuadSection**)tlMemAlloc(4 * iNewSize, 8u, 0);
        for (int i = 0; i < v.mSize; ++i)
            v3[i] = v.mElements[i];
        if (v.mElements != nullptr)
        {
            tlMemFree(v.mElements);
            v.mElements = nullptr;
            v.mCapacity = 0;
        }
        v.mElements = v3;
        v.mCapacity = iNewSize;
        v.mSize = iNewSize;
    }
    else
    {
        v.mSize = iNewSize;
    }
}

// ============================================================================
// PanelAnimObject real virtuals
// ============================================================================

// ea: 0x0056B6E0
void PanelAnimObject::Update(float time_inc)
{
    if (IsShown())
    {
        char flags = this->flags;
        if ((flags & 0x10) != 0)
        {
            float v4 = (time_inc / fade_timer) + visibility;
            visibility = v4;
            if (v4 >= 1.0f)
            {
                visibility = 1.0f;
                this->flags = (char)(flags & 0xCF);
            }
        }
        else if ((flags & 0x20) != 0)
        {
            float v5 = visibility - (time_inc / fade_timer);
            visibility = v5;
            if (v5 <= 0.0f)
            {
                visibility = 0.0f;
                this->flags = (char)(flags & 0xCB);
            }
        }
    }
}

// ea: 0x0056B770
void PanelAnimObject::CopyFrom(const PanelAnimObject* pao)
{
    visibility = pao->visibility;
    z_value = pao->z_value;
    fade_timer = pao->fade_timer;
    flags = pao->flags;
}

// ea: 0x0056B7A0
void PanelAnimObject::SetWidescreenAlign(short wa)
{
    char v2 = (char)(flags & 0x3F);
    flags = v2;
    if (wa != 0)
    {
        if (wa == 2)
            flags = (char)(v2 | 0x80);
    }
    else
    {
        flags = (char)(v2 | 0x40);
    }
}

// ea: 0x0056B7D0
void PanelAnimObject::SetZvalue(float orig_z, panel_layer layer)
{
    float v4 = orig_z;
    if (orig_z < 0.0f)
        v4 = 0.0f;
    else if (orig_z > 1000.0f)
        v4 = 1000.0f;
    SetZvalueAbs((float)((1000 * layer) + v4) * 0.11111111f);
}

// ea: 0x0056B820
void PanelAnimObject::StartFade(bool start, bool fade_in, float time)
{
    if (start)
    {
        char flags = this->flags;
        fade_timer = time;
        if (fade_in)
        {
            if ((flags & 0x10) == 0
                && ((flags & 4) == 0 || (flags & 0x20) != 0))
            {
                this->flags = (char)(flags | 0x14);
                visibility = 0.0f;
            }
        }
        else if ((flags & 0x20) == 0)
        {
            this->flags = (char)(flags | 0x20);
            visibility = 1.0f;
        }
    }
    else
    {
        this->flags &= 0xCFu;
    }
}

// ============================================================================
// PanelQuad ctors / virtuals
// ============================================================================

// ea: 0x0058B780
PanelQuad::PanelQuad()
{
    flags = 4;
    visibility = 1.0f;
    center_point.x = sNaN;
    center_point.y = sNaN;
    center_point.z = sNaN;
    pqs.mElements = nullptr;
    pqs.mCapacity = 0;
    pqs.mSize = 0;
    name = Broc::string((Broc::string::Block*)nullptr);
    rotation = 0.0f;
    am_info = nullptr;
    sc_x = 1.0f;
    sc_y = 1.0f;
    quadBlendModeType = -1;
    quadMapFlags = 193;
}

// ea: 0x0058B840
PanelQuad::PanelQuad(char* n)
{
    flags = 4;
    visibility = 1.0f;
    center_point.x = sNaN;
    center_point.y = sNaN;
    center_point.z = sNaN;
    pqs.mElements = nullptr;
    pqs.mCapacity = 0;
    pqs.mSize = 0;
    name = Broc::string((Broc::string::Block*)nullptr);
    name = n;
    center_point.x = 0.0f;
    center_point.y = 0.0f;
    center_point.z = 0.0f;
    am_info = nullptr;
    rotation = 0.0f;
    sc_x = 1.0f;
    sc_y = 1.0f;
    quadBlendModeType = -1;
    quadMapFlags = 193;
}

// ea: 0x00590F70
PanelQuad::~PanelQuad()
{
    for (int i = 0; i < pqs.mSize; ++i)
        mem_heap_free(pqs.mElements[i]);
    pqs.mSize = 0;
    mem_heap_free(am_info);
    if (pqs.mElements != nullptr)
    {
        tlMemFree(pqs.mElements);
        pqs.mElements = nullptr;
        pqs.mCapacity = 0;
    }
}

// ea: 0x00579BC0
void PanelQuad::Draw()
{
    if (IsShown())
    {
        for (int i = 0; i < pqs.mSize; ++i)
            pqs.mElements[i]->Draw(quadBlendModeType, quadMapFlags);
    }
}

// ea: 0x0057A1F0
void PanelQuad::SetColor(const color32 c)
{
    unsigned int v4 = c.c.b | ((c.c.g | ((c.c.r | (c.c.a << 8)) << 8)) << 8);
    for (int i = 0; i < pqs.mSize; ++i)
    {
        for (int v = 0; v < 4; ++v)
            pqs.mElements[i]->quad.Verts[v].Color = v4;
    }
}

// ea: 0x005B6620
color32 PanelQuad::GetColor()
{
    if (pqs.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    color32 result;
    result.i = pqs.mElements[0]->quad.Verts[0].Color;
    return result;
}

// ea: 0x0057A450
void PanelQuad::SetVisibility(float alpha)
{
    unsigned int alphaa = (unsigned int)(alpha * 255.0f) << 8;
    for (int i = 0; i < pqs.mSize; ++i)
    {
        for (int v = 0; v < 4; ++v)
        {
            unsigned int c = pqs.mElements[i]->quad.Verts[v].Color;
            pqs.mElements[i]->quad.Verts[v].Color =
                c | ((((c >> 8) & 0xFF)
                      | ((alphaa | ((c >> 16) & 0xFF)) << 8))
                     << 8);
        }
    }
}

// ea: 0x576870
void PanelQuad::SetAlpha(float alpha)
{
    SetVisibility(alpha);
}

// ea: 0x57A8A0
void PanelQuad::SetXYInitialToCurrentPos()
{
    for (int i = 0; i < pqs.mSize; ++i)
    {
        PanelQuadSection* v3 = pqs.mElements[i];
        for (int v4 = 0; v4 < 4; ++v4)
        {
            v3->x_initial[v4] = (short)v3->quad.Verts[v4].X;
            v3->y_initial[v4] = (short)v3->quad.Verts[v4].Y;
        }
    }
}

// ea: 0x0058C450
PanelQuad* PanelQuad::Clone(PanelQuad* pPQ)
{
    PanelQuad* v1 = (PanelQuad*)mem_heap_malloc(0x48u);
    PanelQuad* v2;
    if (v1 != nullptr)
        v2 = new (v1) PanelQuad();
    else
        v2 = nullptr;
    if (v2 != nullptr)
        v2->CopyFrom(pPQ);
    return v2;
}

// ea: 0x0056AB80
void PanelQuad::SetBlend(unsigned int type)
{
    quadBlendModeType = type;
}

// ea: 0x0056ABA0
void PanelQuad::SetPos(float x1, float y1, float x2, float y2)
{
    float x[4] = {x1, x2, x1, x2};
    float y[4] = {y1, y1, y2, y2};
    SetPos(x, y);
}

// ea: 0x00583DF0
void PanelQuad::SetPos(float* x, float* y)
{
    float maxY = y[3];
    float v5 = y[1];
    if (y[2] > maxY)
        maxY = y[2];
    if (y[0] > v5)
        v5 = y[0];
    float v31 = v5;
    if (v5 <= maxY)
        v31 = maxY;

    float maxX = x[3];
    float v9 = x[1];
    if (x[2] > maxX)
        maxX = x[2];
    if (x[0] > v9)
        v9 = x[0];
    float v32 = v9;
    if (v9 <= maxX)
        v32 = maxX;

    float v10 = y[3];
    float v11 = y[1];
    if (v10 > y[2])
        v10 = y[2];
    if (v11 > y[0])
        v11 = y[0];
    float minY = v11;
    if (v10 <= v11)
        minY = v10;

    float v12 = x[3];
    float v13 = x[1];
    if (v12 > x[2])
        v12 = x[2];
    if (v13 > x[0])
        v13 = x[0];
    float minX = v13;
    if (v12 <= v13)
        minX = v12;

    if (pqs.mSize == 1)
    {
        PanelQuadSection* s = pqs.mElements[0];
        s->quad.Verts[0].X = x[0];
        s->quad.Verts[0].Y = y[0];
        s->quad.Verts[1].X = x[1];
        s->quad.Verts[1].Y = y[1];
        s->quad.Verts[2].X = x[2];
        s->quad.Verts[2].Y = y[2];
        s->quad.Verts[3].X = x[3];
        s->quad.Verts[3].Y = y[3];
    }
    else
    {
        Broc::vector max_coords = GetMax();
        Broc::vector min_coords = GetMin();
        float old_width = max_coords.x - min_coords.x;
        float old_height = max_coords.y - min_coords.y;
        float new_width = v32 - minX;
        float new_height = v31 - minY;
        if (old_width == 0.0f || old_height == 0.0f)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
            AeAssert::gCurrentLine = 1159;
            AeAssert::gCurrentExpr = "old_width != 0.0f && old_height != 0.0f";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
            old_height = old_height;
            old_width = old_width;
        }
        for (int i = 0; i < pqs.mSize; ++i)
        {
            PanelQuadSection* s = pqs.mElements[i];
            for (int v = 0; v < 4; ++v)
            {
                s->quad.Verts[v].X =
                    ((s->quad.Verts[v].X - min_coords.x) / old_width)
                        * new_width
                    + minX;
                s->quad.Verts[v].Y =
                    ((s->quad.Verts[v].Y - min_coords.y) / old_height)
                        * new_height
                    + minY;
            }
        }
    }
    center_point.x = ((v32 - minX) * 0.5f) + minX;
    center_point.y = ((v31 - minY) * 0.5f) + minY;
    center_point.z = 0.0f;
}

// ea: 0x0056AC00
void PanelQuad::FattenMeForPS2()
{
}

// ea: 0x0056AC10
void PanelQuad::FattenMeForGC()
{
}

namespace View {
float GetXScalingForHUD(int window);       // cg.o
float GetYScalingForHUD(int window);       // cg.o
float GetPreviousHUDXPos(float pos, int window, char justification,
                         float width);    // cg.o
float GetPreviousHUDYPos(float pos, int window, char justification,
                         float height);   // cg.o
float GetCurrentHUDXPos(float pos, int window, char justification,
                        float width);     // cg.o
float GetCurrentHUDYPos(float pos, int window, char justification,
                        float height);    // cg.o
}

// ea: 0x0056AC20
void PanelQuad::FormatHUDForSplitScreen(int viewport, int old_viewport,
                                        int justification, float just_width,
                                        float just_height)
{
    int v6 = viewport;
    if (viewport != old_viewport)
    {
        if (old_viewport != 0)
        {
            float x_scale = View::GetXScalingForHUD(0);
            float y_scale = View::GetYScalingForHUD(0);
            float cx = GetCenterX();
            float x_pos = View::GetPreviousHUDXPos(cx, old_viewport,
                                                   (char)justification,
                                                   just_width);
            float cy = GetCenterY();
            float y_pos = View::GetPreviousHUDYPos(cy, old_viewport,
                                                   (char)justification,
                                                   just_height);
            SetCenterPos(x_pos, y_pos);
            ScaleAbsoluteCenter(x_scale, y_scale);
            v6 = viewport;
        }
        if (v6 != 0)
        {
            float cx = GetCenterX();
            float x_pos = View::GetCurrentHUDXPos(cx, v6,
                                                  (char)justification,
                                                  just_width);
            float cy = GetCenterY();
            float y_pos = View::GetCurrentHUDYPos(cy, v6,
                                                  (char)justification,
                                                  just_height);
            float x_scale = View::GetXScalingForHUD(v6);
            float y_scale = View::GetYScalingForHUD(v6);
            ScaleAbsoluteCenter(x_scale, y_scale);
            SetCenterPos(x_pos, y_pos);
        }
    }
}

// ea: 0x005799E0
void PanelQuad::InstanceFrom(const PanelQuad* pq)
{
    visibility = pq->visibility;
    z_value = pq->z_value;
    fade_timer = pq->fade_timer;
    flags = pq->flags;
    name = pq->name;
    rotation = pq->rotation;
    sc_x = pq->sc_x;
    sc_y = pq->sc_y;
    if (pq->am_info != nullptr)
    {
        PQArcMaskingInfo* v3 =
            (PQArcMaskingInfo*)mem_heap_malloc(0x34u);
        if (v3 != nullptr)
        {
            memcpy(v3, pq->am_info, sizeof(PQArcMaskingInfo));
            am_info = v3;
        }
        else
        {
            am_info = nullptr;
        }
    }
    else
    {
        am_info = nullptr;
    }
    if (pq->pqs.mSize != pqs.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
        AeAssert::gCurrentLine = 575;
        AeAssert::gCurrentExpr = "pq->pqs.size() == pqs.size()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Cant use PanelQuad::CopyFrom2 with differing sizes of pqs"))
            __debugbreak();
    }
    for (int i = 0; i < pq->pqs.mSize; ++i)
        pqs.mElements[i]->CopyFrom(pq->pqs.mElements[i]);
    center_point.x = pq->center_point.x;
    center_point.y = pq->center_point.y;
    center_point.z = 0.0f;
}

// ea: 0x0058B930
void PanelQuad::CopyFrom(const PanelQuad* pq)
{
    visibility = pq->visibility;
    z_value = pq->z_value;
    fade_timer = pq->fade_timer;
    flags = pq->flags;
    name = pq->name;
    rotation = pq->rotation;
    sc_x = pq->sc_x;
    sc_y = pq->sc_y;
    if (pq->am_info != nullptr)
    {
        PQArcMaskingInfo* v4 =
            (PQArcMaskingInfo*)mem_heap_malloc(0x34u);
        if (v4 != nullptr)
        {
            memcpy(v4, pq->am_info, sizeof(PQArcMaskingInfo));
            am_info = v4;
        }
        else
        {
            am_info = nullptr;
        }
    }
    else
    {
        am_info = nullptr;
    }
    for (int i = 0; i < pq->pqs.mSize; ++i)
    {
        PanelQuadSection* v6 =
            (PanelQuadSection*)mem_heap_malloc(0x68u);
        if (v6 != nullptr)
            memcpy(v6, pq->pqs.mElements[i], 0x68u);
        VectorPushBack(pqs, v6);
    }
    center_point.x = pq->center_point.x;
    center_point.y = pq->center_point.y;
    center_point.z = 0.0f;
}

// ea: 0x00579D40
void PanelQuad::Rotate(float rx, float ry, float r, bool absolute)
{
    float offset = r;
    if (absolute)
        offset = r - rotation;
    if (pqs.mSize != 0)
    {
        float s = sinf(offset);
        float c = cosf(offset);
        for (int i = 0; i < pqs.mSize; ++i)
        {
            PanelQuadSection* sec = pqs.mElements[i];
            for (int v = 0; v < 4; ++v)
            {
                float dx = sec->quad.Verts[v].X - rx;
                float dy = sec->quad.Verts[v].Y - ry;
                sec->quad.Verts[v].X = (c * dx) - (s * dy) + rx;
                sec->quad.Verts[v].Y = (s * dx) + (c * dy) + ry;
            }
        }
    }
    rotation += offset;
}

// ea: 0x00579E70
void PanelQuad::Scale(float sx, float sy, float scx, float scy, bool absolute)
{
    float v6 = scx;
    if (scx == 0.0f)
        v6 = 0.01f;
    float v8 = scy;
    if (scy == 0.0f)
        v8 = 0.01f;
    if (absolute)
    {
        v6 = v6 / sc_x;
        v8 = v8 / sc_y;
    }
    for (int i = 0; i < pqs.mSize; ++i)
    {
        PanelQuadSection* sec = pqs.mElements[i];
        for (int v = 0; v < 4; ++v)
        {
            sec->quad.Verts[v].X =
                ((sec->quad.Verts[v].X - sx) * v6) + sx;
            sec->quad.Verts[v].Y =
                ((sec->quad.Verts[v].Y - sy) * v8) + sy;
        }
    }
    sc_x = sc_x * v6;
    sc_y = v8 * sc_y;
    if (am_info != nullptr)
    {
        am_info->arc_radius_x = am_info->arc_radius_x * v6;
        am_info->arc_radius_y = am_info->arc_radius_y * v8;
    }
}

// ea: 0x0057A050
void PanelQuad::ScaleAbsoluteCenter(float scx, float scy)
{
    float v3 = scx;
    if (scx == 0.0f)
    {
        v3 = 0.01f;
        scx = 0.01f;
    }
    float v5 = scy;
    if (scy == 0.0f)
    {
        v5 = 0.01f;
        scy = 0.01f;
    }
    float v7 = v3 / sc_x;
    float v8 = v5 / sc_y;
    sc_x = v3;
    sc_y = v5;
    for (int i = 0; i < pqs.mSize; ++i)
        pqs.mElements[i]->ScaleAbsoluteCenter(center_point.x,
                                              center_point.y, scx, scy);
    if (am_info != nullptr)
    {
        am_info->arc_radius_x = v7 * am_info->arc_radius_x;
        am_info->arc_radius_y = v8 * am_info->arc_radius_y;
    }
    rotation = 0.0f;
}

// ea: 0x0057A2B0
void PanelQuad::SetColorNA(color32 c)
{
    for (int i = 0; i < pqs.mSize; ++i)
    {
        PanelQuadSection* sec = pqs.mElements[i];
        for (int j = 0; j < 4; ++j)
        {
            sec->quad.Verts[j].Color =
                c.c.b
                | ((c.c.g
                    | ((c.c.r
                        | ((sec->quad.Verts[j].Color >> 24) << 8))
                       << 8))
                   << 8);
        }
    }
}

// ea: 0x0057A5C0
void PanelQuad::GetPos(float* x, float* y)
{
    PanelQuadSection* sec = pqs.mElements[0];
    for (int i = 0; i < 4; ++i)
    {
        x[i] = sec->quad.Verts[i].X;
        y[i] = sec->quad.Verts[i].Y;
    }
}

// ea: 0x00584250
void PanelQuad::ResetToInitialXY()
{
    for (int i = 0; i < pqs.mSize; ++i)
    {
        PanelQuadSection* sec = pqs.mElements[i];
        sec->quad.Verts[0].X = sec->x_initial[0];
        sec->quad.Verts[0].Y = sec->y_initial[0];
        sec->quad.Verts[1].X = sec->x_initial[1];
        sec->quad.Verts[1].Y = sec->y_initial[1];
        sec->quad.Verts[2].X = sec->x_initial[2];
        sec->quad.Verts[2].Y = sec->y_initial[2];
        sec->quad.Verts[3].X = sec->x_initial[3];
        sec->quad.Verts[3].Y = sec->y_initial[3];
    }
    Broc::vector min_coords = GetMin();
    Broc::vector max_coords = GetMax();
    center_point.x =
        ((max_coords.x - min_coords.x) * 0.5f) + min_coords.x;
    center_point.y =
        ((max_coords.y - min_coords.y) * 0.5f) + min_coords.y;
    center_point.z =
        ((max_coords.z - min_coords.z) * 0.5f) + min_coords.z;
    sc_x = 1.0f;
    sc_y = 1.0f;
    rotation = 0.0f;
}

// ea: 0x0057A7E0
void PanelQuad::ShiftXYInitial(float off_x, float off_y)
{
    for (int i = 0; i < pqs.mSize; ++i)
    {
        PanelQuadSection* sec = pqs.mElements[i];
        for (int j = 0; j < 4; ++j)
        {
            sec->x_initial[j] = (short)(sec->x_initial[j] + off_x);
            sec->y_initial[j] = (short)(sec->y_initial[j] + off_y);
        }
    }
}

// ea: 0x0056A810
void PanelQuadSection::CopyFrom(PanelQuadSection* pSrc)
{
    quad = pSrc->quad;
}

// ea: 0x005696D0
PanelQuadSection::PanelQuadSection()
{
    quad.Z = 0.0f;
}

// ea: 0x00569720
void PanelQuadSection::SetXYInitialToCurrentPos()
{
    for (int i = 0; i < 4; ++i)
    {
        x_initial[i] = (short)quad.Verts[i].X;
        y_initial[i] = (short)quad.Verts[i].Y;
    }
}

// ea: 0x00569750
void PanelQuadSection::Rotate(float rotate_x, float rotate_y, float rotation)
{
    float s = sinf(rotation);
    float c = cosf(rotation);
    for (int i = 0; i < 4; ++i)
    {
        float dx = quad.Verts[i].X - rotate_x;
        float dy = quad.Verts[i].Y - rotate_y;
        quad.Verts[i].X = (c * dx) - (s * dy) + rotate_x;
        quad.Verts[i].Y = (s * dx) + (c * dy) + rotate_y;
    }
}

// ea: 0x005697E0
void PanelQuadSection::Scale(float sx, float sy, float scx, float scy)
{
    for (int i = 0; i < 4; ++i)
    {
        quad.Verts[i].X = ((quad.Verts[i].X - sx) * scx) + sx;
        quad.Verts[i].Y = ((quad.Verts[i].Y - sy) * scy) + sy;
    }
}

// ea: 0x00569AF0
void PanelQuadSection::SetUV(float* u, float* v)
{
    for (int i = 0; i < 4; ++i)
    {
        quad.Verts[i].U = u[i];
        quad.Verts[i].V = v[i];
    }
}

// ea: 0x00569B70
void PanelQuadSection::SetPos(float* x, float* y)
{
    for (int i = 0; i < 4; ++i)
    {
        quad.Verts[i].X = x[i];
        quad.Verts[i].Y = y[i];
    }
}

// ea: 0x00569BF0
void PanelQuadSection::ResetToInitialXY()
{
    for (int i = 0; i < 4; ++i)
    {
        quad.Verts[i].X = x_initial[i];
        quad.Verts[i].Y = y_initial[i];
    }
}

// ea: 0x0056A050
void PanelQuadSection::Shift(float off_x, float off_y)
{
    for (int i = 0; i < 4; ++i)
    {
        quad.Verts[i].X += off_x;
        quad.Verts[i].Y += off_y;
    }
}

// ea: 0x0056A0E0
void PanelQuadSection::ShiftXYInitial(float off_x, float off_y)
{
    for (int i = 0; i < 4; ++i)
    {
        x_initial[i] = (short)(x_initial[i] + off_x);
        y_initial[i] = (short)(y_initial[i] + off_y);
    }
}

// ea: 0x0056A280
void PanelQuadSection::Fatten(float fatten_width, float about_x)
{
    for (int i = 0; i < 4; ++i)
        quad.Verts[i].X = ((quad.Verts[i].X - about_x) * fatten_width)
                          + about_x;
}

// ea: 0x00579930
void PanelQuadSection::Fatten(float fatten_width)
{
    float minX = 1000.0f;
    for (int i = 0; i < 4; ++i)
    {
        if (x_initial[i] < minX)
            minX = (float)x_initial[i];
    }
    for (int i = 0; i < 4; ++i)
        quad.Verts[i].X = ((quad.Verts[i].X - minX) * fatten_width) + minX;
}

// ea: 0x00579840
void PanelQuadSection::SetAlphaVert(int i, float alpha)
{
    unsigned int c = quad.Verts[i].Color;
    quad.Verts[i].Color =
        (c & 0xFFFFFF) | ((unsigned int)(alpha * 255.0f) << 24);
}

// ea: 0x005B65A0 (inline COMDAT)
void PanelQuad::SetAlpha(int pqsIdx, int vertIdx, float alpha)
{
    unsigned int c = pqs.mElements[pqsIdx]->quad.Verts[vertIdx].Color;
    pqs.mElements[pqsIdx]->quad.Verts[vertIdx].Color =
        (c & 0xFFFFFF) | ((unsigned int)(alpha * 255.0f) << 24);
}

// ea: 0x005B64D0 (inline COMDAT)
void PanelQuad::SetSectionUV(int index, float* u, float* v)
{
    PanelQuadSection* s = pqs.mElements[index];
    for (int i = 0; i < 4; ++i)
    {
        s->quad.Verts[i].U = u[i];
        s->quad.Verts[i].V = v[i];
    }
}

// ea: 0x005B6530 (inline COMDAT)
nglTexture* PanelQuad::GetTexture()
{
    if (pqs.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    return pqs.mElements[0]->quad.Tex;
}

// ea: 0x005B66C0 (inline COMDAT)
color32 PanelQuad::GetColor(int pqsIdx, int vertIdx)
{
    color32 result;
    result.i = pqs.mElements[pqsIdx]->quad.Verts[vertIdx].Color;
    return result;
}

// ea: 0x005698B0
void PanelQuadSection::ScaleAbsoluteCenter(float sx, float sy, float scx,
                                           float scy)
{
    float minX = 1000.0f;
    float maxX = -1000.0f;
    float minY = 1000.0f;
    float maxY = -1000.0f;
    for (int i = 0; i < 4; ++i)
    {
        if (x_initial[i] < minX)
            minX = (float)x_initial[i];
        if (x_initial[i] > maxX)
            maxX = (float)x_initial[i];
        if (y_initial[i] < minY)
            minY = (float)y_initial[i];
        if (y_initial[i] > maxY)
            maxY = (float)y_initial[i];
    }
    float centerX = ((maxX - minX) * 0.5f) + minX;
    float centerY = ((maxY - minY) * 0.5f) + minY;
    quad.Verts[0].X = sx - ((centerX - x_initial[0]) * scx);
    quad.Verts[0].Y = sy - ((centerY - y_initial[0]) * scy);
    quad.Verts[1].X = sx - ((centerX - x_initial[1]) * scx);
    quad.Verts[1].Y = sy - ((centerY - y_initial[1]) * scy);
    quad.Verts[2].X = sx - ((centerX - x_initial[2]) * scx);
    quad.Verts[2].Y = sy - ((centerY - y_initial[2]) * scy);
    quad.Verts[3].X = sx - ((centerX - x_initial[3]) * scx);
    quad.Verts[3].Y = sy - ((centerY - y_initial[3]) * scy);
}

// ea: 0x0057A530
void PanelQuad::SetZvalueAbs(float z)
{
    z_value = z;
    for (int i = 0; i < pqs.mSize; ++i)
        pqs.mElements[i]->quad.Z = z;
}

// ea: 0x0057AA40
void PanelQuad::FattenMeForWidescreen(bool widescreen, float x)
{
    float v4 = widescreen ? 0.75f : 1.3333334f;
    for (int i = 0; i < pqs.mSize; ++i)
    {
        PanelQuadSection* v6 = pqs.mElements[i];
        v6->quad.Verts[0].X = ((v6->quad.Verts[0].X - x) * v4) + x;
        v6->quad.Verts[1].X = ((v6->quad.Verts[1].X - x) * v4) + x;
        v6->quad.Verts[2].X = ((v6->quad.Verts[2].X - x) * v4) + x;
        v6->quad.Verts[3].X = ((v6->quad.Verts[3].X - x) * v4) + x;
    }
    center_point.x = ((center_point.x - x) * v4) + x;
}

// ea: 0x0057A6A0
void PanelQuad::Shift(float off_x, float off_y)
{
    for (int i = 0; i < pqs.mSize; ++i)
    {
        PanelQuadSection* sec = pqs.mElements[i];
        for (int v = 0; v < 4; ++v)
        {
            sec->quad.Verts[v].X += off_x;
            sec->quad.Verts[v].Y += off_y;
        }
    }
    center_point.x += off_x;
    center_point.y += off_y;
    if (am_info != nullptr)
    {
        float* arc = (float*)am_info;
        arc[0] += off_x;
        arc[1] += off_y;
    }
}

// ea: 0x0056AB90
void PanelQuad::SetMaterialFlags(unsigned int mapflags)
{
    quadMapFlags = mapflags;
}

// ea: 0x0057A170
void PanelQuad::SetTexture(nglTexture* tex)
{
    for (int i = 0; i < pqs.mSize; ++i)
        pqs.mElements[i]->quad.Tex = tex;
}

// ea: 0x0058BAC0
void PanelQuad::Init(Broc::vector* xy, color32* col, panel_layer lay,
                     float z, const char* filename)
{
    SetZvalue(z, lay);
    Broc::vector uv[4] = {
        Broc::vector(0.0f, 0.0f, 0.0f),
        Broc::vector(1.0f, 0.0f, 0.0f),
        Broc::vector(0.0f, 1.0f, 0.0f),
        Broc::vector(1.0f, 1.0f, 0.0f),
    };
    PanelQuadSection* v7 = (PanelQuadSection*)mem_heap_malloc(0x68u);
    if (v7 != nullptr)
        v7->quad.Z = 0.0f;
    else
        v7 = nullptr;
    PanelQuadSection* lay2 = v7;
    v7->AddPQSection(xy, uv, col, z_value);
    VectorPushBack(pqs, lay2);
    if (filename != nullptr && *filename != 0)
        SetTexture(LocalizedGetTexture(filename));
    Broc::vector min_coords = GetMin();
    Broc::vector Max = GetMax();
    float y = Max.y;
    float v12 = min_coords.z;
    float v14 = Max.z - min_coords.z;
    center_point.x = ((Max.x - min_coords.x) * 0.5f) + min_coords.x;
    center_point.y = ((y - min_coords.y) * 0.5f) + min_coords.y;
    center_point.z = (v14 * 0.5f) + v12;
}

// ea: 0x0057AB70
void PanelQuad::Animate(math::Mat43* mat, float vis)
{
    for (int i = 0; i < pqs.mSize; ++i)
    {
        pqs.mElements[i]->Animate(mat, z_value, (flags & 1) != 0);
    }
    if ((flags & 1) != 0)
        SetVisibility(visibility * vis);
    else
        SetVisibility(vis);
    flags = (char)(flags | 1);
}

// ============================================================================
// PanelQuadSection extras
// ============================================================================

// ea: 0x0056A130
void PanelQuadSection::Animate(math::Mat43* xform, float z_value,
                               bool xform_was_set)
{
    float out_x[4];
    float out_y[4];
    for (int i = 0; i < 4; ++i)
    {
        float xyz[4];
        if (xform_was_set)
        {
            xyz[0] = quad.Verts[i].X;
            xyz[1] = quad.Verts[i].Y;
            xyz[2] = z_value;
            xyz[3] = 0.0f;
        }
        else
        {
            xyz[0] = x_initial[i];
            xyz[1] = y_initial[i];
            xyz[2] = z_value;
            xyz[3] = 0.0f;
        }
        float vx = xform->x.v.m128_f32[0] * xyz[0]
                   + xform->x.v.m128_f32[1] * xyz[1]
                   + xform->x.v.m128_f32[2] * xyz[2]
                   + xform->w.v.m128_f32[0];
        float vy = xform->y.v.m128_f32[0] * xyz[0]
                   + xform->y.v.m128_f32[1] * xyz[1]
                   + xform->y.v.m128_f32[2] * xyz[2]
                   + xform->w.v.m128_f32[1];
        out_x[i] = vx;
        out_y[i] = vy;
    }
    quad.Verts[0].X = out_x[0];
    quad.Verts[0].Y = out_y[0];
    quad.Verts[1].X = out_x[1];
    quad.Verts[1].Y = out_y[1];
    quad.Verts[2].X = out_x[2];
    quad.Verts[2].Y = out_y[2];
    quad.Verts[3].X = out_x[3];
    quad.Verts[3].Y = out_y[3];
}

// ea: 0x0056A2F0
void PanelQuadSection::Draw(unsigned int type, unsigned int mapflags)
{
    nglQuad drawQuad;
    nglInitQuad(&drawQuad);
    nglSetQuadZ(&drawQuad, quad.Z);
    nglSetQuadMapFlags(&drawQuad, mapflags);
    if (type != -1)
        nglSetQuadBlend(&drawQuad, type);
    nglSetQuadTex(&drawQuad, quad.Tex);
    for (int v4 = 0; v4 < 4; ++v4)
    {
        nglSetQuadVPos(&drawQuad, v4, quad.Verts[v4].X, quad.Verts[v4].Y);
        nglSetQuadVUV(&drawQuad, v4, quad.Verts[v4].U, quad.Verts[v4].V);
        nglSetQuadVColor(&drawQuad, v4, quad.Verts[v4].Color);
    }
    nglListAddQuad(&drawQuad);
}

// ea: 0x005798A0
void PanelQuadSection::SetVisibility(int i, float vis)
{
    unsigned int c = quad.Verts[i].Color;
    quad.Verts[i].Color = c
        | ((((c >> 8) & 0xFF)
            | ((((c >> 16) & 0xFF)
                | ((unsigned int)(vis * 255.0f) << 8))
               << 8))
           << 8);
}

// ea: 0x0056A3B0
void PanelQuadSection::FormatForSplitScreen(int viewport, int old_viewport)
{
    float v3 = 0.0f;
    float v4 = 0.0f;
    float v5 = 1.0f;
    float v6 = 1.0f;
    switch (old_viewport)
    {
    case 3:
        v6 = 2.0f;
        break;
    case 4:
        v6 = 2.0f;
        goto LABEL_8;
    case 5:
        v6 = 2.0f;
        v5 = 2.0f;
        break;
    case 6:
        v6 = 2.0f;
        v3 = -320.0f;
        v5 = 2.0f;
        break;
    case 7:
        goto L138354;
    case 8:
        v3 = -320.0f;
    L138354:
        v6 = 2.0f;
        v5 = 2.0f;
    LABEL_8:
        v4 = -240.0f;
        break;
    default:
        break;
    }
    for (int i = 0; i < 4; ++i)
    {
        quad.Verts[i].X += v3;
        quad.Verts[i].Y += v4;
    }
    for (int i = 0; i < 4; ++i)
    {
        quad.Verts[i].X *= v5;
        quad.Verts[i].Y *= v6;
    }
    float v10 = 0.0f;
    float v11 = 0.0f;
    float v12 = 1.0f;
    float v13 = 1.0f;
    switch (viewport)
    {
    case 3:
        v13 = 0.5f;
        break;
    case 4:
        v13 = 0.5f;
        goto LABEL_16;
    case 5:
        v13 = 0.5f;
        v12 = 0.5f;
        break;
    case 6:
        v13 = 0.5f;
        v10 = 320.0f;
        v12 = 0.5f;
        break;
    case 7:
        goto L138364;
    case 8:
        v10 = 320.0f;
    L138364:
        v13 = 0.5f;
        v12 = 0.5f;
    LABEL_16:
        v11 = 240.0f;
        break;
    default:
        break;
    }
    for (int i = 0; i < 4; ++i)
    {
        quad.Verts[i].X *= v12;
        quad.Verts[i].Y *= v13;
    }
    for (int i = 0; i < 4; ++i)
    {
        quad.Verts[i].X += v10;
        quad.Verts[i].Y += v11;
    }
}

// ea: 0x0056A690
void PanelQuadSection::MoveForSplitScreen(int viewport, int old_viewport)
{
    if (viewport != old_viewport)
    {
        float v3 = 0.0f;
        float v4 = 0.0f;
        switch (old_viewport)
        {
        case 4:
        case 7:
            goto L138381;
        case 6:
            v4 = -280.0f;
            break;
        case 8:
            v4 = -280.0f;
        L138381:
            v3 = -200.0f;
            break;
        default:
            break;
        }
        for (int i = 0; i < 4; ++i)
        {
            quad.Verts[i].X += v4;
            quad.Verts[i].Y += v3;
        }
        v3 = 0.0f;
        v4 = 0.0f;
        switch (viewport)
        {
        case 4:
        case 7:
            goto L138391;
        case 6:
            v4 = 280.0f;
            break;
        case 8:
            v4 = 280.0f;
        L138391:
            v3 = 200.0f;
            break;
        default:
            break;
        }
        for (int i = 0; i < 4; ++i)
        {
            quad.Verts[i].X += v4;
            quad.Verts[i].Y += v3;
        }
    }
}

// ea: 0x0057A940
void PanelQuad::FormatForSplitScreen(int viewport, int old_viewport)
{
    for (int i = 0; i < pqs.mSize; ++i)
        pqs.mElements[i]->FormatForSplitScreen(viewport, old_viewport);
}

// ea: 0x0057A9C0
void PanelQuad::MoveForSplitScreen(int viewport, int old_viewport)
{
    for (int i = 0; i < pqs.mSize; ++i)
        pqs.mElements[i]->MoveForSplitScreen(viewport, old_viewport);
}

// ea: 0x579C40
void PanelQuad::Mask(float percent, mask_type maskType, float uv_width)
{
    if (maskType == NO_MASK)
        return;
    if (pqs.mSize != 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
        AeAssert::gCurrentLine = 833;
        AeAssert::gCurrentExpr =
            "pqs.size() == 1 && \"Cannot mask PanelQuads with more than one Section\"";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (percent < 0.0f || percent > 1.0f)
        percent = percent < 0.0f ? 0.0f : 1.0f;
    float scale = (maskType == LEFT_MASK || maskType == RIGHT_MASK)
                      ? this->sc_x
                      : this->sc_y;
    pqs.mElements[0]->Mask(percent, maskType, uv_width, scale);
}

// ea: 0x579640
void PanelQuadSection::Mask(float mask, mask_type type, float uv_width,
                            float scale)
{
    if (type == NO_MASK)
        return;
    float v5 = mask;
    if (mask > 1.0f)
        v5 = 2.0f - mask;
    int v6 = (type == LEFT_MASK || type == RIGHT_MASK) ? x_initial[3]
                                                       : y_initial[3];
    int v7 = (type == LEFT_MASK || type == RIGHT_MASK) ? x_initial[0]
                                                       : y_initial[0];
    float v8 = (v6 - v7) * scale;
    float X;
    float v10;
    float v11;
    float Y;
    if (type == LEFT_MASK)
    {
        X = quad.Verts[3].X - (v8 * v5);
        v10 = quad.Verts[3].X;
        v11 = quad.Verts[0].Y;
        Y = quad.Verts[3].Y;
    }
    else if (type == RIGHT_MASK)
    {
        X = quad.Verts[0].X;
        v10 = (v8 * v5) + X;
        v11 = quad.Verts[0].Y;
        Y = (v8 * v5) + v11;
    }
    else if (type == TOP_MASK)
    {
        X = quad.Verts[0].X;
        v10 = quad.Verts[3].X;
        v11 = quad.Verts[3].Y - (v8 * v5);
        Y = quad.Verts[3].Y;
    }
    else  // BOTTOM_MASK
    {
        X = quad.Verts[0].X;
        v10 = quad.Verts[3].X;
        v11 = quad.Verts[0].Y;
        Y = (v8 * v5) + v11;
    }
    float U = quad.Verts[1].U;
    float v14 = quad.Verts[0].U;
    bool v15;
    v15 = (v14 < U) ? (U - v14) < 0.001f : (v14 - U) < 0.001f;
    float v27 = v15 ? X : v10;
    float v16 = v15 ? v10 : X;
    float v17 = v15 ? v11 : Y;
    float v18 = v15 ? Y : v11;
    quad.Verts[3].Y = Y;
    quad.Verts[0].Y = v11;
    quad.Verts[0].X = X;
    quad.Verts[1].X = v27;
    quad.Verts[1].Y = v18;
    quad.Verts[2].X = v16;
    quad.Verts[2].Y = v17;
    quad.Verts[3].X = v10;
    if (uv_width <= 0.0f)
        return;
    float v19;
    float v20;
    float V;
    float v22;
    if (type == LEFT_MASK)
    {
        v19 = quad.Verts[3].U - (v5 * uv_width);
        v20 = quad.Verts[3].U;
        V = quad.Verts[0].V;
        v22 = quad.Verts[3].V;
    }
    else if (type == RIGHT_MASK)
    {
        v19 = quad.Verts[0].U;
        v20 = (v5 * uv_width) + v19;
        V = quad.Verts[0].V;
        v22 = (v5 * uv_width) + V;
    }
    else if (type == TOP_MASK)
    {
        v19 = quad.Verts[0].U;
        v20 = quad.Verts[3].U;
        V = quad.Verts[3].V - (v5 * uv_width);
        v22 = quad.Verts[3].V;
    }
    else
    {
        v19 = quad.Verts[0].U;
        v20 = quad.Verts[3].U;
        V = quad.Verts[0].V;
        v22 = (v5 * uv_width) + V;
    }
    float v23 = v15 ? v19 : v20;
    float v24 = v15 ? v20 : v19;
    float v25 = v15 ? V : v22;
    float v26 = v15 ? v22 : V;
    quad.Verts[0].U = v19;
    quad.Verts[0].V = V;
    quad.Verts[1].U = v23;
    quad.Verts[1].V = v26;
    quad.Verts[2].U = v24;
    quad.Verts[2].V = v25;
    quad.Verts[3].U = v20;
    quad.Verts[3].V = v22;
}

// ============================================================================
// PanelFile loaders (FEPanel.cpp)
// ============================================================================

static math::Mat43 ComposeMatrices(const math::Mat43& parent,
                                   const math::Mat43& local)
{
    math::Mat43 out;
    for (int c = 0; c < 3; ++c)
    {
        float lx = local.x.v.m128_f32[c];
        float ly = local.y.v.m128_f32[c];
        float lz = local.z.v.m128_f32[c];
        out.x.v.m128_f32[c] =
            parent.x.v.m128_f32[0] * lx + parent.x.v.m128_f32[1] * ly
            + parent.x.v.m128_f32[2] * lz;
        out.y.v.m128_f32[c] =
            parent.y.v.m128_f32[0] * lx + parent.y.v.m128_f32[1] * ly
            + parent.y.v.m128_f32[2] * lz;
        out.z.v.m128_f32[c] =
            parent.z.v.m128_f32[0] * lx + parent.z.v.m128_f32[1] * ly
            + parent.z.v.m128_f32[2] * lz;
        out.w.v.m128_f32[c] =
            parent.w.v.m128_f32[0] * lx + parent.w.v.m128_f32[1] * ly
            + parent.w.v.m128_f32[2] * lz + local.w.v.m128_f32[c];
    }
    out.x.v.m128_f32[3] = 0.0f;
    out.y.v.m128_f32[3] = 0.0f;
    out.z.v.m128_f32[3] = 0.0f;
    out.w.v.m128_f32[3] = 1.0f;
    return out;
}

// ea: 0x00591O... LoadPanelGeom
void PanelFile::LoadPanelGeom(unsigned char* buffer, int& index,
                              const math::Mat43* parent_matrix)
{
    unsigned char v28 = buffer[index++];
    const char* StringPointer = ReadStringPointer(buffer, index);
    math::Mat43 local;
    ReadMatrix3x4(local, buffer, index);
    math::Mat43 matrix_with_parent = ComposeMatrices(*parent_matrix, local);
    unsigned char v24 = buffer[index++];
    int v17 = buffer[index] | (buffer[index + 1] << 8)
              | (buffer[index + 2] << 16) | (buffer[index + 3] << 24);
    index += 4;
    for (int i = v17; i != 0; --i)
        LoadPanelGeom(buffer, index, &matrix_with_parent);
    switch (v28)
    {
    case 0x91:
        LoadPanelObject(buffer, index, &matrix_with_parent, StringPointer,
                        v24);
        break;
    case 0x94:
        LoadPanelText(buffer, index, &matrix_with_parent, StringPointer,
                      v24);
        break;
    case 0x95:
        LoadPanelText2(buffer, index, &matrix_with_parent, StringPointer,
                       v24);
        break;
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
        AeAssert::gCurrentLine = 1774;
        AeAssert::gCurrentExpr = "0 && \"Invalid PanelGeom type\"";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        break;
    }
}

// ea: 0x0058C770
void PanelFile::LoadPanelObject(unsigned char* buffer, int& index,
                                const math::Mat43* parent_matrix,
                                const char* name, short widescreen_align)
{
    unsigned short v9 = (unsigned short)(buffer[index] | (buffer[index + 1] << 8));
    index += 2;
    int v10 = v9;
    PanelMaterial* v12 = nullptr;
    v12 = (PanelMaterial*)mem_heap_malloc(16 * v10 + 4);
    if (v12 != nullptr)
    {
        PanelMaterial* first = (PanelMaterial*)((char*)v12 + 4);
        *(int*)v12 = v10;
        for (int i = 0; i < v10; ++i)
            new (&first[i]) PanelMaterial();
        PanelMaterial* v13 = first;
        for (int m = 0; m < v10; ++m)
        {
            v13->texture = nullptr;
            ReadPanelMaterial(*v13, buffer, index);
            ++v13;
        }
    }
    unsigned short v15 = (unsigned short)(buffer[index] | (buffer[index + 1] << 8));
    index += 2;
    for (int bufferb = v15; bufferb != 0; --bufferb)
    {
        PanelQuad* v16 = (PanelQuad*)mem_heap_malloc(0x48u);
        PanelQuad* v17 = v16 != nullptr ? new (v16) PanelQuad() : nullptr;
        v17->name = name;
        v17->SetWidescreenAlign(widescreen_align);
        v17->Load(v12 != nullptr
                      ? (PanelMaterial*)((char*)v12 + 4)
                      : nullptr,
                  buffer, index, parent_matrix);
        VectorPushBack(pquads, v17);
    }
    if (v12 != nullptr)
    {
        mem_heap_free(v12);
    }
}

// ea: 0x0058C8E0
void PanelFile::LoadPanelText(unsigned char* buffer, int& index,
                              const math::Mat43* parent_matrix,
                              const char* name, short widescreen_align)
{
    Broc::string fontname;
    ReadString(buffer, index, fontname);
    color32 fi = ReadColor(buffer, index);
    unsigned char flags = buffer[index++];
    int line_spacing = buffer[index] | (buffer[index + 1] << 8)
                       | (buffer[index + 2] << 16) | (buffer[index + 3] << 24);
    index += 4;
    float scale = *(float*)&buffer[index];
    index += 4;
    unsigned short v43 = (unsigned short)(buffer[index] | (buffer[index + 1] << 8));
    index += 2;
    int panel_text_index = buffer[index] | (buffer[index + 1] << 8)
                           | (buffer[index + 2] << 16) | (buffer[index + 3] << 24);
    index += 4;
    Broc::vector boundboxsize;
    Broc::vector boundboxcenter;
    ReadVector3d(boundboxsize, buffer, index);
    ReadVector3d(boundboxcenter, buffer, index);
    font_index Font = g_femanager.FindFont(fontname.c_str(), false);
    if (Font == FONT_NORMAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
        AeAssert::gCurrentLine = 1844;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int v22 = 0;
    int v23 = 0;
    if ((flags & 3) != 0)
    {
        if ((flags & 3) == 1)
            v23 = 0;
        else if ((flags & 3) == 3)
            v23 = 32;
    }
    else
    {
        v23 = 16;
    }
    unsigned char v24 = flags & 0xC;
    if (v24 != 0)
    {
        if (v24 == 4)
            v22 = 0;
        else if (v24 == 12)
            v22 = 128;
    }
    else
    {
        v22 = 64;
    }
    float x = boundboxsize.x;
    if (v23 == 16)
        x = boundboxsize.x - boundboxcenter.x;
    else if (v23 == 32)
        x = boundboxsize.x + boundboxcenter.x;
    float y = boundboxsize.y;
    if (v22 == 64)
        y = boundboxsize.y - boundboxcenter.y;
    else if (v22 == 128)
        y = boundboxsize.y + boundboxcenter.y;
    Broc::vector pos;
    pos.x = parent_matrix->w.v.m128_f32[0] + x;
    pos.y = parent_matrix->w.v.m128_f32[1] + y;
    pos.z = parent_matrix->w.v.m128_f32[2];
    FEText* v31;
    if (v43 == 1)
    {
        FEText* v29 = (FEText*)mem_heap_malloc(0x70u);
        v31 = v29 != nullptr
                  ? new (v29) FEText(Font, defaultFileName, pos.x, pos.y,
                                     (int)pos.z, PANEL_LAYER_TOTAL, scale,
                                     v23, v22, fi)
                  : nullptr;
    }
    else
    {
        FEMultiLineText* v32 = (FEMultiLineText*)mem_heap_malloc(0xA8u);
        if (v32 != nullptr)
            v32 = new (v32) FEMultiLineText(Font, pos.x, pos.y, (int)pos.z,
                                            PANEL_LAYER_TOTAL, scale, v23,
                                            v22, fi);
        v31 = v32;
        if (v32 != nullptr)
        {
            v32->SetNumLines(v43);
            v32->SetCutOffIfTooLong(true);
            v32->SetLineSpacing(line_spacing);
            v32->SetBoxWidth((int)(boundboxcenter.x * 2.0f));
        }
    }
    v31->SetName(name);
    v31->SetWidescreenAlign(widescreen_align);
    v31->xy_initial.x = x;
    v31->xy_initial.y = y;
    v31->xy_initial.z = 0.0f;
    v31->SetPanelTextIndex(panel_text_index);
    VectorPushBack(ptext, v31);
}

// ea: 0x00593A00
bool PanelFile::Load(const char* filename, unsigned char* buffer,
                     int buffer_size)
{
    (void)buffer_size;
    if (buffer == nullptr)
        return false;
    strncpy(mName, filename, 0x40u);
    math::Mat43 v11;
    v11.x.v.m128_f32[0] = 1.0f;
    v11.x.v.m128_f32[1] = 0.0f;
    v11.x.v.m128_f32[2] = 0.0f;
    v11.x.v.m128_f32[3] = 0.0f;
    v11.y.v.m128_f32[0] = 0.0f;
    v11.y.v.m128_f32[1] = 1.0f;
    v11.y.v.m128_f32[2] = 0.0f;
    v11.y.v.m128_f32[3] = 0.0f;
    v11.z.v.m128_f32[0] = 0.0f;
    v11.z.v.m128_f32[1] = 0.0f;
    v11.z.v.m128_f32[2] = 1.0f;
    v11.z.v.m128_f32[3] = 0.0f;
    v11.w.v.m128_f32[0] = 0.0f;
    v11.w.v.m128_f32[1] = 0.0f;
    v11.w.v.m128_f32[2] = 0.0f;
    v11.w.v.m128_f32[3] = 1.0f;
    int v12 = 0;
    int v10 = buffer[v12] | (buffer[v12 + 1] << 8) | (buffer[v12 + 2] << 16)
              | (buffer[v12 + 3] << 24);
    v12 += 4;
    if (v10 > 0)
    {
        do
        {
            LoadPanelGeom(buffer, v12, &v11);
            --v10;
        }
        while (v10 != 0);
    }
    return true;
}

// ea: 0x0058CCF0
void PanelFile::LoadPanelText2(unsigned char* buffer, int& index,
                               const math::Mat43* parent_matrix,
                               const char* name, short widescreen_align)
{
    Broc::string v57;
    ReadString(buffer, index, v57);
    unsigned char flags = buffer[index++];
    int line_spacing = buffer[index] | (buffer[index + 1] << 8)
                       | (buffer[index + 2] << 16) | (buffer[index + 3] << 24);
    index += 4;
    unsigned char scale_type = buffer[index++];
    float scale = *(float*)&buffer[index];
    index += 4;
    float scale_unselected = *(float*)&buffer[index];
    index += 4;
    Broc::string v56;
    ReadString(buffer, index, v56);
    unsigned char color_type = buffer[index++];
    color32 fi = ReadColor(buffer, index);
    color32 unselect_col = ReadColor(buffer, index);
    Broc::string v58;
    ReadString(buffer, index, v58);
    unsigned short scheme_index = (unsigned short)(buffer[index]
                                                   | (buffer[index + 1] << 8));
    index += 2;
    int panel_text_index = buffer[index] | (buffer[index + 1] << 8)
                           | (buffer[index + 2] << 16)
                           | (buffer[index + 3] << 24);
    index += 4;
    Broc::vector boundboxsize;
    Broc::vector boundboxcenter;
    ReadVector3d(boundboxsize, buffer, index);
    ReadVector3d(boundboxcenter, buffer, index);
    font_index Font = g_femanager.FindFont(v57.c_str(), false);
    if (Font == FONT_NORMAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
        AeAssert::gCurrentLine = 1949;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int v26 = 0;
    int v27 = 0;
    if ((flags & 3) != 0)
    {
        if ((flags & 3) == 1)
            v27 = 0;
        else if ((flags & 3) == 3)
            v27 = 32;
    }
    else
    {
        v27 = 16;
    }
    unsigned char v28 = flags & 0xC;
    if (v28 != 0)
    {
        if (v28 == 4)
            v26 = 0;
        else if (v28 == 12)
            v26 = 128;
    }
    else
    {
        v26 = 64;
    }
    float x = boundboxsize.x;
    if (v27 == 16)
        x = boundboxsize.x - boundboxcenter.x;
    else if (v27 == 32)
        x = boundboxsize.x + boundboxcenter.x;
    float y = boundboxsize.y;
    if (v26 == 64)
        y = boundboxsize.y - boundboxcenter.y;
    else if (v26 == 128)
        y = boundboxsize.y + boundboxcenter.y;
    Broc::vector pos;
    pos.x = parent_matrix->w.v.m128_f32[0] + x;
    pos.y = parent_matrix->w.v.m128_f32[1] + y;
    pos.z = parent_matrix->w.v.m128_f32[2];
    FEText* v35;
    if (scheme_index == 1)
    {
        FEText* v33 = (FEText*)mem_heap_malloc(0x70u);
        v35 = v33 != nullptr
                  ? new (v33) FEText(Font, defaultFileName, pos.x, pos.y,
                                     (int)pos.z, PANEL_LAYER_TOTAL, scale,
                                     v27, v26, fi)
                  : nullptr;
    }
    else
    {
        FEMultiLineText* v36 = (FEMultiLineText*)mem_heap_malloc(0xA8u);
        if (v36 != nullptr)
            v36 = new (v36) FEMultiLineText(Font, pos.x, pos.y, (int)pos.z,
                                            PANEL_LAYER_TOTAL, scale, v27,
                                            v26, fi);
        v35 = v36;
        if (v36 != nullptr)
        {
            v36->SetNumLines(scheme_index);
            v36->SetCutOffIfTooLong(true);
            v36->SetLineSpacing(line_spacing);
            v36->SetBoxWidth((int)(boundboxcenter.x * 2.0f));
        }
    }
    if (scale_type != 0)
    {
        if (scale_type == 1)
        {
            v35->SetScaleMenuItem(scale, scale_unselected);
        }
        else if (scale_type != 2)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
            AeAssert::gCurrentLine = 2020;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Unknown text scale type: %d", scale_type))
                __debugbreak();
        }
    }
    switch (color_type)
    {
    case 0:
    case 1:
        break;
    case 2:
        v35->SetColorMenuItem(fi, unselect_col);
        break;
    case 3:
    {
        int scheme = FEMenuColorScheme::GetSchemeFromText(v58);
        if (scheme == -1)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
            AeAssert::gCurrentLine = 2036;
            AeAssert::gCurrentExpr = "scheme_index != -1";
            if (!AeAssert::IsIgnored())
            {
                const char* v40 = v58.mBlock != nullptr
                                      ? (const char*)&v58.mBlock[1]
                                      : defaultFileName;
                if (AeAssert::Assert(
                        "Invalid panel color scheme: %s", v40))
                    __debugbreak();
            }
        }
        v35->SetColorMenuItem(color_schemes[scheme].high1,
                              color_schemes[scheme].unselect);
        break;
    }
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
        AeAssert::gCurrentLine = 2042;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Unknown text color type: %d", color_type))
            __debugbreak();
        break;
    }
    v35->SetName(name);
    v35->SetWidescreenAlign(widescreen_align);
    v35->xy_initial.x = x;
    v35->xy_initial.y = y;
    v35->xy_initial.z = 0.0f;
    v35->SetPanelTextIndex(panel_text_index);
    VectorPushBack(ptext, v35);
}

// ea: 0x0058BC10
void PanelQuad::Load(PanelMaterial* mats, unsigned char* buffer, int& index,
                     const math::Mat43* parent_matrix)
{
    unsigned short v9 = (unsigned short)(buffer[index]
                                         | (buffer[index + 1] << 8));
    index += 2;
    PanelMaterial* ind3 = &mats[v9];
    unsigned short n = (unsigned short)(buffer[index]
                                        | (buffer[index + 1] << 8));
    index += 2;
    Broc::vector* verts = (Broc::vector*)mem_heap_malloc(12 * n);
    if (verts != nullptr)
    {
        for (int j = 0; j < n; ++j)
        {
            verts[j].x = sNaN;
            verts[j].y = sNaN;
            verts[j].z = sNaN;
        }
    }
    color32* colors = (color32*)mem_heap_malloc(4 * n);
    if (colors != nullptr)
        memset(colors, 0, 4 * n);
    Broc::vector* uvs = (Broc::vector*)mem_heap_malloc(12 * n);
    if (uvs != nullptr)
    {
        for (int k = 0; k < n; ++k)
        {
            uvs[k].x = sNaN;
            uvs[k].y = sNaN;
            uvs[k].z = sNaN;
        }
    }
    for (int i = 0; i < n; ++i)
    {
        ReadVector3d(verts[i], buffer, index);
        color32 v97 = ReadColor(buffer, index);
        colors[i] = MultiplyColors(v97, ind3->color);
        float u = *(float*)&buffer[index];
        index += 4;
        uvs[i].x = u;
        float v = *(float*)&buffer[index];
        index += 4;
        uvs[i].y = v;
    }
    unsigned short v36 = (unsigned short)(buffer[index]
                                          | (buffer[index + 1] << 8));
    index += 2;
    if (v36 != 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
        AeAssert::gCurrentLine = 713;
        AeAssert::gCurrentExpr = "strip_count == 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned short v37 = (unsigned short)(buffer[index]
                                          | (buffer[index + 1] << 8));
    index += 2;
    short* didxs = (short*)mem_heap_malloc(2 * v37);
    for (int m = 0; m < v37; ++m)
    {
        didxs[m] = (short)(buffer[index] | (buffer[index + 1] << 8));
        index += 2;
    }
    if (n < 3)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
        AeAssert::gCurrentLine = 721;
        AeAssert::gCurrentExpr = "nwedges >= 3";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    math::Mat43 matcopy = *parent_matrix;
    // IDA's frame has these two four-vector regions back-to-back.  The
    // fourth tmp_initial entry is followed by tmp_wed[0..2] when the quad
    // section consumes four vertices.
    Broc::vector panel_scratch[8];
    Broc::vector* tmp_initial = &panel_scratch[0];
    Broc::vector* tmp_wed = &panel_scratch[4];
    int* wedge_indices = reinterpret_cast<int*>(&tmp_wed[3]);
    Broc::vector uv_out[4];
    for (int i = 0; i < 4; ++i)
    {
        tmp_initial[i].x = sNaN;
        tmp_initial[i].y = sNaN;
        tmp_initial[i].z = sNaN;
        if (i < 3)
        {
            tmp_wed[i].x = sNaN;
            tmp_wed[i].y = sNaN;
            tmp_wed[i].z = sNaN;
        }
    }
    float v93 = 0.0f;
    int num_tri = v37 / 3;
    int tri1 = 0;
    int v97 = 1;
    short* z = didxs;
    if (num_tri > 0)
    {
        for (;;)
        {
            if (v97 >= num_tri)
                goto LABEL_39;
            if (CountSharedVertices(didxs, tri1, v97,
                                    wedge_indices) != 2)
                goto LABEL_39;
            ++tri1;
            ++v97;
            z += 6;
        LABEL_40:
            color32 mat[4];
            for (int v51 = 0; v51 < 4; ++v51)
            {
                int idx = wedge_indices[v51];
                Broc::vector vert = verts[idx];
                tmp_initial[v51 + 3].x =
                    matcopy.x.v.m128_f32[0] * vert.x
                    + matcopy.y.v.m128_f32[0] * vert.y
                    + matcopy.z.v.m128_f32[0] * vert.z
                    + matcopy.w.v.m128_f32[0];
                tmp_initial[v51 + 3].y =
                    matcopy.x.v.m128_f32[1] * vert.x
                    + matcopy.y.v.m128_f32[1] * vert.y
                    + matcopy.z.v.m128_f32[1] * vert.z
                    + matcopy.w.v.m128_f32[1];
                v93 = matcopy.x.v.m128_f32[2] * vert.x
                      + matcopy.y.v.m128_f32[2] * vert.y
                      + matcopy.z.v.m128_f32[2] * vert.z
                      + matcopy.w.v.m128_f32[2];
                uv_out[v51] = uvs[idx];
                mat[v51] = colors[idx];
            }
            PanelQuadSection* v67 =
                (PanelQuadSection*)mem_heap_malloc(0x68u);
            if (v67 != nullptr)
            {
                v67->quad.Tex = nullptr;
                v67->quad.Z = 0.0f;
            }
            v67->AddPQSection(&tmp_initial[3], uv_out, mat, v93);
            for (int v68 = 0; v68 < 4; ++v68)
            {
                v67->x_initial[v68] = (short)tmp_initial[v68 + 3].x;
                v67->y_initial[v68] = (short)tmp_initial[v68 + 3].y;
            }
            VectorPushBack(pqs, v67);
            ++tri1;
            ++v97;
            z += 6;
            if (tri1 >= num_tri)
                goto LABEL_47;
            continue;
        LABEL_39:
            wedge_indices[0] = ((short*)z)[0];
            wedge_indices[1] = ((short*)z)[1];
            wedge_indices[2] = ((short*)z)[2];
            wedge_indices[3] = wedge_indices[2];
            goto LABEL_40;
        }
    }
LABEL_47:
    SetZvalueAbs(v93);
    unsigned int v71 = ind3->bilinearfilter != 0;
    if (!ind3->wrapu)
        v71 |= 0x40;
    if (!ind3->wrapv)
        v71 |= 0x80;
    if (ind3->texture != nullptr && ind3->hasmap)
        SetTexture(ind3->texture);
    if (v71 != 0)
        SetMaterialFlags(v71);
    mem_heap_free(verts);
    mem_heap_free(uvs);
    mem_heap_free(colors);
    mem_heap_free(didxs);
    Broc::vector min_coords = GetMin();
    Broc::vector Max = GetMax();
    float y = Max.y;
    float v12 = min_coords.z;
    center_point.x = ((Max.x - min_coords.x) * 0.5f) + min_coords.x;
    center_point.y = ((y - min_coords.y) * 0.5f) + min_coords.y;
    center_point.z = ((Max.z - v12) * 0.5f) + v12;
}

// ============================================================================
// FEManager
// ============================================================================

// ea: 0x005852E0
font_index FEManager::FindFont(const char* font_filename, bool checkfileext)
{
    ae_fixed_string<32, unsigned char> fontName(font_filename);
    if (font_filename == nullptr || fontName.mBuff[0] == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.cpp";
        AeAssert::gCurrentLine = 389;
        AeAssert::gCurrentExpr = "!fontName.empty() && fontName[0]";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("dont pass in an empty input foo!"))
            __debugbreak();
    }
    if (fontName.find('\\', 0) != -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.cpp";
        AeAssert::gCurrentLine = 390;
        AeAssert::gCurrentExpr = "fontName.find( '\\\\') == -1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("dont pass in paths foo!"))
            __debugbreak();
    }
    if (checkfileext)
    {
        int v4 = fontName.find('.', 0);
        if (v4 != -1)
        {
            fontName.mBuff[v4] = 0;
            fontName.mLength = (unsigned char)v4;
        }
    }
    for (int v5 = 0; v5 < 4; ++v5)
    {
        if (_stricmp((const char*)fontName.mBuff,
                     (const char*)font_name_array[v5].mBuff) == 0)
            return (font_index)v5;
    }
    return FONT_NORMAL;
}

// ea: 0x00593D90
PanelQuad* FEManager::GetDefaultPQ()
{
    if (default_pq == nullptr)
    {
        PanelQuad* v3 = (PanelQuad*)mem_heap_malloc(0x48u);
        if (v3 != nullptr)
            v3 = new (v3) PanelQuad((char*)"default_PQ");
        float xy[12] = {
            0.0f, 0.0f, 0.0f, 320.0f, 0.0f, 0.0f,
            320.0f, 320.0f, 0.0f, 0.0f, 320.0f, 0.0f,
        };
        color32 col[7];
        for (int i = 0; i < 7; ++i)
            col[i].i = 0xFF808080u;
        default_pq = v3;
        v3->Init((Broc::vector*)xy, col, PANEL_LAYER_BACKGROUND, 0.0f,
                 defaultFileName);
    }
    return default_pq;
}

// ============================================================================
// Data (shell.o)
// ============================================================================

// ?color_schemes@@3PAVFEMenuColorScheme@@A @ 0xDF3AE0 (values from $E18_2)
FEMenuColorScheme color_schemes[17] = {
    { 0xFFB4B4B4u, 0xFF8C3232u, 0xFFB4B4B4u, true },
    { 0x8C8C8C8Cu, 0xFFB4B4B4u, 0x8C8C8C8Cu, true },
    { 0xFFB4B4B4u, 0xFFFFFFFFu, 0xFFB4B4B4u, true },
    { 0xFF000000u, 0xFF8C3232u, 0xFF000000u, true },
    { 0xFFFFFFFFu, 0xFF8C3232u, 0xFFFFFFFFu, true },
    { 0xAA282119u, 0xFF282119u, 0xAA282119u, true },
    { 0x8C1F2C2Cu, 0xFF1F2C2Cu, 0xFF1F2C2Cu, false },
    { 0x8C491C1Bu, 0xFF491C1Bu, 0xFF491C1Bu, false },
    { 0xFFB4B4B4u, 0xFFB4B4B4u, 0xFFB4B4B4u, false },
    { 0xFFA89C8Du, 0xFFEBBA50u, 0xFFEBBA50u, false },
    { 0x80D6C4AAu, 0xFFD6C4AAu, 0x80D6C4AAu, true },
    { 0xFFC9B18Fu, 0xFFC9B18Fu, 0xFFC9B18Fu, false },
    { 0xFFBDA077u, 0xFFBDA077u, 0xFFBDA077u, false },
    { 0xFFB18E5Du, 0xFFB18E5Du, 0xFFB18E5Du, false },
    { 0xFF8E6F45u, 0xFF8E6F45u, 0xFF8E6F45u, false },
    { 0xFF755C39u, 0xFF755C39u, 0xFF755C39u, false },
    { 0xFF5A472Cu, 0xFF5A472Cu, 0xFF5A472Cu, false },
};

// ?font_name_array@FEManager@@0PAV?$ae_fixed_string@$0CA@E@@A @ 0xF382C0
ae_fixed_string<32, unsigned char> FEManager::font_name_array[4] = {
    ae_fixed_string<32, unsigned char>("garamond"),
    ae_fixed_string<32, unsigned char>("i_button_icons_xb"),
    ae_fixed_string<32, unsigned char>("i_helvetica_bold"),
    ae_fixed_string<32, unsigned char>("i_helvetica_bold_italic"),
};

// ============================================================================
// PanelFile
// ============================================================================

// ea: 0x005843D0
PanelFile::PanelFile()
{
    pquads.mElements = nullptr;
    pquads.mCapacity = 0;
    pquads.mSize = 0;
    ptext.mElements = nullptr;
    ptext.mCapacity = 0;
    ptext.mSize = 0;
    indexHidden = -1;
    hideText = false;
}

// ea: 0x005B74E0 (inline COMDAT)
PanelFile::~PanelFile()
{
    Cleanup();
    if (ptext.mElements != nullptr)
    {
        tlMemFree(ptext.mElements);
        ptext.mElements = nullptr;
        ptext.mCapacity = 0;
    }
    if (pquads.mElements != nullptr)
    {
        tlMemFree(pquads.mElements);
        pquads.mElements = nullptr;
        pquads.mCapacity = 0;
    }
}

// ea: 0x0058C4C0
void PanelFile::Cleanup()
{
    for (int i = 0; i < pquads.mSize; ++i)
    {
        if (pquads.mElements[i] != nullptr)
            delete pquads.mElements[i];
    }
    pquads.mSize = 0;
    for (int j = 0; j < ptext.mSize; ++j)
    {
        if (ptext.mElements[j] != nullptr)
            delete ptext.mElements[j];
    }
    ptext.mSize = 0;
}

// ea: 0x0058C5B0
PanelFile* PanelFile::Clone()
{
    PanelFile* v15 = (PanelFile*)mem_heap_malloc(0x60u);
    if (v15 != nullptr)
    {
        v15->pquads.mElements = nullptr;
        v15->pquads.mCapacity = 0;
        v15->pquads.mSize = 0;
        v15->ptext.mElements = nullptr;
        v15->ptext.mCapacity = 0;
        v15->ptext.mSize = 0;
        v15->indexHidden = -1;
        v15->hideText = false;
    }
    for (int i = 0; i < pquads.mSize; ++i)
    {
        PanelQuad* v3 = (PanelQuad*)mem_heap_malloc(0x48u);
        if (v3 != nullptr)
        {
            v3 = new (v3) PanelQuad();
            v3->CopyFrom(pquads.mElements[i]);
        }
        VectorPushBack(v15->pquads, v3);
    }
    for (int j = 0; j < ptext.mSize; ++j)
    {
        FEText* iElement = ptext.mElements[j]->Clone();
        VectorPushBack(v15->ptext, iElement);
    }
    v15->indexHidden = indexHidden;
    v15->hideText = hideText;
    strncpy(v15->mName, mName, sizeof(mName) - 1);
    v15->mName[sizeof(mName) - 1] = 0;
    return v15;
}

// ea: 0x0057B900
void PanelFile::Draw()
{
    for (int i = 0; i < pquads.mSize; ++i)
    {
        if (i != indexHidden)
            pquads.mElements[i]->Draw();
    }
    if (!hideText)
    {
        for (int j = 0; j < ptext.mSize; ++j)
        {
            if (!ptext.mElements[j]->IsOnMenu())
                ptext.mElements[j]->Draw();
        }
    }
}

// ea: 0x0057BA40
void PanelFile::Update(float time_inc)
{
    for (int i = 0; i < pquads.mSize; ++i)
        pquads.mElements[i]->Update(time_inc);
}

// ea: 0x0057BAC0
void PanelFile::UpdateWidescreen(bool widescreen, float about_x)
{
    for (int i = 0; i < pquads.mSize; ++i)
        pquads.mElements[i]->FattenMeForWidescreen(widescreen, about_x);
    for (int j = 0; j < ptext.mSize; ++j)
        ptext.mElements[j]->UpdateForWidescreen(widescreen);
}

// ea: 0x0057BBB0
void PanelFile::UpdateSplitScreen(int viewport, int old_viewport)
{
    if (viewport != old_viewport)
    {
        for (int i = 0; i < pquads.mSize; ++i)
            pquads.mElements[i]->FormatForSplitScreen(viewport,
                                                      old_viewport);
        for (int j = 0; j < ptext.mSize; ++j)
            ptext.mElements[j]->UpdateForSplitScreen(viewport, old_viewport);
    }
}

// ea: 0x0057BCB0
void PanelFile::MoveSplitScreen(int viewport, int old_viewport)
{
    if (viewport != old_viewport)
    {
        for (int i = 0; i < pquads.mSize; ++i)
            pquads.mElements[i]->MoveForSplitScreen(viewport, old_viewport);
        for (int j = 0; j < ptext.mSize; ++j)
            ptext.mElements[j]->MoveForSplitScreen(viewport, old_viewport);
    }
}

// ea: 0x0057BDB0
FEText* PanelFile::GetTextPointer(const char* search_name)
{
    if (ptext.mSize == 0)
        return nullptr;
    int v3 = 0;
    while (1)
    {
        Broc::string v9 = ptext.mElements[v3]->GetName();
        const char* v6 = v9.mBlock != nullptr
                             ? (const char*)&v9.mBlock[1]
                             : defaultFileName;
        if (strcmp(v6, search_name) == 0)
            break;
        if (++v3 >= ptext.mSize)
            return nullptr;
    }
    return ptext.mElements[v3];
}

// ea: 0x0057BEE0
void PanelFile::ReadPanelMaterial(PanelMaterial& mat, unsigned char* buffer,
                                  int& index)
{
    mat.color = ReadColor(buffer, index);
    unsigned char v9 = buffer[index++];
    mat.hasmap = v9 != 0;
    if (v9 != 0)
    {
        mat.filename = ReadStringPointer(buffer, index);
        mat.texture = LocalizedGetTexture(mat.filename);
        mat.bilinearfilter = buffer[index++] != 0;
        mat.wrapu = buffer[index++] != 0;
        mat.wrapv = buffer[index++] != 0;
    }
}

// ea: 0x0057BF80
int PanelFile::FindPanelQuadByPointer(PanelQuad* the_pointer)
{
    PanelQuad** mElements = pquads.mElements;
    PanelQuad** v3 = mElements;
    PanelQuad** v4 = &pquads.mElements[pquads.mSize];
    if (mElements == v4)
        return -1;
    while (the_pointer != *v3)
    {
        if (++v3 == v4)
            return -1;
    }
    return (int)(v3 - mElements);
}

// ea: 0x0057BFC0
int PanelFile::FindFETextByPointer(FEText* the_pointer)
{
    FEText** mElements = ptext.mElements;
    FEText** v3 = mElements;
    FEText** v4 = &mElements[ptext.mSize];
    if (mElements == v4)
        return -1;
    while (the_pointer != *v3)
    {
        if (++v3 == v4)
            return -1;
    }
    return (int)(v3 - mElements);
}

// ea: 0x0056ADC0
void PanelFile::PreMashFixup()
{
}

// ea: 0x0057B730
void PanelFile::PostUnmashFixup(panel_layer layer)
{
    for (int i = 0; i < pquads.mSize; ++i)
        pquads.mElements[i]->SetZvalue(pquads.mElements[i]->GetZvalue(),
                                       layer);
    for (int j = 0; j < ptext.mSize; ++j)
        ptext.mElements[j]->SetZvalue(ptext.mElements[j]->GetZvalue(),
                                      layer);
}

// ea: 0x00594790
PanelQuad* PanelFile::GetPointer(const char* search_name)
{
    if (pquads.mSize == 0)
        return g_femanager.GetDefaultPQ();
    int v4 = 0;
    while (1)
    {
        Broc::string& nm = pquads.mElements[v4]->name;
        const char* v6 = nm.mBlock != nullptr
                             ? (const char*)&nm.mBlock[1]
                             : defaultFileName;
        if (strcmp(v6, search_name) == 0)
            break;
        if (++v4 >= pquads.mSize)
            return g_femanager.GetDefaultPQ();
    }
    return pquads.mElements[v4];
}

// ea: 0x005948C0
PanelAnimObject* PanelFile::FindAnimObject(const char* search_name)
{
    PanelQuad* Pointer = GetPointer(search_name);
    if (Pointer == g_femanager.GetDefaultPQ())
        return GetTextPointer(search_name);
    return Pointer;
}

// ea: 0x0059ABC0
void PanelFile::HideQuad(const char* name)
{
    PanelQuad* Pointer = GetPointer(name);
    PanelQuad** mElements = pquads.mElements;
    PanelQuad** v5 = &pquads.mElements[pquads.mSize];
    if (pquads.mElements == v5)
    {
        indexHidden = -1;
    }
    else
    {
        while (Pointer != *mElements)
        {
            if (++mElements == v5)
            {
                indexHidden = -1;
                return;
            }
        }
        indexHidden = (int)(mElements - pquads.mElements);
    }
}

// ea: 0x0059AC10
void PanelFile::SetQuadVisible(const char* name, bool visible)
{
    PanelQuad* Pointer = GetPointer(name);
    PanelQuad** mElements = pquads.mElements;
    PanelQuad** v6 = &pquads.mElements[pquads.mSize];
    int v7;
    if (pquads.mElements == v6)
    {
        v7 = -1;
    }
    else
    {
        while (Pointer != *mElements)
        {
            if (++mElements == v6)
            {
                v7 = -1;
                goto found;
            }
        }
        v7 = (int)(mElements - pquads.mElements);
    }
found:
    pquads.mElements[v7]->SetShown(visible);
}
