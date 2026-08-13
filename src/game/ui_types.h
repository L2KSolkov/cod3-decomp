// ============================================================================
// COD3 UI / Front-End Types â€” PanelFile, PanelAnimObject, FEText, FEMenu
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

// Full FE type set is available; sv_stubs.h keeps its minimal FEMenuSystem
// view out when this header has been included.
#define COD3_FULL_FE_TYPES

#include "core/math_types.h"
#include "engine/broc_types.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// color32 â€” 32-bit RGBA color (4 bytes) â€” verified against IDA
// Class-tagged (V) to match the binary's mangling (??AVcolor32@@);
// anonymous union keeps the packed uint + byte component views.
// ============================================================================
class color32 {
public:
    union {
        unsigned int i;
        struct {
            uint8_t b;  // +0x00
            uint8_t g;  // +0x01
            uint8_t r;  // +0x02
            uint8_t a;  // +0x03
        } c;
    };

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
#ifndef COD3_FONT_INDEX_DEFINED
#define COD3_FONT_INDEX_DEFINED
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
#endif

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
    virtual ~PanelAnimObject() {}            // +0x00 (vfptr)
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

    PanelQuad(char* name);  // ??0PanelQuad@@QAE@PAD@Z (shell.o 0x58B840)
    virtual void Shift(float off_x, float off_y);  // shell.o 0x57A6A0
    virtual void SetCenterPos(float cx, float cy)  // vtable slot 39 (0x9C)
    {
        Shift(cx - center_point.x, cy - center_point.y);
    }
    Broc::vector GetMax();        // shell.o 0x57AC40
    Broc::vector GetMin();        // shell.o 0x57AF20
    Broc::vector GetInitialMax(); // shell.o 0x57B200
    Broc::vector GetInitialMin(); // shell.o 0x57B490
    void SetShown(bool shown);
    void SetVisibility(float v);
};
static_assert(sizeof(PanelQuad) == 0x48, "PanelQuad size mismatch");
static_assert(offsetof(PanelQuad, center_point) == 0x14, "PanelQuad::center_point offset mismatch");
static_assert(offsetof(PanelQuad, pqs) == 0x20, "PanelQuad::pqs offset mismatch");

// ============================================================================
// PanelQuadSection â€” quad section (104 bytes) â€” verified against IDA
// ============================================================================
struct PanelQuadSection {
    struct PQVert {
        float        X;      // +0x00
        float        Y;      // +0x04
        float        U;      // +0x08
        float        V;      // +0x0C
        unsigned int Color;  // +0x10
    };
    struct QuadData {
        PQVert Verts[4];  // +0x00 (80 bytes)
        float  Z;         // +0x50
        char   pad2[4];   // +0x54
    };

    short    x_initial[4];  // +0x00
    short    y_initial[4];  // +0x08
    QuadData quad;          // +0x10 (88 bytes)

    Broc::vector GetMax();            // shell.o 0x569C60
    Broc::vector GetMin();            // shell.o 0x569D00
    Broc::vector GetInitialMax();     // shell.o 0x569DA0
    Broc::vector GetInitialMin();     // shell.o 0x569E50
    Broc::vector GetMaxUV();          // shell.o 0x569F00
    Broc::vector GetMinUV();          // shell.o 0x569FB0
    void SetInitialXY(Broc::vector* tmp_initial);  // shell.o 0x5696F0
    void SetUV(Broc::vector* uv);                  // shell.o 0x569B30
    void SetPos(Broc::vector* xy);                 // shell.o 0x569BB0
    void AddPQSection(Broc::vector* xy, Broc::vector* uv, color32* col,
                      float z);                    // shell.o 0x579550
    void SetColorVert(int i, color32 c);           // shell.o 0x569AB0
    void SetColorNAVert(int i, color32 c);         // shell.o 0x579800
    color32 GetColor(int index);                   // shell.o 0x579900
};
static_assert(sizeof(PanelQuadSection::PQVert) == 20,
              "PQVert size mismatch");
static_assert(sizeof(PanelQuadSection) == 104,
              "PanelQuadSection size mismatch");

// ============================================================================
// FloatingPQ â€” screen-projected quad (84 bytes) â€” verified against IDA
// ============================================================================
struct FloatingPQ : PanelQuad {
    Broc::vector location_3d;  // +0x48

