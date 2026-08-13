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

extern const char* const defaultFileName;  // 0xCD67AE

class nglTexture;
class nglFont;
struct PanelMaterial;

// Minimal controller view (full impl in input/controller.cpp; manglings match)
class controller {
public:
    enum ButtonIndex {
        LEFTBUTTON = 0, DOWNBUTTON = 1, RIGHTBUTTON = 2, UPBUTTON = 3,
        SQUARE = 4, X = 5, CIRCLE = 6, TRIANGLE = 7, R1 = 8, L1 = 9,
        R2 = 10, L2 = 11, R3 = 12, L3 = 13, START = 14, SELECT = 15,
    };
    enum StickIndex {
        LEFTSTICK = 0,
        RIGHTSTICK = 1,
    };
    static controller* inst();          // ?inst@controller@@SAPAV1@XZ
    static int num_controllers;         // ?num_controllers@controller@@2HA
    bool button_pressed_clear(int index, ButtonIndex btn);  // controller.o
    bool button_released_clear(int index, ButtonIndex btn); // controller.o
    bool button_pressed(ButtonIndex btn, int* p_controller);  // controller.o
    bool button_released(ButtonIndex btn, int* p_controller); // controller.o
    int  stick_value_x(StickIndex stick, int* p_controller);  // controller.o
    int  stick_value_y(StickIndex stick, int* p_controller);  // controller.o
    int  locked_port;
    bool is_locked;
};

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

// Forward declarations (class tags match the original binary's manglings)
class FEText;
class FEMultiLineText;
class FEMenu;
class FEMenuSystem;
class PanelFile;
class FEMenuEntry;
struct UIListBox;
struct OverlayMenu;
struct DialogMenuSystem;
struct DialogMenu;
class PanelQuad;
class FEText;

// panel_layer - quad layer enum
enum panel_layer {
    PANEL_LAYER_BACKGROUND = 0,
    PANEL_LAYER_1 = 1,
    PANEL_LAYER_PAUSE_MENU = 2,
    PANEL_LAYER_2 = 2,
    PANEL_LAYER_3 = 3,
    PANEL_LAYER_4 = 4,
    PANEL_LAYER_5 = 5,
    PANEL_LAYER_6 = 6,
    PANEL_LAYER_7 = 7,
    PANEL_LAYER_8 = 8,
    PANEL_LAYER_TOTAL = 9,
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
class PanelFileUser {
public:
    virtual void SetPanelFile(PanelFile* pf) {}        // slot 0 0x5AEA10
    virtual void PanelFileUnloaded(PanelFile* pf) {}   // slot 1 0x5AEA20
    virtual void UpdateWidescreen(bool widescreen) {}  // slot 2 0x5AEA30
};
static_assert(sizeof(PanelFileUser) == 4, "PanelFileUser size mismatch");

// ============================================================================
// PanelAnimObject â€” animated panel element base (20 bytes)
// Size: 0x14 (20 bytes) â€” verified against IDA
// ============================================================================
class PanelAnimObject {
public:
    virtual ~PanelAnimObject() {}            // +0x00 (vfptr)
    float visibility;                        // +0x04
    float z_value;                           // +0x08
    float fade_timer;                        // +0x0C
    char  flags;                             // +0x10
    uint8_t _pad11[3];                       // +0x11

    // Vtable layout matches the binary (verified against ??_7PanelAnimObject).
    virtual void Draw() = 0;                    // +0x04 pure (__purecall)
    virtual void Update(float time_inc);        // +0x08 shell.o 0x56B6E0
    virtual void CopyFrom(const PanelAnimObject* pao);  // +0x0C 0x56B770
    virtual void StartAnim(bool on)             // +0x10 inline 0x5B1A90
    {
        flags = on ? (char)(flags | 2) : (char)(flags & 0xFD);
    }
    virtual void ResetXform()                   // +0x14 inline 0x5B1AC0
    {
        flags = (char)(flags & ~1);
    }
protected:
    virtual void Animate(math::Mat43*, float) = 0;  // +0x18 pure
public:
    virtual void SetWidescreenAlign(short wa);       // +0x1C 0x56B7A0
    virtual void SetZvalue(float z, panel_layer layer);  // +0x20 0x56B7D0
    virtual void StartFade(bool start, bool fade_in,
                           float time);              // +0x24 0x56B820
    virtual void SetZvalueAbs(float z)               // +0x28 inline 0x5AD5B0
    {
        z_value = z;
    }
    virtual float GetZvalue()                        // +0x2C inline 0x5AD5D0
    {
        return z_value;
    }
    virtual float GetY()                             // +0x30 inline 0x5AD5E0
    {
        return 0.0f;
    }
    virtual bool IsFading()                          // +0x34 inline 0x5B1AD0
    {
        return (flags & 0x10) != 0 || (flags & 0x20) != 0;
    }
    virtual bool IsShown()                           // +0x38 inline 0x5B1AE0
    {
        return (flags & 1) != 0;
    }
    virtual void SetShown(bool shown)                // +0x3C inline 0x5B1AF0
    {
        flags = shown ? (char)(flags | 1) : (char)(flags & 0xFE);
    }
    virtual void SetColor(color32 c) = 0;            // +0x40 pure
    virtual color32 GetColor() = 0;                  // +0x44 pure
};
static_assert(sizeof(PanelAnimObject) == 0x14, "PanelAnimObject size mismatch");
static_assert(offsetof(PanelAnimObject, visibility) == 0x04, "PanelAnimObject::visibility offset mismatch");

// ============================================================================
// PanelQuad â€” panel quad (72 bytes)
// Size: 0x48 (72 bytes) â€” verified against IDA
// ============================================================================
class PanelQuadSection;

class PanelQuad : public PanelAnimObject {
public:
    Broc::vector   center_point;              // +0x14
    ae_vector<PanelQuadSection*> pqs;         // +0x20 (12 bytes)
    void*          am_info;                   // +0x2C (PQArcMaskingInfo*)
    float          rotation;                  // +0x30
    float          sc_x;                      // +0x34
    float          sc_y;                      // +0x38
    unsigned int   quadMapFlags;              // +0x3C
    unsigned int   quadBlendModeType;         // +0x40
    Broc::string   name;                      // +0x44

