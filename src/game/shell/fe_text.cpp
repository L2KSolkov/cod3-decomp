// ============================================================================
// fe_text.cpp - FETextFlashInfo, FEMenuColorScheme, MultiLineString (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"

#include <math.h>
#include <string.h>

#include "ngl/nglFont.h"

extern float sNaN;                       // ?sNaN@@3MA @ 0x10F19D0
extern int cg_widescreen_integer;        // cg.o
float widescreen_scale_0 = 0.75f;        // shell.o data @ 0xF30D00-ish
extern const char defaultFileName[];  // 0xCD67AE
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);            // core.o
extern bool CompareButton(const char* text, const char* button,
                          const char** buttonCode,
                          const char* would_be_button_code,
                          int& length);  // fe_util.cpp

extern FEManager g_femanager;

namespace View {
float GetXScalingForHUD(int window);   // cg.o
float GetYScalingForHUD(int window);   // cg.o
float GetPreviousHUDXPos(float pos, int window, char justification,
                         float width);   // cg.o
float GetCurrentHUDXPos(float pos, int window, char justification,
                        float width);    // cg.o
float GetCurrentHUDYPos(float pos, int window, char justification,
                        float height);   // cg.o
float GetPreviousHUDYPos(float pos, int window, char justification,
                         float height);  // cg.o
}
extern float GetYScalingForWindow(int window);  // fe_util.cpp
float widescreen_scale = 0.75f;                // shell.o data @ 0xDF43C8

class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A
    const char* GetSTBString(const char* pszReference);  // core.o
    const char* GetSTBString(unsigned int hash);         // core.o
};

// ============================================================================
// FEText (112 bytes - ui_types.h verified)
// ============================================================================

// ea: 0x005AD690
FEText::FEText()
    : PanelAnimObject(), flash_info(nullptr), text(), name()
{
    PanelAnimObject::flags = 4;
    visibility = 1.0f;
    xy_initial.x = sNaN;
    xy_initial.y = sNaN;
    xy_initial.z = sNaN;
    xy.x = sNaN;
    xy.y = sNaN;
    xy.z = sNaN;
    scale.x = sNaN;
    scale.y = sNaN;
    scale.z = sNaN;
    scale_init.x = sNaN;
    scale_init.y = sNaN;
    scale_init.z = sNaN;
    scale_unselected.x = sNaN;
    scale_unselected.y = sNaN;
    scale_unselected.z = sNaN;
    color1.i = 0;
    color_unselected.i = 0;
}

// ea: 0x0056BA80
FEText::FEText(font_index f, const char* t, float x1, float y1, int z1,
               panel_layer layer, float s, int horizJust, int vertJust,
               color32 col)
{
    PanelAnimObject::flags = 4;
    visibility = 1.0f;
    text = Broc::string((Broc::string::Block*)nullptr);
    xy_initial.x = sNaN;
    xy_initial.y = sNaN;
    xy_initial.z = sNaN;
    xy.x = sNaN;
    xy.y = sNaN;
    xy.z = sNaN;
    scale.x = sNaN;
    scale.y = sNaN;
    scale.z = sNaN;
    scale_init.x = sNaN;
    scale_init.y = sNaN;
    scale_init.z = sNaN;
    scale_unselected.x = sNaN;
    scale_unselected.y = sNaN;
    scale_unselected.z = sNaN;
    color1.i = 0;
    color_unselected.i = 0;
    name = Broc::string((Broc::string::Block*)nullptr);
    font = f;
    text = t;
    float v14 = (float)z1;
    xy.x = x1;
    xy.y = y1;
    xy.z = 0.0f;
    if (layer == PANEL_LAYER_TOTAL)
    {
        z_value = v14;
    }
    else
    {
        if (v14 < 0.0f)
            v14 = 0.0f;
        else if (v14 > 1000.0f)
            v14 = 1000.0f;
        SetZvalueAbs((float)((1000 * layer) + v14) * 0.11111111f);
    }
    scale.x = s;
    scale_unselected.x = s;
    scale.y = s;
    scale_unselected.y = s;
    scale_init.x = s;
    scale.z = 0.0f;
    scale_unselected.z = 0.0f;
    scale_init.y = s;
    panel_text_index = -1;
    scale_init.z = 0.0f;
    color1 = col;
    color_unselected = col;
    name = defaultFileName;
    unsigned char a = color1.c.a;
    flash_info = nullptr;
    flags = (int16_t)(horizJust | vertJust | 4);
    if (a != 0)
        flags = (int16_t)(horizJust | vertJust | 5);
}

// ea: 0x005ADD00
font_index FEText::GetFont()
{
    return font;
}

// ea: 0x0056BCE0
FEText::~FEText()
{
    FETextFlashInfo* flash_info = this->flash_info;
    if (flash_info != nullptr)
    {
        mem_heap_free(flash_info);
        this->flash_info = nullptr;
    }
}

// ea: 0x0056BD50
FEText* FEText::Clone()
{
    FEText* v2 = (FEText*)mem_heap_malloc(0x70u);
    FEText* v3 = v2 != nullptr ? new (v2) FEText() : nullptr;
    if (v3 != nullptr)
        v3->CopyFrom(this);
    return v3;
}

// ea: 0x005ADE90
int FEText::ConvertColor(color32 c)
{
    return c.c.b | ((c.c.g | ((c.c.r | (c.c.a << 8)) << 8)) << 8);
}

// ea: 0x0056BDC0
void FEText::CopyFrom(FEText* fet)
{
    visibility = fet->visibility;
    z_value = fet->z_value;
    fade_timer = fet->fade_timer;
    PanelAnimObject::flags = fet->PanelAnimObject::flags;
    if (flash_info != nullptr)
    {
        mem_heap_free(flash_info);
        flash_info = nullptr;
    }
    if (fet->flash_info != nullptr)
    {
        FETextFlashInfo* v3 = (FETextFlashInfo*)mem_heap_malloc(0x14u);
        if (v3 != nullptr)
        {
            *v3 = *fet->flash_info;
            flash_info = v3;
        }
        else
        {
            flash_info = nullptr;
        }
    }
    else
    {
        flash_info = nullptr;
    }
    font = fet->font;
    text = fet->text;
    xy_initial = fet->xy_initial;
    xy = fet->xy;
    scale = fet->scale;
    scale_unselected = fet->scale_unselected;
    scale_init = fet->scale_init;
    color1.i = fet->color1.i;
    color_unselected.i = fet->color_unselected.i;
    name = fet->name;
    flags = fet->flags;
    panel_text_index = fet->panel_text_index;
}