    FloatingPQ(char* n);          // shell.o 0x591070
    virtual void UpdateInScene(); // shell.o 0x56AD40
};
static_assert(sizeof(FloatingPQ) == 84, "FloatingPQ size mismatch");

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

    FETextFlashInfo(color32 col, float period);  // shell.o 0x56B940
    void Update(float time_inc);                 // shell.o 0x56B970
    color32 GetColor(color32 normal_color);      // shell.o 0x56B9B0
    void Reset();                                // shell.o 0x56BA70
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

    FEText() {}  // inline default (FE subclass ctors)
    FEText(font_index f, const char* s, float x, float y, int z,
           panel_layer layer, float scale, int hJustify, int vJustify,
           color32 col);                    // shell.o 0x56BA80
    virtual ~FEText();                      // shell.o 0x56BCE0
    virtual FEText* Clone();                // shell.o 0x56BD50
    virtual void CopyFrom(FEText* fet);     // shell.o 0x56BDC0
    virtual void Update(float time_inc);    // shell.o 0x56BEF0
    virtual void UpdateForWidescreen(bool widescreen);  // shell.o 0x56BFF0
    virtual void UpdateForWidescreen(bool widescreen, int viewport);
                                            // shell.o 0x56C120
    virtual void MoveForSplitScreen(int viewport, int old_viewport);
                                            // shell.o 0x56C250
    virtual void UpdateForHUDSplitScreen(int viewport, int old_viewport,
                                         int client, float widescreen,
                                         float split);  // shell.o 0x56C380
    virtual void UpdateForSplitScreen(int viewport, int old_viewport);
                                            // shell.o 0x56C4F0
    virtual void SetHJustify(int h);        // shell.o 0x56C7F0
    virtual void SetVJustify(int v);        // shell.o 0x56C830
    virtual void SetText(const char* s);    // shell.o 0x56C870
    virtual void SetText(unsigned int hash);// shell.o 0x56C900
    virtual void SetNoFlash(color32 c);     // shell.o 0x56C930
    virtual void SetFlash(color32 c1, color32 c2, float period);
                                            // shell.o 0x56C950
    virtual void SetNoColor();              // shell.o 0x56C9D0
    static void CreateNGLColorCode(color32 c, char* dest);  // shell.o 0x56C9E0
    virtual void Draw(bool localize);       // shell.o 0x57C390
    virtual float GetWidth(const float* p); // shell.o 0x57C6E0
    virtual float GetHeight(const float* p);// shell.o 0x57C910
protected:
    virtual void Animate(math::Mat43* m, float time);  // shell.o 0x56CA30
    virtual void AdjustForJustification(float& x, float& y, float z);
                                            // shell.o 0x56CB60
public:
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
    virtual ~FEMenuEntry() {}            // +0x00 (vfptr)
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
    void CommonConstructor(FEText* text, FEMenu* menu);  // shell.o 0x56FBC0
    virtual void CopyFrom(FEText* fet);   // shell.o 0x56FC20
    virtual void SetText(FEText* fet);    // shell.o 0x56FC30
protected:
    virtual void AdjustColor(FEText* fet);// shell.o 0x56FCC0
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

    static bool GetInfo(char index, color32& un, color32& h1,
                        color32& h2);   // shell.o 0x56FB10
    static bool GetInfo(char index, color32& un,
                        color32& sel);  // shell.o 0x56FB50
    static int GetSchemeFromText(Broc::string& schemeText);  // shell.o 0x56FB80
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
    static Broc::string ReplaceEndlines(Broc::string t);  // shell.o 0x56E670
    FEMultiLineText(font_index f, float x1, float y1, float z1,
                    panel_layer layer, float s, int horizJust, int vertJust,
                    color32 col);
};
static_assert(sizeof(FEMultiLineText) == 0xA8, "FEMultiLineText size mismatch");
static_assert(offsetof(FEMultiLineText, lines) == 0x90, "FEMultiLineText::lines offset mismatch");

// ============================================================================
// FEMenuListBoxItem â€” list-box data row (28 bytes) â€” verified against IDA
// ============================================================================
struct FEMenuListBoxItem {
    int          mIndex;         // +0x00
    Broc::string mText;          // +0x04
    void*        mData;          // +0x08
    int          mSubItemCount;  // +0x0C
    Broc::string mSubItems[3];   // +0x10

    const Broc::string& GetSubItem(unsigned int index);  // shell.o 0x571D90
};
static_assert(sizeof(FEMenuListBoxItem) == 28,
              "FEMenuListBoxItem size mismatch");