    PanelQuad();  // ??0PanelQuad@@QAE@XZ (shell.o 0x58B780)
    PanelQuad(char* name);  // ??0PanelQuad@@QAE@PAD@Z (shell.o 0x58B840)
    virtual ~PanelQuad();   // ??1PanelQuad@@UAE@XZ (shell.o 0x590F70)
    virtual void Draw();    // ?Draw@PanelQuad@@UAEXXZ (shell.o 0x579BC0)
    virtual void SetColor(color32 c);    // 0x57A1F0
    virtual color32 GetColor();          // 0x5B6620
    virtual void SetVisibility(float alpha);  // 0x57A450
    virtual void SetAlpha(float alpha);       // ?SetAlpha@PanelQuad@@UAEXM@Z 0x576870
    virtual void SetZvalueAbs(float z);       // 0x57A530
    virtual float GetCenterX() { return center_point.x; }  // inline 0x5B6490
    virtual void GetCenterPos(float& cx, float& cy)  // inline 0x5B6470
    {
        cx = center_point.x;
        cy = center_point.y;
    }
    virtual void SetCenterPos(float cx, float cy)  // inline 0x5B6430
    {
        Shift(cx - center_point.x, cy - center_point.y);
    }
    virtual void SetMaterialFlags(unsigned int mapflags);  // 0x56AB90
    virtual void SetTexture(nglTexture* tex);              // 0x57A170
    virtual void Init(Broc::vector* xy, color32* col,
                      panel_layer lay, float z,
                      const char* filename);  // 0x58BAC0
    virtual void Load(PanelMaterial* mats, unsigned char* buffer,
                      int& index,
                      const math::Mat43* parent_matrix);  // 0x58BC10
    virtual void Shift(float off_x, float off_y);  // shell.o 0x57A6A0
    void FormatForSplitScreen(int viewport, int old_viewport);  // 0x57A940
    void MoveForSplitScreen(int viewport, int old_viewport);    // 0x57A9C0
    void FattenMeForWidescreen(bool widescreen, float x);       // 0x57AA40
protected:
    virtual void Animate(math::Mat43* mat, float vis);  // 0x57AB70
public:
    Broc::vector GetMax();        // shell.o 0x57AC40
    Broc::vector GetMin();        // shell.o 0x57AF20
    Broc::vector GetInitialMax(); // shell.o 0x57B200
    Broc::vector GetInitialMin(); // shell.o 0x57B490
};
static_assert(sizeof(PanelQuad) == 0x48, "PanelQuad size mismatch");
static_assert(offsetof(PanelQuad, center_point) == 0x14, "PanelQuad::center_point offset mismatch");
static_assert(offsetof(PanelQuad, pqs) == 0x20, "PanelQuad::pqs offset mismatch");

// ============================================================================
// PanelMaterial â€” per-quad material (16 bytes) â€” verified against IDA
// (ReadPanelMaterial 0x57BEE0: texture +0, filename +4, color +8, hasmap +0xC)
// ============================================================================
struct PanelMaterial {
    nglTexture* texture;         // +0x00
    const char* filename;        // +0x04
    color32     color;           // +0x08
    bool        hasmap;          // +0x0C
    bool        bilinearfilter;  // +0x0D
    bool        wrapu;           // +0x0E
    bool        wrapv;           // +0x0F

    PanelMaterial() : texture(nullptr), filename(nullptr), hasmap(false),
                      bilinearfilter(false), wrapu(false), wrapv(false)
    {
        color.i = 0;
    }
};
static_assert(sizeof(PanelMaterial) == 0x10, "PanelMaterial size mismatch");

// ============================================================================
// PanelQuadSection â€” quad section (104 bytes) â€” verified against IDA
// ============================================================================
class PanelQuadSection {
public:
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
        nglTexture* Tex;  // +0x54
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
    void Animate(math::Mat43* xform, float z_value,
                 bool xform_was_set);              // shell.o 0x56A130
    void Draw(unsigned int type, unsigned int mapflags);  // shell.o 0x56A2F0
    void SetVisibility(int i, float vis);          // shell.o 0x5798A0
    void FormatForSplitScreen(int viewport, int old_viewport);  // 0x56A3B0
    void MoveForSplitScreen(int viewport, int old_viewport);    // 0x56A690
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
class PanelFile {
public:
    ae_vector<PanelQuad*> pquads;      // +0x00 (12 bytes)
    ae_vector<FEText*>    ptext;       // +0x0C (12 bytes)
    int       indexHidden;             // +0x18
    bool      hideText;                // +0x1C
    uint8_t   _pad1D[3];               // +0x1D
    char      mName[64];               // +0x20