// ea: 0x0056BEF0
void FEText::Update(float time_inc)
{
    if (IsShown())
    {
        char flags = PanelAnimObject::flags;
        if ((flags & 0x10) != 0)
        {
            float v4 = (time_inc / fade_timer) + visibility;
            visibility = v4;
            if (v4 < 1.0f)
                goto LABEL_9;
            {
                char v6 = (char)(flags & 0xCF);
                visibility = 1.0f;
                PanelAnimObject::flags = v6;
            }
        }
        else if ((flags & 0x20) != 0)
        {
            float v7 = visibility - (time_inc / fade_timer);
            visibility = v7;
            if (v7 <= 0.0f)
            {
                visibility = 0.0f;
                PanelAnimObject::flags = (char)(flags & 0xCB);
            }
        }
    }
LABEL_9:
    if ((this->flags & 8) != 0)
    {
        if (flash_info == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
            AeAssert::gCurrentLine = 162;
            AeAssert::gCurrentExpr = "flash_info";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        FETextFlashInfo* flash_info = this->flash_info;
        float time_inca = flash_info->flash_timer + time_inc;
        flash_info->flash_timer = time_inca;
        flash_info->flash_intensity =
            sinf(time_inca / flash_info->flash_period * 6.2831855f) * 0.5f;
    }
}

// ea: 0x0056BFF0
void FEText::UpdateForWidescreen(bool widescreen)
{
    if (widescreen)
    {
        scale.x = scale.x * 0.75f;
        scale_unselected.x = scale_unselected.x * 0.75f;
        SetY(GetY() - 3.0f);
        xy.x = ((xy.x - 320.0f) * 0.75f) + 320.0f;
    }
    else
    {
        if (scale_unselected.x == scale.x
            && scale_unselected.y == scale.y
            && scale_unselected.z == scale.z)
        {
            scale.x = scale_init.x;
            scale.y = scale_init.y;
            scale.z = scale_init.z;
            scale_unselected.x = scale_init.x;
            scale_unselected.y = scale_init.y;
            scale_unselected.z = scale_init.z;
        }
        else
        {
            scale.x = scale_init.x;
            scale.y = scale_init.y;
            scale.z = scale_init.z;
            scale_unselected.x = scale_init.z;
            scale_unselected.y = scale_init.z;
        }
        SetY(GetY() + 3.0f);
        xy.x = ((xy.x - 320.0f) * 1.3333334f) + 320.0f;
    }
}

// ea: 0x0056C120
void FEText::UpdateForWidescreen(bool widescreen, int about_x)
{
    float screenCenter = (float)about_x;
    if (widescreen)
    {
        scale.x = scale.x * 0.75f;
        scale_unselected.x = scale_unselected.x * 0.75f;
        SetY(GetY() - 3.0f);
        xy.x = ((xy.x - screenCenter) * 0.75f) + screenCenter;
    }
    else
    {
        if (scale_unselected.x == scale.x
            && scale_unselected.y == scale.y
            && scale_unselected.z == scale.z)
        {
            scale.x = scale_init.x;
            scale.y = scale_init.y;
            scale.z = scale_init.z;
            scale_unselected.x = scale_init.x;
            scale_unselected.y = scale_init.y;
            scale_unselected.z = scale_init.z;
        }
        else
        {
            scale.x = scale_init.x;
            scale.y = scale_init.y;
            scale.z = scale_init.z;
            scale_unselected.x = scale_init.z;
            scale_unselected.y = scale_init.z;
        }
        SetY(GetY() + 3.0f);
        xy.x = ((xy.x - screenCenter) * 1.3333334f) + screenCenter;
    }
}

// ea: 0x0056C250
void FEText::MoveForSplitScreen(int viewport, int old_viewport)
{
    if (viewport != old_viewport)
    {
        float x_trans = GetX();
        float y_trans = GetY();
        switch (old_viewport)
        {
        case 4:
        case 7:
            goto L140603;
        case 6:
            x_trans = x_trans - 280.0f;
            break;
        case 8:
            x_trans = x_trans - 280.0f;
        L140603:
            y_trans = y_trans - 200.0f;
            break;
        default:
            break;
        }
        SetPos(x_trans, y_trans);
        float x_transa = GetX();
        float y_transa = GetY();
        switch (viewport)
        {
        case 4:
        case 7:
            goto L140613;
        case 6:
            x_transa = x_transa + 280.0f;
            break;
        case 8:
            x_transa = x_transa + 280.0f;
        L140613:
            y_transa = y_transa + 200.0f;
            break;
        default:
            break;
        }
        SetPos(x_transa, y_transa);
    }
}

// ea: 0x0056C380
void FEText::UpdateForHUDSplitScreen(int viewport, int old_viewport,
                                     int justification, float just_width,
                                     float just_height)
{
    if (viewport != old_viewport)
    {
        if (old_viewport != 0)
        {
            float x_scale = View::GetXScalingForHUD(old_viewport);
            float y_scale = View::GetYScalingForHUD(old_viewport);
            float x_pos = View::GetPreviousHUDXPos(
                GetX(), old_viewport, (char)justification, just_width);
            float y_pos = View::GetPreviousHUDYPos(
                GetY(), old_viewport, (char)justification, just_height);
            scale.x = scale.x * (1.0f / x_scale);
            scale_unselected.x = scale_unselected.x * (1.0f / x_scale);
            scale.y = scale.y * (1.0f / y_scale);
            scale_unselected.y = scale_unselected.y * (1.0f / y_scale);
            SetPos(x_pos, y_pos);
        }
        if (viewport != 0)
        {
            float x_scalea = View::GetXScalingForHUD(viewport);
            float y_scalea = View::GetYScalingForHUD(viewport);
            float just_widtha = View::GetCurrentHUDXPos(
                GetX(), viewport, (char)justification, just_width);
            float justificationa = View::GetCurrentHUDYPos(
                GetY(), viewport, (char)justification, just_height);
            scale.x = x_scalea * scale.x;
            scale_unselected.x = x_scalea * scale_unselected.x;
            scale.y = y_scalea * scale.y;
            scale_unselected.y = y_scalea * scale_unselected.y;
            SetPos(just_widtha, justificationa);
        }
    }
}

// ea: 0x0056C4F0
void FEText::UpdateForSplitScreen(int viewport, int old_viewport)
{
    if (viewport != old_viewport)
    {
        float x_trans = GetX();
        float y_trans = GetY();
        float v6 = 0.75f;
        float v7 = 0.75f;
        if (cg_widescreen_integer == 0)
            v7 = 1.0f;
        if (old_viewport >= 5 && old_viewport <= 8)
            v7 = v7 * 0.60000002f;
        if (cg_widescreen_integer == 0)
            v6 = 1.0f;
        if (viewport >= 5 && viewport <= 8)
            v6 = v6 * 0.60000002f;
        float x_newScaleFactor = v6;
        float y_oldScaleFactor = GetYScalingForWindow(old_viewport);
        float y_newScaleFactor = GetYScalingForWindow(viewport);
        scale.x = scale.x * (1.0f / v7);
        scale_unselected.x = scale_unselected.x * (1.0f / v7);
        scale.y = scale.y * (1.0f / y_oldScaleFactor);
        scale_unselected.y = scale_unselected.y * (1.0f / y_oldScaleFactor);
        switch (old_viewport)
        {
        case 3:
            goto L140660;
        case 4:
            goto L140661;
        case 5:
            {
                float v10 = x_trans;
                x_trans = (v10 * 0.003125f) * 640.0f;
            }
            goto L140660;
        case 6:
            {
                float v10 = x_trans - 320.0f;
                x_trans = (v10 * 0.003125f) * 640.0f;
            }
            goto L140660;
        case 7:
            {
                float v12 = x_trans;
                x_trans = (v12 * 0.003125f) * 640.0f;
            }
            goto L140661;
        case 8:
            {
                float v12 = x_trans - 320.0f;
                x_trans = (v12 * 0.003125f) * 640.0f;
            }
            goto L140661;
        default:
            break;
        }
        goto L140662;
    L140660:
        {
            float v11 = y_trans;
            y_trans = (v11 * 0.0041666669f) * 480.0f;
        }
        goto L140662;
    L140661:
        {
            float v11 = y_trans - 240.0f;
            y_trans = (v11 * 0.0041666669f) * 480.0f;
        }
    L140662:
        SetPos(x_trans, y_trans);
        float x_transa = GetX();
        float y_transa = GetY();
        scale.x = scale.x * x_newScaleFactor;
        scale_unselected.x = scale_unselected.x * x_newScaleFactor;
        scale.y = scale.y * y_newScaleFactor;
        scale_unselected.y = scale_unselected.y * y_newScaleFactor;
        switch (viewport)
        {
        case 3:
            goto L140670;
        case 4:
            goto L140671;
        case 5:
            x_transa = (x_transa * 0.0015625f) * 320.0f;
            goto L140670;
        case 6:
            x_transa = ((x_transa * 0.0015625f) + 1.0f) * 320.0f;
            goto L140670;
        case 7:
            x_transa = (x_transa * 0.0015625f) * 320.0f;
            goto L140671;
        case 8:
            x_transa = ((x_transa * 0.0015625f) + 1.0f) * 320.0f;
            y_transa = ((y_transa * 0.0020833334f) + 1.0f) * 240.0f;
            goto L140672;
        default:
            goto L140672;
        }
    L140670:
        y_transa = (y_transa * 0.0020833334f) * 240.0f;
        goto L140672;
    L140671:
        y_transa = ((y_transa * 0.0020833334f) + 1.0f) * 240.0f;
    L140672:
        SetPos(x_transa, y_transa);
    }
}

// ea: 0x0056C7F0
void FEText::SetHJustify(int hjust)
{
    SetFlag(48, false);
    if (hjust == 16 || hjust == 32)
        SetFlag(hjust, true);
}

// ea: 0x0056C830
void FEText::SetVJustify(int vjust)
{
    SetFlag(192, false);
    if (vjust == 64 || vjust == 128)
        SetFlag(vjust, true);
}

// ea: 0x0056C870
void FEText::SetText(const char* reference)
{
    if (reference != nullptr)
    {
        const char* STBString = STBManager::sInst->GetSTBString(reference);
        if (STBString != nullptr)
            SetTextNoLocalize(STBString);
        else
            SetTextNoLocalize(reference);
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 585;
        AeAssert::gCurrentExpr = "reference";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("NULL char* being passed into FEText::SetText"))
            __debugbreak();
    }
}

// ea: 0x0056C900
void FEText::SetText(unsigned int hash)
{
    const char* STBString = STBManager::sInst->GetSTBString(hash);
    if (STBString != nullptr)
        SetTextNoLocalize(STBString);
}

// ea: 0x0056C930
void FEText::SetNoFlash(const color32 col)
{
    color_unselected = col;
    flags = (int16_t)((flags & 0xFFF6) | 1);
}

// ea: 0x005AD880
void FEText::SetInitialXY(Broc::vector pos)
{
    xy_initial = pos;
}

// ea: 0x0056C950
void FEText::SetFlash(color32 col1, color32 col2, float period)
{
    color_unselected = col1;
    FETextFlashInfo* flash_info = this->flash_info;
    flags = (int16_t)(flags | 9);
    if (flash_info != nullptr)
    {
        flash_info->flash_color = col2;
        this->flash_info->flash_period = period;
        FETextFlashInfo* v7 = this->flash_info;
        v7->flash_timer = 0.0f;
        v7->flash_intensity = 0.0f;
    }
    else
    {
        float* v6 = (float*)mem_heap_malloc(0x14u);
        if (v6 != nullptr)
        {
            v6[3] = period;
            *(color32*)v6 = col2;
            v6[1] = 0.0f;
            v6[2] = 0.0f;
            *(unsigned char*)(v6 + 4) = 0;
            this->flash_info = (FETextFlashInfo*)v6;
        }
        else
        {
            this->flash_info = nullptr;
        }
    }
}

// ea: 0x0056C9D0
void FEText::SetNoColor()
{
    flags = (int16_t)(flags & 0xF6);
}

// ea: 0x0056C9E0
void FEText::CreateNGLColorCode(color32 c, char* output)
{
    _snprintf(output, 0xDu, "^%d%d%d%d", c.c.r, c.c.g, c.c.b, c.c.a);
    for (int i = 0; i < 13; ++i)
    {
        if (output[i] == ' ')
            output[i] = '0';
    }
}

// ea: 0x0056CA30
void FEText::Animate(math::Mat43* mat, float vis)
{
    char v4 = (char)(PanelAnimObject::flags & 1);
    float v11 = mat->w.v.m128_f32[0];
    float v5;
    float y;
    if (v4 != 0)
    {
        xy.x = v11 + xy.x;
        v5 = _mm_shuffle_ps(mat->w.v, mat->w.v, 0x55).m128_f32[0];
        y = xy.y;
    }
    else
    {
        xy.x = xy_initial.x + v11;
        v5 = _mm_shuffle_ps(mat->w.v, mat->w.v, 0x55).m128_f32[0];
        y = xy_initial.y;
    }
    xy.y = y + v5;
    __m128 v7 = mat->x.v;
    if (v4 != 0)
    {
        scale.x = v7.m128_f32[0] * scale.x;
        scale.y = _mm_shuffle_ps(mat->y.v, mat->y.v, 0x55).m128_f32[0]
                  * scale.y;
        SetAlpha(visibility * vis);
    }
    else
    {
        scale.x = scale_init.x * v7.m128_f32[0];
        scale.y = scale_init.y
                  * _mm_shuffle_ps(mat->y.v, mat->y.v, 0x55).m128_f32[0];
        SetAlpha(vis);
    }
}

