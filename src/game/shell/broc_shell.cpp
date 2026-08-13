// ============================================================================
// broc_shell.cpp - PanelQuadSection/PanelQuad bounds + binary readers (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"

#include <string.h>

extern float sNaN;  // ?sNaN@@3MA @ 0x10F19D0
extern const char* const defaultFileName;  // 0xCD67AE

// ============================================================================
// PanelQuadSection bounds
// ============================================================================

// ea: 0x005696F0
void PanelQuadSection::SetInitialXY(Broc::vector* tmp_initial)
{
    for (int i = 0; i < 4; ++i)
    {
        x_initial[i] = (short)tmp_initial->x;
        y_initial[i] = (short)tmp_initial->y;
        ++tmp_initial;
    }
}

// ea: 0x00569B30
void PanelQuadSection::SetUV(Broc::vector* uv)
{
    quad.Verts[0].U = uv[0].x;
    quad.Verts[0].V = uv[0].y;
    quad.Verts[1].U = uv[1].x;
    quad.Verts[1].V = uv[1].y;
    quad.Verts[2].U = uv[2].x;
    quad.Verts[2].V = uv[2].y;
    quad.Verts[3].U = uv[3].x;
    quad.Verts[3].V = uv[3].y;
}

// ea: 0x00569BB0
void PanelQuadSection::SetPos(Broc::vector* xy)
{
    quad.Verts[0].X = xy[0].x;
    quad.Verts[0].Y = xy[0].y;
    quad.Verts[1].X = xy[1].x;
    quad.Verts[1].Y = xy[1].y;
    quad.Verts[2].X = xy[2].x;
    quad.Verts[2].Y = xy[2].y;
    quad.Verts[3].X = xy[3].x;
    quad.Verts[3].Y = xy[3].y;
}

// ea: 0x00579550
void PanelQuadSection::AddPQSection(Broc::vector* xy, Broc::vector* uv,
                                    color32* col, float z)
{
    for (int v5 = 0; v5 < 4; ++v5)
    {
        x_initial[v5] = (short)xy[v5].x;
        y_initial[v5] = (short)xy[v5].y;
        quad.Verts[v5].X = xy[v5].x;
        quad.Verts[v5].Y = xy[v5].y;
        quad.Verts[v5].U = uv[v5].x;
        quad.Verts[v5].V = uv[v5].y;
        quad.Verts[v5].Color = col[v5].i;
    }
    quad.Z = z;
}

// ea: 0x00569AB0
void PanelQuadSection::SetColorVert(int i, color32 c)
{
    quad.Verts[i].Color = c.c.b | ((c.c.g | (c.c.r << 8)) << 8);
}

// ea: 0x00579800
void PanelQuadSection::SetColorNAVert(int i, color32 c)
{
    quad.Verts[i].Color =
        c.c.b | ((c.c.g | ((c.c.r | ((quad.Verts[i].Color >> 24) << 8)) << 8))
                << 8);
}

// ea: 0x00579900
color32 PanelQuadSection::GetColor(int index)
{
    color32 result;
    result.i = quad.Verts[index].Color;
    return result;
}

// ea: 0x00569C60
Broc::vector PanelQuadSection::GetMax()
{
    Broc::vector max_coords;
    max_coords.z = sNaN;
    float X = quad.Verts[1].X;
    float v3 = quad.Verts[3].X;
    if (quad.Verts[2].X > v3)
        v3 = quad.Verts[2].X;
    if (quad.Verts[0].X > X)
        X = quad.Verts[0].X;
    max_coords.x = X;
    if (X <= v3)
        max_coords.x = v3;
    float Y = quad.Verts[3].Y;
    float v5 = quad.Verts[1].Y;
    if (quad.Verts[2].Y > Y)
        Y = quad.Verts[2].Y;
    if (quad.Verts[0].Y > v5)
        v5 = quad.Verts[0].Y;
    max_coords.y = v5;
    if (v5 <= Y)
        max_coords.y = Y;
    return max_coords;
}

// ea: 0x00569D00
Broc::vector PanelQuadSection::GetMin()
{
    Broc::vector min_coords;
    min_coords.z = sNaN;
    float X = quad.Verts[1].X;
    float v3 = quad.Verts[3].X;
    if (v3 > quad.Verts[2].X)
        v3 = quad.Verts[2].X;
    if (X > quad.Verts[0].X)
        X = quad.Verts[0].X;
    min_coords.x = X;
    if (v3 <= X)
        min_coords.x = v3;
    float Y = quad.Verts[3].Y;
    float v5 = quad.Verts[1].Y;
    if (Y > quad.Verts[2].Y)
        Y = quad.Verts[2].Y;
    if (v5 > quad.Verts[0].Y)
        v5 = quad.Verts[0].Y;
    min_coords.y = v5;
    if (Y <= v5)
        min_coords.y = Y;
    return min_coords;
}