    // shell.o members (?Clone@PanelFile@@QAEPAV1@XZ etc.)
    PanelFile();  // ??0PanelFile@@QAE@XZ (shell.o 0x5843D0)
    PanelFile* Clone();
    PanelQuad* GetPointer(const char* search_name);
    FEText* GetTextPointer(const char* search_name);
    void Draw();
    void UpdateSplitScreen(int viewport, int old_viewport);
    void UpdateWidescreen(bool widescreen, float about_x);
    void Update(float time_inc);                 // shell.o 0x57BA40
    void MoveSplitScreen(int viewport, int old_viewport);  // 0x57BCB0
    void PreMashFixup();                         // shell.o 0x56ADC0
    void PostUnmashFixup(panel_layer layer);     // shell.o 0x57B730
    void ReadPanelMaterial(PanelMaterial& mat, unsigned char* buffer,
                           int& index);          // shell.o 0x57BEE0
    int FindPanelQuadByPointer(PanelQuad* the_pointer);  // 0x57BF80
    int FindFETextByPointer(FEText* the_pointer);        // 0x57BFC0
    bool Load(const char* filename, unsigned char* buffer,
              int buffer_size);                  // shell.o 0x593A00
    PanelAnimObject* FindAnimObject(const char* search_name);  // 0x5948C0
    ~PanelFile();  // ??1PanelFile@@QAE@XZ
private:
    void Cleanup();  // ??0PanelFile... Cleanup@PanelFile@@AAEXXZ (0x58C4C0)
    void LoadPanelGeom(unsigned char* buffer, int& index,
                       const math::Mat43* parent_matrix);  // 0x5910E0
    void LoadPanelObject(unsigned char* buffer, int& index,
                         const math::Mat43* parent_matrix,
                         const char* name, short widescreen_align);
    void LoadPanelText(unsigned char* buffer, int& index,
                       const math::Mat43* parent_matrix,
                       const char* name, short widescreen_align);
    void LoadPanelText2(unsigned char* buffer, int& index,
                        const math::Mat43* parent_matrix,
                        const char* name, short widescreen_align);
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
class FEText : public PanelAnimObject {
public:
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

    // shell.o inline COMDATs (verified manglings)
    virtual void Draw() { Draw(false); }  // ?Draw@FEText@@UAEXXZ (0x5AD830)
    virtual void SetColor(color32 c) { SetNoFlash(c); }  // 0x5ADC20
    virtual void SetTextNoLocalize(const char* s) { text = s; }
    virtual void SetPos(float x, float y) { xy.x = x; xy.y = y; }
    virtual void SetY(float y) { xy.y = y; }
    virtual void SetAlpha(float a) { (void)a; }
    virtual void SetColorMenuItem(color32 normal, color32 selected)
    {
        color1 = normal;
        color_unselected = selected;
    }
    virtual float GetScaleX() const { return scale.x; }
    virtual color32 GetColor() { return color1; }
    virtual color32 GetUnselectedColor() { return color_unselected; }
    virtual float GetX() { return xy.x; }
    virtual float GetY() { return xy.y; }
    virtual bool GetFlag(int f) { return (flags & f) != 0; }
    virtual bool IsOnMenu() { return GetFlag(2); }       // 0x5AD860
    virtual void AddedToMenu(bool add) { SetFlag(2, add); }  // 0x5AD840
    virtual bool IsMultiLineObject() { return false; }    // 0x5AD870
    virtual Broc::string GetName() { return name; }      // 0x5ADC50
    virtual void SetName(const char* n) { name = n; }    // 0x5ADAA0
    virtual Broc::string GetText() { return text; }      // 0x5ADC80
    virtual void SetPanelTextIndex(int the_index)        // 0x5ADC10
    {
        panel_text_index = the_index;
    }
    virtual void SetFont(font_index f) { font = f; }     // 0x5AD8B0
    virtual void SetEvenNumberSpacing(bool on)           // 0x5AD8C0
    {
        flags = (int16_t)(on ? (flags | 4) : (flags & ~4));
    }
    virtual void SetScale(float s)                       // 0x5AD8E0
    {
        scale.x = s;
        scale.y = s;
        scale_unselected.x = s;
        scale_unselected.y = s;
    }
    virtual void SetScale(float sx, float sy)            // 0x5AD950
    {
        scale.x = sx;
        scale.y = sy;
        scale_unselected.x = sx;
        scale_unselected.y = sy;
        scale_unselected.z = 0.0f;
    }
    virtual void SetScaleInit(float sx, float sy)        // 0x5ADA50
    {
        scale_init.x = sx;
        scale_init.y = sy;
        scale_init.z = 0.0f;
    }
    virtual void Shift(float offx, float offy)           // 0x5ADAB0
    {
        SetPos(GetX() + offx, GetY() + offy);
    }
    virtual void ShiftXYInitial(Broc::vector offset)     // 0x5ADAF0
    {
        xy_initial.x += offset.x;
        xy_initial.y += offset.y;
        xy_initial.z += offset.z;
    }
    virtual void SetX(float posX) { xy.x = posX; }       // 0x5ADBA0
    virtual void SetScaleMenuItem(float s_selected,      // 0x5AD9D0
                                  float s_unselected)
    {
        scale.x = s_selected;
        scale.y = s_selected;
        scale.z = 0.0f;
        scale_unselected.x = s_unselected;
        scale_unselected.y = s_unselected;
        scale_unselected.z = 0.0f;
        scale_init.z = s_unselected;
    }
    virtual void SetLineSpacing(int) {}                  // 0x5ADE10 (empty)
    virtual void UpdateInScene(bool) {}                  // 0x5ADE00 (empty)
    virtual void SetLocation3D(Broc::vector) {}          // 0x5ADE30 (empty)
    virtual void SetBehaviorNF(float, float) {}          // 0x5ADE40 (empty)
    virtual void SetBehavior(bool) {}                    // 0x5ADE50 (empty)
    virtual int GetLineNum() { return 1; }               // 0x5ADD80
    virtual void AddFont(int, font_index) {}             // delegate (FEMultiLineText)
    virtual float GetScaleY() const { return scale.y; }
    virtual float GetScaleInitX() const { return scale_init.x; }
    virtual float GetScaleInitY() const { return scale_init.y; }
    virtual bool GetEvenNumberSpacing() const
    {
        return (flags & 4) != 0;
    }
    virtual int GetFlags() { return flags; }
    virtual int GetHJustify() { return flags & 0x30; }
    virtual int GetVJustify() { return flags & 0xC0; }
    virtual Broc::vector GetLocation3D()
    {
        Broc::vector v;
        v.x = v.y = v.z = 0.0f;
        return v;
    }
    virtual void SetFlag(int f, bool on)
    {
        if (on)
            flags = (int16_t)(flags | f);
        else
            flags = (int16_t)(flags & ~f);
    }

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
class FEMenuEntry {
public:
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