// ea: 0x0056CB60
void FEText::AdjustForJustification(float& x, float& y, float current_scale)
{
    float tmp_hf = GetHeight(&current_scale);
    float tmp_wf = GetWidth(&current_scale);
    if (GetFlag(32))
    {
        x = x - tmp_wf;
    }
    else if (!GetFlag(16))
    {
        x = x - (tmp_wf * 0.5f);
    }
    if (GetFlag(128))
    {
        y = y - tmp_hf;
    }
    else if (!GetFlag(64))
    {
        y = y - (tmp_hf * 0.5f);
    }
}

// ea: 0x0057C390
void FEText::Draw(bool selected)
{
    if (!IsShown() || (text == Broc::string(defaultFileName)))
        return;
    color32* p_color1 = &color1;
    if (selected != 1)
        p_color1 = &color_unselected;
    color32 tmp_color;
    tmp_color.i = p_color1->i;
    Broc::vector* p_scale = &scale;
    if (selected != 1)
        p_scale = &scale_unselected;
    float v5 = p_scale->y;
    float v6 = p_scale->z;
    float v7 = p_scale->x;
    short flags = this->flags;
    float tmp_scale = v7;
    float ScaleY = v5;
    float v44 = v6;
    (void)v44;
    if ((flags & 0x100) != 0)
        tmp_color.i = color1.i;
    if ((flags & 8) != 0)
        tmp_color.i = flash_info->GetColor(color_unselected).i;
    tmp_color.c.a = (unsigned char)(tmp_color.c.a * visibility);
    float x = xy.x;
    float y = xy.y;
    AdjustForJustification(x, y, v7);
    int length = text.length();
    int v12 = 0;
    int drawn = 0;
    char substr[256];
    if (length > 0)
    {
        do
        {
            unsigned int i = v12;
            while (i < (unsigned int)length && text.mBlock->mBuff[i] != '~'
                   && text.mBlock->mBuff[i] != 0)
                ++i;
            if (i > (unsigned int)v12)
            {
                strncpy(substr, text.c_str() + v12, 0x100u);
                unsigned int v18 = i - drawn;
                if (v18 >= 256)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
                    AeAssert::gCurrentLine = 501;
                    AeAssert::gCurrentExpr = "end-start<256";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Overflow!"))
                        __debugbreak();
                }
                substr[v18] = 0;
                unsigned int v33 = tmp_color.c.b
                                   | ((tmp_color.c.g
                                       | ((tmp_color.c.r | (tmp_color.c.a << 8)) << 8))
                                      << 8);
                float z = GetZvalue();
                nglListAddString(g_femanager.GetFont(font), substr, x, y, z,
                                 v33, tmp_scale, ScaleY);
                unsigned int tmp_w, tmp_h;
                nglGetStringDimensions(g_femanager.GetFont(font), substr,
                                       &tmp_w, &tmp_h, tmp_scale, ScaleY);
                drawn = i;
                v12 = i;
                x = (float)tmp_w + x;
            }
            if (v12 >= length)
                break;
            unsigned int v25 = i + 1;
            char v26 = (v25 < (unsigned int)length) ? text.mBlock->mBuff[v25] : 0;
            if (v26 == '$')
                v26 = '%';
            else if (v26 == '%')
                v26 = '$';
            float v27 = widescreen_scale;
            if (cg_widescreen_integer == 0)
                v27 = 1.0f;
            float button_scale_x = v27 * ScaleY;
            unsigned int v34 = tmp_color.c.b
                               | ((tmp_color.c.g
                                   | ((tmp_color.c.r | (tmp_color.c.a << 8)) << 8))
                                  << 8);
            float za = GetZvalue();
            char buttonChar[2] = {v26, 0};
            nglListAddString(g_femanager.fonts[1], buttonChar, x, y, za, v34,
                             button_scale_x, ScaleY);
            unsigned int tmp_w2, tmp_h2;
            nglGetStringDimensions(g_femanager.fonts[1], buttonChar, &tmp_w2,
                                   &tmp_h2, tmp_scale, ScaleY);
            v12 += 2;
            x = (float)tmp_w2 + x;
            drawn = v12;
        }
        while (v12 < length);
    }
}

// ea: 0x0057C6E0
float FEText::GetWidth(const float* scale_ptr)
{
    float v4 = scale_ptr != nullptr ? *scale_ptr : scale.x;
    float current_scale = v4;
    int length = text.length();
    unsigned int x = 0;
    if (length <= 0)
        return 0.0f;
    int v2 = 0;
    char substr[256];
    while (1)
    {
        unsigned int i = v2;
        while (i < (unsigned int)length && text.mBlock->mBuff[i] != '~'
               && text.mBlock->mBuff[i] != 0)
            ++i;
        float v13;
        if (i <= (unsigned int)v2)
        {
            v13 = current_scale;
        }
        else
        {
            strncpy(substr, text.c_str() + v2, 0x100u);
            if ((i - v2) >= 256)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
                AeAssert::gCurrentLine = 699;
                AeAssert::gCurrentExpr = "end-start<256";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Overflow!"))
                    __debugbreak();
            }
            substr[i - v2] = 0;
            nglFont* v12;
            if (font == FONT_BIG || font == FONT_NORMAL)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.cpp";
                AeAssert::gCurrentLine = 1303;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
                v12 = nullptr;
            }
            else
            {
                v12 = g_femanager.fonts[font];
            }
            v13 = current_scale;
            unsigned int tmp_w, tmp_h;
            nglGetStringDimensions(v12, substr, &tmp_w, &tmp_h, current_scale,
                                   current_scale);
            x += tmp_w;
            v2 = i;
        }
        if (v2 >= length)
            break;
        unsigned int v15 = i + 1;
        char v16 = (v15 < (unsigned int)length) ? text.mBlock->mBuff[v15] : 0;
        char buttonChar[2] = {v16, 0};
        unsigned int tmp_w2, tmp_h2;
        nglGetStringDimensions(g_femanager.fonts[1], buttonChar, &tmp_w2,
                               &tmp_h2, v13, v13);
        v2 += 2;
        x += tmp_w2;
        if (v2 >= length)
            return (float)x;
    }
    return (float)x;
}

// ea: 0x0057C910
float FEText::GetHeight(const float* scale_ptr)
{
    float x = scale_ptr != nullptr ? *scale_ptr : scale.x;
    unsigned int tmp_w;
    unsigned int height = (unsigned int)x;
    nglFont* Font = g_femanager.GetFont(font);
    nglGetStringDimensions(Font, text.c_str(), &tmp_w, &height, x, x);
    return (float)height;
}

// ea: 0x0056B940
FETextFlashInfo::FETextFlashInfo(color32 col, float period)
{
    flash_period = period;
    flash_color = col;
    flash_timer = 0.0f;
    flash_intensity = 0.0f;
    reset = false;
}

// ea: 0x005AD680
void FETextFlashInfo::SetColor(color32 col)
{
    flash_color = col;
}

// ea: 0x005AD690
void FETextFlashInfo::SetPeriod(float period)
{
    flash_period = period;
}

// ea: 0x0056B970
void FETextFlashInfo::Update(float time_inc)
{
    float time_inca = time_inc + flash_timer;
    flash_timer = time_inca;
    flash_intensity = sinf(time_inca / flash_period * 6.2831855f) * 0.5f;
}

// ea: 0x0056B9B0
color32 FETextFlashInfo::GetColor(color32 normal_color)
{
    float v3 = flash_intensity + 0.5f;
    color32 result;
    result.c.b = (unsigned char)(((flash_color.c.b - normal_color.c.b) * v3)
                                 + normal_color.c.b);
    result.c.g = (unsigned char)(((flash_color.c.g - normal_color.c.g) * v3)
                                 + normal_color.c.g);
    result.c.r = (unsigned char)(((flash_color.c.r - normal_color.c.r) * v3)
                                 + normal_color.c.r);
    result.c.a = (unsigned char)(((flash_color.c.a - normal_color.c.a) * v3)
                                 + normal_color.c.a);
    return result;
}

// ea: 0x0056BA70
void FETextFlashInfo::Reset()
{
    flash_timer = 0.0f;
    flash_intensity = 0.0f;
}

extern FEMenuColorScheme color_schemes[];  // 0xDF3AE0
const char* const FEMenuColorSchemeText[17] = {
    "WHITE_RED",
    "WHITE_FADE",
    "REALLY_WHITE_FADE",
    "BLACK_RED",
    "REALLY_WHITE_RED",
    "BROWN_BROWN",
    "GREY_GREY",
    "RED_RED",
    "ALL_GREY",
    "COLORS_MP_INGAME",
    "BUTTON_TEXT_01",
    "BODY_TEXT_01",
    "BODY_TEXT_02",
    "BODY_TEXT_03",
    "BODY_TEXT_04",
    "BODY_TEXT_05",
    "BODY_TEXT_06",
};

// ea: 0x005ADED0
MultiLineButtons::MultiLineButtons()
{
    text_start_index = 0;
    x_offset = 0;
}

// ea: 0x005ADEF0
void MultiLineButtons::CopyFrom(MultiLineButtons* fet)
{
    *this = *fet;
}

// ea: 0x0056CC40
MultiLineString::MultiLineString()
{
    xy.x = sNaN;
    xy.y = sNaN;
    xy.z = sNaN;
    data = Broc::string((Broc::string::Block*)nullptr);
    data = defaultFileName;
    xy.x = 0.0f;
    xy.y = 0.0f;
    xy.z = 0.0f;
    font = FONT_NORMAL;
    button_array = nullptr;
    button_array_size = 0;
}

// ea: 0x0056CD00
MultiLineString::~MultiLineString()
{
    if (button_array != nullptr)
    {
        mem_heap_free(button_array);
        button_array = nullptr;
    }
}