// ea: 0x00569DA0
Broc::vector PanelQuadSection::GetInitialMax()
{
    Broc::vector max_coords;
    max_coords.z = sNaN;
    float v2 = (float)x_initial[2];
    float v3 = (float)x_initial[3];
    float v4 = (float)x_initial[1];
    if (v2 > v3)
        v3 = v2;
    if ((float)x_initial[0] > v4)
        v4 = (float)x_initial[0];
    max_coords.x = v4;
    if (v4 <= v3)
        max_coords.x = v3;
    float v5 = (float)y_initial[2];
    float v6 = (float)y_initial[3];
    float v7 = (float)y_initial[1];
    if (v5 > v6)
        v6 = v5;
    if ((float)y_initial[0] > v7)
        v7 = (float)y_initial[0];
    max_coords.y = v7;
    if (v7 <= v6)
        max_coords.y = v6;
    return max_coords;
}

// ea: 0x00569E50
Broc::vector PanelQuadSection::GetInitialMin()
{
    Broc::vector min_coords;
    min_coords.z = sNaN;
    float v2 = (float)x_initial[2];
    float v3 = (float)x_initial[3];
    float v4 = (float)x_initial[1];
    if (v3 > v2)
        v3 = v2;
    if (v4 > (float)x_initial[0])
        v4 = (float)x_initial[0];
    min_coords.x = v4;
    if (v3 <= v4)
        min_coords.x = v3;
    float v5 = (float)y_initial[2];
    float v6 = (float)y_initial[3];
    float v7 = (float)y_initial[1];
    if (v6 > v5)
        v6 = v5;
    if (v7 > (float)y_initial[0])
        v7 = (float)y_initial[0];
    min_coords.y = v7;
    if (v6 <= v7)
        min_coords.y = v6;
    return min_coords;
}

// ea: 0x00569F00
Broc::vector PanelQuadSection::GetMaxUV()
{
    Broc::vector result;
    float U = quad.Verts[0].U;
    float V = quad.Verts[0].V;
    result.x = U;
    result.y = V;
    if (quad.Verts[1].U > U)
    {
        U = quad.Verts[1].U;
        result.x = U;
    }
    if (quad.Verts[1].V > V)
    {
        V = quad.Verts[1].V;
        result.y = V;
    }
    if (quad.Verts[2].U > U)
    {
        U = quad.Verts[2].U;
        result.x = U;
    }
    if (quad.Verts[2].V > V)
    {
        V = quad.Verts[2].V;
        result.y = V;
    }
    if (quad.Verts[3].U > U)
        result.x = quad.Verts[3].U;
    if (quad.Verts[3].V > V)
        result.y = quad.Verts[3].V;
    result.z = 0.0f;
    return result;
}

// ea: 0x00569FB0
Broc::vector PanelQuadSection::GetMinUV()
{
    Broc::vector result;
    float U = quad.Verts[0].U;
    float V = quad.Verts[0].V;
    result.x = U;
    result.y = V;
    if (U > quad.Verts[1].U)
    {
        U = quad.Verts[1].U;
        result.x = U;
    }
    if (V > quad.Verts[1].V)
    {
        V = quad.Verts[1].V;
        result.y = V;
    }
    if (U > quad.Verts[2].U)
    {
        U = quad.Verts[2].U;
        result.x = U;
    }
    if (V > quad.Verts[2].V)
    {
        V = quad.Verts[2].V;
        result.y = V;
    }
    if (U > quad.Verts[3].U)
        result.x = quad.Verts[3].U;
    if (V > quad.Verts[3].V)
        result.y = quad.Verts[3].V;
    result.z = 0.0f;
    return result;
}

// ============================================================================
// PanelQuad bounds (aggregate over sections)
// ============================================================================

// ea: 0x0057AC40
Broc::vector PanelQuad::GetMax()
{
    Broc::vector max_coords;
    max_coords.z = sNaN;
    if (pqs.mSize <= 0)
        return max_coords;
    max_coords = pqs.mElements[0]->GetMax();
    for (int v8 = 1; v8 < pqs.mSize; ++v8)
    {
        Broc::vector v22 = pqs.mElements[v8]->GetMax();
        if (v22.x > max_coords.x)
            max_coords.x = v22.x;
        Broc::vector v21 = pqs.mElements[v8]->GetInitialMax();
        if (v21.y > max_coords.y)
            max_coords.y = v21.y;
    }
    return max_coords;
}

