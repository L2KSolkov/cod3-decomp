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

// ============================================================================
// FETextFlashInfo (20 bytes - ui_types.h verified)
// ============================================================================
class FETextFlashInfo {
public:
    color32 flash_color;      // +0x00
    float   flash_timer;      // +0x04
    float   flash_intensity;  // +0x08
    float   flash_period;     // +0x0C
    bool    reset;            // +0x10

    FETextFlashInfo(color32 col, float period);  // shell.o 0x56B940
    void Update(float time_inc);                 // shell.o 0x56B970
    color32 GetColor(color32 normal_color);      // shell.o 0x56B9B0
    void Reset();                                // shell.o 0x56BA70
};
static_assert(sizeof(FETextFlashInfo) == 20, "FETextFlashInfo size mismatch");

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

// ============================================================================
// FEMenuColorScheme (16-byte scheme records at 0xDF3AE0)
// ============================================================================
class FEMenuColorScheme {
public:
    color32 unselect;     // +0x00
    color32 highlight1;   // +0x04
    color32 highlight2;   // +0x08
    char    flags;        // +0x0C
    char    _pad[3];

    static bool GetInfo(char index, color32& un, color32& h1,
                        color32& h2);   // shell.o 0x56FB10
    static bool GetInfo(char index, color32& un,
                        color32& sel);  // shell.o 0x56FB50
    static int GetSchemeFromText(Broc::string& schemeText);  // shell.o 0x56FB80
};
static_assert(sizeof(FEMenuColorScheme) == 16,
              "FEMenuColorScheme size mismatch");

extern FEMenuColorScheme color_schemes[];  // 0xDF3AE0
extern const char* const FEMenuColorSchemeText[];  // 0xCEF370 (17 entries)

// ea: 0x0056FB10
bool FEMenuColorScheme::GetInfo(char index, color32& un, color32& h1,
                                color32& h2)
{
    un.i = color_schemes[index].unselect.i;
    h1.i = color_schemes[index].highlight1.i;
    h2.i = color_schemes[index].highlight2.i;
    return color_schemes[index].flags != 0;
}

// ea: 0x0056FB50
bool FEMenuColorScheme::GetInfo(char index, color32& un, color32& sel)
{
    un.i = color_schemes[index].unselect.i;
    sel.i = color_schemes[index].highlight1.i;
    return color_schemes[index].flags != 0;
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
