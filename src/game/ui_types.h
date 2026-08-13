// ============================================================================
// COD3 UI / Front-End Types â€” PanelFile, PanelAnimObject, FEText, FEMenu
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "core/math_types.h"
#include "engine/broc_types.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// color32 â€” 32-bit RGBA color (4 bytes) â€” verified against IDA
// Union of packed uint + byte components (b, g, r, a order)
// ============================================================================
union color32 {
    unsigned int i;
    struct {
        uint8_t b;  // +0x00
        uint8_t g;  // +0x01
        uint8_t r;  // +0x02
        uint8_t a;  // +0x03
    } c;

    color32() {}
    // ??0color32@@QAE@I@Z (anim.o 0x53A320)
    color32(unsigned int ic) { i = ic; }
};
static_assert(sizeof(color32) == 4, "color32 size mismatch");

// ============================================================================
// ae_vector<T> â€” dynamic array (12 bytes) â€” verified against IDA
// ============================================================================
template <typename T>
struct ae_vector {
    T*  mElements;  // +0x00
    int mCapacity;  // +0x04
    int mSize;      // +0x08
};
static_assert(sizeof(ae_vector<char>) == 0x0C, "ae_vector size mismatch");

// ============================================================================
// font_index â€” font selection enum (from IDA, all values verified)
// ============================================================================
enum font_index {
    FONT_GARAMOND = 0,
    FONT_GEMFONTONE = 1,
    FONT_BUTTON = 2,
    FONT_ARIAL14 = 3,
    FONT_BIG = 4,
    FONT_NORMAL = 5,
    FONT_IMPACT = 6,
    FONT_HELVETICA_BOLD = 7,
    NUM_FONTS = 8,
    INVALID_FONT = 9,
};

// ============================================================================
// FEMENUCMD â€” menu command bit flags (from IDA, all values verified)
// ============================================================================
enum FEMENUCMD {
    FEMENUCMD_SELECT = 1,
    FEMENUCMD_START = 2,
    FEMENUCMD_UP = 4,
    FEMENUCMD_DOWN = 8,
    FEMENUCMD_LEFT = 16,
    FEMENUCMD_RIGHT = 32,
    FEMENUCMD_CROSS = 64,
    FEMENUCMD_TRIANGLE = 128,
    FEMENUCMD_SQUARE = 256,
    FEMENUCMD_CIRCLE = 512,
    FEMENUCMD_L1 = 1024,
    FEMENUCMD_R1 = 2048,
    FEMENUCMD_L2 = 4096,
    FEMENUCMD_R2 = 8192,
    FEMENUCMD_END = 16384,
};

// Forward declarations
struct FEText;
struct FEMultiLineText;
struct FEMenu;
struct FEMenuSystem;
struct PanelFile;
struct FEMenuEntry;
struct UIListBox;
struct OverlayMenu;
struct DialogMenuSystem;
struct DialogMenu;
struct PanelQuad;
struct FEText;

// panel_layer - quad layer enum
enum panel_layer {
    PANEL_LAYER_BACKGROUND = 0,
};

// ============================================================================
// ae_array<T,N> - fixed-size array (elements only; bounds asserts in callers)
// ============================================================================
template <typename T, int CAPACITY>
struct ae_array {
    T m_elements[CAPACITY];  // +0x00

    T& operator[](int idx) { return m_elements[idx]; }
    const T& operator[](int idx) const { return m_elements[idx]; }
};

// ============================================================================
// PanelFileUser â€” panel file user base (4 bytes, vtable only)
// Size: 0x04 (4 bytes) â€” verified against IDA
// ============================================================================
struct PanelFileUser {
    struct PanelFileUser_vtbl* __vftable;  // +0x00
};
static_assert(sizeof(PanelFileUser) == 4, "PanelFileUser size mismatch");

// ============================================================================
// PanelAnimObject â€” animated panel element base (20 bytes)
// Size: 0x14 (20 bytes) â€” verified against IDA
// ============================================================================
struct PanelAnimObject {
    struct PanelAnimObject_vtbl* __vftable;  // +0x00
    float visibility;                        // +0x04
    float z_value;                           // +0x08
    float fade_timer;                        // +0x0C
    char  flags;                             // +0x10
    uint8_t _pad11[3];                       // +0x11
};
static_assert(sizeof(PanelAnimObject) == 0x14, "PanelAnimObject size mismatch");
static_assert(offsetof(PanelAnimObject, visibility) == 0x04, "PanelAnimObject::visibility offset mismatch");