// ea: 0x0056CD30
void MultiLineString::CopyFrom(MultiLineString* fet)
{
    data = fet->data;
    xy = fet->xy;
    font = fet->font;
    int button_array_size = fet->button_array_size;
    this->button_array_size = button_array_size;
    if (button_array_size > 0)
    {
        MultiLineButtons* v4 = (MultiLineButtons*)mem_heap_malloc(
            4 * button_array_size);
        if (v4 != nullptr)
        {
            for (int i = 0; i < button_array_size; ++i)
            {
                v4[i].text_start_index = 0;
                v4[i].x_offset = 0;
            }
        }
        else
        {
            v4 = nullptr;
        }
        button_array = v4;
        for (int v5 = 0; v5 < this->button_array_size; ++v5)
        {
            button_array[v5].text_start_index =
                fet->button_array[v5].text_start_index;
            button_array[v5].x_offset = fet->button_array[v5].x_offset;
        }
    }
}

// ea: 0x0056CDE0
void MultiLineString::AdjustForScale(float scale_factor)
{
    for (int i = 0; i < button_array_size; ++i)
        button_array[i].x_offset =
            (short)(button_array[i].x_offset * scale_factor);
}

// ea: 0x0056CEE0
int MultiLineString::ConvertStringToButtonCode(const char* text,
                                               const char** buttonCode,
                                               const Broc::string& whole_string)
{
    (void)whole_string;
    if (*text != '~')
        return 0;
    int length = 0;
    *buttonCode = defaultFileName;
    if (CompareButton(text, "~cross", buttonCode, "\"", length)
        || CompareButton(text, "~square", buttonCode, "#", length)
        || CompareButton(text, "~gc_z", buttonCode, "(", length)
        || CompareButton(text, "~menu_back", buttonCode, "%", length)
        || CompareButton(text, "~not_menu_back", buttonCode, "$", length)
        || CompareButton(text, "~r2", buttonCode, "(", length)
        || CompareButton(text, "~l2", buttonCode, "'", length)
        || CompareButton(text, "~r1", buttonCode, "!", length)
        || CompareButton(text, "~l1", buttonCode, "&", length)
        || CompareButton(text, "~triangle", buttonCode, "$", length)
        || CompareButton(text, "~circle", buttonCode, "%", length)
        || CompareButton(text, "~right", buttonCode, "*", length)
        || CompareButton(text, "~back", buttonCode, ",", length)
        || CompareButton(text, "~forward", buttonCode, ")", length)
        || CompareButton(text, "~left", buttonCode, "+", length)
        || CompareButton(text, "~start", buttonCode, "-", length)
        || CompareButton(text, "~select", buttonCode, "1", length)
        || CompareButton(text, "~updown", buttonCode, "/", length)
        || CompareButton(text, "~both_lr", buttonCode, ".", length))
    {
        return length;
    }
    bool v5 = CompareButton(text, "~navig_all", buttonCode, "0", length);
    return v5 ? length : 0;
}

// ea: 0x0057CC20
float MultiLineString::GetWidth(const char* text, float scale, font_index f)
{
    if (text == nullptr)
        return 0.0f;
    if (f == FONT_NORMAL)
        return 0.0f;
    if (g_femanager.GetFont(f) == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 1034;
        AeAssert::gCurrentExpr = "g_femanager.GetFont(f)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Missing font!"))
            __debugbreak();
    }
    unsigned int width = 0;
    unsigned int height = 0;
    nglFont* Font = g_femanager.GetFont(f);
    nglGetStringDimensions(Font, text, &width, &height, scale, scale);
    return (float)width;
}

// ea: 0x00584910
float MultiLineString::GetStringWidth(const char* text, font_index f,
                                      float scale, float button_scale)
{
    if (*text == 0)
        return 0.0f;
    int v5 = 0;
    float cur_width = 0.0f;
    Broc::string tmp(text);
    do
    {
        int v6 = tmp.find((unsigned int)v5, '~');
        int v7 = v6;
        if (v6 == -1)
            break;
        Broc::string v17 = tmp.substr((unsigned int)v5,
                                      (unsigned int)(v6 - v5));
        cur_width = GetWidth(v17.c_str(), scale, f) + cur_width;
        const char* buttonCode = nullptr;
        v5 = v7 + ConvertStringToButtonCode(
                       tmp.c_str() + v7, &buttonCode, tmp);
        cur_width = GetWidth(buttonCode, button_scale, FONT_GEMFONTONE)
                    + cur_width;
    }
    while (v5 >= 0);
    float cur_widthb;
    if (v5 != 0)
    {
        Broc::string buttonCode = tmp.substr(
            (unsigned int)v5, (unsigned int)(tmp.length() - v5));
        cur_widthb = GetWidth(buttonCode.c_str(), scale, f) + cur_width;
    }
    else
    {
        cur_widthb = GetWidth(tmp.c_str(), scale, f) + cur_width;
    }
    return cur_widthb;
}

