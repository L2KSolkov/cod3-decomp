// ============================================================================
// g_inspector.cpp - InspectorManager debug menu system (game2.o)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_inspector.h"
#include "game/logic/g_local.h"
#include "ngl/ngl_scene.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// Cross-object externs
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern level_locals_t level;
extern float scaleScalar;  // ?scaleScalar (render.o)
extern float RE_Text_Paint(float x, float y, int font, float scale,
                           const float* color, const char* text, float a7,
                           int a8, int a9);  // ?RE_Text_Paint (render.o)
extern int RE_Text_Width(const char* text, int font, float scale,
                         float charWidth, int limit);  // ?RE_Text_Width

// ShaderCommon glow state (cdGlowShader.o / render.o)
namespace ShaderCommon {
extern float gGlowIntensity;  // ?gGlowIntensity@ShaderCommon@@3MA
extern float gGlowExpansion;  // ?gGlowExpansion@ShaderCommon@@3MA
extern int   gGlowEnable;     // ?gGlowEnable@ShaderCommon@@3HA
extern int   gGlowPasses;     // ?gGlowPasses@ShaderCommon@@3HA
extern bool  gGlowGodRays;    // ?gGlowGodRays@ShaderCommon@@3_NA
extern float gGlowBrighten;   // ?gGlowBrighten@ShaderCommon@@3MA
}

// FogConfig helpers (render.o)
namespace FogConfig {
void GetEnabled(int& enable);                    // ?GetEnabled@FogConfig@@YAXAAH@Z
void GetColor(float& red, float& green, float& blue);  // ?GetColor@FogConfig@@YAXAAM00@Z
void GetRange(float& n, float& f);               // ?GetRange@FogConfig@@YAXAAM0@Z
void GetVal(float& s, float& e);                 // ?GetVal@FogConfig@@YAXAAM0@Z
void SetColor(float r, float g, float b);        // ?SetColor@FogConfig@@YAXMMM@Z
void SetRange(float n, float f);                 // ?SetRange@FogConfig@@YAXMM@Z
void SetVal(float s, float e);                   // ?SetVal@FogConfig@@YAXMM@Z
void SetEnabled(int enable);                     // ?SetEnabled@FogConfig@@YAXH@Z
}

// game2.o data globals (glow + fog editor state, verified against the map)
int   g_GlowEnable;       // @ 0x011C86F8
int   g_GlowPasses;       // @ 0x011C86FC
float g_GlowIntensity;    // @ 0x011C8700
float g_GlowExpansion;    // @ 0x011C8704
float g_GlowBrightness;   // @ 0x011C8708
int   g_GlowGodRaysEnable;// @ 0x011C870C
int   g_FogEnable;        // @ 0x011C8710
float g_FogNear;          // @ 0x011C8714
float g_FogFar;           // @ 0x011C8718
float g_FogEnd;           // @ 0x011C871C
float g_FogRed;           // @ 0x011C8720
float g_FogGreen;         // @ 0x011C8724
float g_FogBlue;          // @ 0x011C8728
float g_FogStart;         // @ 0x012F3DF8

// cg.o / core.o externs used by Render
extern int gScreenshotInProgress;  // ?gScreenshotInProgress
extern const char* sBuildId;       // ?sBuildId
enum { CUBEMAPSHOT_NONE = 0 };

_INSPECTOR_MENU g_inspectorRootMenu;   // ?g_inspectorRootMenu (game2.o)
InspectorManager* g_inspectorManager;  // ?g_inspectorManager (game2.o)

// Minimal controller view (full implementation in input/controller.cpp).
// ButtonIndex values verified against ReadKeys disassembly.
struct controller {
    enum ButtonIndex {
        LEFTBUTTON = 0,
        DOWNBUTTON = 1,
        RIGHTBUTTON = 2,
        UPBUTTON = 3,
        SQUARE = 4,
        CIRCLE = 5,
        TRIANGLE = 7,
        R1 = 8,
        L1 = 9,
        R2 = 10,
        L2 = 11,
        R3 = 12,
        SELECT = 15,
    };
    static controller* inst();
    int button_value(ButtonIndex i_button, int* p_controller);
    bool button_pressed(ButtonIndex i_button, int* p_controller);
};