// ============================================================================
// PanelQuad â€” panel quad (72 bytes)
// Size: 0x48 (72 bytes) â€” verified against IDA
// ============================================================================
struct PanelQuadSection;

struct PanelQuad : PanelAnimObject {
    Broc::vector   center_point;              // +0x14
    ae_vector<PanelQuadSection*> pqs;         // +0x20 (12 bytes)
    void*          am_info;                   // +0x2C (PQArcMaskingInfo*)
    float          rotation;                  // +0x30
    float          sc_x;                      // +0x34
    float          sc_y;                      // +0x38
    unsigned int   quadMapFlags;              // +0x3C
    unsigned int   quadBlendModeType;         // +0x40
    Broc::string   name;                      // +0x44

    void SetShown(bool shown);
    void SetVisibility(float v);
};
static_assert(sizeof(PanelQuad) == 0x48, "PanelQuad size mismatch");
static_assert(offsetof(PanelQuad, center_point) == 0x14, "PanelQuad::center_point offset mismatch");
static_assert(offsetof(PanelQuad, pqs) == 0x20, "PanelQuad::pqs offset mismatch");

// ============================================================================
// PanelFile â€” panel definition file (96 bytes)
// Size: 0x60 (96 bytes) â€” verified against IDA
// ============================================================================
struct PanelFile {
    ae_vector<PanelQuad*> pquads;      // +0x00 (12 bytes)
    ae_vector<FEText*>    ptext;       // +0x0C (12 bytes)
    int       indexHidden;             // +0x18
    bool      hideText;                // +0x1C
    uint8_t   _pad1D[3];               // +0x1D
    char      mName[64];               // +0x20

    // shell.o members (?Clone@PanelFile@@QAEPAV1@XZ etc.)
    PanelFile* Clone();
    PanelQuad* GetPointer(const char* search_name);
    FEText* GetTextPointer(const char* search_name);
    void Draw();
    void UpdateSplitScreen(int viewport, int old_viewport);
    void UpdateWidescreen(bool widescreen, float about_x);
    ~PanelFile();  // ??1PanelFile@@QAE@XZ
};
static_assert(sizeof(PanelFile) == 0x60, "PanelFile size mismatch");
static_assert(offsetof(PanelFile, pquads) == 0x00, "PanelFile::pquads offset mismatch");
static_assert(offsetof(PanelFile, ptext) == 0x0C, "PanelFile::ptext offset mismatch");
static_assert(offsetof(PanelFile, mName) == 0x20, "PanelFile::mName offset mismatch");

// ============================================================================
// FETextFlashInfo â€” text flash animation state (20 bytes)
// Size: 0x14 (20 bytes) â€” verified against IDA
// ============================================================================
struct FETextFlashInfo {
    color32 flash_color;       // +0x00
    float   flash_timer;       // +0x04
    float   flash_intensity;   // +0x08
    float   flash_period;      // +0x0C
    bool    reset;             // +0x10
    uint8_t _pad11[3];         // +0x11
};
static_assert(sizeof(FETextFlashInfo) == 0x14, "FETextFlashInfo size mismatch");

// ============================================================================
// FEText â€” text element (112 bytes)
// Size: 0x70 (112 bytes) â€” verified against IDA
// ============================================================================
struct FEText : PanelAnimObject {
    FETextFlashInfo* flash_info;            // +0x14
    font_index       font;                  // +0x18
    Broc::string     text;                  // +0x1C
    Broc::vector     xy_initial;            // +0x20
    Broc::vector     xy;                    // +0x2C
    Broc::vector     scale;                 // +0x38
    Broc::vector     scale_init;            // +0x44
    Broc::vector     scale_unselected;      // +0x50
    color32          color1;                // +0x5C
    color32          color_unselected;      // +0x60
    Broc::string     name;                  // +0x64
    int              panel_text_index;      // +0x68
    int16_t          flags;                 // +0x6C
    uint8_t          _pad6E[2];             // +0x6E

