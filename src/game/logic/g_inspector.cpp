// ============================================================================
// g_inspector.cpp - InspectorManager debug menu system (game2.o)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_inspector.h"
#include "game/logic/g_local.h"
#include "ngl/ngl_scene.h"

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
