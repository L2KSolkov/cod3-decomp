// ============================================================================
// g_inspector.h - InspectorManager debug menu system (game2.o)
// Layouts verified against IDA local types.
// ============================================================================

#pragma once

#include <stdint.h>

// _INSPECTOR_TYPE - 4-byte value union (union of pointer/callback types)
union _INSPECTOR_TYPE {
    void* vp;                            // +0x00
    void (*fn)();                        // +0x00
    float (*ffn)(float);                 // +0x00
    int (*ifn)(int);                     // +0x00
    int* ip;                             // +0x00
    float* fp;                           // +0x00
    uint8_t* ucp;                        // +0x00
};

struct _INSPECTOR_MENU_ITEM {
    uint32_t type;                       // +0x00 (bitfield flags)
    char* text;                          // +0x04
    _INSPECTOR_TYPE value;               // +0x08
    _INSPECTOR_MENU_ITEM* next;          // +0x0C
    _INSPECTOR_MENU_ITEM* prev;          // +0x10
};
static_assert(sizeof(_INSPECTOR_MENU_ITEM) == 0x14,
              "_INSPECTOR_MENU_ITEM size mismatch");

struct _INSPECTOR_MENU {
    char* heading;                       // +0x00
    int numItems;                        // +0x04
    _INSPECTOR_MENU_ITEM* first;         // +0x08
    _INSPECTOR_MENU_ITEM* last;          // +0x0C
    _INSPECTOR_MENU_ITEM* lastCurrentItem;  // +0x10
    _INSPECTOR_MENU* parentMenu;         // +0x14
};
static_assert(sizeof(_INSPECTOR_MENU) == 0x18, "_INSPECTOR_MENU size mismatch");

struct _INSPECTOR_DATA {
    _INSPECTOR_MENU* rootMenu;           // +0x00
    _INSPECTOR_MENU* currentMenu;        // +0x04
    _INSPECTOR_MENU_ITEM* currentItem;   // +0x08
    int active;                          // +0x0C
    int memory;                          // +0x10
};
static_assert(sizeof(_INSPECTOR_DATA) == 0x14, "_INSPECTOR_DATA size mismatch");

// ============================================================================
// InspectorManager - 0x88 bytes, verified against IDA
// ============================================================================
class InspectorManager {
public:
    float m_currentRgba[4];              // +0x00
    float m_headingRgba[4];              // +0x10
    float m_textRgba[4];                 // +0x20
    int m_KEY_INSPECTOR_ONOFF;           // +0x30
    int m_KEY_SELECT;                    // +0x34
    int m_KEY_UP;                        // +0x38
    int m_KEY_DOWN;                      // +0x3C
    int m_KEY_MENU_BACK;                 // +0x40
    int m_KEY_LEFT;                      // +0x44
    int m_KEY_LEFT_FAST;                 // +0x48
    int m_KEY_LEFT_VERY_FAST;            // +0x4C
    int m_KEY_RIGHT;                     // +0x50
    int m_KEY_RIGHT_FAST;                // +0x54
    int m_KEY_RIGHT_VERY_FAST;           // +0x58
    int m_KEY_LEFT_DEBOUNCE;             // +0x5C
    int m_KEY_LEFT_FAST_DEBOUNCE;        // +0x60
    int m_KEY_LEFT_VERY_FAST_DEBOUNCE;   // +0x64
    int m_KEY_RIGHT_DEBOUNCE;            // +0x68
    int m_KEY_RIGHT_FAST_DEBOUNCE;       // +0x6C
    int m_KEY_RIGHT_VERY_FAST_DEBOUNCE;  // +0x70
    _INSPECTOR_DATA m_data;              // +0x74

    InspectorManager();                  // ea: 0x4F7100
    int IsActive();                       // game2.o 0x004EB460
    void ResetKeys();                    // ea: 0x4EC050
    void ReadKeys();                     // ea: 0x4EC090
    _INSPECTOR_MENU* AddSubMenu(_INSPECTOR_MENU* parent, char* heading);  // ea: 0x4EC260
    _INSPECTOR_MENU_ITEM* AddItem(_INSPECTOR_MENU* parent, char* text,
                                  void* value, int type);  // ea: 0x4EC3B0
    void FreeSubMenu(_INSPECTOR_MENU* menu);   // ea: 0x4EC490
    void UpdateInput();                   // ea: 0x4EC530
    void ProfilerUpdate();                // ea: 0x4EC550
    int RenderStart();                    // ea: 0x4EC560
    void RenderFinish();                  // ea: 0x4EC570
    void RenderSetStates();               // ea: 0x4EC580
    void RenderRestoreStates();           // ea: 0x4EC590
    void SetFontColor(float r, float g, float b, float a);  // ea: 0x4EC5A0
    void Print(char* text, int x, int y, float scale);      // ea: 0x4EC5D0
    void GetTextSize(char* pString, int* xSize, int* ySize,
                     float scale);        // ea: 0x4EC690
    void FreeAll();                       // ea: 0x4F7180
    void Update();                        // ea: 0x4F71C0
    void Initialise();                    // ea: 0x50E250
    void Render();                        // ea: 0x50BED0
    void SetupUserMenus();                // ea: 0x50E1B0
    void UserRenderHook();                // ea: 0x509CA0
    void AddRenderMenus();                // ea: 0x4EC720
    void AddPhysicsMenus();               // ea: 0x4EC8A0
    void AddAimAssistMenus(_INSPECTOR_MENU* parent);  // ea: 0x4F0C00
    void AddCollisionMenus();             // ea: 0x4F0D40
    void AddPlayerMenus();                // ea: 0x4F7880
    void AddDesignerMenus();              // ea: 0x4F78F0
    void AddFXMenus();                    // ea: 0x4F79F0
    void AddVehicleMenus(_INSPECTOR_MENU* parent);   // ea: 0x4F7D60
    void AddMultiplayerMenus();           // ea: 0x4F8570
    void AddDebuggingMenus();             // ea: 0x50C420
    void AddWeaponMenus(_INSPECTOR_MENU* parent);    // ea: 0x50C790
    void AddSettingsMenus();              // ea: 0x50D840
};
static_assert(sizeof(InspectorManager) == 0x88,
              "InspectorManager size mismatch");

extern InspectorManager g_inspectorManager;  // ?g_inspectorManager (game2.o)
extern _INSPECTOR_MENU g_inspectorRootMenu;   // ?g_inspectorRootMenu (game2.o)