    // Vftable order verified against ??_7FEMenuEntry (52 slots).
    virtual ~FEMenuEntry();                     // 0x5AE460
protected:
    virtual void OnHighlight(bool anim) {}      // slot 1 (0x5AE860, empty)
public:
    virtual void Load() {}                      // slot 2 (0x5AE490, empty)
    virtual short OnUp() { return up; }         // slot 3 0x5AE4F0
    virtual short OnDown() { return down; }     // slot 4 0x5AE500
    virtual short OnLeft() { return left; }     // slot 5 0x5AE510
    virtual short OnRight() { return right; }   // slot 6 0x5AE520
    virtual void Draw()                         // slot 7 0x5AE530
    {
        if (text != nullptr)
            text->Draw(highlight);
    }
    virtual void Update(float time_inc)         // slot 8 0x5AE550
    {
        if (text != nullptr)
            text->Update(time_inc);
    }
    virtual void UpdateInScene()                // slot 9 0x5AE570
    {
        if (text != nullptr)
            text->UpdateInScene(false);
    }
    virtual void CopyFrom(FEText* fet);         // slot 10 0x56FC20
    virtual void SetText(Broc::string ref)      // slot 11 0x5B1D00
    {
        if (text != nullptr)
            text->SetText(ref.mBlock != nullptr
                               ? (const char*)&ref.mBlock[1]
                               : defaultFileName);
    }
    virtual void SetText(const char* s)         // slot 12 0x5AE5E0
    {
        if (text != nullptr)
            text->SetText(s);
    }
    virtual void SetText(FEText* fet);          // slot 13 0x56FC30
    virtual void SetShown(bool on)              // slot 14 0x5AE590
    {
        if (text != nullptr)
            text->SetShown(on);
    }
    virtual void Highlight(bool h, bool anim);  // slot 15 0x56FC40
    virtual void Disable(bool d);               // slot 16 0x56FC70
    virtual bool GetDisable()                   // slot 17 0x5AE5A0
    {
        return disabled;
    }
    virtual void StartFade(bool s, bool f,      // slot 18 0x5AE5B0
                           float t)
    {
        if (text != nullptr)
            text->StartFade(s, f, t);
    }
    virtual void SetPos(float x, float y)       // slot 19 0x5AE5C0
    {
        if (text != nullptr)
            text->SetPos(x, y);
    }
    virtual void SetTextNoLocalize(char* txt)   // slot 20 0x5AE600
    {
        if (text != nullptr)
            text->SetTextNoLocalize(txt);
    }
    virtual void SetTextNoLocalize(Broc::string str)  // slot 21 0x5B1D80
    {
        if (text != nullptr)
            text->SetTextNoLocalize(str.mBlock != nullptr
                                        ? (const char*)&str.mBlock[1]
                                        : defaultFileName);
    }
    virtual void SetLocation3D(Broc::vector loc)  // slot 22 0x5AE620
    {
        if (text != nullptr)
            text->SetLocation3D(loc);
    }
    virtual void SetHJustify(int h)             // slot 23 0x5AE660
    {
        if (text != nullptr)
            text->SetHJustify(h);
    }
    virtual void SetVJustify(int v)             // slot 24 0x5AE670
    {
        if (text != nullptr)
            text->SetVJustify(v);
    }
    virtual void SetLineSpacing(int s)          // slot 25 0x5AE680
    {
        if (text != nullptr)
            text->SetLineSpacing(s);
    }
    virtual void SetFont(font_index f)          // slot 26 0x5AE6A0
    {
        if (text != nullptr)
            text->SetFont(f);
    }
    virtual void SetBehaviorNF(float x, float y)  // slot 27 0x5AE6B0
    {
        if (text != nullptr)
            text->SetBehaviorNF(x, y);
    }
    virtual void SetBehavior(bool nfb)          // slot 28 0x5AE6D0
    {
        if (text != nullptr)
            text->SetBehavior(nfb);
    }
    virtual void SetColorSchemeIndex(char csi)  // slot 29 0x5AE6F0
    {
        color_scheme_index = csi;
        AdjustColor();
    }
    virtual void SetScale(float s, float su)    // slot 30 0x5AE720
    {
        if (text != nullptr)
            text->SetScaleMenuItem(s, su);
    }
    virtual void SetScale(float s)              // slot 31 0x5AE710
    {
        if (text != nullptr)
            text->SetScale(s);
    }
    virtual void SetZ(float z, panel_layer layer)  // slot 32 0x5AE730
    {
        if (text != nullptr)
            text->SetZvalue(z, layer);
    }
    virtual Broc::string GetText()              // slot 33 0x5AE740
    {
        return text != nullptr ? text->GetText() : Broc::string();
    }
    virtual float GetWidth()                    // slot 34 0x5AE770
    {
        return text != nullptr ? text->GetWidth(nullptr) : 0.0f;
    }
    virtual float GetX()                        // slot 35 0x5AE780
    {
        return text != nullptr ? text->GetX() : 0.0f;
    }
    virtual float GetY()                        // slot 36 0x5AE790
    {
        return text != nullptr ? text->GetY() : 0.0f;
    }
    virtual int GetLineNum()                    // slot 37 0x5AE7A0
    {
        return text != nullptr ? text->GetLineNum() : 0;
    }
    virtual float GetScaleX()                   // slot 38 0x5AE7B0
    {
        return text != nullptr ? text->GetScaleX() : 0.0f;
    }
    virtual float GetScaleY()                   // slot 39 0x5AE7C0
    {
        return text != nullptr ? text->GetScaleY() : 0.0f;
    }
    virtual char GetColorSchemeIndex()          // slot 40
    {
        return color_scheme_index;
    }
    virtual color32 GetColor()                  // slot 41
    {
        return text != nullptr ? text->GetColor() : color32(0);
    }
    virtual float GetZ()                        // slot 42
    {
        return text != nullptr ? text->GetZvalue() : 0.0f;
    }
    virtual void AddFont(int index, font_index f)  // slot 43 0x5AE810
    {
        if (text != nullptr)
            text->AddFont(index, f);
    }
    virtual color32 WithAlpha(color32 c, int alpha);  // slot 44 0x56FC90
protected:
    virtual void AdjustColor(FEText* fet);      // slot 45 0x56FCC0
public:
    virtual void AdjustColor();                 // slot 46 0x56FCB0
    virtual void MoveForSplitScreen(int viewport,
                                    int old_viewport);  // slot 47 0x56FE70
    virtual void UpdateWidescreen(bool) {}      // slot 48 0x5AE830 (empty)
    virtual void OnSelect() {}                  // slot 49 0x5AE4A0 (empty)
    virtual int GetValue() { return 0; }        // slot 50 0x5AE850
    virtual void SetValue(int) {}               // slot 51 0x5AE840 (empty)