// ============================================================================
// InspectorManager::InspectorManager - ea: 0x4F7100
// ============================================================================
InspectorManager::InspectorManager()
{
    m_headingRgba[0] = 1.0f;
    m_headingRgba[1] = 1.0f;
    m_headingRgba[2] = 1.0f;
    m_headingRgba[3] = 1.0f;
    m_textRgba[0] = 1.0f;
    m_textRgba[1] = 1.0f;
    m_textRgba[2] = 0.0f;
    m_textRgba[3] = 1.0f;
    m_currentRgba[0] = 1.0f;
    m_currentRgba[1] = 1.0f;
    m_currentRgba[2] = 0.0f;
    m_currentRgba[3] = 1.0f;
    m_KEY_INSPECTOR_ONOFF = 0;
    m_KEY_SELECT = 0;
    m_KEY_UP = 0;
    m_KEY_DOWN = 0;
    m_KEY_MENU_BACK = 0;
    m_KEY_LEFT = 0;
    m_KEY_LEFT_FAST = 0;
    m_KEY_LEFT_VERY_FAST = 0;
    m_KEY_RIGHT = 0;
    m_KEY_RIGHT_FAST = 0;
    m_KEY_RIGHT_VERY_FAST = 0;
    m_KEY_LEFT_DEBOUNCE = 0;
    m_KEY_LEFT_FAST_DEBOUNCE = 0;
    m_KEY_LEFT_VERY_FAST_DEBOUNCE = 0;
    m_KEY_RIGHT_DEBOUNCE = 0;
    m_KEY_RIGHT_FAST_DEBOUNCE = 0;
    m_KEY_RIGHT_VERY_FAST_DEBOUNCE = 0;
}

// ============================================================================
// InspectorManager::ResetKeys - ea: 0x4EC050
// ============================================================================
void InspectorManager::ResetKeys()
{
    m_KEY_INSPECTOR_ONOFF = 0;
    m_KEY_SELECT = 0;
    m_KEY_UP = 0;
    m_KEY_DOWN = 0;
    m_KEY_MENU_BACK = 0;
    m_KEY_LEFT = 0;
    m_KEY_LEFT_FAST = 0;
    m_KEY_LEFT_VERY_FAST = 0;
    m_KEY_RIGHT = 0;
    m_KEY_RIGHT_FAST = 0;
    m_KEY_RIGHT_VERY_FAST = 0;
    m_KEY_LEFT_DEBOUNCE = 0;
    m_KEY_LEFT_FAST_DEBOUNCE = 0;
    m_KEY_LEFT_VERY_FAST_DEBOUNCE = 0;
    m_KEY_RIGHT_DEBOUNCE = 0;
    m_KEY_RIGHT_FAST_DEBOUNCE = 0;
    m_KEY_RIGHT_VERY_FAST_DEBOUNCE = 0;
}