// ea: 0x0057AF20
Broc::vector PanelQuad::GetMin()
{
    Broc::vector min_coords;
    min_coords.z = sNaN;
    if (pqs.mSize <= 0)
        return min_coords;
    min_coords = pqs.mElements[0]->GetMin();
    for (int v8 = 1; v8 < pqs.mSize; ++v8)
    {
        Broc::vector v22 = pqs.mElements[v8]->GetMin();
        if (min_coords.x > v22.x)
            min_coords.x = v22.x;
        Broc::vector v21 = pqs.mElements[v8]->GetInitialMin();
        if (min_coords.y > v21.y)
            min_coords.y = v21.y;
    }
    return min_coords;
}

// ea: 0x0057B200
Broc::vector PanelQuad::GetInitialMax()
{
    Broc::vector max_coords;
    max_coords.z = sNaN;
    if (pqs.mSize <= 0)
        return max_coords;
    max_coords = pqs.mElements[0]->GetInitialMax();
    for (int v3 = 1; v3 < pqs.mSize; ++v3)
    {
        Broc::vector v23 = pqs.mElements[v3]->GetInitialMax();
        if (v23.x > max_coords.x)
            max_coords.x = v23.x;
        Broc::vector v21 = pqs.mElements[v3]->GetInitialMax();
        if (v21.y > max_coords.y)
            max_coords.y = v21.y;
    }
    return max_coords;
}

// ea: 0x0057B490
Broc::vector PanelQuad::GetInitialMin()
{
    Broc::vector min_coords;
    min_coords.z = sNaN;
    if (pqs.mSize <= 0)
        return min_coords;
    min_coords = pqs.mElements[0]->GetInitialMin();
    for (int v3 = 1; v3 < pqs.mSize; ++v3)
    {
        Broc::vector v23 = pqs.mElements[v3]->GetInitialMin();
        if (min_coords.x > v23.x)
            min_coords.x = v23.x;
        Broc::vector v21 = pqs.mElements[v3]->GetInitialMin();
        if (min_coords.y > v21.y)
            min_coords.y = v21.y;
    }
    return min_coords;
}

// ============================================================================
// Binary readers
// ============================================================================

// ea: 0x0056AF00
void ReadString(unsigned char* buffer, int& index, Broc::string& ret)
{
    short v4 = *(short*)&buffer[index];
    index += 2;
    if (v4 > 0)
    {
        for (int i = v4; i != 0; --i)
        {
            char c = (char)buffer[index++];
            ret += c;
        }
    }
}

// ea: 0x0056AFA0
void ReadVector3d(Broc::vector& v, unsigned char* buffer, int& index)
{
    int v5 = index + 4;
    unsigned int x = buffer[index]
                     | ((buffer[index + 1] | ((buffer[index + 2] | (buffer[index + 3] << 8)) << 8)) << 8);
    index = v5;
    v.x = *(float*)&x;
    int v7 = index + 4;
    unsigned int y = buffer[index]
                     | ((buffer[index + 1] | ((buffer[index + 2] | (buffer[index + 3] << 8)) << 8)) << 8);
    index = v7;
    v.y = *(float*)&y;
    int v9 = index + 4;
    unsigned int z = buffer[index]
                     | ((buffer[index + 1] | ((buffer[index + 2] | (buffer[index + 3] << 8)) << 8)) << 8);
    index = v9;
    v.z = *(float*)&z;
}

// ea: 0x0056B050
void ReadVector2d(Broc::vector& v, unsigned char* buffer, int& index)
{
    int v5 = index + 4;
    unsigned int x = buffer[index]
                     | ((buffer[index + 1] | ((buffer[index + 2] | (buffer[index + 3] << 8)) << 8)) << 8);
    index = v5;
    v.x = *(float*)&x;
    int v7 = index;
    unsigned int y = buffer[index]
                     | ((buffer[index + 1] | ((buffer[index + 2] | (buffer[index + 3] << 8)) << 8)) << 8);
    index = v7 + 4;
    v.y = *(float*)&y;
}

// ============================================================================
// FEMultiLineText / FEMenuListBoxItem helpers
// ============================================================================

// ea: 0x0056E670
Broc::string FEMultiLineText::ReplaceEndlines(Broc::string t)
{
    for (int i = t.find("\\n", 0); i > 0; i = t.find("\\n", i + 2))
    {
        t.set_char((unsigned int)i, ' ');
        t.set_char((unsigned int)(i + 1), '\n');
    }
    return Broc::string(t);
}

// ea: 0x00571D90
const Broc::string& FEMenuListBoxItem::GetSubItem(unsigned int index)
{
    if (index >= mSubItemCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenuListBox.cpp";
        AeAssert::gCurrentLine = 20;
        AeAssert::gCurrentExpr = "index < mSubItemCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return mSubItems[index];
}