    void CommonConstructor(FEText* text, FEMenu* menu);  // shell.o 0x56FBC0
    FEMenuEntry(FEText* t, FEMenu* m, bool delete_me);  // inline 0x5B1C90
    FEMenuEntry(const char* text, FEMenu* m, bool floating,
                font_index ft, int nlines);     // shell.o 0x585C70
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
class FEMenuSystem : public PanelFileUser {
public:
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

    // Vftable order verified against ??_7FEMenuSystem (37 slots).
    virtual ~FEMenuSystem();                        // slot 3 0x570A10
    virtual void InitAll();                         // slot 4 0x570AA0
    virtual void Add(FEMenu* m);                    // slot 5 0x570A30
    virtual void ReturnToPreviousMenu(int fallback);  // slot 6 0x570AD0
    virtual void MakeActive(int index);             // slot 7 0x570B20
    virtual void MakeActive(int index,
                            int return_to_menu);    // slot 8 0x57DDF0
    virtual void MakeActiveAndReturn(int index,
                                     int return_to);  // slot 9 0x570B80
    virtual void MakeActiveAndReturn(int index);    // slot 10 0x570B40
    virtual bool IsMenuActive(int menu);            // slot 11 0x570BC0
    virtual void ClearReturnMenu(int menu);         // slot 12 0x570BE0
    virtual void UpdateSplitScreen();               // slot 13 0x570E10
    virtual void AddOverlay(int index);             // slot 14 0x570C00
    virtual void RemoveOverlay();                   // slot 15 0x570D00
    virtual void Update(float time_inc);            // slot 16 0x570DD0
    virtual void UpdateButtonPresses();             // slot 17 0x57DFB0
    virtual void UpdateButtonDown();                // slot 18 0x570E40
    virtual void Draw();                            // slot 19 0x570E90
    virtual void Draw3D();                          // slot 20 0x570ED0
    virtual void SetDefaultColorScheme(char csi)    // slot 21 inline 0x5AE990
    {
        default_color_scheme = csi;
    }
    virtual char GetDefaultColorScheme()            // slot 22 inline 0x5AFA10
    {
        return default_color_scheme;
    }
    virtual bool IsSystemActive()                   // slot 23 inline 0x5AFA20
    {
        return is_active;
    }
    virtual int GetActiveMenu();                    // slot 24 0x571370
    virtual void SetActiveMenu(int menu);           // slot 25 0x571380
    virtual int GetCurrentClient();                 // slot 26 0x571300
    virtual int GetCurrentClientController();       // slot 27 0x571310
protected:
    virtual void OnButtonPress(int button,
                               int controller);     // slot 28 0x570F10
    virtual void OnButtonRelease(int button,
                                 int controller);   // slot 29 0x5711A0
    virtual bool GetAnalogPressed(int button,
                                  int* p_controller);  // slot 30 0x5711D0
    virtual bool GetButtonPressed(controller::ButtonIndex button,
                                  int* p_controller);  // slot 31 0x571330
    virtual bool GetButtonReleased(controller::ButtonIndex button,
                                   int* p_controller);  // slot 32 0x571350
    virtual int GetStickValueX(controller::StickIndex stick,
                               int* p_controller);  // slot 33 0x571390
    virtual int GetStickValueY(controller::StickIndex stick,
                               int* p_controller);  // slot 34 0x5713B0
    virtual int GetClientFromController(int c);     // slot 35 0x571320
    virtual void NewMenuActive() {}                 // slot 36 inline 0x5AFA30
public:
    int CurrentOverlay();                           // ?CurrentOverlay@FEMenuSystem@@QAEHXZ 0x570DC0
    void SetSystemActive(bool active);  // ?SetSystemActive@FEMenuSystem@@QAEX_N@Z (sv.o 0x51E150)
};
static_assert(sizeof(FEMenuSystem) == 0x2C, "FEMenuSystem size mismatch");
static_assert(offsetof(FEMenuSystem, menus) == 0x04, "FEMenuSystem::menus offset mismatch");
static_assert(offsetof(FEMenuSystem, m_active) == 0x1C, "FEMenuSystem::m_active offset mismatch");

// ============================================================================
// FEMenu â€” menu (76 bytes)
// Size: 0x4C (76 bytes) â€” verified against IDA
// ============================================================================

// FEMenu (shell.o FEMenu.cpp) - vftable order verified against ??_7FEMenu
class FEMenu : public PanelFileUser {
public:
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