// ============================================================================
// InspectorManager::ReadKeys - ea: 0x4EC090
// ============================================================================
void InspectorManager::ReadKeys()
{
    bool v1 = false;
    if (level.framenum >= 10)
    {
        m_KEY_SELECT = controller::inst()->button_pressed(
            (controller::ButtonIndex)(controller::SQUARE | controller::DOWNBUTTON),
            nullptr);
        m_KEY_UP = controller::inst()->button_pressed(controller::UPBUTTON, nullptr);
        m_KEY_DOWN = controller::inst()->button_pressed(controller::DOWNBUTTON, nullptr);
        m_KEY_MENU_BACK = controller::inst()->button_pressed(
            controller::TRIANGLE, nullptr);
        m_KEY_LEFT = controller::inst()->button_value(controller::LEFTBUTTON, nullptr);
        m_KEY_LEFT_FAST = controller::inst()->button_value(controller::L1, nullptr);
        m_KEY_LEFT_VERY_FAST = controller::inst()->button_value(controller::L2, nullptr);
        m_KEY_RIGHT = controller::inst()->button_value(controller::RIGHTBUTTON, nullptr);
        m_KEY_RIGHT_FAST = controller::inst()->button_value(controller::R1, nullptr);
        m_KEY_RIGHT_VERY_FAST = controller::inst()->button_value(controller::R2, nullptr);
        m_KEY_LEFT_DEBOUNCE = controller::inst()->button_pressed(
            controller::LEFTBUTTON, nullptr);
        m_KEY_LEFT_FAST_DEBOUNCE = controller::inst()->button_pressed(
            controller::L1, nullptr);
        m_KEY_LEFT_VERY_FAST_DEBOUNCE = controller::inst()->button_pressed(
            controller::L2, nullptr);
        m_KEY_RIGHT_DEBOUNCE = controller::inst()->button_pressed(
            controller::RIGHTBUTTON, nullptr);
        m_KEY_RIGHT_FAST_DEBOUNCE = controller::inst()->button_pressed(
            controller::R1, nullptr);
        m_KEY_RIGHT_VERY_FAST_DEBOUNCE = controller::inst()->button_pressed(
            controller::R2, nullptr);
        if (m_data.active != 0)
        {
            m_KEY_INSPECTOR_ONOFF = controller::inst()->button_pressed(
                controller::SELECT, nullptr);
        }
        else
        {
            if (controller::inst()->button_pressed(controller::SELECT, nullptr))
            {
                v1 = controller::inst()->button_value(controller::R3, nullptr) != 0;
            }
            m_KEY_INSPECTOR_ONOFF = v1;
        }
    }
    else
    {
        ResetKeys();
    }
}

// ============================================================================
// InspectorManager::AddSubMenu - ea: 0x4EC260
// ============================================================================
_INSPECTOR_MENU* InspectorManager::AddSubMenu(_INSPECTOR_MENU* parent,
                                              char* heading)
{
    if (parent == nullptr)
        parent = m_data.rootMenu;
    _INSPECTOR_MENU_ITEM* v4 =
        (_INSPECTOR_MENU_ITEM*)mem_heap_malloc(0x14u);
    m_data.memory += 20;
    _INSPECTOR_MENU* menu = (_INSPECTOR_MENU*)mem_heap_malloc(0x18u);
    m_data.memory += 24;
    memset(v4, 0, sizeof(*v4));
    v4->text = (char*)mem_heap_malloc(strlen(heading) + 1);
    m_data.memory += (int)strlen(heading) + 1;
    strcpy(v4->text, heading);
    v4->next = nullptr;
    v4->prev = nullptr;
    v4->value.vp = menu;
    menu->heading = (char*)mem_heap_malloc(strlen(heading) + 1);
    m_data.memory += (int)strlen(heading) + 1;
    strcpy(menu->heading, heading);
    menu->numItems = 0;
    menu->first = nullptr;
    menu->parentMenu = parent;
    ++parent->numItems;
    _INSPECTOR_MENU_ITEM* first = parent->first;
    if (first != nullptr)
    {
        for (; first->next != nullptr; first = first->next)
            ;
        first->next = v4;
        v4->prev = first;
    }
    else
    {
        parent->first = v4;
        parent->lastCurrentItem = v4;
        v4->prev = nullptr;
    }
    parent->last = v4;
    m_data.currentItem = m_data.currentMenu->first;
    return menu;
}

