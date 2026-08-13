// ============================================================================
// fe_text.cpp - FETextFlashInfo, FEMenuColorScheme, MultiLineString (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"

#include <math.h>
#include <string.h>

#include "ngl/nglFont.h"

extern float sNaN;                       // ?sNaN@@3MA @ 0x10F19D0
extern int cg_widescreen_integer;        // cg.o
extern float widescreen_scale_0;         // shell.o data @ 0xF30D00-ish
extern const char* const defaultFileName;  // 0xCD67AE
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);            // core.o
extern bool CompareButton(const char* text, const char* button,
                          const char** buttonCode,
                          const char* would_be_button_code,
                          int& length);  // fe_util.cpp

extern FEManager g_femanager;

extern float GetXScalingForHUD(int window);   // cg.o
extern float GetYScalingForHUD(int window);   // cg.o
extern float GetPreviousHUDXPos(float pos, int window, char justification,
                                float width);   // cg.o
extern float GetCurrentHUDXPos(float pos, int window, char justification,
                               float width);    // cg.o
extern float GetCurrentHUDYPos(float pos, int window, char justification,
                               float height);   // cg.o
extern float GetPreviousHUDYPos(float pos, int window, char justification,
                                float height);  // cg.o
extern float GetYScalingForWindow(int window);  // fe_util.cpp
extern float widescreen_scale;                  // shell.o data @ 0xDF43C8

class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A
    const char* GetSTBString(const char* pszReference);  // core.o
    const char* GetSTBString(unsigned int hash);         // core.o
};

// ============================================================================
// FEText (112 bytes - ui_types.h verified)
// ============================================================================

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
        if (v14 < 0.0f || v14 > 1000.0f)
            v14 = 1148846080.0f;
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
            float x_scale = GetXScalingForHUD(old_viewport);
            float y_scale = GetYScalingForHUD(old_viewport);
            float x_pos = GetPreviousHUDXPos(GetX(), old_viewport,
                                             (char)justification, just_width);
            float y_pos = GetPreviousHUDYPos(GetY(), old_viewport,
                                             (char)justification, just_height);
            scale.x = scale.x * (1.0f / x_scale);
            scale_unselected.x = scale_unselected.x * (1.0f / x_scale);
            scale.y = scale.y * (1.0f / y_scale);
            scale_unselected.y = scale_unselected.y * (1.0f / y_scale);
            SetPos(x_pos, y_pos);
        }
        if (viewport != 0)
        {
            float x_scalea = GetXScalingForHUD(viewport);
            float y_scalea = GetYScalingForHUD(viewport);
            float just_widtha = GetCurrentHUDXPos(
                GetX(), viewport, (char)justification, just_width);
            float justificationa = GetCurrentHUDYPos(
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
                unsigned int v33 = tmp_color.c.b | ((tmp_color.c.g | (tmp_color.c.r << 8)) << 8);
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
            unsigned int v34 = tmp_color.c.b | ((tmp_color.c.g | (tmp_color.c.r << 8)) << 8);
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
extern const char* const FEMenuColorSchemeText[];  // 0xCEF370 (17 entries)

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
// MultiLineString (32 bytes) - verified against IDA
// ============================================================================
struct MultiLineButtons {
    short text_start_index;  // +0x00
    short x_offset;          // +0x02
};

class MultiLineString {
public:
    font_index     font;             // +0x00
    Broc::vector   xy;               // +0x04
    float          total_width;      // +0x10
    Broc::string   data;             // +0x14
    MultiLineButtons* button_array;  // +0x18
    int            button_array_size;  // +0x1C

    MultiLineString();               // shell.o 0x56CC40
    ~MultiLineString();              // shell.o 0x56CD00
    void CopyFrom(MultiLineString* fet);  // shell.o 0x56CD30
    void AdjustForScale(float scale_factor);  // shell.o 0x56CDE0
    static int ConvertStringToButtonCode(const char* text,
                                         const char** buttonCode,
                                         const Broc::string& whole_string);
                                      // shell.o 0x56CEE0
    void Draw(float z, int col, int button_col,
              float scale_x, float scale_y, float button_scale,
              float button_y_offset);  // shell.o 0x57C980
    static float GetWidth(const char* text, float scale, font_index f);
                                      // shell.o 0x57CC20
    static float GetStringWidth(const char* text, font_index f, float scale,
                                float button_scale);  // shell.o 0x584910
    void Set(const char* d, font_index f, float scale,
             float button_scale);    // shell.o 0x58D3B0
private:
    void ParseForButtons(float scale, float button_scale);  // shell.o 0x584AB0
};
static_assert(sizeof(MultiLineString) == 32, "MultiLineString size mismatch");
static_assert(sizeof(MultiLineButtons) == 4, "MultiLineButtons size mismatch");

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
    total_width = 0.0f;
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
            Broc::string v35 = v34 + Broc::string(buttonCode);
            Broc::string v36 = v35 + v33;
            data = v36;
            v5 = (unsigned int)(strlen(buttonCode) + v8);
            if (data.mBlock != nullptr && v5 < (unsigned int)data.length())
            {
                cur_width = GetWidth(buttonCode, button_scale,
                                     FONT_GEMFONTONE) + cur_width;
                button_array[2 * i + 1].text_start_index = (short)v5;
                button_array[2 * i + 1].x_offset = (short)cur_width;
            }
            else
            {
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
            total_width = GetWidth(data.c_str(), scale, font);
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