    virtual void PanelFileUnloaded(PanelFile* pf)  // slot 1 0x5B7570
    {
        Cleanup();
    }
    virtual void UpdateWidescreen(bool widescreen);  // slot 2 0x57DF20
    virtual ~FEMenu();                              // slot 3 (??_G) 0x592150
    virtual void AddEntry(int index, FEText* t,
                          bool delete_me);          // slot 4 0x57DBA0
    virtual void AddEntry(int index,
                          const char* text);        // slot 5 0x585DA0
    virtual void EnableNavigationSound(bool enable)  // slot 6 0x5AF770
    {
        enableNavigationSound = enable;
    }
    virtual void PlayNavigationSound();             // slot 7 0x57DD00
    virtual void PlayNavigationSoundWait();         // slot 8 0x58E0B0
    virtual void InputLock(bool enable)             // slot 9 0x5AF780
    {
        lockInput = enable;
    }
    virtual class FEComboBox* AddComboBox(
        int index, short numOptions, FEText* text, FEText* label,
        PanelQuad* leftArrow, PanelQuad* rightArrow);  // slot 10 0x585F30
    virtual class FEComboBox* AddComboBox(
        int index, short numOptions, FEText* text, PanelQuad* leftArrow,
        PanelQuad* rightArrow);                      // slot 11 0x585E60
    virtual class FESlider* AddSlider(int index, FEText* barText,
                                      FEText* label);  // slot 12 0x5860D0
    virtual class FESlider* AddSlider(int index, PanelQuad* bar,
                                      FEText* label);  // slot 13 0x586000
    virtual class FEDoubleEntry* AddDoubleEntry(int index, FEText* barText,
                                                FEText* label);  // slot 14 0x5861A0
    virtual class FEMenuListBox* AddListBoxEntry(int index, FEText* t,
                                                 int numLines);  // slot 15 0x58DFF0
    virtual void OnDeactivate(FEMenu* m) {}         // slot 16 (empty inline)
    virtual void Init();                            // slot 17 0x570400
    virtual void Load()                             // slot 18 0x5AF7A0
    {
        Load(false);
    }
    virtual void Load(bool floating) {}             // slot 19 0x5AF790 (empty)
    virtual void Draw();                            // slot 20 0x570550
    virtual void Draw3D() {}                        // slot 21 (empty inline)
    virtual void UpdateInScene() {}                 // slot 22 (empty inline)
    virtual void Update(float time_inc);            // slot 23 0x570660
    virtual void HighlightDefault();                // slot 24 0x570700
    virtual void Select(int entry_num, int controller)  // slot 25 0x5AF7E0
    {
        Select(entry_num);
    }
    virtual void Select(int entry_num) {}           // slot 26 0x5AF7D0 (empty)
    virtual void OnActivate(int prev)               // slot 27 0x5AF800
    {
        OnActivate();
    }
    virtual void OnActivate();                      // slot 28 0x570750
    virtual void OnSelect(int c) {}                 // slot 29 (empty inline)
    virtual void OnSquare(int c) {}                 // slot 30 (empty inline)
    virtual void OnCircle(int c) {}                 // slot 31 (empty inline)
    virtual void OnUp(int c)                        // slot 32 0x5AE910
    {
        Up();
    }
    virtual void OnDown(int c)                      // slot 33 0x5AE920
    {
        Down();
    }
    virtual void OnLeft(int c)                      // slot 34 0x5AF820
    {
        Left();
    }
    virtual void OnRight(int c)                     // slot 35 0x5AF830
    {
        Right();
    }
    virtual void OnCross(int c);                    // slot 36 0x5707D0
    virtual void OnL1(int c) {}                     // slot 37 (empty inline)
    virtual void OnR1(int c) {}                     // slot 38 (empty inline)
    virtual void OnL2(int c) {}                     // slot 39 (empty inline)
    virtual void OnTrueCircle(int c) {}             // slot 40 (empty inline)
    virtual void OnTrueTriangle(int c) {}           // slot 41 (empty inline)
    virtual void OnStart(int c) {}                  // slot 42 (empty inline)
    virtual void OnR2(int c) {}                     // slot 43 (inline 0x5AF890)
    virtual void OnAnyButtonPress(int c, int b);    // slot 44 0x570810
    virtual void OnButtonRelease(int c, int b);     // slot 45 0x570850
    virtual void OnTriangle(int c) {}               // slot 44 (empty inline)
    virtual void UpdateSplitScreen() {}             // slot 47 (empty inline)
    virtual void SetHigh(int index, bool anim);     // slot 48 0x57DC70
    virtual void SetVis(int first);                 // slot 49 0x570280
    virtual void SetDistanceBetweenEntries(int dbe)  // slot 50 0x5AF8D0
    {
        y_distance = dbe;
    }
    virtual void SetVerticalJust(bool top, bool bottom);  // slot 51 0x570350
    virtual void SetScaleThroughout(float sc);      // slot 52 0x570390
    virtual void SetZThroughout(float z,
                                panel_layer layer);  // slot 53 0x5703C0
    virtual void SetDefaultColorScheme(char csi)    // slot 54 0x5AE930
    {
        default_color_scheme = csi;
    }
    virtual char GetDefaultColorScheme()            // slot 55 0x5AF8E0
    {
        return default_color_scheme;
    }
    virtual FEMenuSystem* GetSystem()               // slot 56 0x5AF8F0
    {
        return system;
    }
    virtual int GetFlags()                          // slot 57 0x5AF900
    {
        return flags;
    }
protected:
    virtual void Up();                              // slot 58 0x570020
    virtual void Down();                            // slot 59 0x56FF00
    virtual void Left();                            // slot 60 0x570140
    virtual void Right();                           // slot 61 0x5701E0
    virtual void ButtonHeldAction();                // slot 62 0x570890
public:
    FEMenu();                                       // 0x57DA80
    FEMenu(FEMenuSystem* menuSystem, int num, int x, int y, short mve,
           short flg);                              // 0x57DAE0
    void Cleanup();                                 // ?Cleanup@FEMenu@@QAEXXZ 0x58DF20
    bool GetFlag(int f)                             // ?GetFlag@FEMenu@@QAE_NH@Z 0x5AE940
    {
        return (flags & f) != 0;
    }
    void SetFlag(int f, bool b)                     // ?SetFlag@FEMenu@@QAEXH_N@Z 0x5AE960
    {
        flags = b ? (int16_t)(flags | f) : (int16_t)(flags & ~f);
    }
protected:
    void ClearAllButtons();                         // ?ClearAllButtons@FEMenu@@IAEXXZ 0x5708C0
    void ClearButton(controller::ButtonIndex a_eButton);  // ?ClearButton@FEMenu@@IAEXW4ButtonIndex@controller@@@Z 0x570910
    void ConnectEntries(short index);               // ?ConnectEntries@FEMenu@@IAEXF@Z 0x56FE80
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
class FEMultiLineText : public FEText {
public:
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
    virtual void SetNumLines(int n);      // ?SetNumLines@FEMultiLineText@@UAEXH@Z (0x56D6D0)
    virtual void SetText(const char* s);  // ?SetText@FEMultiLineText@@UAEXPBD@Z (0x56D7E0)
    virtual void SetLineSpacing(int new_spacing);  // 0x57CFA0
    virtual void SetBoxWidth(int width)            // inline 0x5B1C30
    {
        box_width = width;
    }
    virtual void SetCutOffIfTooLong(bool coitl)    // inline 0x5B1C70
    {
        cut_off_if_too_long = coitl;
    }
    static Broc::string ReplaceEndlines(Broc::string t);  // shell.o 0x56E670
    FEMultiLineText(font_index f, float x1, float y1, int z1,
                    panel_layer layer, float s, int horizJust, int vertJust,
                    color32 col);
};
static_assert(sizeof(FEMultiLineText) == 0xA8, "FEMultiLineText size mismatch");
static_assert(offsetof(FEMultiLineText, lines) == 0x90, "FEMultiLineText::lines offset mismatch");

// ============================================================================
// FEMenuListBoxItem â€” list-box data row (28 bytes) â€” verified against IDA
// ============================================================================
class FEMenuListBoxItem {
public:
    int          mIndex;         // +0x00
    Broc::string mText;          // +0x04
    void*        mData;          // +0x08
    int          mSubItemCount;  // +0x0C
    Broc::string mSubItems[3];   // +0x10