// ============================================================================
// InspectorManager::AddItem - ea: 0x4EC3B0
// ============================================================================
_INSPECTOR_MENU_ITEM* InspectorManager::AddItem(_INSPECTOR_MENU* parent,
                                                char* text, void* value, int type)
{
    if (parent == nullptr)
        parent = m_data.rootMenu;
    _INSPECTOR_MENU_ITEM* v6 =
        (_INSPECTOR_MENU_ITEM*)mem_heap_malloc(0x14u);
    m_data.memory += 20;
    v6->type = type;
    v6->text = (char*)mem_heap_malloc(strlen(text) + 1);
    m_data.memory += (int)strlen(text) + 1;
    strcpy(v6->text, text);
    v6->value.vp = value;
    v6->next = nullptr;
    ++parent->numItems;
    _INSPECTOR_MENU_ITEM* first = parent->first;
    if (first != nullptr)
    {
        for (; first->next != nullptr; first = first->next)
            ;
        first->next = v6;
        v6->prev = first;
    }
    else
    {
        parent->first = v6;
        parent->lastCurrentItem = v6;
        v6->prev = nullptr;
    }
    parent->last = v6;
    m_data.currentItem = m_data.currentMenu->first;
    return v6;
}

// ============================================================================
// InspectorManager::FreeSubMenu - ea: 0x4EC490
// ============================================================================
void InspectorManager::FreeSubMenu(_INSPECTOR_MENU* menu)
{
    _INSPECTOR_MENU_ITEM* first = menu->first;
    int n = 0;
    if (menu->numItems > 0)
    {
        do
        {
            if (first->type == 0)
                FreeSubMenu((_INSPECTOR_MENU*)first->value.vp);
            if (first->text != nullptr)
                mem_heap_free(first->text);
            _INSPECTOR_MENU_ITEM* next = first->next;
            mem_heap_free(first);
            ++n;
            first = next;
        } while (n < menu->numItems);
    }
    if (menu == m_data.rootMenu)
    {
        menu->first = nullptr;
        menu->numItems = 0;
    }
    else
    {
        if (menu->heading != nullptr)
            mem_heap_free(menu->heading);
        mem_heap_free(menu);
    }
}

// ============================================================================
// InspectorManager::UpdateInput - ea: 0x4EC530
// ============================================================================
void InspectorManager::UpdateInput()
{
    ReadKeys();
    if (m_KEY_INSPECTOR_ONOFF != 0)
        m_data.active ^= 1u;
}

// ============================================================================
// InspectorManager::ProfilerUpdate - ea: 0x4EC550
// ============================================================================
void InspectorManager::ProfilerUpdate()
{
}

// ============================================================================
// InspectorManager::RenderStart - ea: 0x4EC560
// ============================================================================
int InspectorManager::RenderStart()
{
    nglListBeginScene(NGLSCENE_PARENT);
    return 1;
}

// ============================================================================
// InspectorManager::RenderFinish - ea: 0x4EC570
// ============================================================================
void InspectorManager::RenderFinish()
{
    nglListEndScene();
}

// ============================================================================
// InspectorManager::RenderSetStates - ea: 0x4EC580
// ============================================================================
void InspectorManager::RenderSetStates()
{
}

// ============================================================================
// InspectorManager::RenderRestoreStates - ea: 0x4EC590
// ============================================================================
void InspectorManager::RenderRestoreStates()
{
}

// ============================================================================
// InspectorManager::SetFontColor - ea: 0x4EC5A0
// ============================================================================
void InspectorManager::SetFontColor(float r, float g, float b, float a)
{
    m_currentRgba[0] = r;
    m_currentRgba[1] = g;
    m_currentRgba[2] = b;
    m_currentRgba[3] = a;
}

// ============================================================================
// InspectorManager::Print - ea: 0x4EC5D0
// ============================================================================
void InspectorManager::Print(char* text, int x, int y, float scale)
{
    int black[4];
    memset(black, 0, 12);
    black[3] = 0x3F800000;  // 1.0f
    float ya = (float)y;
    float xa = (float)x;
    RE_Text_Paint(xa + 2.0f, ya + 2.0f, 5, scaleScalar * scale,
                  (const float*)black, text, 0, 0, 0);
    RE_Text_Paint(xa, ya, 5, scaleScalar * scale, m_currentRgba, text, 0, 0, 0);
}