// ea: 0x00584AB0
void MultiLineString::ParseForButtons(float scale, float button_scale)
{
    unsigned int v5 = 0;
    int i = 0;
    float cur_width = 0.0f;
    int v38 = 1;
    if (button_array == nullptr || button_array_size <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 994;
        AeAssert::gCurrentExpr = "button_array && button_array_size > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int v6 = button_array_size / 2;
    if (v6 > 0)
    {
        int v21 = 0;
        do
        {
            unsigned int v8 = (unsigned int)data.find(v5, '~');
            Broc::string result = data.substr(v5, v8 - v5);
            cur_width = GetWidth(result.c_str(), scale, font) + cur_width;
            button_array[2 * i].text_start_index = (short)v8;
            button_array[2 * i].x_offset = (short)cur_width;
            const char* buttonCode = nullptr;
            int v15 = ConvertStringToButtonCode(data.c_str() + v8,
                                                &buttonCode, data);
            Broc::string v33 = data.substr(
                (unsigned int)(v8 + v15),
                (unsigned int)(data.length() - v15 - v8));
            Broc::string v34 = data.substr(0, (unsigned int)v8);
            Broc::string v35 = v34 + buttonCode;
            Broc::string v36 = v35 + v33;
            data = v36;
            v5 = (unsigned int)(strlen(buttonCode) + v8);
            if (data.mBlock != nullptr && v5 < (unsigned int)data.length())
            {
                cur_width = GetWidth(buttonCode, button_scale,
                                     FONT_GEMFONTONE) + cur_width;
                button_array[2 * i + 1].text_start_index = (short)v5;
                button_array[2 * i + 1].x_offset = (short)cur_width;
                v21 = i;
            }
            else
            {
                v21 = i;
                button_array_size = v38;
            }
            if (v21 == button_array_size / 2 - 1
                || data.mBlock == nullptr
                || v5 >= (unsigned int)data.length())
            {
                Broc::string v32 = data.substr(
                    v5, (unsigned int)(data.length() - v5));
                total_width = GetWidth(v32.c_str(), scale, font) + cur_width;
            }
            v38 += 2;
            i = v21 + 1;
        }
        while (v21 + 1 < button_array_size / 2);
    }
}

// ea: 0x0058D3B0
void MultiLineString::Set(const char* d, font_index f, float scale,
                          float button_scale)
{
    if (d != nullptr)
    {
        data = d;
        font = f;
        xy.x = 0.0f;
        xy.y = 0.0f;
        xy.z = 0.0f;
        total_width = 0.0f;
        button_array_size = 0;
        int v7 = 0;
        while (data.mBlock != nullptr
               && data.mBlock != (Broc::string::Block*)-12
               && data.mBlock->mBuff[0] != 0
               && v7 >= 0)
        {
            v7 = data.find((unsigned int)v7, '~');
            if (v7 >= 0)
            {
                button_array_size += 2;
                ++v7;
            }
        }
        if (button_array != nullptr)
        {
            mem_heap_free(button_array);
            button_array = nullptr;
        }
        int button_array_size = this->button_array_size;
        if (button_array_size <= 0)
        {
            // IDA 0x0058D3B0 uses the string block payload directly and
            // falls back to defaultFileName when the block is absent.
            const char* text = defaultFileName;
            if (data.mBlock != nullptr)
                text = reinterpret_cast<const char*>(&data.mBlock[1]);
            total_width = GetWidth(text, scale, font);
        }
        else
        {
            MultiLineButtons* v9 = (MultiLineButtons*)mem_heap_malloc(
                4 * button_array_size);
            if (v9 != nullptr)
            {
                for (int i = 0; i < button_array_size; ++i)
                {
                    v9[i].text_start_index = 0;
                    v9[i].x_offset = 0;
                }
            }
            else
            {
                v9 = nullptr;
            }
            button_array = v9;
            ParseForButtons(scale, button_scale);
        }
    }
}

// ea: 0x0057C980
void MultiLineString::Draw(float z, int col, int button_col,
                           float scale_x, float scale_y, float button_scale,
                           float button_y_offset)
{
    float v8 = widescreen_scale_0;
    if (cg_widescreen_integer == 0)
        v8 = 1.0f;
    float button_scale_x = v8 * button_scale;
    if (button_array_size <= 0)
    {
        const char* v33 = data.c_str();
        nglListAddString(g_femanager.GetFont(font), v33, xy.x, xy.y, z,
                         col, scale_x, scale_y);
        return;
    }
    Broc::string result = data.substr(0, (unsigned int)button_array[0].text_start_index);
    nglListAddString(g_femanager.GetFont(font), result.c_str(), xy.x, xy.y, z,
                     col, scale_x, scale_y);
    int button_array_size = this->button_array_size;
    for (int i = 0; i < (button_array_size + 1) / 2; ++i)
    {
        MultiLineButtons* v17 = &button_array[2 * i];
        int v9 = 2 * i;
        int mLength;
        if (v9 == button_array_size - 1)
            mLength = data.length();
        else
            mLength = v17[1].text_start_index;
        Broc::string v21 = data.substr(
            (unsigned int)v17->text_start_index,
            (unsigned int)(mLength - v17->text_start_index));
        nglListAddString(g_femanager.fonts[1], v21.c_str(),
                         button_array[2 * i].x_offset + xy.x,
                         xy.y - button_y_offset, z, button_col,
                         button_scale_x, button_scale);
        if (v9 < button_array_size - 1)
        {
            unsigned int v25 = button_array[2 * i + 1].text_start_index;
            int v28;
            if (v9 == button_array_size - 2)
                v28 = data.length();
            else
                v28 = button_array[2 * i + 2].text_start_index;
            Broc::string v29 = data.substr(v25, (unsigned int)(v28 - v25));
            nglListAddString(g_femanager.GetFont(font), v29.c_str(),
                             button_array[2 * i + 1].x_offset + xy.x,
                             xy.y, z, col, scale_x, scale_y);
        }
    }
}

// ============================================================================
// FEMultiLineText (FEText.cpp family)
// ============================================================================

// ea: 0x005B1B90
FEMultiLineText::FEMultiLineText()
    : FEText()
{
    button_color.i = 0;
}

// ea: 0x005ADF10
void MultiLineString::SetPos(float xp, float yp)
{
    xy.x = xp;
    xy.y = yp;
    xy.z = 0.0f;
}

// ea: 0x005ADF60
void MultiLineString::Shift(float x, float y)
{
    const float z = xy.z;
    xy.x += x;
    xy.y += y;
    xy.z = z;
}

// ea: 0x005ADFA0
float MultiLineString::GetTotalWidth()
{
    return total_width;
}

// ea: 0x005AE400
FEMenuColorScheme::FEMenuColorScheme(color32 u, color32 h1)
{
    high1.i = 0;
    high2.i = 0;
    flash = false;
    unselect = u;
    high1 = h1;
    high2 = h1;
}

// ea: 0x005AE430
FEMenuColorScheme::FEMenuColorScheme(color32 u, color32 h1, color32 h2)
{
    high1.i = 0;
    high2.i = 0;
    unselect = u;
    flash = true;
    high1 = h1;
    high2 = h2;
}

// ea: 0x00584DA0
FEMultiLineText::FEMultiLineText(font_index f, float x1, float y1, int z1,
                                 panel_layer layer, float s, int horizJust,
                                 int vertJust, color32 col)
    : FEText(f, defaultFileName, x1, y1, z1, layer, s, horizJust, vertJust,
             col)
{
    button_color.i = 0;
    line_num = 0;
    line_avail_num = 0;
    lines = nullptr;
    scrollable = false;
    scroll_edge_based = false;
    scroll_box_height = 0;
    scroll_first = 0;
    scroll_last = 0;
    cut_off_if_too_long = false;
    scroll_offset = 0.0f;
    button_scale = 1.0f;
    button_y_offset = 0.0f;
    button_color.i = 0xFFFFFFFF;
    box_width = -1;
    nglFont* v11 = g_femanager.GetFont(font);
    unsigned int width;
    unsigned int height;
    nglGetStringDimensions(v11, "!", &width, &height, scale.x, scale.y);
    line_spacing_init = (float)height;
    line_spacing = (float)height;
}

// ea: 0x0056D6D0
void FEMultiLineText::SetNumLines(int n)
{
    if (n == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 1277;
        AeAssert::gCurrentExpr = "n != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    line_avail_num = n;
    line_num = 0;
    if (lines != nullptr)
    {
        int count = ((int*)lines)[-1];
        for (int i = 0; i < count; ++i)
            lines[i].~MultiLineString();
        mem_heap_free((int*)lines - 1);
    }
    int* v6 = (int*)mem_heap_malloc(32 * line_avail_num + 4);
    if (v6 != nullptr)
    {
        MultiLineString* v7 = (MultiLineString*)(v6 + 1);
        *v6 = line_avail_num;
        for (int i = 0; i < line_avail_num; ++i)
            new (&v7[i]) MultiLineString();
        lines = v7;
    }
    else
    {
        lines = nullptr;
    }
}

// ea: 0x0056E5A0
void FEMultiLineText::SetTextBox(const char* reference, int w,
                                 float sc_override)
{
    const char* STBString =
        STBManager::sInst->GetSTBString(reference);
    if (STBString != nullptr)
        SetTextBoxNoLocalize(Broc::string(STBString), w, sc_override);
    else
        SetTextBoxNoLocalize(Broc::string(reference), 640, sc_override);
}

// ea: 0x0057CFA0
void FEMultiLineText::SetLineSpacing(int new_spacing)
{
    if (new_spacing == -1)
    {
        unsigned int width;
        unsigned int new_spacinga;
        nglFont* Font = g_femanager.GetFont(font);
        nglGetStringDimensions(Font, "!", &width, &new_spacinga, scale.x,
                               scale.y);
        line_spacing_init = (float)new_spacinga;
        line_spacing = line_spacing_init;
    }
    else
    {
        line_spacing_init = (float)new_spacing;
        line_spacing = line_spacing_init;
    }
}

// ea: 0x0056D7E0
void FEMultiLineText::SetText(const char* reference)
{
    const char* STBString = STBManager::sInst->GetSTBString(reference);
    if (STBString != nullptr)
        SetTextNoLocalize(STBString);
    else
        SetTextNoLocalize(reference);
}
// ============================================================================
// FEMultiLineText helpers
// ============================================================================
extern int currCl;                  // ?currCl@@3HA @ 0xF1579C
extern nglFont* nglSysFont;         // ?nglSysFont@@3PAVnglFont@@A
char gResultString[512];            // ?gResultString@@3PADA @ 0xF30B48

struct KeyInfoEntry {
    int  mKey;           // +0x00
    char* mBoundCmdName; // +0x04
};
struct KeyInfo {
    struct KeyTable {
        KeyInfoEntry m_elements[256];
        int          m_size;
    };
    static KeyTable mKeys[1];  // ?mKeys@KeyInfo@@0V?$ae_sized_array@V?$ae_sized_array@VKeyInfoEntry@@$0BAA@@@$00@@A (cl.o)
    static int GetKey(const char* boundCmdName, int clnt);  // ?GetKey@KeyInfo@@SAHPBDH@Z
};

// ============================================================================
// FEMultiLineText (FEText.cpp family)
// ============================================================================

// ea: 0x0056D180
void FEMultiLineText::CopyFrom(FEMultiLineText* fet)
{
    FEText::CopyFrom(fet);
    button_color.i = fet->button_color.i;
    button_scale = fet->button_scale;
    line_spacing_init = fet->line_spacing_init;
    line_spacing = fet->line_spacing;
    button_y_offset = fet->button_y_offset;
    box_width = fet->box_width;
    line_num = fet->line_num;
    line_avail_num = fet->line_avail_num;
    int line_avail_num = this->line_avail_num;
    scroll_box_height = fet->scroll_box_height;
    scroll_first = fet->scroll_first;
    scroll_last = fet->scroll_last;
    scroll_offset = fet->scroll_offset;
    scrollable = fet->scrollable;
    scroll_edge_based = fet->scroll_edge_based;
    cut_off_if_too_long = fet->cut_off_if_too_long;
    int* v5 = (int*)mem_heap_malloc(32 * line_avail_num + 4);
    MultiLineString* v6;
    if (v5 != nullptr)
    {
        *v5 = line_avail_num;
        MultiLineString* ia = (MultiLineString*)(v5 + 1);
        for (int i = 0; i < line_avail_num; ++i)
            new (&ia[i]) MultiLineString();
        v6 = ia;
    }
    else
    {
        v6 = nullptr;
    }
    lines = v6;
    for (int i = 0; i < this->line_avail_num; ++i)
        lines[i].CopyFrom(&fet->lines[i]);
}

// ea: 0x0056D2F0
void FEMultiLineText::Draw(bool selected)
{
    (void)selected;
    if (IsShown())
    {
        if (scrollable)
            Draw(scroll_first, scroll_last);
        else
            Draw(0, line_num);
    }
}

// ea: 0x0056D340
float FEMultiLineText::GetWidth()
{
    int line_num = this->line_num;
    int v2 = 0;
    int max_width = 0;
    if (line_num > 0)
    {
        float* p_total_width = &lines->total_width;
        do
        {
            if (*p_total_width > v2)
            {
                v2 = (int)*p_total_width;
                max_width = v2;
            }
            p_total_width += 8;
            --line_num;
        }
        while (line_num != 0);
    }
    return (float)max_width;
}

// ea: 0x0056D380
void FEMultiLineText::AddFont(int index, font_index f)
{
    if (index >= line_num)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 1200;
        AeAssert::gCurrentExpr = "index < line_num";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    lines[index].font = f;
}

// ea: 0x0056D3F0
void FEMultiLineText::SetFont(font_index f)
{
    int line_num = this->line_num;
    this->font = f;
    for (int v3 = 0; v3 < line_num; ++v3)
        lines[v3].font = this->font;
}

// ea: 0x0056D430
void FEMultiLineText::Shift(float x_shift, float y_shift)
{
    for (int v3 = 0; v3 < line_num; ++v3)
    {
        MultiLineString* lines = this->lines;
        float x = lines[v3].xy.x;
        float z = lines[v3].xy.z;
        lines[v3].xy.x = x + x_shift;
        lines[v3].xy.y = lines[v3].xy.y + y_shift;
        lines[v3].xy.z = z;
    }
}

// ea: 0x0056D4A0
void FEMultiLineText::Scroll(float offset)
{
    if (scrollable)
    {
        float scroll_offset = this->scroll_offset;
        bool scroll_edge_based = this->scroll_edge_based;
        int scroll_box_height = this->scroll_box_height;
        float v6 = (float)line_num * line_spacing;
        this->scroll_offset = offset + scroll_offset;
        float v7;
        if (scroll_edge_based)
            v7 = v6 + (2 * scroll_box_height);
        else
        {
            v7 = v6 - scroll_box_height;
            if (scroll_box_height > v7)
                v7 = (float)scroll_box_height;
        }
        float v8 = (float)scroll_box_height;
        if (!scroll_edge_based)
            v8 = 0.0f;
        if ((offset + scroll_offset) > v8)
            this->scroll_offset = v8;
        if ((0.0f - v7) > this->scroll_offset)
            this->scroll_offset = 0.0f - v7;
        Shift(0.0f, this->scroll_offset - scroll_offset);
        float v9 = 1.0f / line_spacing;
        int v10 = (int)(0.0f - (v9 * this->scroll_offset));
        float v11 = (float)scroll_box_height * v9;
        scroll_first = v10;
        scroll_last = (int)((v11 + v10) + 1.0f);
    }
}

// ea: 0x0056D5B0
void FEMultiLineText::SetScrollable(int height, bool edge_based)
{
    scrollable = true;
    scroll_edge_based = edge_based;
    scroll_box_height = height;
    float v3 = (float)height;
    if (!edge_based)
        v3 = 0.0f;
    scroll_offset = v3;
    scroll_first = 0;
    int v4;
    if (edge_based)
        v4 = 0;
    else
        v4 = (int)(((float)height / line_spacing) + 1.0f);
    scroll_last = v4;
    SetPos(xy.x, xy.y + v3);
}

// ea: 0x0056D630
float FEMultiLineText::GetPercentage()
{
    float ret = 0.0f;
    if (scrollable)
    {
        float v1 = (float)line_num * line_spacing;
        if (!scroll_edge_based)
            v1 = v1 - scroll_box_height;
        if (v1 <= 0.0f)
            return 1.0f;
        float v2 = 0.0f - (scroll_offset / v1);
        ret = v2;
        if (v2 > 1.0f)
            return 1.0f;
        if (v2 < 0.0f)
            return 0.0f;
    }
    return ret;
}

// ea: 0x0056D820
void FEMultiLineText::SetText(unsigned int hash)
{
    const char* STBString = STBManager::sInst->GetSTBString(hash);
    if (STBString != nullptr)
        SetTextNoLocalize(STBString);
}

// ea: 0x0056D850
const char* FEMultiLineText::ConvertActionToButton(const char* stringIn)
{
    if (stringIn == nullptr)
        return nullptr;
    if (strlen(stringIn) >= 0x200)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 1323;
        AeAssert::gCurrentExpr = "strlen(stringIn)<512";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    char stringHolder[512];
    strncpy(stringHolder, stringIn, 0x200u);
    memset(gResultString, 0, sizeof(gResultString));
    char* v3 = strtok(stringHolder, " ");
    char* v4 = v3;
    if (v3 != nullptr)
    {
        int iTokenLength = (int)strlen(v3);
        while (1)
        {
            const char* v5 = TranslateAction(v4);
            if (v5 != nullptr)
            {
                strcat(gResultString, v5);
            }
            else
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
                AeAssert::gCurrentLine = 1335;
                AeAssert::gCurrentExpr = "tokenTranslated";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            char* v7 = v4;
            char* v8 = strtok(nullptr, " ");
            v4 = v8;
            if (v8 == nullptr)
                break;
            int v9 = (int)(&v4[-iTokenLength] - v7);
            if (v7 != nullptr)
            {
                if (v9 > 0)
                {
                    do
                    {
                        --v9;
                        strcat(gResultString, " ");
                    }
                    while (v9 != 0);
                }
            }
            else
            {
                strcat(gResultString, " ");
            }
            iTokenLength = (int)strlen(v8);
        }
    }
    if (strlen(gResultString) >= 0x200)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 1365;
        AeAssert::gCurrentExpr = "strlen(gResultString)<512";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return gResultString;
}

// ea: 0x0056DCB0
const char* FEMultiLineText::TranslateAction(const char* token)
{
    if (token == nullptr)
        return nullptr;
    if (strcmp(token, "[USE]") == 0 || strcmp(token, "[ACTIVATE]") == 0)
    {
        int Key = KeyInfo::GetKey("+activate", currCl);
        if (Key != -1)
            goto LABEL_75;
        goto LABEL_74;
    }
    if (strcmp(token, "[ATTACK]") == 0)
    {
        KeyInfo::GetKey("+attack", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[AIM]") == 0)
    {
        int Key = KeyInfo::GetKey("+speed", currCl);
        if (Key != -1)
            goto LABEL_75;
        goto LABEL_9;
    }
    if (strcmp(token, "[MELEE]") == 0)
    {
        KeyInfo::GetKey("+melee", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[GRENADE]") == 0)
    {
        KeyInfo::GetKey("+grenadeattack", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[TOGGLE_AIM]") == 0)
    {
    LABEL_9:
        KeyInfo::GetKey("toggle cl_run", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[SPRINT]") == 0)
    {
        KeyInfo::GetKey("+sprint", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[RELOAD]") == 0)
    {
        int Key = KeyInfo::GetKey("+reload", currCl);
        if (Key == -1)
        LABEL_74:
            Key = KeyInfo::GetKey("+activatereload", currCl);
    LABEL_75:
        switch (Key)
        {
        case 212: return "~cross";
        case 213: return "~circle";
        case 214: return "~not_menu_back";
        case 211: return "~square";
        case 215: return "~r1";
        case 217: return "~r2";
        case 219: return STBManager::sInst->GetSTBString("INGAME_R3");
        case 216: return "~l1";
        case 218: return "~l2";
        case 220: return STBManager::sInst->GetSTBString("INGAME_L3");
        case 13: return "~start";
        case 27: return "~select";
        case 154: return "~forward";
        case 155: return "~back";
        case 156: return "~left";
        case 157: return "~right";
        default: break;
        }
        return token;
    }
    if (strcmp(token, "[STANCE_UP]") == 0)
    {
        KeyInfo::GetKey("+moveup", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[STANCE_DOWN]") == 0)
    {
        KeyInfo::GetKey("lowerstance", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[WEAPON_SWAP]") == 0)
    {
        KeyInfo::GetKey("weapnext", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[LEAN_LEFT]") == 0)
    {
        KeyInfo::GetKey("+leanleftswitchnext", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[LEAN_RIGHT]") == 0)
    {
        KeyInfo::GetKey("+leanrightswitchnext", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[SELECT]") == 0)
    {
        KeyInfo::GetKey("togglemenu", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[START]") == 0)
    {
        KeyInfo::GetKey("togglepaused", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[CLASS]") == 0)
    {
        KeyInfo::GetKey("+class", currCl);
        goto LABEL_75;
    }
    if (strcmp(token, "[PRESS_USE]") == 0
        || strcmp(token, "[PRESS_ACTIVATE]") == 0)
    {
        int v4 = KeyInfo::GetKey("+activate", currCl);
        goto LABEL_55;
    }
    if (strcmp(token, "[PRESS_ATTACK]") == 0)
    {
        int v4 = KeyInfo::GetKey("+attack", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_AIM]") == 0)
    {
        int v4 = KeyInfo::GetKey("+speed", currCl);
        if (v4 != -1)
            goto LABEL_44;
        goto LABEL_43;
    }
    if (strcmp(token, "[PRESS_MELEE]") == 0)
    {
        int v4 = KeyInfo::GetKey("+melee", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_GRENADE]") == 0)
    {
        int v4 = KeyInfo::GetKey("+grenadeattack", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_TOGGLE_AIM]") == 0)
    {
    LABEL_43:
        int v4 = KeyInfo::GetKey("toggle cl_run", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_SPRINT]") == 0)
    {
        int v4 = KeyInfo::GetKey("+sprint", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_RELOAD]") == 0)
    {
        int v4 = KeyInfo::GetKey("+reload", currCl);
    LABEL_55:
        if (v4 == -1)
            v4 = KeyInfo::GetKey("+activatereload", currCl);
    LABEL_44:
        switch (v4)
        {
        case 212: return STBManager::sInst->GetSTBString("INGAME_PRESS_CROSS");
        case 213: return STBManager::sInst->GetSTBString("INGAME_PRESS_CIRCLE");
        case 211: return STBManager::sInst->GetSTBString("INGAME_PRESS_SQUARE");
        case 214: return STBManager::sInst->GetSTBString("INGAME_PRESS_TRIANGLE");
        case 215: return STBManager::sInst->GetSTBString("INGAME_PRESS_R1");
        case 217: return STBManager::sInst->GetSTBString("INGAME_PRESS_R2");
        case 219: return STBManager::sInst->GetSTBString("INGAME_PRESS_R3");
        case 216: return STBManager::sInst->GetSTBString("INGAME_PRESS_L1");
        case 218: return STBManager::sInst->GetSTBString("INGAME_PRESS_L2");
        case 220: return STBManager::sInst->GetSTBString("INGAME_PRESS_L3");
        case 13: return STBManager::sInst->GetSTBString("INGAME_PRESS_START");
        case 27: return STBManager::sInst->GetSTBString("INGAME_PRESS_SELECT");
        case 154: return STBManager::sInst->GetSTBString("INGAME_PRESS_FORWARD");
        case 155: return STBManager::sInst->GetSTBString("INGAME_PRESS_BACK");
        case 156: return STBManager::sInst->GetSTBString("INGAME_PRESS_LEFT");
        case 157: return STBManager::sInst->GetSTBString("INGAME_PRESS_RIGHT");
        default: break;
        }
        return token;
    }
    if (strcmp(token, "[PRESS_STANCE_UP]") == 0)
    {
        int v4 = KeyInfo::GetKey("+moveup", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_STANCE_DOWN]") == 0)
    {
        int v4 = KeyInfo::GetKey("lowerstance", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_WEAPON_SWAP]") == 0)
    {
        int v4 = KeyInfo::GetKey("weapnext", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_LEAN_LEFT]") == 0)
    {
        int v4 = KeyInfo::GetKey("+leanleftswitchnext", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_LEAN_RIGHT]") == 0)
    {
        int v4 = KeyInfo::GetKey("+leanrightswitchnext", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_SELECT]") == 0)
    {
        int v4 = KeyInfo::GetKey("togglemenu", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_START]") == 0)
    {
        int v4 = KeyInfo::GetKey("togglepaused", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[PRESS_CLASS]") == 0)
    {
        int v4 = KeyInfo::GetKey("+class", currCl);
        goto LABEL_44;
    }
    if (strcmp(token, "[USE_LEFT_STICK]") == 0)
        return STBManager::sInst->GetSTBString("INGAME_XBOX_USE_LEFT_STICK");
    if (strcmp(token, "[USE_RIGHT_STICK]") == 0)
        return STBManager::sInst->GetSTBString("INGAME_XBOX_USE_RIGHT_STICK");
    if (strcmp(token, "~l3") == 0)
        return STBManager::sInst->GetSTBString("INGAME_L3_XBOX");
    if (strcmp(token, "~r3") == 0)
        return STBManager::sInst->GetSTBString("INGAME_R3_XBOX");
    return token;
}

// ea: 0x0056E600
bool FEMultiLineText::CheckIfNotTooLong(int num)
{
    if (num < line_avail_num)
        return true;
    if (!cut_off_if_too_long)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 1958;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("MultiLineString is too long"))
            __debugbreak();
    }
    return false;
}

// ea: 0x0057CD60
FEText* FEMultiLineText::Clone()
{
    FEMultiLineText* v3 = (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v3 != nullptr)
    {
        new (v3) FEMultiLineText();
    }
    else
    {
        v3 = nullptr;
    }
    if (v3 != nullptr)
        v3->CopyFrom(this);
    return v3;
}

// ea: 0x0057CDE0
void FEMultiLineText::Draw(int start_line, int end_line)
{
    if (IsShown())
    {
        if (start_line < 0)
            start_line = 0;
        if (end_line > line_num)
            end_line = line_num;
        color32 tmp_color;
        tmp_color.i = color1.i;
        if ((FEText::flags & 8) != 0)
            tmp_color.i = flash_info->GetColor(color1).i;
        // The reference packs ARGB as b | (g<<8) | (r<<16) | (a<<24).  The
        // shift nesting here had the channels reversed, producing
        // 0xBBGGRRAA: alpha landed in the low byte and blue in the high
        // byte, so the vertex colour reached NGL fully transposed.
        int v6 = (int)(tmp_color.c.b
                       | ((tmp_color.c.g
                           | ((tmp_color.c.r
                               | ((unsigned char)(tmp_color.c.a * visibility)
                                  << 8))
                              << 8))
                          << 8));
        int v7 = HIWORD(button_color.i) << 8;
        int v8 = (int)((unsigned char)button_color.c.b
                       | (((unsigned char)button_color.c.g | v7) << 8));
        if (start_line < end_line)
        {
            int v9 = 32 * start_line;
            int v10 = end_line - start_line;
            int start_linea = start_line;
            int end_linea = v10;
            do
            {
                MultiLineString* v11 = &lines[start_linea];
                if (v11->data.mBlock != nullptr
                    && !(v11->data == defaultFileName))
                {
                    float z = GetZvalue();
                    lines[start_linea].Draw(z, v6, v8, scale.x, scale.y,
                                            button_scale, button_y_offset);
                }
                v9 = start_linea * 32 + 32;
                ++start_linea;
                --end_linea;
            }
            while (end_linea != 0);
        }
    }
}

// ea: 0x0057CF20
float FEMultiLineText::GetHeight()
{
    Broc::string::Block* mBlock = text.mBlock;
    const char* v3;
    if (mBlock != nullptr)
        v3 = (const char*)&mBlock[1];
    else
        v3 = defaultFileName;
    unsigned int Width;
    unsigned int Height;
    nglFont* Font = g_femanager.GetFont(font);
    nglGetStringDimensions(Font, v3, &Width, &Height, scale.x, scale.y);
    return (float)(GetLineNum() - 1) * line_spacing + (float)Height;
}

// ea: 0x0057D020
void FEMultiLineText::AdjustForJustification()
{
    nglFont* Font = g_femanager.GetFont(font);
    nglFont* pFont = Font;
    if (Font == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 2024;
        AeAssert::gCurrentExpr = "pFont";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "AdjustForJustification: Could not find font: %s",
                FEManager::font_name_array[this->font].mBuff))
            __debugbreak();
        pFont = nglSysFont;
        if (nglSysFont == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
            AeAssert::gCurrentLine = 2028;
            AeAssert::gCurrentExpr = "pFont";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "AdjustForJustification: Could not find nglSysFont as fallback. Aborting. "))
                __debugbreak();
            return;
        }
        Font = nglSysFont;
    }
    unsigned int width;
    unsigned int height;
    nglGetStringDimensions(Font, " ", &width, &height, scale.x, scale.y);
    float h = (float)height;
    int v3 = 0;
    if (line_num > 0)
    {
        for (int v4 = 0; v3 < line_num; ++v4)
        {
            float w = lines[v4].total_width;
            float x;
            if (GetFlag(16))
            {
                x = xy.x;
            }
            else
            {
                float v7 = xy.x;
                if (GetFlag(32))
                    x = v7 - w;
                else
                    x = v7 - (w * 0.5f);
            }
            float tmp_x = x;
            float v8;
            if (GetFlag(64))
            {
                v8 = (v3 * line_spacing) + xy.y;
            }
            else if (GetFlag(128))
            {
                v8 = (((v3 - line_num + 1) * line_spacing) + xy.y) - h;
            }
            else
            {
                v8 = ((((v3 + 0.5f) - (line_num * 0.5f)) * line_spacing)
                      + xy.y)
                     - (h * 0.5f);
            }
            lines[v4].xy.x = tmp_x;
            lines[v4].xy.y = v8;
            lines[v4].xy.z = 0.0f;
            ++v3;
        }
        Font = pFont;
    }
    int FirstGlyph = Font->Header.FirstGlyph;
    int v11 = Font->Header.NumGlyphs - 1;
    if (65 - FirstGlyph >= 0)
    {
        if (65 - FirstGlyph <= v11)
            v11 = 65 - FirstGlyph;
    }
    else
    {
        v11 = 0;
    }
    nglGlyphInfo* GlyphInfo = Font->GlyphInfo;
    float v14 = (float)GlyphInfo[v11].GlyphSize[1];
    float v15 = (float)GlyphInfo[v11].GlyphOrigin[1];
    int v16 = g_femanager.fonts[1]->Header.FirstGlyph;
    int v17 = g_femanager.fonts[1]->Header.NumGlyphs - 1;
    float v18 = ((v14 * 0.5f) + v15) * scale.y;
    if (33 - v16 >= 0)
    {
        if (33 - v16 <= v17)
            v17 = 33 - v16;
    }
    else
    {
        v17 = 0;
    }
    button_y_offset =
        ((((float)g_femanager.fonts[1]->GlyphInfo[v17].GlyphSize[1] * 0.5f)
          + (float)g_femanager.fonts[1]->GlyphInfo[v17].GlyphOrigin[1])
         * button_scale)
            - v18
        + 1.0f;
}

// ea: 0x00584ED0
void FEMultiLineText::SetScaleAdjustButtons(float sx, float sy)
{
    float v3 = scale.x == 0.0f ? 1.0f : sx / scale.x;
    scale.x = sx;
    scale.y = sy;
    scale.z = 0.0f;
    scale_unselected.x = sx;
    scale_unselected.y = sy;
    scale_unselected.z = 0.0f;
    for (int i = 0; i < line_num; ++i)
    {
        MultiLineString* v7 = &lines[i];
        int button_array_size = v7->button_array_size;
        for (int v8 = 0; v8 < button_array_size; ++v8)
            v7->button_array[v8].x_offset =
                (int)(v7->button_array[v8].x_offset * v3);
    }
    AdjustForJustification();
}

// ea: 0x00584FC0
void FEMultiLineText::SetPos(float x1, float y1)
{
    xy.x = x1;
    xy.y = y1;
    xy.z = 0.0f;
    AdjustForJustification();
}

// ea: 0x00585010
void FEMultiLineText::Animate(math::Mat43* mat, float vis)
{
    char v6 = (char)(PanelAnimObject::flags & 1);
    float v7 = mat->w.v.m128_f32[1];
    math::Position3 v8 = mat->w;
    float v11;
    float v9;
    if (v6 != 0)
    {
        v11 = xy.x + v8.v.m128_f32[0];
        v9 = xy.y + v7;
    }
    else
    {
        v11 = xy_initial.x + v8.v.m128_f32[0];
        v9 = xy_initial.y + v7;
    }
    xy.x = v11;
    xy.y = v9;
    xy.z = 0.0f;
    if (v6 != 0)
    {
        scale.x = scale.x * mat->x.v.m128_f32[0];
        scale.y = 0.0f - (scale.y * mat->y.v.m128_f32[1]);
    }
    else
    {
        float v12 = mat->y.v.m128_f32[1];
        scale.x = mat->x.v.m128_f32[0];
        scale.y = 0.0f - v12;
        scale.z = 0.0f;
    }
    line_spacing = line_spacing_init * scale.y;
    if (v6 != 0)
        SetAlpha(visibility * vis);
    else
        SetAlpha(vis);
    AdjustForJustification();
}

// ea: 0x0058D500
void FEMultiLineText::SetTextNoLocalize(const char* s)
{
    if (line_avail_num == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 1669;
        AeAssert::gCurrentExpr = "line_avail_num != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    const char* v4 = ConvertActionToButton(s);
    int v5 = (int)strlen(v4);
    int i = 1;
    for (int v6 = 0; v6 < v5; ++v6)
    {
        if (v4[v6] == 10)
            ++i;
    }
    if (i - 1 < line_avail_num)
    {
        line_num = i;
    }
    else
    {
        if (!cut_off_if_too_long)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
            AeAssert::gCurrentLine = 1958;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("MultiLineString is too long"))
                __debugbreak();
        }
        line_num = line_avail_num;
    }
    int j = 0;
    if (line_num > 0)
    {
        int v3 = 0;
        int v20 = 0;
        while (1)
        {
            size_t v10 = strcspn(&v4[v3], "\n");
            if (v10 >= 255)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
                AeAssert::gCurrentLine = 1697;
                AeAssert::gCurrentExpr = "index < 255";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                        "Single line length over 255.\n %s", v4))
                    __debugbreak();
            }
            char token[256];
            strncpy(token, &v4[v3], v10);
            token[v10] = 0;
            v3 += (int)v10 + 1;
            lines[v20].Set(token, font, scale.x, button_scale);
            ++j;
            v20 += 32;
            if (j >= line_num)
                break;
        }
    }
    Broc::string::Block* mBlock = lines->data.mBlock;
    const char* v14;
    if (mBlock != nullptr)
        v14 = (const char*)&mBlock[1];
    else
        v14 = defaultFileName;
    text = v14;
    AdjustForJustification();
}

// ea: 0x0058D710
void FEMultiLineText::SetTextAllocNoLocalize(const char* buffer,
                                             int buffer_size)
{
    (void)buffer_size;
    const char* v4 = buffer;
    line_num = 1;
    for (unsigned char i = (unsigned char)*buffer; i != 0; ++v4)
    {
        if (i == 10)
            ++line_num;
        i = (unsigned char)v4[1];
    }
    if (lines != nullptr)
    {
        int count = ((int*)lines)[-1];
        for (int i = 0; i < count; ++i)
            lines[i].~MultiLineString();
        mem_heap_free((int*)lines - 1);
    }
    int line_num = this->line_num;
    int* v9 = (int*)mem_heap_malloc(32 * line_num + 4);
    MultiLineString* v10;
    if (v9 != nullptr)
    {
        v10 = (MultiLineString*)(v9 + 1);
        *v9 = line_num;
        for (int i = 0; i < line_num; ++i)
            new (&v10[i]) MultiLineString();
    }
    else
    {
        v10 = nullptr;
    }
    int v11 = this->line_num;
    lines = v10;
    line_avail_num = v11;
    if (v11 > 0)
    {
        int v12 = 0;
        do
        {
            size_t v14 = strcspn(buffer, "\n");
            if (v14 >= 255)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
                AeAssert::gCurrentLine = 1787;
                AeAssert::gCurrentExpr = "index < 255";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            char token[256];
            strncpy(token, buffer, v14);
            token[v14] = 0;
            lines[v12].Set(token, font, scale.x, button_scale);
            buffer += (int)v14 + 1;
            ++v12;
        }
        while (v12 < this->line_num);
    }
    Broc::string::Block* mBlock = lines->data.mBlock;
    const char* v20;
    if (mBlock != nullptr)
        v20 = (const char*)&mBlock[1];
    else
        v20 = defaultFileName;
    text = v20;
    AdjustForJustification();
}

// ea: 0x0058D8E0
int FEMultiLineText::MakeBox(const char* buffer, int buffer_size, int w,
                             float sc_x, float sc_y, bool save)
{
    (void)buffer_size;
    if (font == FONT_NORMAL)
        return 0;
    int v9 = 0;
    int line_count = 0;
    float cur_width = 0.0f;
    Broc::string current(defaultFileName);
    char lastDelim = 0;
    if (*buffer != 0)
    {
        float v31 = (float)w;
        unsigned int v35 = 0;
        while (1)
        {
            size_t v12 = strcspn(&buffer[v9], " \n\r");
            if (v12 >= 255)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
                AeAssert::gCurrentLine = 1863;
                AeAssert::gCurrentExpr = "index < 255";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            int v13 = (int)v12 + v9;
            char v40 = buffer[v13];
            // IDA represented this as a two-byte local because the original
            // stack frame overlays the token with adjacent scratch locals;
            // strncpy writes up to the asserted 255-byte token limit.
            char word[256];
            strncpy(word, &buffer[v9], v12);
            word[v12] = 0;
            int buffer_index;
            if (v40 != 0)
            {
                buffer_index = v13 + 1;
                if (v40 == 45)
                    strcat(word, "-");
                else if (v40 == 32)
                    strcat(word, " ");
            }
            else
            {
                buffer_index = v13;
            }
            int v18 = strncmp(word, "\x5B", 2u);
            FEMultiLineText* v20 = this;
            if (v18 == 0)
            {
                char nptr[2];
                nptr[0] = word[2];
                nptr[1] = word[3];
                if (nptr[0] < 48 || nptr[0] > 57)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\FEText.cpp";
                    AeAssert::gCurrentLine = 1898;
                    AeAssert::gCurrentExpr =
                        "word[2] >= '0' && word[2] <= '9' && "
                        "\"INVALID FONT TOKEN\"";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                font_index v19 = (font_index)atoi(nptr);
                v20 = this;
                this->font = v19;
                if (nptr[1] != 93)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\FEText.cpp";
                    AeAssert::gCurrentLine = 1900;
                    AeAssert::gCurrentExpr =
                        "word[3] == ']' && \"INVALID FONT TOKEN\"";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                memmove(word, word + 4, strlen(word + 4) + 1);
            }
            float word_width_f = MultiLineString::GetStringWidth(
                word, v20->font, v20->scale.x, v20->button_scale);
            float v30 = word_width_f + cur_width;
            if (v30 > v31 || lastDelim == 10)
            {
                cur_width = word_width_f;
                current.remove_leading(" \n\t\r");
                current.remove_trailing(" \n\t\r");
                if (save && CheckIfNotTooLong(line_count))
                {
                    if (v20->lines == nullptr)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\FEText.cpp";
                        AeAssert::gCurrentLine = 1916;
                        AeAssert::gCurrentExpr = "lines";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                    const char* v23 = current.mBlock != nullptr
                                          ? (const char*)&current.mBlock[1]
                                          : defaultFileName;
                    v20->lines[v35 / 0x20].Set(
                        v23, v20->font, v20->scale.x, v20->button_scale);
                }
                current = word;
                ++line_count;
                v35 += 32;
            }
            else
            {
                current += word;
                cur_width = v30;
            }
            lastDelim = v40;
            if (buffer[buffer_index] == 0)
            {
                current.remove_leading(" \n\t\r");
                current.remove_trailing(" \n\t\r");
                if (current.mBlock != nullptr
                    && current.mBlock->mLength != 0)
                {
                    if (save && CheckIfNotTooLong(line_count))
                    {
                        if (v20->lines == nullptr)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\FEText.cpp";
                            AeAssert::gCurrentLine = 1938;
                            AeAssert::gCurrentExpr = "lines";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert("old cod assert"))
                                __debugbreak();
                        }
                        const char* v24 =
                            current.mBlock != nullptr
                                ? (const char*)&current.mBlock[1]
                                : defaultFileName;
                        v20->lines[v35 / 0x20].Set(
                            v24, v20->font, v20->scale.x,
                            v20->button_scale);
                    }
                    ++line_count;
                    v35 += 32;
                }
            }
            if (buffer[buffer_index] == 0)
                break;
            v9 = buffer_index;
        }
    }
    return line_count;
}

// ea: 0x005917C0
void FEMultiLineText::SetTextBoxNoLocalize(Broc::string s, int w,
                                           float sc_override)
{
    if (line_avail_num == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 1724;
        AeAssert::gCurrentExpr = "line_avail_num != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    const char* v5 = s.mBlock != nullptr ? (const char*)&s.mBlock[1]
                                         : defaultFileName;
    const char* v6 = ConvertActionToButton(v5);
    s = v6;
    int v7 = w;
    float sc_x = scale.x;
    float sc_y = scale.y;
    if (sc_override != -1.0f)
    {
        sc_x = sc_override;
        sc_y = sc_override;
    }
    box_width = v7;
    int mLength = s.mBlock != nullptr ? s.mBlock->mLength : 0;
    const char* v11 = s.mBlock != nullptr ? (const char*)&s.mBlock[1]
                                          : defaultFileName;
    int Box = MakeBox(v11, mLength, v7, sc_x, sc_y, true);
    if (CheckIfNotTooLong(Box - 1))
        line_num = Box;
    else
        line_num = line_avail_num;
    Broc::string::Block* mBlock = lines->data.mBlock;
    const char* v14 = mBlock != nullptr ? (const char*)&mBlock[1]
                                        : defaultFileName;
    text = v14;
    AdjustForJustification();
}

// ea: 0x00591930
void FEMultiLineText::SetTextBoxAllocNoLocalize(Broc::string t, int w,
                                                float sc_override)
{
    int v5 = w;
    float sc_x = scale.x;
    float sc_y = scale.y;
    if (sc_override != -1.0f)
    {
        sc_x = sc_override;
        sc_y = sc_override;
    }
    box_width = w;
    int mLength = t.mBlock != nullptr ? t.mBlock->mLength : 0;
    const char* v9 = t.mBlock != nullptr ? (const char*)&t.mBlock[1]
                                         : defaultFileName;
    int Box = MakeBox(v9, mLength, v5, sc_x, sc_y, false);
    line_num = Box;
    if (Box <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEText.cpp";
        AeAssert::gCurrentLine = 1820;
        AeAssert::gCurrentExpr = "line_num > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (lines != nullptr)
    {
        int count = ((int*)lines)[-1];
        for (int i = 0; i < count; ++i)
            lines[i].~MultiLineString();
        mem_heap_free((int*)lines - 1);
    }
    int line_num = this->line_num;
    int* v14 = (int*)mem_heap_malloc(32 * line_num + 4);
    MultiLineString* v16;
    if (v14 != nullptr)
    {
        v16 = (MultiLineString*)(v14 + 1);
        *v14 = line_num;
        for (int i = 0; i < line_num; ++i)
            new (&v16[i]) MultiLineString();
    }
    else
    {
        v16 = nullptr;
    }
    lines = v16;
    line_avail_num = this->line_num;
    int mLength2 = t.mBlock != nullptr ? t.mBlock->mLength : 0;
    const char* v18 = t.mBlock != nullptr ? (const char*)&t.mBlock[1]
                                          : defaultFileName;
    MakeBox(v18, mLength2, v5, sc_x, sc_y, true);
    Broc::string::Block* mBlock = lines->data.mBlock;
    const char* v20 = mBlock != nullptr ? (const char*)&mBlock[1]
                                        : defaultFileName;
    text = v20;
    AdjustForJustification();
}

// ea: 0x0056FB10
bool FEMenuColorScheme::GetInfo(char index, color32& un, color32& h1,
                                color32& h2)
{
    un.i = color_schemes[index].unselect.i;
    h1.i = color_schemes[index].high1.i;
    h2.i = color_schemes[index].high2.i;
    return color_schemes[index].flash;
}

// ea: 0x0056FB50
bool FEMenuColorScheme::GetInfo(char index, color32& un, color32& sel)
{
    un.i = color_schemes[index].unselect.i;
    sel.i = color_schemes[index].high1.i;
    return color_schemes[index].flash;
}

// ea: 0x0056FB80
int FEMenuColorScheme::GetSchemeFromText(Broc::string& schemeText)
{
    int v1 = 0;
    while (!(schemeText == Broc::string(FEMenuColorSchemeText[v1])))
    {
        if (++v1 >= 17)
            return -1;
    }
    return v1;
}

// ============================================================================