    const Broc::string& GetSubItem(unsigned int index);  // shell.o 0x571D90
};
static_assert(sizeof(FEMenuListBoxItem) == 28,
              "FEMenuListBoxItem size mismatch");

// ============================================================================
// UIListBox - 172 bytes (verified against IDA; shell.o owns the impl)
// ============================================================================
class UIListBox {
public:
    enum EScrollQuads {
        kScrollBarArrowUp = 0,
        kScrollBarArrowDown = 1,
        kScrollBarIndicator = 2,
        kScrollBarTrack = 3,
        kScrollBarDetail1 = 4,
        kScrollBarDetail2 = 5,
    };

    class UIListBoxRow;

    class UIListBoxData {
    public:
        Broc::string mText;   // +0x00
        int          mState;  // +0x04

        UIListBoxData();      // 0x5AEBB0
        void SetState(int state);  // 0x5AEBE0
        void SetText(const char* text);  // 0x5AEC70
        const char* GetText();  // 0x5B2580
    };
    static_assert(sizeof(UIListBoxData) == 8, "UIListBoxData size mismatch");

    class UIListBoxDataRow {
    public:
        enum { kTypeNone = 0, kTypeText = 1, kTypeQuad = 2 };

        ae_vector<UIListBoxData> mColumns;  // +0x00 (12 bytes)
        int  mColumnCount;                  // +0x0C
        bool mEnabled;                      // +0x10

        UIListBoxDataRow();  // 0x5B6160
        void ClearItem();    // 0x5B2DB0
        void SetColumnCount(int columns);  // 0x5B6180
        void SetItemState(int column, int state);  // 0x5B2BF0
        void SetText(int column, const char* text);  // 0x5B2C80
        const char* GetText(int column);  // 0x5B2D10
    };
    static_assert(sizeof(UIListBoxDataRow) == 20,
                  "UIListBoxDataRow size mismatch");

    class UIListBoxItem {
    public:
        enum { kTypeNone = 0, kTypeText = 1, kTypeQuad = 2 };
        friend class UIListBoxRow;

        int          mType;       // +0x00
        int          mState;      // +0x04
        int          mStateCount; // +0x08
        ae_vector<PanelAnimObject*> mObjects;  // +0x0C (12 bytes)