// ============================================================================
// InspectorManager::GetTextSize - ea: 0x4EC690
// ============================================================================
void InspectorManager::GetTextSize(char* pString, int* xSize, int* ySize,
                                   float scale)
{
    if (xSize != nullptr)
        *xSize = RE_Text_Width(pString, 5, scale, 0.0f, 0);
    if (ySize != nullptr)
        *ySize = 17;
}

// ============================================================================
// InspectorManager::FreeAll - ea: 0x4F7180
// ============================================================================
void InspectorManager::FreeAll()
{
    if (m_data.rootMenu != nullptr)
    {
        FreeSubMenu(m_data.rootMenu);
        m_data.rootMenu->first = nullptr;
        m_data.rootMenu->last = nullptr;
        m_data.rootMenu->numItems = 0;
        m_data.rootMenu->parentMenu = nullptr;
        m_data.rootMenu->lastCurrentItem = nullptr;
        m_data.memory = 0;
    }
}

// ============================================================================
// InspectorManager::Update - ea: 0x4F71C0
// Menu navigation + per-item value adjustment.
// ============================================================================
void InspectorManager::Update()
{
    if (!m_data.active)
        return;
    g_GlowIntensity = ShaderCommon::gGlowIntensity;
    g_GlowExpansion = ShaderCommon::gGlowExpansion;
    g_GlowEnable = ShaderCommon::gGlowEnable;
    g_GlowPasses = ShaderCommon::gGlowPasses;
    g_GlowGodRaysEnable = ShaderCommon::gGlowGodRays ? 1 : 0;
    g_GlowBrightness = ShaderCommon::gGlowBrighten;
    FogConfig::GetEnabled(g_FogEnable);
    FogConfig::GetColor(g_FogRed, g_FogGreen, g_FogBlue);
    FogConfig::GetRange(g_FogNear, g_FogFar);
    FogConfig::GetVal(g_FogStart, g_FogEnd);

    _INSPECTOR_MENU_ITEM* currentItem = m_data.currentItem;
    if (currentItem)
    {
        if (m_KEY_UP)
        {
            _INSPECTOR_MENU_ITEM* prev = currentItem->prev;
            if (!prev)
                prev = m_data.currentMenu->last;
            m_data.currentItem = prev;
        }
        if (m_KEY_DOWN)
        {
            _INSPECTOR_MENU_ITEM* next = m_data.currentItem->next;
            if (!next)
                next = m_data.currentMenu->first;
            m_data.currentItem = next;
        }
        _INSPECTOR_MENU_ITEM* v5 = m_data.currentItem;
        float v6;
        float angle;
        int veryFast = 0;
        if ((0x20000 & v5->type) != 0)
        {
            if (m_KEY_LEFT_DEBOUNCE || m_KEY_LEFT_FAST_DEBOUNCE
                || m_KEY_LEFT_VERY_FAST_DEBOUNCE)
            {
                v6 = -1.0f;
                angle = -1.0f;
                if (m_KEY_LEFT_FAST_DEBOUNCE)
                {
                    v6 = -12.0f;
                    angle = -12.0f;
                }
                veryFast = m_KEY_LEFT_VERY_FAST_DEBOUNCE;
            }
            else if (!m_KEY_RIGHT_DEBOUNCE && !m_KEY_RIGHT_FAST_DEBOUNCE
                     && !m_KEY_RIGHT_VERY_FAST_DEBOUNCE)
            {
                v6 = 0.0f;
                angle = v6;
                goto input_done;
            }
            else
            {
                v6 = 1.0f;
                angle = 1.0f;
                if (m_KEY_RIGHT_FAST_DEBOUNCE)
                {
                    v6 = 12.0f;
                    angle = 12.0f;
                }
                veryFast = m_KEY_RIGHT_VERY_FAST_DEBOUNCE;
            }
        }
        else if (m_KEY_LEFT || m_KEY_LEFT_FAST || m_KEY_LEFT_VERY_FAST)
        {
            v6 = -1.0f;
            angle = -1.0f;
            if (m_KEY_LEFT_FAST)
            {
                v6 = -12.0f;
                angle = -12.0f;
            }
            veryFast = m_KEY_LEFT_VERY_FAST;
        }
        else if (!m_KEY_RIGHT && !m_KEY_RIGHT_FAST && !m_KEY_RIGHT_VERY_FAST)
        {
            v6 = 0.0f;
            angle = v6;
            goto input_done;
        }
        else
        {
            v6 = 1.0f;
            angle = 1.0f;
            if (m_KEY_RIGHT_FAST)
            {
                v6 = 12.0f;
                angle = 12.0f;
            }
            veryFast = m_KEY_RIGHT_VERY_FAST;
        }
        if (veryFast)
            v6 = v6 * 80.0f;
        angle = v6;
    input_done:
        int v8 = (int)v6;
        if ((0x10000 & v5->type) == 0)
        {
            switch (v5->type & 0xFFFF)
            {
            case 0u:  // submenu
                if (m_KEY_SELECT)
                {
                    m_data.currentMenu->lastCurrentItem = v5;
                    _INSPECTOR_MENU* vp = (_INSPECTOR_MENU*)v5->value.vp;
                    m_data.currentMenu = vp;
                    m_data.currentItem = vp->lastCurrentItem;
                }
                break;
            case 1u:  // int adjust
                *v5->value.ip += v8;
                if ((0x40000 & v5->type) != 0)
                {
                    if (*v5->value.ip < 0)
                        *v5->value.ip = 0;
                }
                if ((0x80000 & v5->type) != 0)
                {
                    if (*v5->value.ip > 0)
                        *v5->value.ip = 1;
                }
                break;
            case 2u:  // int toggle
                if (m_KEY_SELECT)
                    *v5->value.ip ^= 1u;
                break;
            case 3u:
                *v5->value.fp = *v5->value.fp + v6;
                goto clamp_float;
            case 4u:
                *v5->value.fp = (v6 * 5.0f) + *v5->value.fp;
                goto clamp_float;
            case 5u:
                *v5->value.fp = (v6 * 1000.0f) + *v5->value.fp;
                goto clamp_float;
            case 6u:
                *v5->value.fp = (v6 * 0.05f) + *v5->value.fp;
                goto clamp_float;
            case 7u:
                *v5->value.fp = (v6 * 0.005f) + *v5->value.fp;
                goto clamp_float;
            case 8u:
                *v5->value.fp = (v6 * 0.0002f) + *v5->value.fp;
            clamp_float:
                if ((0x40000 & v5->type) != 0)
                {
                    if (*v5->value.fp < 0.0f)
                        *v5->value.fp = 0.0f;
                }
                if ((0x80000 & v5->type) != 0)
                {
                    if (*v5->value.fp > 1.0f)
                        *v5->value.fp = 1.0f;
                }
                break;
            case 9u:
                *v5->value.fp = (v6 * 0.005f) + *v5->value.fp;
                if (*v5->value.fp < 0.0f)
                    *v5->value.fp = 0.0f;
                if (*v5->value.fp > 1.0f)
                    *v5->value.fp = 1.0f;
                break;
            case 0xAu:
                *v5->value.fp = (v6 * 0.01f) + *v5->value.fp;
                if (*v5->value.fp < 0.0f)
                    *v5->value.fp = 0.0f;
                if (*v5->value.fp > 2.0f)
                    *v5->value.fp = 2.0f;
                break;
            case 0xBu:
                *v5->value.fp = *v5->value.fp + v6;
                if (*v5->value.fp < 0.0f)
                    *v5->value.fp = 0.0f;
                if (*v5->value.fp > 255.0f)
                    *v5->value.fp = 255.0f;
                break;
            case 0xCu:  // angle (radians)
            {
                float* v10 = v5->value.fp;
                float v11 = (float)fmod(angle * 0.017453292f + *v10
                                        + 251.32741f, 6.283185482025146f);
                float v12 = v11;
                if (v11 >= 3.1415927f)
                    v12 = v11 - 6.2831855f;
                *v10 = v12;
                break;
            }
            case 0xDu:  // angle (degrees)
            {
                float* v13 = v5->value.fp;
                float v14 = (float)fmod(*v13 * 3.1415927f * 0.0055555557f
                                        + angle * 0.017453292f + 251.32741f,
                                        6.283185482025146f);
                float v15 = v14;
                if (v14 >= 3.1415927f)
                    v15 = v14 - 6.2831855f;
                *v13 = (v15 * 180.0f) * 0.31830987f;
                break;
            }
            case 0xEu:
                if (m_KEY_SELECT)
                    *v5->value.ip = 1;
                break;
            case 0xFu:
                *v5->value.ucp += v8;
                break;
            case 0x10u:
                if (m_KEY_SELECT)
                    *v5->value.ucp ^= 1u;
                break;
            case 0x11u:
                if (m_KEY_SELECT)
                    v5->value.fn();
                break;
            case 0x12u:
                if (m_KEY_SELECT)
                {
                    _INSPECTOR_MENU* currentMenu = m_data.currentMenu;
                    _INSPECTOR_MENU_ITEM* first = currentMenu->first;
                    int v31 = 0;
                    if (first != currentMenu->last)
                    {
                        do
                        {
                            if (first == v5)
                                break;
                            first = first->next;
                            ++v31;
                        } while (first != m_data.currentMenu->last);
                    }
                    ((void(*)(int))v5->value.vp)(v31);
                }
                break;
            case 0x16u:
            case 0x17u:
                ((void(*)(float))v5->value.vp)(v6);
                break;
            case 0x18u:
                ((void(*)(float))v5->value.vp)(v6 * 0.005f);
                break;
            default:
                break;
            }
        }
    }
    if (m_KEY_MENU_BACK)
    {
        _INSPECTOR_MENU* v32 = m_data.currentMenu;
        if (v32->parentMenu)
        {
            v32->lastCurrentItem = m_data.currentItem;
            _INSPECTOR_MENU* parentMenu = m_data.currentMenu->parentMenu;
            m_data.currentMenu = parentMenu;
            m_data.currentItem = parentMenu->lastCurrentItem;
        }
        else
        {
            m_data.active = 0;
        }
    }
    ShaderCommon::gGlowIntensity = g_GlowIntensity;
    ShaderCommon::gGlowExpansion = g_GlowExpansion;
    ShaderCommon::gGlowBrighten = g_GlowBrightness;
    ShaderCommon::gGlowPasses = g_GlowPasses;
    ShaderCommon::gGlowGodRays = g_GlowGodRaysEnable != 0;
    ShaderCommon::gGlowEnable = g_GlowEnable;
    if (g_FogNear > g_FogFar)
        g_FogNear = g_FogFar;
    if (g_FogStart > g_FogEnd)
        g_FogStart = g_FogEnd;
    FogConfig::SetColor(g_FogRed, g_FogGreen, g_FogBlue);
    FogConfig::SetRange(g_FogNear, g_FogFar);
    FogConfig::SetVal(g_FogStart, g_FogEnd);
    FogConfig::SetEnabled(g_FogEnable);
}