    void SetAlpha(int a);
    void SetColorMenuItem(unsigned int normal, unsigned int selected);
    void SetText(const char* s, int a3);
    void SetShown(bool shown);
    void Draw();
    unsigned int GetColor();
    unsigned int GetUnselectedColor();
    float GetScaleX();
    float GetX();
    float GetY();
    font_index GetFont();
};
static_assert(sizeof(FEText) == 0x70, "FEText size mismatch");
static_assert(offsetof(FEText, font) == 0x18, "FEText::font offset mismatch");
static_assert(offsetof(FEText, text) == 0x1C, "FEText::text offset mismatch");
static_assert(offsetof(FEText, xy) == 0x2C, "FEText::xy offset mismatch");
static_assert(offsetof(FEText, scale) == 0x38, "FEText::scale offset mismatch");
static_assert(offsetof(FEText, color1) == 0x5C, "FEText::color1 offset mismatch");
static_assert(offsetof(FEText, panel_text_index) == 0x68, "FEText::panel_text_index offset mismatch");
static_assert(offsetof(FEText, flags) == 0x6C, "FEText::flags offset mismatch");

// ============================================================================
// FEMenuEntry â€” selectable menu entry (24 bytes)
// Size: 0x18 (24 bytes) â€” verified against IDA
// ============================================================================
struct FEMenuEntry {
    struct FEMenuEntry_vtbl* __vftable;  // +0x00
    FEMenu*    menu;                     // +0x04
    int16_t    up;                       // +0x08
    int16_t    down;                     // +0x0A
    int16_t    left;                     // +0x0C
    int16_t    right;                    // +0x0E
    FEText*    text;                     // +0x10
    char       color_scheme_index;       // +0x14
    bool       highlight;                // +0x15
    bool       disabled;                 // +0x16
    bool       must_delete_text;         // +0x17

    // vtable helpers (slots: 12=SetString, 16=SetEnabled)
    void SetString(const char* s);
    void SetEnabled(bool e);
};
static_assert(sizeof(FEMenuEntry) == 0x18, "FEMenuEntry size mismatch");
static_assert(offsetof(FEMenuEntry, text) == 0x10, "FEMenuEntry::text offset mismatch");

// ============================================================================
// FEMenuColorScheme â€” menu color scheme (16 bytes)
// Size: 0x10 (16 bytes) â€” verified against IDA
// ============================================================================
struct FEMenuColorScheme {
    color32 unselect;  // +0x00
    color32 high1;     // +0x04
    color32 high2;     // +0x08
    bool    flash;     // +0x0C
    uint8_t _pad0D[3]; // +0x0D
};
static_assert(sizeof(FEMenuColorScheme) == 0x10, "FEMenuColorScheme size mismatch");

// ============================================================================
// FEMenuSystem â€” menu system (44 bytes)
// Size: 0x2C (44 bytes) â€” verified against IDA
// ============================================================================
struct FEMenuSystem : PanelFileUser {
    FEMenu**    menus;                 // +0x04
    font_index  font;                  // +0x08
    int         size;                  // +0x0C
    int         count;                 // +0x10
    int         background;            // +0x14
    bool        drawHelpbar;           // +0x18
    uint8_t     _pad19[3];             // +0x19
    int         m_active;              // +0x1C
    char        default_color_scheme;  // +0x20
    uint8_t     _pad21[1];             // +0x21
    int16_t     button_down_flags[4];  // +0x22 (8 bytes)
    bool        is_active;             // +0x2A
    uint8_t     _pad2B[1];             // +0x2B

    int  GetCurrentClient();
    void AddOverlay(int a2);
    void ReturnToPreviousMenu(int a2);
    void SetSystemActive(bool active);  // ?SetSystemActive@FEMenuSystem@@QAEX_N@Z (sv.o 0x51E150)
};
static_assert(sizeof(FEMenuSystem) == 0x2C, "FEMenuSystem size mismatch");
static_assert(offsetof(FEMenuSystem, menus) == 0x04, "FEMenuSystem::menus offset mismatch");
static_assert(offsetof(FEMenuSystem, m_active) == 0x1C, "FEMenuSystem::m_active offset mismatch");