        UIListBoxItem();  // 0x5B4D50
        void SetState(int state);       // 0x5B1E30
        void SetStateCount(int count);  // 0x5B4DC0
        void SetText(const char* text); // 0x5813A0
        void SetSelected(bool selected, bool flashing);  // 0x581460
        void SetColor(color32 unselectedColor,
                      color32 selectedcolor);  // 0x581580
        void SetEnabled(bool enabled);  // 0x581650
        void ClearText();               // 0x5B4EC0
        void RemoveItems();             // 0x5B2070
        color32 GetColor();             // 0x5B22B0
        color32 GetUnselectedColor();   // 0x5B23D0
        float GetY();                   // 0x5B21A0
    private:
        void SetObject(PanelAnimObject* object, int state);  // 0x5B24E0
    };
    static_assert(sizeof(UIListBoxItem) == 24, "UIListBoxItem size mismatch");

    class UIListBoxRow {
    public:
        ae_vector<UIListBoxItem> mColumns;  // +0x00 (12 bytes)
        int  mColumnCount;                  // +0x0C

        UIListBoxRow();  // 0x5B6E30
        void ClearItem();                    // 0x5B5390
        void RemoveItems();                  // 0x5B2630
        void Draw();                         // 0x5B26D0
        void Update(float time_delta);       // 0x5B2870
        void SetColumnCount(int columns);    // 0x5B7580
        void SetColumnStateCount(int column, int count);  // 0x5B5130
        void SetItem(int column, FEText* text, int state);   // 0x5B51C0
        void SetItem(int column, PanelQuad* quad, int state);  // 0x5B5260
        void SetText(int column, const char* text);  // 0x5B5300
        void SetItemState(int column, int state);    // 0x5B25A0
        void SetColumnColor(int column, color32 unselectedColor,
                            color32 selectedcolor);  // 0x5B5450
        void SetSelected(int column, bool selected,
                         bool flashing);             // 0x5B54E0
        void SetEnabled(bool enabled);               // 0x5B5570
        float GetY(int column);                      // 0x5B2A20
        color32 GetColumnSelectedColor(int column);  // 0x5B2AB0
        color32 GetColumnUnselectedColor(int column);// 0x5B2B40
    };
    static_assert(sizeof(UIListBoxRow) == 16, "UIListBoxRow size mismatch");

    // Members
    ae_vector<UIListBoxRow> mItemRows;        // +0x04
    ae_vector<UIListBoxDataRow> mDataRows;    // +0x10
    bool mSelectedFlashing;                   // +0x1C
    ae_vector<color32> mSelectedRowOriginalColor;      // +0x20
    ae_vector<bool> mSelectedRowColorChangeColumns;    // +0x2C
    PanelQuad* mScrollBarQuads[6];            // +0x38
    int mLastRowContainingData;               // +0x50
    int mItemRowsCount;                       // +0x54
    int mItemColumnsCount;                    // +0x58
    int mDataRowsCount;                       // +0x5C
    int mTopLine;                             // +0x60
    int mSelectedLine;                        // +0x64
    int mScrollBarTopY;                       // +0x68
    int mScrollBarBottomY;                    // +0x6C
    float mScrollBarYInc;                     // +0x70
    bool mIsWrapping;                         // +0x74
    bool mBlockRefresh;                       // +0x75
    int mIncrementBy;                         // +0x78
    struct PanelQuadFader {
        PanelQuad* mQuad;        // +0x00
        float mAlpha;            // +0x04
        float mAlphaTo;          // +0x08
        float mTime;             // +0x0C
        float mAlphaDelta;       // +0x10
        bool  mFading;           // +0x14
        uint8_t _pad[3];         // +0x15
    } mScrollBarUpFader;         // +0x7C
    PanelQuadFader mScrollBarDownFader;  // +0x94

    UIListBox(int visibleRows, int visibleColumns, int maxDataRows,
              bool bIsWrapping);  // 0x59BB70
    virtual ~UIListBox();         // 0x5B9BB0
    virtual void Clear();         // 0x58FFC0
    virtual void ClearRow(int row);  // 0x590050
    virtual void SelectLine(int selection, int top_line);  // 0x5904A0
    virtual void SelectLine(int selection);  // 0x5903F0
    virtual short OnUp(int c);      // 0x590290
    virtual short OnDown(int c);    // 0x590340
    virtual void Update(float time_delta);  // 0x581750
    virtual void Draw();            // 0x581910
    virtual void Refresh();         // 0x590520

    void RemoveAllItems();          // 0x5816B0
    void SetItemState(int row, int column, int state);  // 0x581990
    void PageDown(int numPageRows);  // 0x581B00
    void PageUp(int numPageRows);    // 0x581BC0
    void SetAllColumnsSelectable(bool selectable);  // 0x581D40
    void SetRowEnabled(int row, bool enabled);      // 0x5878D0
    void SetColumnStateCount(int column, int count);  // 0x587960
    void SetItem(int row, int column, FEText* text, int state);  // 0x587BA0
    void SetItem(int row, int column, PanelQuad* quad, int state); // 0x587CB0
    void SetText(int row, int column, const char* text);  // 0x587DC0
    void SetScrollBarQuad(EScrollQuads index, PanelQuad* quad);  // 0x577050
    void SetScrollBarFromPanelFile(PanelFile* pf);  // 0x598120
    void FormatForSplitScreen(int viewport, int old_viewport);  // 0x5770B0
    void FormatForWidescreen(bool widescreen, float about_x);   // 0x5770C0
    void ResizeDataRows(int rowCount);  // 0x590150
protected:
    void MoveUpTo(int newSelectedLine);  // 0x576F70
    void MoveDownTo(int newSelectedLine);// 0x576FE0
    short MoveUp(int c);                 // 0x587F00
    short MoveDown(int c);               // 0x587FF0
    void DeselectRow();                  // 0x5880E0
    void SelectRow();                    // 0x588180
    void UpdateScrollBar();              // 0x581C70
    void PlayNavigationSound();          // 0x581A90
};
static_assert(sizeof(UIListBox) == 0xAC, "UIListBox size mismatch");