// ============================================================================
// InspectorManager::Initialise - ea: 0x50E250
// ============================================================================
void InspectorManager::Initialise()
{
    m_data.active = 0;
    m_data.rootMenu = &g_inspectorRootMenu;
    m_data.currentMenu = &g_inspectorRootMenu;
    m_data.currentItem = nullptr;
    SetupUserMenus();
}

// ============================================================================
// InspectorManager::Render - ea: 0x50BED0
// ============================================================================
void InspectorManager::Render()
{
    if (gScreenshotInProgress != 0
        || cgGlobal.cubemapShot != CUBEMAPSHOT_NONE)
        return;
    nglListBeginScene(NGLSCENE_PARENT);
    g_debugThread.Render();
    UserRenderHook();
    if (m_data.active == 0)
    {
        Print((char*)sBuildId, 15, 40, 0.55f);
        nglListEndScene();
        return;
    }
    if (m_data.currentItem == nullptr)
    {
        Print((char*)"Empty Menu", 40, 40, 0.55f);
        return;
    }
    m_currentRgba[0] = m_headingRgba[0];
    m_currentRgba[1] = m_headingRgba[1];
    m_currentRgba[2] = m_headingRgba[2];
    m_currentRgba[3] = m_headingRgba[3];
    char insp_s[208];
    char insp_val[208];
    char selected[4];
    _INSPECTOR_MENU* currentMenu = m_data.currentMenu;
    sprintf(insp_s, "---- %s ----", currentMenu->heading);
    Print(insp_s, 40, 40, 0.55f);
    m_currentRgba[0] = m_textRgba[0];
    m_currentRgba[1] = m_textRgba[1];
    m_currentRgba[2] = m_textRgba[2];
    m_currentRgba[3] = m_textRgba[3];
    _INSPECTOR_MENU_ITEM* first = m_data.currentMenu->first;
    int v11 = 57;
    if (first == nullptr)
    {
        nglListEndScene();
        return;
    }
    do
    {
        *selected = first != m_data.currentItem ? 32 : 62;
        switch (first->type & 0xFFFF)
        {
        case 0u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "->");
            break;
        case 1u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%d", *first->value.ip);
            break;
        case 2u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%s", *first->value.ip == 0 ? "OFF" : "ON");
            break;
        case 3u:
        case 4u:
        case 5u:
        case 6u:
        case 7u:
        case 8u:
        case 0xBu:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%g", *first->value.fp);
            break;
        case 9u:
        case 0xAu:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%1.3f", *first->value.fp);
            break;
        case 0xCu:
        {
            float v = (float)fmod(*first->value.fp + 251.32741f,
                                  6.283185482025146f) * 180.0f * 0.31830987f;
            if (v > 180.0f)
                v = v - 360.0f;
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%4.2f", v);
            break;
        }
        case 0xDu:
        {
            float v = (float)fmod(*first->value.fp * 3.1415927f * 0.0055555557f
                                  + 251.32741f,
                                  6.283185482025146f) * 180.0f * 0.31830987f;
            if (v > 180.0f)
                v = v - 360.0f;
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%4.2f", v);
            break;
        }
        case 0xEu:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%s", *first->value.ip == 0 ? "-" : "SELECTED");
            break;
        case 0xFu:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%d", *first->value.ucp);
            break;
        case 0x10u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%s", *first->value.ucp == 0 ? "OFF" : "ON");
            break;
        case 0x11u:
        case 0x12u:
            sprintf(insp_s, "%s %s", selected, first->text);
            insp_val[0] = 0;
            break;
        case 0x16u:
        case 0x17u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%d", first->value.ifn(0));
            break;
        case 0x18u:
            sprintf(insp_s, "%s %s", selected, first->text);
            sprintf(insp_val, "%g", first->value.ffn(0));
            break;
        default:
            break;
        }
        Print(insp_s, 40, v11, 0.55f);
        if (insp_val[0] != 0)
            Print(insp_val, 290, v11, 0.55f);
        first = first->next;
        v11 += 17;
    } while (first != nullptr);
    nglListEndScene();
}