// ============================================================================
// FEMenu â€” menu (76 bytes)
// Size: 0x4C (76 bytes) â€” verified against IDA
// ============================================================================

// FEMenu base methods (shell.o provides the real implementations)
struct FEMenuVtbl;
struct FEMenu {
    FEMenuVtbl* __vftable;                       // +0x00
    FEMenuEntry** entries;                       // +0x04
    FEMenuSystem* system;                        // +0x08
    int    center_x;                             // +0x0C
    int    center_y;                             // +0x10
    int    y_distance;                           // +0x14
    int    half_height;                          // +0x18
    float  button_held_timer;                    // +0x1C
    int16_t first_vis_entry;                     // +0x20
    int16_t highlighted;                         // +0x22
    int16_t highlightedDefault;                  // +0x24
    int16_t num_entries;                         // +0x26
    int16_t max_vis_entries;                     // +0x28
    int16_t flags;                               // +0x2A
    void*  sound;                                // +0x2C (DbLinkedHandle)
    bool   lockInput;                            // +0x30
    bool   enableNavigationSound;                // +0x31
    char   button_held_down;                     // +0x32
    char   default_color_scheme;                 // +0x33
    int    mReturnMenu;                          // +0x34
    FEText* helpbar;                             // +0x38
    FEMultiLineText* helpbar1;                   // +0x3C
    FEMultiLineText* helpbar2;                   // +0x40
    FEMultiLineText* helpbar3;                   // +0x44
    PanelFile* panel;                            // +0x48

    FEMenu(FEMenuSystem* menuSystem, int num, int x, int y, int mve, int flg);
    ~FEMenu();
    void Draw();
    void Update(float time_inc);
    void OnActivate();
    void Cleanup();
    void ClearAllButtons();
    void Left();
    void Right();
    void Up(int a2);
    void Down(int a2);
    void SetHigh(int a2, int a3, bool a4);
    void ReturnToPreviousMenu(int a2);
    void AddOverlay(int a2);
    void MakeActiveAndReturn(int a2);
    void UpdateWidescreen(BOOL widescreen);
    void SetItem(int row, FEText* text, int state);
};
static_assert(sizeof(FEMenu) == 0x4C, "FEMenu size mismatch");
static_assert(offsetof(FEMenu, entries) == 0x04, "FEMenu::entries offset mismatch");
static_assert(offsetof(FEMenu, panel) == 0x48, "FEMenu::panel offset mismatch");

// ============================================================================
// MultiLineString â€” multiline text storage (opaque)
// ============================================================================
struct MultiLineString;

// ============================================================================
// FEMultiLineText â€” multiline text element (168 bytes)
// Size: 0xA8 (168 bytes) â€” verified against IDA
// ============================================================================
struct FEMultiLineText : FEText {
    color32 button_color;              // +0x70
    float   button_scale;              // +0x74
    float   line_spacing_init;         // +0x78
    float   line_spacing;              // +0x7C
    float   button_y_offset;           // +0x80
    int     box_width;                 // +0x84
    int     line_num;                  // +0x88
    int     line_avail_num;            // +0x8C
    MultiLineString* lines;            // +0x90
    int     scroll_box_height;         // +0x94
    int     scroll_first;              // +0x98
    int     scroll_last;               // +0x9C
    float   scroll_offset;             // +0xA0
    bool    scrollable;                // +0xA4
    bool    scroll_edge_based;         // +0xA5
    bool    cut_off_if_too_long;       // +0xA6
    uint8_t _padA7[1];                 // +0xA7

    void Draw();
    void SetTextBoxNoLocalize(const char* s, int a3, int a4);
    void UpdateForSplitScreen(int viewport, int old_viewport);
    void UpdateForWidescreen(bool widescreen);
    void SetNumLines(int n);
    void SetText(const char* s);
    FEMultiLineText(font_index f, float x1, float y1, float z1,
                    panel_layer layer, float s, int horizJust, int vertJust,
                    color32 col);
};
static_assert(sizeof(FEMultiLineText) == 0xA8, "FEMultiLineText size mismatch");
static_assert(offsetof(FEMultiLineText, lines) == 0x90, "FEMultiLineText::lines offset mismatch");
